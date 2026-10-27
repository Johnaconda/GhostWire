#!/usr/bin/env python3
"""Provisional weighted effort and Windows gate planning; never an authority claim."""
from __future__ import annotations
import json
from pathlib import Path
from module_registry import ROOT, REGISTRY, load_and_validate

FORECAST = ROOT / 'docs/development/PROJECT_EFFORT_FORECAST.json'

class ForecastError(ValueError):
    pass


def calculate(registry=REGISTRY, forecast=FORECAST):
    data = load_and_validate(registry, ROOT)
    assumptions = json.loads(Path(forecast).read_text(encoding='utf-8'))
    weights = assumptions.get('weight_points')
    ids = {m['id'] for m in data['modules']}
    if not isinstance(weights, dict) or set(weights) != ids:
        raise ForecastError('effort weights must cover every registered module exactly once')
    if any(type(v) is not int or not 1 <= v <= 100 for v in weights.values()):
        raise ForecastError('every effort weight must be a positive integer')
    total = sum(weights.values())
    if total != assumptions.get('total_effort_points'):
        raise ForecastError('effort weight total does not match declared total')
    if assumptions.get('status') != 'provisional_effort_estimate_not_owner_accepted':
        raise ForecastError('forecast may not misrepresent owner approval')
    if data['project_completion_percent'] is not None:
        raise ForecastError('forecast does not assign official project completion')
    confirmed = sum(weights[m['id']] for m in data['modules']
                    if m['state'] == 'verified' and m['owner_accepted'])
    percent = round(100 * confirmed / total, 2)
    gates = assumptions.get('windows_gate_percentages')
    if gates != list(range(10,101,10)):
        raise ForecastError('expected acceptance gates 10 to 100')
    reached = [g for g in gates if percent >= g]
    next_gate = next((g for g in gates if percent < g), None)
    return {'status':'PROVISIONAL - not project completion certification',
            'effort_points_total': total, 'owner_accepted_verified_points': confirmed,
            'confirmed_weighted_floor_percent': percent,
            'gates_reached': reached, 'next_gate_percent': next_gate,
            'official_module_registry_completion_percent': None}


if __name__ == '__main__':
    print(json.dumps(calculate(),indent=2))
