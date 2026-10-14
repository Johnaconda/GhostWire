#!/usr/bin/env python3
"""Regenerate the pinned upstream portable amalgamations; no downloads or patches."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tarfile
import tempfile

ROOT = Path(__file__).resolve().parents[2]
VERSION = '3.13.0'
ARCHIVE_SHA = '12f5a8358890bbee82edfe9d2e7769b0a610b6dd0e0698aea13d20a675d84620'
MODULES = 'system_rng,chacha20poly1305,ed25519,sha2_32,locking_allocator'
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--check', action='store_true', help='compare without changing source files')
args = parser.parse_args()
vendor = ROOT / 'third_party/botan'
archive = vendor / f'Botan-{VERSION}.tar.xz'
if hashlib.sha256(archive.read_bytes()).hexdigest() != ARCHIVE_SHA:
    parser.error('Pinned upstream source hash mismatch')
with tempfile.TemporaryDirectory(prefix='ghostwire-botan-') as temporary:
    work = Path(temporary)
    with tarfile.open(archive) as source:
        members = []
        for member in source.getmembers():
            path = Path(member.name)
            if path.is_absolute() or '..' in path.parts or path.parts[0] != f'Botan-{VERSION}':
                parser.error('Unsafe upstream archive member')
            # The upstream .github/codecov.yml symlink is not needed to build.
            if member.isfile() or member.isdir():
                members.append(member)
        source.extractall(work, members=members, filter='data')
    generated = {}
    for system, compiler in [('linux', 'gcc'), ('windows', 'msvc'), ('macos', 'clang')]:
        out = work / system
        out.mkdir()
        command = [sys.executable, str(work / f'Botan-{VERSION}/configure.py'),
                   f'--os={system}', f'--cc={compiler}', '--cc-min-version=0.0',
                   '--cpu=generic', '--disable-asm', '--minimized-build',
                   f'--enable-modules={MODULES}', '--disable-shared-library',
                   '--build-targets=static', '--amalgamation', '--link-method=copy',
                   '--without-documentation', '--distribution-info=GhostWire-A03',
                   # Upstream embeds prefix and configure arguments in its output.
                   # A fixed, unused installation prefix makes regeneration exact;
                   # this script never installs or writes to that location.
                   '--prefix=/opt/ghostwire-botan',
                   '--system-cert-bundle=']
        if system == 'windows':
            command += ['--without-os-features=rtlgenrandom', '--with-os-features=crypto_ng']
        configured = subprocess.run(command, cwd=out, capture_output=True, text=True)
        if configured.returncode:
            parser.error(configured.stdout + configured.stderr)
        for name in ['botan_all.h', 'botan_all.cpp']:
            data = (out / name).read_bytes()
            relative = f'{system}/{name}'
            generated[relative] = hashlib.sha256(data).hexdigest()
            destination = vendor / relative
            if args.check:
                if not destination.is_file() or destination.read_bytes() != data:
                    parser.error(f'Regeneration differs: {relative}')
            else:
                destination.parent.mkdir(parents=True, exist_ok=True)
                destination.write_bytes(data)
    manifest = {'version': VERSION, 'archive_sha256': ARCHIVE_SHA,
                'revision': '2a81eef56c96c237e590c27f9a75e60317c9c700',
                'modules': MODULES.split(','), 'generated_sha256': generated}
    pins = (json.dumps(manifest, indent=2, sort_keys=True) + '\n').encode()
    if args.check:
        if (vendor / 'pins.json').read_bytes() != pins:
            parser.error('Dependency pin inventory differs')
    else:
        (vendor / 'pins.json').write_bytes(pins)
print('Botan source pin and all three portable amalgamations verified' if args.check else
      'Generated three upstream portable amalgamations and pins.json')
