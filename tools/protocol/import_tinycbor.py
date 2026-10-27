#!/usr/bin/env python3
"""Verify pinned upstream TinyCBOR v7.0 source without making network requests.

Default: verify only. --install imports a verified subset into the test-only
vendor directory; builds never download dependencies. It is NOT a security audit.
"""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import shutil
import stat
import sys
import tempfile
import zipfile

ROOT = Path(__file__).resolve().parents[2]
PIN = ROOT / 'tools' / 'protocol' / 'tinycbor_pins.json'
DEST = ROOT / 'third_party' / 'tinycbor' / 'verified'


def check_bytes(name, data, spec):
    length, checksum = spec
    if len(data) != length or hashlib.sha256(data).hexdigest() != checksum:
        raise ValueError(f'pinned upstream mismatch: {name}')
    return data


def load_input(src, pins):
    required = pins['files']
    if src.is_dir():
        # The root and its ancestors can themselves be symlinks. This is a
        # local source-import boundary, never a trust assertion about a remote
        # repository or a replacement for pinned checksums.
        if any(part.is_symlink() for part in (src, *src.parents)):
            raise ValueError('symbolic link in source root')
        result = {}
        for name, spec in required.items():
            path = src / name
            if any((src / parent).is_symlink() for parent in PurePosixPath(name).parents if str(parent) != '.'):
                raise ValueError('symbolic link in source path')
            if path.is_symlink() or not stat.S_ISREG(path.stat().st_mode):
                raise ValueError(f'non-regular upstream source file: {name}')
            if path.stat().st_size != spec[0]:
                raise ValueError(f'pinned upstream size mismatch: {name}')
            result[name] = check_bytes(name, path.read_bytes(), spec)
        return result
    if not zipfile.is_zipfile(src):
        raise ValueError('expected extracted upstream source directory or ZIP')
    with zipfile.ZipFile(src) as arc:
        names = set()
        names_folded = set()
        selected = {}
        archive_roots = set()
        total_size = 0
        for info in arc.infolist():
            # zipfile normalizes Windows backslashes in info.filename, but
            # info.orig_filename retains the decoded archive member name.
            # Reject normalization instead of trusting the rewritten path.
            if (info.orig_filename != info.filename or
                    '\\' in info.orig_filename or '\x00' in info.orig_filename):
                raise ValueError('unsafe or rewritten archive member')
            path = PurePosixPath(info.orig_filename)
            normalized = path.as_posix() + ('/' if info.is_dir() else '')
            if (not path.parts or path.is_absolute() or '..' in path.parts or
                    '\\' in info.filename or normalized != info.filename):
                raise ValueError('unsafe or non-normalized archive member')
            if info.filename in names or info.filename.casefold() in names_folded:
                raise ValueError('duplicate or case-colliding archive member')
            names.add(info.filename)
            names_folded.add(info.filename.casefold())
            kind = stat.S_IFMT(info.external_attr >> 16)
            if kind == stat.S_IFLNK or (kind not in (0, stat.S_IFDIR, stat.S_IFREG)):
                raise ValueError('non-regular archive member')
            if info.file_size > 2 * 1024 * 1024:
                raise ValueError('oversized archive member')
            total_size += info.file_size
            if total_size > 16 * 1024 * 1024 or len(names) > 512:
                raise ValueError('oversized archive')
            if info.is_dir() and len(path.parts) == 1:
                archive_roots.add(path.parts[0])
                continue
            if len(path.parts) < 2:
                raise ValueError('expected one archive root directory')
            archive_roots.add(path.parts[0])
            if info.is_dir():
                continue
            # All members, not merely the pinned subset, must have the same
            # root. Otherwise a misleading source archive could be accepted.
            rel = '/'.join(path.parts[1:])
            if rel in required:
                if rel in selected:
                    raise ValueError('duplicate required upstream path')
                selected[rel] = check_bytes(rel, arc.read(info), required[rel])
        if len(archive_roots) != 1:
            raise ValueError('archive mixes upstream source roots')
        if set(selected) != set(required):
            raise ValueError('required upstream source file missing')
        if arc.testzip() is not None:
            raise ValueError('upstream archive failed CRC')
        return selected


def publish_verified(files, destination=DEST):
    """Stage a verified subset; reject preexisting destinations incl. dead links.

    This is an offline cooperative publication path, not a defense against an
    actively hostile process racing filesystem mutations in the same parent.
    """
    if destination.exists() or destination.is_symlink():
        raise ValueError('verified vendor destination already exists; remove it explicitly before replacing')
    with tempfile.TemporaryDirectory(prefix='gw-upstream-', dir=destination.parent) as td:
        stage = Path(td) / 'verified'
        for name, data in files.items():
            out = stage / name
            out.parent.mkdir(parents=True, exist_ok=True)
            out.write_bytes(data)
        if destination.exists() or destination.is_symlink():
            raise ValueError('verified vendor destination appeared during staging')
        # On ordinary cooperative builds rename is atomic. The destination
        # parent must still be secured by its owner against hostile races.
        stage.rename(destination)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--install', action='store_true', help='stage verified sources for optional test builds')
    args = parser.parse_args()
    pins = json.loads(PIN.read_text(encoding='utf-8'))
    if pins['tag'] != 'v7.0' or pins['commit'] != '6442e749ca811e24afad1551338a45c75e09808f':
        raise ValueError('unexpected upstream source revision')
    files = load_input(args.source, pins)
    if args.install:
        # Do not modify or overlay an earlier accepted vendor destination.
        publish_verified(files)
    print(f'TinyCBOR v7.0 pinned source verification: PASS ({len(files)}/{len(pins["files"])})'
          + ('; TEST-ONLY sources staged' if args.install else '; no files changed'))


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, zipfile.BadZipFile) as exc:
        print(f'TinyCBOR pinned source verification: FAIL ({exc})', file=sys.stderr)
        sys.exit(1)
