#!/usr/bin/env python3
"""Independent seeded CBOR AST/reference-vs-C++ codec differential test.

Requires a built test-only ghostwire-protocol-probe executable as argument.
This is not an upstream CBOR implementation or a cryptographic audit.
"""
import argparse
import json
from pathlib import Path
import random
import re
import struct
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from reference_check import decode_frame, Reject

SEED = 0xA05F0003

# These frozen A05-P01 inputs violate GENERIC canonical CBOR, not merely
# GhostWire's more restrictive field registry. A maintained upstream validator
# configured for our canonical profile MUST reject each one. This makes the
# upstream gate useful even if a careless wrapper begins returning OK for
# every parseable byte sequence.
CANONICAL_CBOR_REJECT_IDS = frozenset({
    'duplicate_top_key', 'duplicate_nested_key', 'out_of_order_top_keys',
    'nonpreferred_uint_encoding', 'indefinite_map',
})
# These are canonical GENERIC CBOR but disallowed by GhostWire's application
# schema. A supposed independent generic parser must not apply the GhostWire
# closed registry accidentally and call that independent CBOR interoperability.
GENERIC_CBOR_ACCEPT_IDS = frozenset({
    'wrong_major', 'wrong_minor', 'reserved_kind',
    'nonempty_unsupported_caps', 'unknown_top_level_key',
})


def verify_upstream_verdicts(fixtures, cpp_lines, upstream_lines, frozen_ids):
    """Check an independently executed upstream parser, never its implementation.

    A generic canonical CBOR parser is intentionally allowed to accept frames
    that GhostWire rejects for schema/version/permissions reasons. The converse
    is forbidden, and five frozen CBOR-noncanonical vectors must be rejected.
    """
    if len(cpp_lines) != len(fixtures) or len(upstream_lines) != len(fixtures):
        raise AssertionError('upstream result count does not match corpus')
    if not (CANONICAL_CBOR_REJECT_IDS | GENERIC_CBOR_ACCEPT_IDS).issubset(set(frozen_ids)):
        raise AssertionError('mandatory generic-CBOR control vectors were removed')
    accepted = rejected = stricter = 0
    error_pattern = re.compile(r'ERR [0-9]+\Z')
    for i, (raw, cpp, up) in enumerate(zip(fixtures, cpp_lines, upstream_lines)):
        if cpp != 'OK ' + raw.hex() and not error_pattern.fullmatch(cpp):
            raise AssertionError(f'malformed GhostWire probe verdict case {i}')
        if up not in ('OK', 'ERR'):
            raise AssertionError(f'upstream invalid fixed verdict case {i}')
        if cpp.startswith('OK ') and up != 'OK':
            raise AssertionError(f'upstream rejects GhostWire-approved canonical CBOR case {i}')
        if i < len(frozen_ids):
            if frozen_ids[i] in CANONICAL_CBOR_REJECT_IDS and up != 'ERR':
                raise AssertionError(f'upstream accepted noncanonical CBOR: {frozen_ids[i]}')
            if frozen_ids[i] in GENERIC_CBOR_ACCEPT_IDS and up != 'OK':
                raise AssertionError(f'upstream rejected canonical generic CBOR: {frozen_ids[i]}')
        if up == 'OK':
            accepted += 1
            if cpp.startswith('ERR '):
                stricter += 1
        else:
            rejected += 1
    if not accepted or not rejected:
        raise AssertionError('upstream comparison has no accepted or rejected evidence')
    return accepted, rejected, stricter

def head(major, n):
    if n < 24:
        return bytes([(major << 5) | n])
    if n <= 255:
        return bytes([(major << 5) | 24, n])
    if n <= 65535:
        return bytes([(major << 5) | 25]) + struct.pack('>H', n)
    if n <= 0xffffffff:
        return bytes([(major << 5) | 26]) + struct.pack('>I', n)
    return bytes([(major << 5) | 27]) + struct.pack('>Q', n)

def encode(obj):
    if type(obj) is int:
        return head(0, obj)
    if type(obj) is bytes:
        return head(2, len(obj)) + obj
    if type(obj) is list:
        return head(4, len(obj)) + b''.join(map(encode, obj))
    if type(obj) is dict:
        assert list(obj) == sorted(obj)
        return head(5, len(obj)) + b''.join(encode(k) + encode(v) for k,v in obj.items())
    raise AssertionError('invalid fixture value')

def frame(obj):
    body = encode(obj)
    return struct.pack('>I', len(body)) + body

def positive(rng):
    kind = rng.choice((1, 2, 10))
    if kind == 1:
        body = {0: [[1, 0]], 1: []}
    elif kind == 2:
        body = {0: [1, 0], 1: []}
    else:
        size = rng.choice((182, 183, 255, 256, 1024, 1718, rng.randint(182, 1718)))
        body = {0: rng.randbytes(size)}
    obj = {0: 1, 1: 0, 2: kind, 3: body}
    if rng.choice((True, False)):
        routing = {0: rng.randbytes(16)}
        if rng.choice((True, False)):
            routing[1] = rng.randrange(17)
        obj[4] = routing
    return frame(obj)

def cases():
    rng = random.Random(SEED)
    valid = [positive(rng) for _ in range(500)]
    mutations = []
    for _ in range(3500):
        b = bytearray(rng.choice(valid))
        mode = rng.randrange(8)
        if mode == 0:  # truncation
            b = b[:rng.randrange(len(b))]
        elif mode == 1:  # garbage append
            b.extend(rng.randbytes(rng.randrange(1, 9)))
        elif mode == 2:  # tamper prefix
            b[:4] = rng.randbytes(4)
        elif mode == 3:  # flip a field/type/key/signature carrier byte
            b[rng.randrange(len(b))] ^= (1 << rng.randrange(8))
        elif mode == 4:  # overwrite a token with forbidden float or indefinite
            b[rng.randrange(4,len(b))] = rng.choice((0xff,0xbf,0x5f,0xf9,0xc0))
        elif mode == 5:  # under/over stated length
            b[:4] = struct.pack('>I', rng.choice((0,1,0xffff,0x10001,0xffffffff)))
        elif mode == 6:
            i = rng.randrange(len(b))
            b[i:i] = rng.randbytes(rng.randrange(1,4))
        else:
            i = rng.randrange(len(b))
            del b[i:i+rng.randrange(1,4)]
        mutations.append(bytes(b))
    # Explicit adversarial structural variants otherwise unlikely under random mutation.
    malformed = [
        bytes.fromhex('00000000'),
        bytes.fromhex('00000001a0'),
        bytes.fromhex('0000000ca400010100021403a0'),
        bytes.fromhex('00000010a400010100020103a200818201000180'),
        bytes.fromhex('00000011a40018010100020103a200818201000180'),
    ]
    # Frozen design vectors were authored in A05-P01 and must not silently
    # disappear from the actual maintained-library interoperability run.
    file = Path(__file__).resolve().parents[2] / 'tests/fixtures/protocol/v1-design-vectors.json'
    vectors = json.loads(file.read_text(encoding='utf-8'))
    frozen = [bytes.fromhex(v['hex']) for category in ('valid', 'invalid')
              for v in vectors[category]]
    return frozen + valid + mutations + malformed

def main():
    p = argparse.ArgumentParser()
    p.add_argument('probe', type=Path, help='built ghostwire-protocol-probe executable')
    p.add_argument('--upstream-probe', type=Path, help='optional separate pinned TinyCBOR validation process')
    args = p.parse_args()
    fixtures = cases()
    vectors = json.loads((Path(__file__).resolve().parents[2] /
        'tests/fixtures/protocol/v1-design-vectors.json').read_text(encoding='utf-8'))
    frozen_ids = tuple(x['id'] for category in ('valid', 'invalid')
                       for x in vectors[category])
    source = '\n'.join(raw.hex() for raw in fixtures) + '\n'
    result = subprocess.run([str(args.probe.resolve())], input=source,
                            text=True, capture_output=True, timeout=90, check=True)
    lines = result.stdout.splitlines()
    if len(lines) != len(fixtures):
        raise AssertionError(f'probe response count {len(lines)} vs {len(fixtures)}')
    valid = invalid = 0
    for i,(raw,line) in enumerate(zip(fixtures,lines)):
        try:
            decode_frame(raw)
        except Reject:
            invalid += 1
            if not line.startswith('ERR '):
                raise AssertionError(f'case {i}: reference rejects, C++ {line[:80]}: {raw[:100].hex()}')
        else:
            valid += 1
            if line != 'OK ' + raw.hex():
                raise AssertionError(f'case {i}: canonical reference accepts, C++ {line[:80]}: {raw[:100].hex()}')
    if args.upstream_probe is not None:
        upstream = subprocess.run([str(args.upstream_probe.resolve())], input=source,
                                  text=True, capture_output=True, timeout=90, check=True)
        upstream_lines = upstream.stdout.splitlines()
        upstream_accepted, upstream_rejected, strict_only = verify_upstream_verdicts(
            fixtures, lines, upstream_lines, frozen_ids)
        print(f'Pinned TinyCBOR interop: PASS {len(fixtures)} cases; '
              f'{upstream_accepted} generic canonical accepted, '
              f'{upstream_rejected} rejected, {strict_only} GhostWire-only stricter rejects')
    print(f'Differential CBOR reference/C++ probe: PASS seed={SEED}, '
          f'{valid} accepted, {invalid} rejected, {len(fixtures)} cases')

if __name__ == '__main__':
    main()
