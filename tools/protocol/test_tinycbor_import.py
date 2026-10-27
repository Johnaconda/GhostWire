#!/usr/bin/env python3
"""Offline importer regression matrix, with synthetic data only."""
import hashlib
from pathlib import Path
import stat
import sys
import tempfile
import zipfile
sys.path.insert(0, str(Path(__file__).resolve().parent))
import import_tinycbor as imp


def must_reject(input_path, pins, scenario):
    try:
        imp.load_input(input_path, pins)
    except (ValueError, OSError, zipfile.BadZipFile):
        return
    raise AssertionError(f'{scenario}: malformed upstream input accepted')


def make_zip(path, members):
    with zipfile.ZipFile(path, 'w') as archive:
        for name, value in members:
            if isinstance(value, zipfile.ZipInfo):
                archive.writestr(value, b'alias')
            else:
                archive.writestr(name, value)


def test():
    payload = b'test pinned input'
    pins = {'files': {'LICENSE': [len(payload), hashlib.sha256(payload).hexdigest()]}}
    count = 0
    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        src = root / 'src'; src.mkdir()
        (src / 'LICENSE').write_bytes(payload)
        assert imp.load_input(src, pins) == {'LICENSE': payload}; count += 1
        (src / 'LICENSE').write_bytes(b'tampered')
        must_reject(src, pins, 'tampered file'); count += 1
        (src / 'LICENSE').write_bytes(payload)
        if hasattr(Path, 'symlink_to'):
            try:
                (root / 'linked').symlink_to(src, target_is_directory=True)
                must_reject(root / 'linked', pins, 'symlinked root'); count += 1
                (root / 'linked').unlink()
                (src / 'LICENSE').unlink()
                (src / 'LICENSE').symlink_to(root / 'real-license')
                (root / 'real-license').write_bytes(payload)
                must_reject(src, pins, 'symlinked required file'); count += 1
            except (OSError, NotImplementedError):
                # Creating symlinks requires extra privileges on some Windows hosts.
                pass
            finally:
                (src / 'LICENSE').unlink(missing_ok=True)
                (src / 'LICENSE').write_bytes(payload)
        archive = root / 'source.zip'
        good = [('tinycbor-7.0/LICENSE', payload)]
        make_zip(archive, good)
        assert imp.load_input(archive, pins) == {'LICENSE': payload}; count += 1
        cases = [
            ('traversal', [('../danger', b'a'), *good]),
            ('duplicate', [*good, *good]),
            ('multiple roots', [*good, ('other/README', b'extra')]),
            ('case collision', [*good, ('tinycbor-7.0/license', b'extra')]),
            ('non-normalized name', [*good, ('tinycbor-7.0/./README', b'extra')]),
            ('missing pinned file', [('tinycbor-7.0/README', b'extra')]),
            ('oversized unrelated file', [*good, ('tinycbor-7.0/blob', b'!' * (2 * 1024 * 1024 + 1))]),
        ]
        for scenario, members in cases:
            make_zip(archive, members)
            must_reject(archive, pins, scenario); count += 1
        # Construct an actual malformed ZIP on disk. zipfile.writestr()
        # normalizes backslashes on Windows before writing, so passing a
        # backslash to that API would accidentally generate a VALID archive.
        original = b'tinycbor-7.0/README'
        malformed = b'tinycbor-7.0\\README'
        assert len(original) == len(malformed)
        make_zip(archive, [*good, ('tinycbor-7.0/README', b'extra')])
        raw = archive.read_bytes()
        assert raw.count(original) == 2  # local and central directory names
        archive.write_bytes(raw.replace(original, malformed))
        must_reject(archive, pins, 'backslash'); count += 1
        symlink = zipfile.ZipInfo('tinycbor-7.0/LICENSE')
        symlink.create_system = 3
        symlink.external_attr = (stat.S_IFLNK | 0o777) << 16
        make_zip(archive, [(symlink.filename, symlink)])
        must_reject(archive, pins, 'symlink archive entry'); count += 1
        # Publication must reject a dangling symlink, not just existing paths.
        # Also verify ordinary first-time staging, and never modify an existing
        # imported directory; the real pinned manifest is tested separately.
        destination = root / 'vendor'
        imp.publish_verified({'src/test.h': payload}, destination)
        assert (destination / 'src/test.h').read_bytes() == payload; count += 1
        try:
            imp.publish_verified({'src/test.h': b'overwrite'}, destination)
        except ValueError:
            assert (destination / 'src/test.h').read_bytes() == payload
            count += 1
        else:
            raise AssertionError('existing vendor destination replaced')
        if hasattr(Path, 'symlink_to'):
            dead = root / 'dangling-vendor'
            try:
                dead.symlink_to(root / 'uncreated-target', target_is_directory=True)
                try:
                    imp.publish_verified({'src/test.h': payload}, dead)
                except ValueError:
                    assert dead.is_symlink()
                    count += 1
                else:
                    raise AssertionError('dangling symlink vendor destination replaced')
            except (OSError, NotImplementedError):
                pass  # Windows may require symlink creation privileges.
        directory_entry = zipfile.ZipInfo('tinycbor-7.0/LICENSE/')
        directory_entry.create_system = 3
        directory_entry.external_attr = (stat.S_IFDIR | 0o755) << 16
        make_zip(archive, [(directory_entry.filename, directory_entry)])
        must_reject(archive, pins, 'directory masquerading as required file'); count += 1
    print(f'TinyCBOR import verifier matrix: PASS {count} scenarios (symlink cases privilege-conditional)')


if __name__ == '__main__':
    test()
