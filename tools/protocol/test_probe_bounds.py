#!/usr/bin/env python3
"""Test-only stdin boundary test, not a protocol security assessment."""
import argparse
import json
from pathlib import Path
import subprocess


def main():
    p = argparse.ArgumentParser()
    p.add_argument('probe', type=Path)
    p.add_argument('--upstream', action='store_true')
    args = p.parse_args()
    fixtures = json.loads((Path(__file__).resolve().parents[2] / 'tests/fixtures/protocol/v1-design-vectors.json').read_text())
    good = fixtures['valid'][0]['hex']
    # Deliberately much larger than the 131080-character maximum: must reject
    # the whole record but resynchronize for the subsequent valid record.
    payload = b'0' * (4 * 1024 * 1024) + b'\n' + good.encode() + b'\n' + b'xyz'
    finished = subprocess.run([str(args.probe.resolve())], input=payload,
                              capture_output=True, timeout=25, check=True)
    lines = finished.stdout.decode('ascii').splitlines()
    if args.upstream:
        assert lines == ['ERR', 'OK', 'ERR'], lines
    else:
        assert lines == ['INVALID_HEX', 'OK ' + good, 'INVALID_HEX'], lines
    assert not finished.stderr
    print('Probe oversized-stdin rejection and resynchronization: PASS')


if __name__ == '__main__':
    main()
