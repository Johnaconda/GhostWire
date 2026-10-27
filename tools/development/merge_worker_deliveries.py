#!/usr/bin/env python3
"""Offline, non-executing merger for nonoverlapping source worker deliveries.

Each worker directory has delivery.json and files/<repo-relative paths>:
{
  "worker_id": "protocol-codec",
  "base_manifest_sha256": "64 lowercase SHA-256 hex chars",
  "files": [{"path":"src/protocol/wire_codec.cpp", "sha256":"...", "operation":"replace"}]
}

This checks the baseline, per-worker input bytes, collisions and path escapes,
then produces a fresh source tree. It does not compile or trust worker code.
"""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import shutil
import stat
import tempfile

SHA = re.compile(r'^[0-9a-f]{64}$')
ALLOWED_DIRS = {'apps', 'cmake', 'docs', 'examples', 'include', 'schemas',
                'src', 'tests', 'third_party', 'tools', '.github'}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def normalized_file(name):
    if not isinstance(name, str) or not name or '\\' in name or '\0' in name:
        raise ValueError('invalid source path')
    path = PurePosixPath(name)
    root_allowed = {'AGENTS.md', 'ARCHITECTURE.md', 'BUILD.md', 'CMakeLists.txt',
                    'CMakePresets.json', 'CONTRIBUTING.md', 'LICENSE', 'README.md',
                    'ROADMAP.md', 'SECURITY.md', 'THIRD_PARTY_NOTICES.md', 'TODO.md',
                    'vcpkg.json', 'vcpkg-configuration.json', '.editorconfig',
                    '.gitattributes', '.gitignore', '.clang-format', '.clang-tidy'}
    allowed = (name in root_allowed if len(path.parts) == 1 else
               path.parts[0] in ALLOWED_DIRS)
    if (path.is_absolute() or '..' in path.parts or path.as_posix() != name or
            not allowed or any(part in {'.git', '__pycache__'} for part in path.parts)):
        raise ValueError('unsafe or non-normalized source path')
    if name in {'SOURCE_MANIFEST.json', 'docs/development/WORKER_INTEGRATION.json'}:
        raise ValueError('reserved generated file')
    return path


def read_regular(path, root):
    if not path.is_relative_to(root):
        raise ValueError('path escapes worker/source root')
    for part in (path, *path.parents):
        if part == root.parent:
            break
        if part.is_symlink():
            raise ValueError('symlink source or parent')
    if not stat.S_ISREG(path.stat().st_mode):
        raise ValueError('not a regular source file')
    return path.read_bytes()


def load_baseline(root):
    manifest_path = root / 'SOURCE_MANIFEST.json'
    blob = read_regular(manifest_path, root)
    manifest = json.loads(blob)
    hashes = manifest['files']
    if not isinstance(hashes, dict):
        raise ValueError('invalid baseline inventory')
    folded = set()
    for name, expected in hashes.items():
        if not SHA.fullmatch(expected):
            raise ValueError('invalid baseline digest')
        if '/' not in name:
            if name not in {'AGENTS.md', 'ARCHITECTURE.md', 'BUILD.md', 'CMakeLists.txt',
                            'CMakePresets.json', 'CONTRIBUTING.md', 'LICENSE', 'README.md',
                            'ROADMAP.md', 'SECURITY.md', 'THIRD_PARTY_NOTICES.md',
                            'TODO.md', 'vcpkg.json', 'vcpkg-configuration.json',
                            '.editorconfig', '.gitattributes', '.gitignore', '.clang-format',
                            '.clang-tidy'}:
                raise ValueError('unsafe root source path')
            path = PurePosixPath(name)
        else:
            path = normalized_file(name)
        if name.casefold() in folded:
            raise ValueError('baseline case collision')
        folded.add(name.casefold())
        if digest(read_regular(root.joinpath(*path.parts), root)) != expected:
            raise ValueError(f'baseline source mismatch: {name}')
    return digest(blob), hashes


def load_delivery(root, directory, base_hash, baseline_files):
    if directory.is_symlink() or not directory.is_dir():
        raise ValueError('unsafe worker directory')
    package = json.loads(read_regular(directory / 'delivery.json', directory))
    worker = package.get('worker_id')
    if (not isinstance(worker, str) or not re.fullmatch(r'[a-z0-9][a-z0-9-]{1,47}', worker)):
        raise ValueError('invalid worker identifier')
    if package.get('base_manifest_sha256') != base_hash:
        raise ValueError(f'worker {worker} used a different source baseline')
    if not isinstance(package.get('files'), list) or not package['files']:
        raise ValueError('empty worker delivery')
    entries = []
    for item in package['files']:
        path = normalized_file(item['path'])
        name = path.as_posix()
        expected = item['sha256']
        operation = item['operation']
        if not isinstance(expected, str) or not SHA.fullmatch(expected):
            raise ValueError('invalid worker file hash')
        if operation not in ('add', 'replace'):
            raise ValueError('unsupported worker operation')
        if (operation == 'add') != (name not in baseline_files):
            raise ValueError('worker addition/replacement mismatch with baseline')
        data = read_regular(directory.joinpath('files', *path.parts), directory)
        if digest(data) != expected:
            raise ValueError(f'worker source integrity mismatch: {worker}/{name}')
        entries.append((name, data, {'path': name, 'sha256': expected,
                                     'operation': operation}))
    return worker, entries


def integrate(baseline, output, workers):
    if output.exists() or output.is_symlink():
        raise ValueError('output destination already exists')
    base_hash, files = load_baseline(baseline)
    claims = {}
    worker_ids = set()
    root_names = {name.casefold() for name in files}
    accepted = []
    for directory in workers:
        name, entries = load_delivery(baseline, directory, base_hash, files)
        if name in worker_ids:
            raise ValueError('duplicate worker ID')
        worker_ids.add(name)
        for path, data, info in entries:
            folded = path.casefold()
            if folded in claims or (folded in root_names and path not in files):
                raise ValueError(f'conflicting worker file ownership: {path}')
            claims[folded] = name
            accepted.append((path, data, name, info))
    if not accepted:
        raise ValueError('empty integration')
    # Never write into a source or a worker delivery. A failed merge must
    # leave both originals intact for a later independent review.
    if any(output.resolve().is_relative_to(input_dir.resolve())
           for input_dir in (baseline, *workers)):
        raise ValueError('output nested inside input directory')
    output.parent.mkdir(parents=True, exist_ok=True)
    stage = Path(tempfile.mkdtemp(prefix='gw-merge-', dir=output.parent))
    try:
        for name in sorted(files):
            src = baseline.joinpath(*PurePosixPath(name).parts)
            dst = stage.joinpath(*PurePosixPath(name).parts)
            dst.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(src, dst)
        for name, data, _, _ in accepted:
            dest = stage.joinpath(*PurePosixPath(name).parts)
            dest.parent.mkdir(parents=True, exist_ok=True)
            dest.write_bytes(data)
        report = {'format': 1, 'baseline_manifest_sha256': base_hash,
                  'workers': sorted(worker_ids),
                  'merged_files': [dict(info, worker_id=worker)
                                   for _, _, worker, info in sorted(accepted)]}
        record = stage / 'docs/development/WORKER_INTEGRATION.json'
        record.parent.mkdir(parents=True, exist_ok=True)
        record.write_text(json.dumps(report, sort_keys=True, indent=2) + '\n', encoding='utf-8')
        stage.rename(output)
    finally:
        if stage.exists():
            shutil.rmtree(stage)
    return len(accepted), len(worker_ids)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--worker', type=Path, required=True, action='append')
    opts = parser.parse_args()
    files, workers = integrate(opts.baseline.resolve(), opts.output.resolve(),
                               [w.resolve() for w in opts.worker])
    print(f'Worker merge PASS: {workers} independent deliveries, {files} files, no source collisions')


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, KeyError, TypeError, json.JSONDecodeError) as exc:
        print(f'Worker merge rejected: {exc}', file=__import__('sys').stderr)
        raise SystemExit(1)
