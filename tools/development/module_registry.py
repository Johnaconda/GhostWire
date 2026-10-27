#!/usr/bin/env python3
"""Validate the scoped GhostWire module/dependency registry; report dependency-ready work.

This tool does not infer project completion percentages from checkpoint numbers.
It has no networking, build execution, privileged actions or GitHub writes.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
REGISTRY = ROOT / 'docs/development/MODULE_REGISTRY.json'
MODULE_STATES = frozenset({'verified', 'partial', 'planned', 'blocked', 'deferred'})
TASK_STATES = frozenset({'completed', 'ready', 'blocked', 'deferred'})


class RegistryError(ValueError):
    pass


def _unique_entries(entries, label):
    if not isinstance(entries, list):
        raise RegistryError(f'{label} must be a list')
    result = {}
    for entry in entries:
        if not isinstance(entry, dict) or not isinstance(entry.get('id'), str) or not entry['id']:
            raise RegistryError(f'{label} entry missing nonempty id')
        if entry['id'] in result:
            raise RegistryError(f'{label} duplicate id: {entry["id"]}')
        result[entry['id']] = entry
    return result


def _acyclic(nodes, label):
    visited, visiting = set(), set()

    def visit(node):
        if node in visiting:
            raise RegistryError(f'{label} dependency cycle at {node}')
        if node in visited:
            return
        visiting.add(node)
        for dependency in nodes[node].get('depends_on', []):
            if dependency not in nodes:
                raise RegistryError(f'{label} {node} unknown dependency {dependency}')
            visit(dependency)
        visiting.remove(node)
        visited.add(node)

    for node in nodes:
        visit(node)


def _evidence_file(root, raw, allowed_roots=None):
    if not isinstance(raw, str):
        return False
    candidate = Path(raw)
    if candidate.is_absolute() or not candidate.parts or '..' in candidate.parts:
        return False
    if allowed_roots is not None and candidate.parts[0] not in allowed_roots:
        return False
    full = Path(root) / candidate
    if full.is_symlink() or not full.is_file():
        return False
    return full.resolve().is_relative_to(Path(root).resolve())


def load_and_validate(path=REGISTRY, root=ROOT):
    data = json.loads(Path(path).read_text(encoding='utf-8'))
    if data.get('schema_version') != 1:
        raise RegistryError('unsupported registry schema_version')
    if data.get('project_completion_percent') is not None:
        raise RegistryError('project completion must stay null until agreed weighted scope/evidence exists')
    modules = _unique_entries(data.get('modules'), 'modules')
    if not modules:
        raise RegistryError('empty module registry')
    tasks = _unique_entries(data.get('tasks'), 'tasks')
    for ident, item in modules.items():
        if item.get('state') not in MODULE_STATES:
            raise RegistryError(f'{ident} has unrecognized module state')
        if not isinstance(item.get('depends_on'), list):
            raise RegistryError(f'{ident} requires depends_on list')
        if item.get('state') == 'verified':
            evidence = item.get('evidence', [])
            if not evidence or not all(_evidence_file(root, e, ('docs', 'tests')) for e in evidence):
                raise RegistryError(f'{ident} verified without local evidence files')
        if item.get('owner_accepted') and item.get('state') != 'verified':
            raise RegistryError(f'{ident} owner acceptance on unverified module')
    _acyclic(modules, 'modules')
    for ident, task in tasks.items():
        if task.get('module') not in modules:
            raise RegistryError(f'{ident} references unknown module')
        if task.get('state') not in TASK_STATES:
            raise RegistryError(f'{ident} has unrecognized task state')
        if not isinstance(task.get('depends_on'), list):
            raise RegistryError(f'{ident} requires depends_on list')
        if task['state'] == 'blocked' and not task.get('blocker'):
            raise RegistryError(f'{ident} blocked without explanation')
        if task['state'] == 'completed':
            confidence = task.get('confidence')
            if not isinstance(confidence, (float, int)) or isinstance(confidence, bool) or confidence <= 0.80 or confidence > 1:
                raise RegistryError(f'{ident} does not exceed 0.80 confidence threshold')
            evidence = task.get('evidence', [])
            if not evidence or not all(_evidence_file(root, e) for e in evidence):
                raise RegistryError(f'{ident} completed without evidence')
        if task['state'] == 'ready':
            assigned = modules[task['module']]
            if assigned['state'] in ('deferred', 'blocked'):
                raise RegistryError(f'{ident} ready under deferred/blocked module')
            if any(modules[d]['state'] != 'verified' for d in assigned['depends_on']):
                raise RegistryError(f'{ident} ready before module prerequisites verified')
            pending = [dep for dep in task['depends_on'] if tasks.get(dep, {}).get('state') != 'completed']
            if pending:
                raise RegistryError(f'{ident} claimed ready before tasks {pending} completed')
    _acyclic(tasks, 'tasks')
    return data


def summary(data):
    modules = data['modules']
    tasks = data['tasks']
    return {
        'checkpoint': data['checkpoint'],
        'project_completion_percent': None,
        'note': 'No invented completion percentage; 10% owner gates require agreed weighted functional scope.',
        'module_counts': {key: sum(m['state'] == key for m in modules) for key in sorted(MODULE_STATES)},
        'completed_tasks': sum(t['state'] == 'completed' for t in tasks),
        'ready_tasks': [t['id'] for t in tasks if t['state'] == 'ready'],
        'blocked_tasks': [t['id'] for t in tasks if t['state'] == 'blocked'],
    }


def main(argv=None):
    p = argparse.ArgumentParser()
    p.add_argument('--registry', type=Path, default=REGISTRY)
    p.add_argument('--json', action='store_true', help='machine readable status')
    args = p.parse_args(argv)
    try:
        data = load_and_validate(args.registry)
    except (RegistryError, OSError, ValueError) as e:
        print(f'MODULE REGISTRY FAIL: {e}', file=sys.stderr)
        return 1
    result = summary(data)
    if args.json:
        print(json.dumps(result, indent=2))
    else:
        print(f'MODULE REGISTRY PASS: {len(data["modules"])} modules, '
              f'{len(data["tasks"])} tasks; ready={len(result["ready_tasks"])}; '
              f'blocked={len(result["blocked_tasks"])}; project % unassigned')
        for ident in result['ready_tasks']:
            print(f'  NEXT {ident}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
