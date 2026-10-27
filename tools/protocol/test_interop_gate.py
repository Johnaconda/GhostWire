#!/usr/bin/env python3
"""Offline fault injection into the interop verdict gate (upstream remains UNRUN)."""
import json
from pathlib import Path

from differential_check import (CANONICAL_CBOR_REJECT_IDS, GENERIC_CBOR_ACCEPT_IDS,
                                verify_upstream_verdicts)

ROOT = Path(__file__).resolve().parents[2]


def expect_failure(label, fixtures, cpp, upstream, ids):
    try:
        verify_upstream_verdicts(fixtures, cpp, upstream, ids)
    except AssertionError:
        return
    raise AssertionError(f'{label}: invalid upstream verdicts were accepted')


def test():
    doc = json.loads((ROOT / 'tests/fixtures/protocol/v1-design-vectors.json').read_text())
    vectors = doc['valid'] + doc['invalid']
    ids = tuple(item['id'] for item in vectors)
    raw = tuple(bytes.fromhex(item['hex']) for item in vectors)
    cpp = tuple('OK ' + item['hex'] if i < len(doc['valid']) else 'ERR 1'
                for i, item in enumerate(vectors))
    upstream = tuple('ERR' if key in CANONICAL_CBOR_REJECT_IDS else 'OK' for key in ids)
    accepted, rejected, stricter = verify_upstream_verdicts(raw, cpp, upstream, ids)
    assert (accepted, rejected, stricter) == (13, 5, 11)
    checks = 1
    for key in sorted(CANONICAL_CBOR_REJECT_IDS):
        broken = list(upstream)
        broken[ids.index(key)] = 'OK'
        expect_failure('noncanonical ' + key, raw, cpp, broken, ids)
        checks += 1
    for key in sorted(GENERIC_CBOR_ACCEPT_IDS):
        broken = list(upstream)
        broken[ids.index(key)] = 'ERR'
        expect_failure('generic canonical ' + key, raw, cpp, broken, ids)
        checks += 1
    for label, broken in [('accept everything', ['OK'] * len(raw)),
                          ('reject everything', ['ERR'] * len(raw)),
                          ('unrecognized verdict', ['YES'] + list(upstream[1:])),
                          ('lost record', list(upstream[:-1]))]:
        expect_failure(label, raw, cpp, broken, ids)
        checks += 1
    expect_failure('missing negative vector', raw, cpp, upstream, tuple(
        'missing' if x == 'duplicate_top_key' else x for x in ids)); checks += 1
    # Ensure a compromised/malformed GhostWire test probe cannot bypass upstream checks.
    for label, broken_cpp in [
        ('missing cpp result', cpp[:-1]),
        ('invented success', ('OK deadbeef',) + cpp[1:]),
        ('invented error', ('ERR invented',) + cpp[1:]),
        ('invalid verdict', ('YES',) + cpp[1:]),
        ('line injection', ('OK ' + vectors[0]['hex'] + '\\nERR 1',) + cpp[1:]),
        ('empty verdict', ('',) + cpp[1:]),
    ]:
        expect_failure(label, raw, broken_cpp, upstream, ids)
        checks += 1
    print(f'TinyCBOR interop verdict guard: PASS ({checks} offline checks; actual upstream UNRUN)')


if __name__ == '__main__':
    test()
