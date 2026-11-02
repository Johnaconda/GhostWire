#!/usr/bin/env python3
"""Deterministic, allowlisted source ZIP with an embedded SHA-256 inventory."""
import argparse
import hashlib
import json
from pathlib import Path
import zipfile

# Owner-provided patch attribution label, separate from compiled runtime identity.
PATCH_AUTHOR_TAG = 'GPT-6.1-Sol-Max'
ROOT_FILES = {'CMakeLists.txt', 'CMakePresets.json', 'vcpkg.json', 'vcpkg-configuration.json',
              '.gitignore', '.gitattributes', '.editorconfig', '.clang-format', '.clang-tidy',
              'README.md', 'START-HERE.md', 'BUILD.md', 'ARCHITECTURE.md', 'ROADMAP.md', 'current-development-map.md', 'TODO.md', 'AGENTS.md',
              'CONTRIBUTING.md', 'SECURITY.md', 'LICENSE', 'THIRD_PARTY_NOTICES.md'}
DIRECTORIES = {'.github', 'apps', 'include', 'src', 'cmake', 'tests', 'tools', 'docs',
               'examples', 'third_party', 'schemas'}
SUFFIXES = {'.cpp', '.hpp', '.h', '.md', '.txt', '.cmake', '.in', '.json', '.yml', '.yaml',
            '.ps1', '.sh', '.py', '.conf', '.cddl'}
# Exact signed source-distribution exceptions, never a blanket binary/cache rule.
SOURCE_DISTRIBUTIONS = {'third_party/botan/Botan-3.13.0.tar.xz',
                        'third_party/botan/Botan-3.13.0.tar.xz.asc',
                        'third_party/tinycbor/verified/LICENSE'}
parser = argparse.ArgumentParser()
parser.add_argument('--output', type=Path)
parser.add_argument('--checkpoint', default='GW-DEV-CONTEXT-BOOTSTRAP')
args = parser.parse_args()
root = Path(__file__).resolve().parents[2]
destination = args.output or root / f'dist/GHOSTWIRE-GW-DEV-CONTEXT-BOOTSTRAP-source-{PATCH_AUTHOR_TAG}.zip'
destination = destination.resolve()
# Imported upstream code may only be shipped when all pinned bytes revalidate.
upstream = root / 'third_party' / 'tinycbor' / 'verified'
if upstream.exists():
    import sys
    sys.path.insert(0, str(root / 'tools' / 'protocol'))
    import import_tinycbor
    pins = json.loads((root / 'tools' / 'protocol' / 'tinycbor_pins.json').read_text())
    import_tinycbor.load_input(upstream, pins)
    # Only the verified pinned subset may introduce upstream C source files.
    SOURCE_DISTRIBUTIONS.update('third_party/tinycbor/verified/' + name for name in pins['files'])

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
manifest = json.dumps({'checkpoint': args.checkpoint,
                       'source_revision': 1,
                       'patch_author_tag': PATCH_AUTHOR_TAG,
                       'patch_author_tag_origin': 'owner-provided label',
                       'implementation_checkpoint': 'GW-APP-P02-MEMBERSHIP-LIFECYCLE',
                       'parent_checkpoint': 'GW-APP-P02-MEMBERSHIP-LIFECYCLE',
                       'parent_source_revision': 2,
                       'parent_archive_sha256': '45fa5488149b548b1bad58ecd06362ba7587636b84508954c5b0a8a4c200d73e',
                       'algorithm': 'SHA-256',
                       'files': inventory}, indent=2, sort_keys=True).encode() + b'\n'
destination.parent.mkdir(parents=True, exist_ok=True)
with zipfile.ZipFile(destination, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
    def add(name, data, executable=False):
        info = zipfile.ZipInfo('GhostWire/' + name, (2026, 11, 2, 0, 0, 0))
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
