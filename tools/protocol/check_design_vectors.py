#!/usr/bin/env python3
"""Documentation fixture consistency, NOT a protocol encoder/decoder test."""
from pathlib import Path
import json

root = Path(__file__).resolve().parents[2]
fixture = root / 'tests/fixtures/protocol/v1-design-vectors.json'
vectors = json.loads(fixture.read_text(encoding='utf-8'))
assert vectors['checkpoint'] == 'GW-A05-P01-PROTOCOL-SCHEMA-VERSIONING-DESIGN'
assert (root / 'schemas/ghostwire-wire-v1.cddl').exists()
assert (root / 'docs/architecture/PROTOCOL_SCHEMA.md').exists()
assert len(vectors['valid']) >= 2 and len(vectors['invalid']) >= 10
known_ids = set()
for kind, items in [('valid', vectors['valid']), ('invalid', vectors['invalid'])]:
    for item in items:
        assert item['id'] not in known_ids, item['id']
        known_ids.add(item['id'])
        raw = bytes.fromhex(item['hex'])
        if item['id'] == 'truncated_header':
            assert len(raw) < 4
            continue
        assert len(raw) >= 4, item['id']
        declared = int.from_bytes(raw[:4], 'big')
        actual = len(raw) - 4
        if kind == 'valid':
            assert 1 <= declared <= 65536 and declared == actual, item['id']
        elif item['id'] == 'zero_frame_length':
            assert declared == 0
        elif item['id'] == 'oversized_prefix_65537':
            assert declared == 65537
        elif item['id'] == 'truncated_offer':
            assert actual < declared
        elif item['id'] == 'trailing_extra_byte':
            assert actual > declared
        else:
            assert 1 <= declared <= 65536 and actual == declared, item['id']
        if kind == 'invalid':
            assert item['category'] in {
                'incomplete_frame', 'frame_limit', 'unsupported_version',
                'unsupported_kind', 'noncanonical_encoding',
                'malformed_frame', 'unsupported_capability',
            }, item['id']
print('Design fixture shape and prefix lengths: PASS '
      f'({len(vectors["valid"])} valid + {len(vectors["invalid"])} negative)')
print('NOTE: this does not implement or verify CBOR canonical decoding or A04 signatures')
