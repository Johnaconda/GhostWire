#!/usr/bin/env python3
"""Deterministic negative tests for GhostWire's provisional 10-percent gate plan."""
from __future__ import annotations
import copy
import json
from pathlib import Path
import tempfile
import unittest
from effort_forecast import FORECAST, ForecastError, calculate


class ForecastTests(unittest.TestCase):
    def setUp(self):
        self.good = json.loads(FORECAST.read_text(encoding='utf-8'))

    def check(self, obj):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'assumptions.json'
            path.write_text(json.dumps(obj), encoding='utf-8')
            return calculate(forecast=path)

    def test_unmodified_forecast_is_explicitly_provisional(self):
        report = self.check(self.good)
        self.assertEqual(report['effort_points_total'], 110)
        self.assertEqual(report['confirmed_weighted_floor_percent'], 10.0)
        self.assertEqual(report['gates_reached'], [10])
        self.assertIsNone(report['official_module_registry_completion_percent'])

    def test_missing_module_cannot_fabricate_more_progress(self):
        bad = copy.deepcopy(self.good)
        bad['weight_points'].pop('GW-A25')
        with self.assertRaises(ForecastError): self.check(bad)

    def test_extra_module_cannot_fabricate_less_progress(self):
        bad = copy.deepcopy(self.good)
        bad['weight_points']['GW-A99'] = 10
        with self.assertRaises(ForecastError): self.check(bad)

    def test_declared_sum_must_match_weights(self):
        bad = copy.deepcopy(self.good)
        bad['total_effort_points'] = 50
        with self.assertRaises(ForecastError): self.check(bad)

    def test_no_invalid_weights(self):
        for invalid in (0, -1, 1.5, True, '5'):
            bad = copy.deepcopy(self.good)
            bad['weight_points']['GW-A03'] = invalid
            with self.assertRaises(ForecastError): self.check(bad)

    def test_no_claim_of_owner_approval(self):
        bad = copy.deepcopy(self.good)
        bad['status'] = 'accepted'
        with self.assertRaises(ForecastError): self.check(bad)

    def test_windows_gates_are_each_ten_percent(self):
        bad = copy.deepcopy(self.good)
        bad['windows_gate_percentages'] = [5,10,100]
        with self.assertRaises(ForecastError): self.check(bad)

if __name__ == '__main__':
    unittest.main()
