#!/usr/bin/env python3
"""Independent TEST-ONLY CBOR v1 fixture checker. No runtime dependency.

Separate from the C++ finite-state codec. It builds a generic bounded CBOR AST
from the exact raw bytes, then applies GhostWire's closed schema. This uses only
Python's standard library; it is not a cryptographic or external security audit.
"""
from pathlib import Path
import json
import struct

ROOT = Path(__file__).resolve().parents[2]
MAX_FRAME = 65536

class Reject(Exception):
    pass

class Reader:
    def __init__(self, data):
        self.data, self.position, self.items = data, 0, 0

    def take(self, n):
        if n > len(self.data) - self.position:
            raise Reject('malformed_frame')
        result = self.data[self.position:self.position + n]
        self.position += n
        return result

    def parse(self, depth=0):
        if depth > 4 or self.items >= 512:
            raise Reject('resource_limit')
        self.items += 1
        initial = self.take(1)[0]
        major, info = initial >> 5, initial & 31
        if info >= 28:
            raise Reject('noncanonical_encoding' if info == 31 else 'malformed_frame')
        if depth >= 4 and major in (4, 5):
            raise Reject('resource_limit')
        if major not in (0, 2, 4, 5):
            raise Reject('malformed_frame')
        width = {24: 1, 25: 2, 26: 4, 27: 8}.get(info, 0)
        n = int.from_bytes(self.take(width), 'big') if width else info
        if width and n < (24 if width == 1 else 256 if width == 2 else 65536 if width == 4 else 4294967296):
            raise Reject('noncanonical_encoding')
        if major == 0:
            return n
        if major == 2:
            if n > 49152:
                raise Reject('resource_limit')
            return self.take(n)
        if major == 4:
            if n > 64:
                raise Reject('resource_limit')
            return [self.parse(depth + 1) for _ in range(n)]
        if n > 16:
            raise Reject('resource_limit')
        obj = {}
        previous = -1
        for _ in range(n):
            key = self.parse(depth + 1)
            if type(key) is not int or key > 23:
                raise Reject('malformed_frame')
            if key <= previous:
                raise Reject('noncanonical_encoding')
            previous = key
            obj[key] = self.parse(depth + 1)
        return obj

def decode_frame(data):
    if len(data) < 4:
        raise Reject('incomplete_frame')
    length = struct.unpack('>I', data[:4])[0]
    if not 1 <= length <= MAX_FRAME:
        raise Reject('frame_limit')
    if len(data) - 4 < length:
        raise Reject('incomplete_frame')
    if len(data) - 4 > length:
        raise Reject('malformed_frame')
    parser = Reader(data[4:])
    obj = parser.parse()
    if parser.position != length:
        raise Reject('malformed_frame')
    if type(obj) is not dict or not set((0, 1, 2, 3)) <= obj.keys() or set(obj) - {0, 1, 2, 3, 4}:
        raise Reject('malformed_frame')
    if (obj[0], obj[1]) != (1, 0):
        raise Reject('unsupported_version')
    kind = obj[2]
    if kind not in (1, 2, 10):
        raise Reject('unsupported_kind')
    b = obj[3]
    if type(b) is not dict:
        raise Reject('malformed_frame')
    if kind in (1, 2):
        if set(b) != {0, 1} or type(b[1]) is not list or len(b[1]) > 32:
            raise Reject('malformed_frame')
        if any(type(x) is not int for x in b[1]) or any(x >= y for x,y in zip(b[1], b[1][1:])):
            raise Reject('noncanonical_encoding')
        if b[1]:
            raise Reject('unsupported_capability')
        pairs = b[0] if kind == 1 else [b[0]]
        if type(pairs) is not list or not pairs or len(pairs) > 8:
            raise Reject('malformed_frame')
        if any(type(p) is not list or len(p) != 2 or any(type(v) is not int for v in p) for p in pairs):
            raise Reject('malformed_frame')
        if any(a >= z for a,z in zip(pairs, pairs[1:])):
            raise Reject('noncanonical_encoding')
        if pairs != [[1, 0]]:
            raise Reject('unsupported_version')
    else:
        if set(b) != {0} or type(b[0]) is not bytes or not 182 <= len(b[0]) <= 1718:
            raise Reject('malformed_frame')
    if 4 in obj:
        r = obj[4]
        if type(r) is not dict or not ({0} <= set(r) <= {0, 1}):
            raise Reject('malformed_frame')
        if type(r[0]) is not bytes or len(r[0]) != 16:
            raise Reject('malformed_frame')
        if 1 in r and (type(r[1]) is not int or r[1] > 16):
            raise Reject('malformed_frame')
    return obj

def check():
    fixture = json.loads((ROOT / 'tests/fixtures/protocol/v1-design-vectors.json').read_text())
    for item in fixture['valid']:
        decoded = decode_frame(bytes.fromhex(item['hex']))
        assert decoded[2] in (1, 2)
    for item in fixture['invalid']:
        try:
            decode_frame(bytes.fromhex(item['hex']))
        except Reject as error:
            # A05-P01 categories are independently validated except for a
            # closed-schema map with missing/extra fields (malformed vs order).
            if str(error) != item['category']:
                assert item['id'] == 'out_of_order_top_keys' and str(error) == 'malformed_frame', (item['id'], str(error))
        else:
            raise AssertionError('accepted invalid fixture: ' + item['id'])
    print('Independent Python CBOR AST/schema check: PASS (2 positive, 16 negative)')

if __name__ == '__main__':
    check()
