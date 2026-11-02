#!/usr/bin/env python3
"""Behavior checks for narrow context, gate preservation and clean-source hashing."""
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

import task_context as context


class ProjectRoutes(unittest.TestCase):
    def test_every_registered_route_resolves_and_fits_both_formats(self):
        registry = json.loads((context.ROOT / context.REGISTRY).read_text())
        index = context.load_index(context.ROOT, registry)
        selections = [dict(area=a) for a in index['areas']]
        selections += [dict(task_id=t['id']) for t in registry['tasks']]
        selections += [{}]
        for selection in selections:
            with self.subTest(selection=selection):
                packet = context.build_packet(**selection)
                for as_json in (True, False):
                    self.assertLessEqual(len(context.render(packet, as_json).encode()), 8192)

    def test_ready_app_task_is_narrow_and_unimplemented(self):
        packet = context.build_packet(task_id='GW-APP-PUBLIC-RECORD-EXCHANGE')
        self.assertEqual(packet['task']['state'], 'ready')
        self.assertIsNone(packet['task']['owner'])
        self.assertEqual(packet['initial_write_scope'],
                         ['apps/cli/identity_session.cpp', 'tests/integration/identity_session.cmake'])
        self.assertTrue(all(t['state'] == 'completed' for t in packet['task_prerequisites']))
        self.assertLessEqual(len(packet['read']), 16)
        self.assertTrue(any(r.get('find') == 'verify_record(' for r in packet['read']))
        output = context.render(packet)
        self.assertNotIn('third_party/', output)
        self.assertNotIn('## Original revision', output)
        self.assertIn('AUTH-PRE', output)

    def test_default_omits_completed_history(self):
        packet = context.build_packet()
        self.assertTrue(packet['open_tasks'])
        self.assertTrue(all(t['state'] != 'completed' for t in packet['open_tasks']))
        self.assertNotIn('read', packet)

    def test_gate_and_completed_packets_do_not_propose_writes_or_reruns(self):
        for task in ['GW-A05-PARSER-DECISION', 'GW-A06-TRANSPORT', 'GW-AUTH-PRE-COMPOSITION',
                     'GW-APP-PROMPT-NAVIGATION']:
            with self.subTest(task=task):
                packet = context.build_packet(task_id=task)
                self.assertIn('review_only', packet['mode'])
                self.assertEqual(packet['initial_write_scope'], [])
                self.assertEqual(packet['checks_to_choose_after_changes'], [])

    def test_unknown_task_does_not_silently_pick_an_area(self):
        with self.assertRaises(context.ContextError):
            context.build_packet(task_id='not-a-task')

    def test_cli_works_outside_the_source_and_has_no_write_side_effects(self):
        # A new issue must remain triage, rather than borrowing a task's readiness.
        def snapshot():
            return {p: p.stat().st_mtime_ns for p in context.ROOT.rglob('*')
                    if p.is_file() and '__pycache__' not in p.parts and 'out' not in p.parts}
        before = snapshot()
        with tempfile.TemporaryDirectory() as directory:
            result = subprocess.run([sys.executable, str(context.ROOT / 'tools/development/task_context.py'),
                                     '--area', 'cli', '--json'], cwd=directory,
                                    capture_output=True, text=True, timeout=15)
        self.assertEqual(result.returncode, 0, result.stderr)
        packet = json.loads(result.stdout)
        self.assertTrue(packet['mode'].startswith('issue_triage'))
        self.assertEqual(before, snapshot())


class FixtureRoutes(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root / 'docs/development').mkdir(parents=True)
        (self.root / 'sample.md').write_text('# Notes\n## Selected\nDO NOT DUMP BODY\n### Child\nMore body\n## Other\nUnrelated\n')
        self.registry = {
            'schema_version': 1, 'checkpoint': 'fixture', 'source_revision': 1,
            'patch_author_tag': 'test-author', 'runtime_version': '0.4.1',
            'project_completion_percent': None,
            'modules': [{'id': 'module', 'state': 'verified', 'depends_on': [], 'evidence': ['docs/development/evidence.md']}],
            'tasks': [{'id': 'task', 'module': 'module', 'state': 'ready', 'depends_on': [], 'description': 'Scoped fixture'}]}
        (self.root / 'docs/development/evidence.md').write_text('local test evidence\n')
        self.index = {'schema_version': 1, 'rules': ['Always preserve invariants'],
                      'gates': ['Do not authorize blocked work'], 'module_areas': {'module': 'sample'}, 'tasks': {},
                      'areas': {'sample': {'read': [{'path': 'sample.md', 'find': '## Selected'}],
                                           'write': ['sample.md'], 'checks': ['choose scoped check'], 'expand': ['shared API']}}}
        self.write_indexes()

    def write_indexes(self):
        for name, data in [(context.REGISTRY, self.registry), (context.INDEX, self.index)]:
            (self.root / name).write_text(json.dumps(data))

    def manifest(self):
        files = {p.relative_to(self.root).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
                 for p in self.root.rglob('*') if p.is_file() and p.name != 'SOURCE_MANIFEST.json'}
        data = {k: self.registry[k] for k in ('checkpoint', 'source_revision', 'patch_author_tag')}
        data.update(algorithm='SHA-256', files=files)
        (self.root / 'SOURCE_MANIFEST.json').write_text(json.dumps(data))
        return data

    def test_section_locator_includes_child_but_never_prints_body(self):
        packet = context.build_packet(self.root, task_id='task')
        self.assertEqual(packet['read'][0]['line'], 2)
        self.assertEqual(packet['read'][0]['through_line'], 5)
        self.assertNotIn('DO NOT DUMP', context.render(packet))

    def test_missing_and_ambiguous_locators_fail_instead_of_expanding(self):
        for needle in ['not present', '## ']:
            with self.subTest(needle=needle), self.assertRaises(context.ContextError):
                context.locate(self.root, {'path': 'sample.md', 'find': needle})

    def test_unsafe_paths_and_symlink_components_are_rejected(self):
        for path in ['../sample.md', '/tmp/sample.md', 'C:/sample.md', 'a\\sample.md', './sample.md', 'a//sample.md']:
            with self.subTest(path=path), self.assertRaises(context.ContextError):
                context.safe_path(self.root, path)
        try:
            (self.root / 'linked').symlink_to(self.root / 'docs', target_is_directory=True)
        except OSError:
            self.skipTest('symlinks unavailable on this host')
        with self.assertRaises(context.ContextError):
            context.safe_path(self.root, 'linked/development/evidence.md')

    def test_packet_overflow_fails_without_dropping_rules(self):
        packet = context.build_packet(self.root, task_id='task')
        packet['rules'].append('é' * 8192)
        for as_json in (True, False):
            with self.subTest(as_json=as_json), self.assertRaises(context.ContextError):
                context.render(packet, as_json)

    def test_clean_source_verifies_and_modified_source_fails(self):
        manifest = self.manifest()
        packet = context.build_packet(self.root, task_id='task', verify=True)
        self.assertEqual(packet['source_verification']['listed_files_verified'], len(manifest['files']))
        (self.root / 'sample.md').write_text('tampered')
        with self.assertRaises(context.ContextError):
            context.build_packet(self.root, task_id='task', verify=True)

    def test_stale_manifest_and_inventory_escape_fail(self):
        manifest = self.manifest()
        manifest['source_revision'] = 0
        (self.root / 'SOURCE_MANIFEST.json').write_text(json.dumps(manifest))
        with self.assertRaises(context.ContextError):
            context.verify_source(self.root, self.registry)
        manifest['source_revision'] = 1
        manifest['files']['../outside'] = '0' * 64
        (self.root / 'SOURCE_MANIFEST.json').write_text(json.dumps(manifest))
        with self.assertRaises(context.ContextError):
            context.verify_source(self.root, self.registry)

    def test_invalid_index_task_and_too_broad_route_fail(self):
        self.index['tasks']['unknown'] = {'area': 'sample'}
        self.write_indexes()
        with self.assertRaises(context.ContextError):
            context.build_packet(self.root, task_id='task')
        self.index['tasks'].clear()
        self.index['areas']['sample']['read'] = [{'path': 'sample.md', 'find': f'needle {i}'} for i in range(25)]
        self.write_indexes()
        with self.assertRaisesRegex(context.ContextError, 'Too many'):
            context.build_packet(self.root, task_id='task')

    def test_cli_failure_is_nonzero_and_does_not_echo_unknown_input(self):
        secret = 'unknown-private-input'
        result = subprocess.run([sys.executable, '-B', str(context.ROOT / 'tools/development/task_context.py'),
                                 '--task', secret], capture_output=True, text=True, timeout=15)
        self.assertEqual(result.returncode, 1)
        self.assertEqual(result.stdout, '')
        self.assertNotIn(secret, result.stderr)


if __name__ == '__main__':
    unittest.main()
