#!/usr/bin/env python3
"""Deterministic, allowlisted source ZIP with an embedded SHA-256 inventory."""
import argparse
import hashlib
import json
from pathlib import Path
import zipfile

ROOT_FILES = {'CMakeLists.txt', 'CMakePresets.json', 'vcpkg.json', 'vcpkg-configuration.json',
              '.gitignore', '.gitattributes', '.editorconfig', '.clang-format', '.clang-tidy',
              'README.md', 'BUILD.md', 'ARCHITECTURE.md', 'ROADMAP.md', 'TODO.md', 'AGENTS.md',
              'CONTRIBUTING.md', 'SECURITY.md', 'LICENSE', 'THIRD_PARTY_NOTICES.md'}
DIRECTORIES = {'.github', 'apps', 'include', 'src', 'cmake', 'tests', 'tools', 'docs',
               'examples', 'third_party'}
SUFFIXES = {'.cpp', '.hpp', '.h', '.md', '.txt', '.cmake', '.in', '.json', '.yml', '.yaml',
            '.ps1', '.sh', '.py', '.conf'}
# Exact signed source-distribution exceptions, never a blanket binary/cache rule.
SOURCE_DISTRIBUTIONS = {'third_party/botan/Botan-3.13.0.tar.xz',
                        'third_party/botan/Botan-3.13.0.tar.xz.asc'}
parser = argparse.ArgumentParser()
parser.add_argument('--output', type=Path)
args = parser.parse_args()
root = Path(__file__).resolve().parents[2]
destination = args.output or root / 'dist/GHOSTWIRE-GW-A03-CRYPTOGRAPHIC-PROVIDER-source.zip'
destination = destination.resolve()
files = []
for name in sorted(ROOT_FILES):
    path = root / name
    if not path.is_file() or path.is_symlink():
        parser.error(f'Missing or unsafe required file: {name}')
    files.append(path)
for directory in sorted(DIRECTORIES):
    for path in sorted((root / directory).rglob('*')):
        if path.is_symlink():
            parser.error(f'Symlink not permitted in source package: {path.relative_to(root)}')
        if '__pycache__' in path.parts:
            continue
        if path.is_file():
            if path.suffix not in SUFFIXES and path.relative_to(root).as_posix() not in SOURCE_DISTRIBUTIONS:
                parser.error(f'Unexpected file in source directory: {path.relative_to(root)}')
            files.append(path)
files.sort(key=lambda p: p.relative_to(root).as_posix())
if destination == root or destination.is_relative_to(root / 'docs') or destination in files:
    parser.error('Output must not overwrite source')
inventory = {path.relative_to(root).as_posix(): hashlib.sha256(path.read_bytes()).hexdigest() for path in files}
manifest = json.dumps({'checkpoint': 'GW-A03-CRYPTOGRAPHIC-PROVIDER', 'algorithm': 'SHA-256',
                       'files': inventory}, indent=2, sort_keys=True).encode() + b'\n'
destination.parent.mkdir(parents=True, exist_ok=True)
with zipfile.ZipFile(destination, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
    def add(name, data, executable=False):
        info = zipfile.ZipInfo('GhostWire/' + name, (2026, 10, 13, 0, 0, 0))
        info.compress_type = zipfile.ZIP_DEFLATED
        info.create_system = 3
        info.external_attr = (0o100755 if executable else 0o100644) << 16
        archive.writestr(info, data)
    for path in files:
        add(path.relative_to(root).as_posix(), path.read_bytes(), path.suffix in {'.sh', '.py'})
    add('SOURCE_MANIFEST.json', manifest)
with zipfile.ZipFile(destination) as archive:
    if archive.testzip() is not None:
        raise RuntimeError('ZIP CRC verification failed')
    if len(archive.namelist()) != len(files) + 1:
        raise RuntimeError('ZIP entry count mismatch')
    for name, expected in inventory.items():
        if hashlib.sha256(archive.read('GhostWire/' + name)).hexdigest() != expected:
            raise RuntimeError(f'ZIP content mismatch: {name}')
checksum = hashlib.sha256(destination.read_bytes()).hexdigest()
destination.with_suffix(destination.suffix + '.sha256').write_text(f'{checksum}  {destination.name}\n')
print(f'{destination}\nSHA-256: {checksum}\nVerified {len(files)} source files + inventory; {destination.stat().st_size} bytes')
