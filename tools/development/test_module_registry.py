#!/usr/bin/env python3
"""No-execute structural unit tests for the module/queue gate."""
from __future__ import annotations

import copy
import json
from pathlib import Path
import tempfile
import unittest

from module_registry import ROOT, REGISTRY, RegistryError, load_and_validate, summary


class RegistryTests(unittest.TestCase):
    def setUp(self):
        self.good = json.loads(REGISTRY.read_text(encoding='utf-8'))

    def check(self, obj):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp) / 'registry.json'
            p.write_text(json.dumps(obj), encoding='utf-8')
            return load_and_validate(p, ROOT)

    def test_actual_registry_valid_and_no_fabricated_progress(self):
        data = self.check(self.good)
        self.assertIsNone(summary(data)['project_completion_percent'])
        # A task queue is permitted to have no dependency-ready work if the
        # remaining work is genuinely blocked by independent security gates.
        self.assertTrue(summary(data)['ready_tasks'] or summary(data)['blocked_tasks'])

    def add_synthetic_ready(self, bad):
        task = copy.deepcopy(next(t for t in bad['tasks'] if t['state'] == 'completed' and t['module'] == 'GW-A05'))
        task['id'] = 'GW-TEST-SYNTHETIC-READY'
        task['state'] = 'ready'
        task['depends_on'] = []
        bad['tasks'].append(task)
        return task

    def test_reject_project_completion_claim(self):
        bad = copy.deepcopy(self.good)
        bad['project_completion_percent'] = 25
        with self.assertRaises(RegistryError): self.check(bad)

    def test_reject_duplicate_modules(self):
        bad = copy.deepcopy(self.good)
        bad['modules'].append(copy.deepcopy(bad['modules'][0]))
        with self.assertRaises(RegistryError): self.check(bad)

    def test_reject_evidence_path_escape(self):
        bad = copy.deepcopy(self.good)
        bad['modules'][0]['evidence'] = ['docs/../../outside.md']
        with self.assertRaises(RegistryError): self.check(bad)

    def test_reject_ready_with_unverified_module(self):
        bad = copy.deepcopy(self.good)
        next_ready = self.add_synthetic_ready(bad)
        module = next(m for m in bad['modules'] if m['id'] == next_ready['module'])
        dep = module['depends_on'][0]
        next(m for m in bad['modules'] if m['id'] == dep)['state'] = 'partial'
        with self.assertRaises(RegistryError): self.check(bad)

    def test_reject_missing_dependency(self):
        bad = copy.deepcopy(self.good)
        bad['modules'][0]['depends_on'] = ['GW-A99']
        with self.assertRaises(RegistryError): self.check(bad)

    def test_reject_dependency_cycle(self):
        bad = copy.deepcopy(self.good)
        bad['modules'][0]['depends_on'] = ['GW-A05']
        with self.assertRaises(RegistryError): self.check(bad)

    def test_reject_verified_without_evidence(self):
        bad = copy.deepcopy(self.good)
        bad['modules'][0]['evidence'] = ['docs/MISSING.md']
        with self.assertRaises(RegistryError): self.check(bad)

    def test_reject_owner_acceptance_on_partial(self):
        bad = copy.deepcopy(self.good)
        m = next(m for m in bad['modules'] if m['state'] == 'partial')
        m['owner_accepted'] = True
        with self.assertRaises(RegistryError): self.check(bad)

    def test_reject_ready_with_open_prerequisite(self):
        bad = copy.deepcopy(self.good)
        done = next(t for t in bad['tasks'] if t['state'] == 'completed')
        task = self.add_synthetic_ready(bad)
        done['state'] = 'blocked'
        done['blocker'] = 'simulation'
        task['depends_on'].append(done['id'])
        with self.assertRaises(RegistryError): self.check(bad)

    def test_reject_low_confidence_completed_work(self):
        bad = copy.deepcopy(self.good)
        done = next(t for t in bad['tasks'] if t['state'] == 'completed')
        done['confidence'] = 0.80
        with self.assertRaises(RegistryError): self.check(bad)

    def test_reject_completed_task_without_evidence(self):
        bad = copy.deepcopy(self.good)
        done = next(t for t in bad['tasks'] if t['state'] == 'completed')
        done['evidence'] = []
        with self.assertRaises(RegistryError): self.check(bad)

    def test_reject_unexplained_blocker(self):
        bad = copy.deepcopy(self.good)
        blocked = next(t for t in bad['tasks'] if t['state'] == 'blocked')
        blocked.pop('blocker', None)
        with self.assertRaises(RegistryError): self.check(bad)


if __name__ == '__main__':
    unittest.main()
