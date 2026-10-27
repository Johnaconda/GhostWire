#!/usr/bin/env python3
"""Synthetic and adversarial regressions for nonexecuting offline worker merger."""
import hashlib
import json
from pathlib import Path
import sys
import tempfile
sys.path.insert(0, str(Path(__file__).resolve().parent))
import merge_worker_deliveries as merger


def h(data):
    return hashlib.sha256(data).hexdigest()


def deliver(root, name, manifest_hash, file_specs):
    root.mkdir()
    files = []
    for path, operation, data in file_specs:
        file = root / 'files' / path
        file.parent.mkdir(parents=True, exist_ok=True)
        file.write_bytes(data)
        files.append({'path': path, 'operation': operation, 'sha256': h(data)})
    (root / 'delivery.json').write_text(json.dumps({'worker_id': name,
        'base_manifest_sha256': manifest_hash, 'files': files}), encoding='utf-8')


def must_reject(base, out, workers):
    prior_exists = out.exists()
    prior_contents = (out / 'src/source.cpp').read_bytes() if prior_exists else None
    try:
        merger.integrate(base, out, workers)
    except (ValueError, OSError, KeyError, TypeError):
        assert out.exists() == prior_exists, 'failed merge changed output existence'
        if prior_exists:
            assert (out / 'src/source.cpp').read_bytes() == prior_contents
        return
    raise AssertionError('unsafe/inconsistent worker merge accepted')


def test():
    with tempfile.TemporaryDirectory() as d:
        root = Path(d)
        base = root / 'source'; (base / 'src').mkdir(parents=True)
        original = b'int original = 42;\n'
        (base / 'src/source.cpp').write_bytes(original)
        manifest = {'algorithm': 'SHA-256', 'files': {'src/source.cpp': h(original)}}
        (base / 'SOURCE_MANIFEST.json').write_text(json.dumps(manifest), encoding='utf-8')
        base_hash = h((base / 'SOURCE_MANIFEST.json').read_bytes())
        a, b = root / 'a', root / 'b'
        deliver(a, 'worker-codec', base_hash, [('src/source.cpp', 'replace', b'int source = 7;\n')])
        deliver(b, 'worker-tests', base_hash, [('tests/codec_test.cpp', 'add', b'void test() {}\n')])
        assert merger.integrate(base, root / 'merged', [a, b]) == (2, 2)
        assert (base / 'src/source.cpp').read_bytes() == original
        assert (root / 'merged/src/source.cpp').read_bytes() == b'int source = 7;\n'
        assert (root / 'merged/tests/codec_test.cpp').read_bytes() == b'void test() {}\n'
        assert not (root / 'merged/SOURCE_MANIFEST.json').exists()
        report = json.loads((root / 'merged/docs/development/WORKER_INTEGRATION.json').read_text())
        assert report['workers'] == ['worker-codec', 'worker-tests']
        must_reject(base, root / 'merged', [a, b])
        c = root / 'c'
        deliver(c, 'worker-conflict', base_hash, [('src/source.cpp', 'replace', b'X')])
        must_reject(base, root / 'collision', [a, c])
        stale = root / 'stale'; deliver(stale, 'worker-stale', '0' * 64,
                                     [('tools/future.py', 'add', b'A')])
        must_reject(base, root / 'out-stale', [stale])
        (b / 'files/tests/codec_test.cpp').write_bytes(b'tamper')
        must_reject(base, root / 'out-tamper', [a, b])
        (b / 'files/tests/codec_test.cpp').write_bytes(b'void test() {}\n')
        (base / 'src/source.cpp').write_bytes(b'changed baseline')
        must_reject(base, root / 'out-baseline', [a, b])
        (base / 'src/source.cpp').write_bytes(original)
        bad = root / 'bad'; deliver(bad, 'worker-malicious', base_hash,
                                  [('tools/future.py', 'add', b'payload')])
        spec = json.loads((bad / 'delivery.json').read_text())
        for path in ('../escape.py', '/tmp/escape.py', 'tools\\bad.py', 'tools//bad.py',
                     'tools/../bad.py', '.git/config', 'SOURCE_MANIFEST.json'):
            spec['files'][0]['path'] = path
            (bad / 'delivery.json').write_text(json.dumps(spec))
            must_reject(base, root / 'out-unsafe', [bad])
        spec['files'][0]['path'] = 'tools/future.py'
        (bad / 'delivery.json').write_text(json.dumps(spec))
        must_reject(base, bad / 'nested', [bad])
        duplicate = root / 'duplicate'; deliver(duplicate, 'worker-tests', base_hash,
                                               [('tools/new.py', 'add', b'pass\n')])
        must_reject(base, root / 'out-duplicate-worker', [b, duplicate])
        print('Worker delivery merger: PASS (integration, ownership, integrity, collisions, paths, atomicity)')


if __name__ == '__main__':
    test()
