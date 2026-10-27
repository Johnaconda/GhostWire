#!/usr/bin/env python3
"""Explicit opt-in TinyCBOR v7.0 source acquisition, never used by builds.

Only the immutable upstream commit URL is allowed. All required files are checked
against the locally pinned SHA-256 manifest before the ZIP is published. The
remote source is NOT trusted merely because HTTPS or GitHub responded.
"""
import argparse
import json
from pathlib import Path
import shutil
import sys
import tempfile
import zipfile
import time
from urllib.request import Request, urlopen

import import_tinycbor

COMMIT = '6442e749ca811e24afad1551338a45c75e09808f'
URL = f'https://codeload.github.com/intel/tinycbor/zip/{COMMIT}'
MAX_COMPRESSED = 8 * 1024 * 1024
MAX_ELAPSED = 90


def retrieve_verified(destination: Path, pins: dict, opener=urlopen):
    """Bounded download; publish no bytes unless entire pinned ZIP validates.

    `opener` injection permits *networkless* behavioral testing; production only
    uses the fixed URL and stdlib HTTPS opener. Existing outputs are never replaced.
    """
    if destination.exists() or destination.is_symlink():
        raise ValueError('output already exists')
    destination.parent.mkdir(parents=True, exist_ok=True)
    deadline = time.monotonic() + MAX_ELAPSED
    with tempfile.TemporaryDirectory(prefix='gw-tinycbor-', dir=destination.parent) as td:
        stage = Path(td) / 'verified.zip'
        with opener(Request(URL, headers={'User-Agent': 'GhostWire-pinned-source-verifier/1'}), timeout=15) as reply:
            if reply.geturl() != URL:
                raise ValueError('unexpected download redirect')
            total = 0
            with stage.open('xb') as sink:
                while True:
                    if time.monotonic() > deadline:
                        raise ValueError('upstream fetch exceeded time budget')
                    chunk = reply.read(min(65536, MAX_COMPRESSED - total + 1))
                    if not chunk:
                        break
                    total += len(chunk)
                    if total > MAX_COMPRESSED:
                        raise ValueError('upstream download exceeds size budget')
                    sink.write(chunk)
        # Validating only the selected pinned subset is deliberate: the importer
        # additionally checks every archive member's path, type, count and size.
        import_tinycbor.load_input(stage, pins)
        created = False
        try:
            with destination.open('xb') as sink:
                created = True
                with stage.open('rb') as source:
                    shutil.copyfileobj(source, sink, 65536)
        except BaseException:
            # Only remove an output we created ourselves. In particular, a
            # raced exclusive-create failure must NEVER delete someone else's
            # previously existing file.
            if created:
                destination.unlink(missing_ok=True)
            raise
        return total


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--acknowledge-network', action='store_true',
                        help='explicitly permit one HTTPS fetch to pinned public GitHub source')
    parser.add_argument('--output', type=Path, required=True,
                        help='new destination ZIP (must not exist)')
    args = parser.parse_args()
    if not args.acknowledge_network:
        parser.error('network fetch requires --acknowledge-network')
    pins = json.loads(import_tinycbor.PIN.read_text(encoding='utf-8'))
    if pins['tag'] != 'v7.0' or pins['commit'] != COMMIT:
        raise ValueError('upstream revision differs from local pin')
    size = retrieve_verified(args.output, pins)
    print(f'Upstream v7.0 source verified: {len(pins["files"])} pinned files; '
          f'{size} compressed bytes saved to {args.output}')
    print('TEST-ONLY source: separately run import_tinycbor.py --source ZIP --install; '
          'no product library or network parser is enabled')


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, RuntimeError, EOFError, zipfile.BadZipFile) as exc:
        print(f'Explicit upstream fetch: FAIL ({exc})', file=sys.stderr)
        sys.exit(1)
