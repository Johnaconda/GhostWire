#!/usr/bin/env python3
"""Read-only, bounded task routing. Reads indexes/code in Python; prints locators only.

No builds, subprocesses, network, model calls or repository writes. Routing narrows
the initial read set; it does not prove that other callers are unaffected.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import sys

# Keep the CLI read-only even when its interpreter normally caches imports.
sys.dont_write_bytecode = True
from module_registry import load_and_validate

ROOT = Path(__file__).resolve().parents[2]
INDEX = 'docs/development/TASK_CONTEXTS.json'
REGISTRY = 'docs/development/MODULE_REGISTRY.json'
MAX_PACKET_BYTES = 8192
MAX_REFERENCES = 24
MAX_LOCATOR_FILE_BYTES = 1024 * 1024


class ContextError(ValueError):
    pass


def safe_path(root, raw):
    """Require an existing in-tree path, with no symlink in any component."""
    if (not isinstance(raw, str) or not raw or '\\' in raw or ':' in raw
            or raw.startswith('/') or any(p in ('', '.', '..') for p in raw.split('/'))):
        raise ContextError('Route contains an unsafe path')
    current = Path(root)
    for part in PurePosixPath(raw).parts:
        current = current / part
        if current.is_symlink():
            raise ContextError('Route contains a symlink')
    if not current.exists() or not current.resolve().is_relative_to(Path(root).resolve()):
        raise ContextError('Route path is missing or outside the source')
    return current


def load_index(root, registry):
    data = json.loads(safe_path(root, INDEX).read_text(encoding='utf-8'))
    if data.get('schema_version') != 1 or not isinstance(data.get('areas'), dict):
        raise ContextError('Unsupported context index')
    modules = {m['id'] for m in registry['modules']}
    tasks = {t['id'] for t in registry['tasks']}
    if (not set(data.get('module_areas', {})) <= modules
            or not set(data.get('tasks', {})) <= tasks):
        raise ContextError('Context index references an unknown module or task')
    for route in [*data['areas'].values(), *data.get('tasks', {}).values()]:
        if not isinstance(route, dict):
            raise ContextError('Invalid context route')
        if 'area' in route and route['area'] not in data['areas']:
            raise ContextError('Context route has an unknown area')
    if any(a not in data['areas'] for a in data.get('module_areas', {}).values()):
        raise ContextError('Context module route has an unknown area')
    return data


def locate(root, ref):
    """Return one exact declaration line or a Markdown section range; no bodies."""
    if not isinstance(ref, dict) or not isinstance(ref.get('path'), str):
        raise ContextError('Invalid file reference')
    path = safe_path(root, ref['path'])
    if not path.is_file():
        raise ContextError('Read reference must be a file')
    result = {'path': ref['path']}
    if 'find' not in ref:
        return result
    needle = ref['find']
    if not isinstance(needle, str) or not needle or path.stat().st_size > MAX_LOCATOR_FILE_BYTES:
        raise ContextError('Invalid or oversized locator input')
    lines = path.read_text(encoding='utf-8').splitlines()
    found = [i for i, line in enumerate(lines) if needle in line]
    if len(found) != 1:
        raise ContextError('Locator missing or ambiguous; refresh the selected route')
    start = found[0]
    result['line'] = start + 1
    result['find'] = needle
    heading = re.match(r'^(#{1,6}) ', lines[start])
    if heading:
        end = len(lines)
        for i in range(start + 1, len(lines)):
            other = re.match(r'^(#{1,6}) ', lines[i])
            if other and len(other[1]) <= len(heading[1]):
                end = i
                break
        result['through_line'] = end
    return result


def verify_source(root, registry):
    """Check only the manifest's listed files; external ZIP checksum is separate."""
    manifest = json.loads(safe_path(root, 'SOURCE_MANIFEST.json').read_text(encoding='utf-8'))
    if any(manifest.get(k) != registry.get(k) for k in
           ('checkpoint', 'source_revision', 'patch_author_tag')):
        raise ContextError('Manifest identity differs from working metadata; repackage after edits')
    inventory = manifest.get('files')
    if (manifest.get('algorithm') != 'SHA-256' or not isinstance(inventory, dict)
            or not inventory or REGISTRY not in inventory or INDEX not in inventory):
        raise ContextError('Source inventory is missing or invalid')
    for raw, expected in inventory.items():
        if not isinstance(expected, str) or re.fullmatch('[0-9a-f]{64}', expected) is None:
            raise ContextError('Invalid inventory digest')
        path = safe_path(root, raw)
        if not path.is_file():
            raise ContextError('Inventory entry must be a file')
        digest = hashlib.sha256()
        with path.open('rb') as stream:
            for block in iter(lambda: stream.read(128 * 1024), b''):
                digest.update(block)
        if digest.hexdigest() != expected:
            raise ContextError('Source inventory mismatch; modified tree or invalid extraction')
    return {'listed_files_verified': len(inventory),
            'limit': 'Not an external ZIP checksum, signature, untracked-file check or security audit'}


def build_packet(root=ROOT, task_id=None, area=None, verify=False):
    registry = load_and_validate(safe_path(root, REGISTRY), root=root)
    index = load_index(root, registry)
    tasks = {t['id']: t for t in registry['tasks']}
    modules = {m['id']: m for m in registry['modules']}
    packet = {'source': {k: registry.get(k) for k in
                        ('checkpoint', 'source_revision', 'runtime_version', 'patch_author_tag')},
              'rules': index['rules'], 'gates': index['gates']}
    if verify:
        packet['source_verification'] = verify_source(root, registry)
    if not task_id and not area:
        packet['available_areas'] = sorted(index['areas'])
        packet['open_tasks'] = [{k: t[k] for k in ('id', 'state', 'description', 'blocker') if k in t}
                                for t in registry['tasks'] if t['state'] != 'completed']
        packet['next'] = 'Select --task TASK_ID, or --area AREA for a new issue. Completed history is omitted.'
        return packet
    task = None
    override = {}
    if task_id:
        if task_id not in tasks:
            raise ContextError('Unknown task; use --list or an explicit --area for a new issue')
        task = tasks[task_id]
        override = index.get('tasks', {}).get(task_id, {})
        area = override.get('area') or index['module_areas'].get(task['module'])
    if area not in index['areas']:
        raise ContextError('No route for this selection; register an explicit area route')
    route = index['areas'][area]
    packet['area'] = area
    if task:
        packet['task'] = {k: task[k] for k in
                          ('id', 'state', 'owner', 'description', 'blocker', 'acceptance') if k in task}
        module = modules[task['module']]
        packet['module'] = {'id': module['id'], 'state': module['state'],
                            'prerequisites': [{'id': d, 'state': modules[d]['state']}
                                              for d in module['depends_on']]}
        packet['task_prerequisites'] = [{'id': d, 'state': tasks[d]['state']} for d in task['depends_on']]
        packet['mode'] = {'ready': 'implementation_candidate; confirm owner scope',
                          'completed': 'review_only; do not reimplement or rerun without new evidence',
                          'blocked': 'gate_review_only; gated implementation is not authorized',
                          'deferred': 'gate_review_only; gated implementation is not authorized'}[task['state']]
    else:
        packet['mode'] = 'issue_triage; register a bounded task before claiming readiness'
    refs = [*route['read'], *override.get('read', [])]
    # Preserve intentional different sections in the same file, remove exact duplicates.
    refs = list({json.dumps(ref, sort_keys=True): ref for ref in refs}.values())
    if len(refs) > MAX_REFERENCES:
        raise ContextError('Too many file references; split the task route')
    packet['read'] = [locate(root, ref) for ref in refs]
    can_implement = task is None or task['state'] == 'ready'
    writes = override.get('write', route['write']) if can_implement else []
    for path in writes:
        safe_path(root, path)
    packet['initial_write_scope'] = writes
    packet['checks_to_choose_after_changes'] = override.get('checks', route['checks']) if can_implement else []
    packet['expand_scope_when'] = [*route['expand'], *override.get('expand', [])]
    return packet


def render(packet, as_json=False):
    if as_json:
        result = json.dumps(packet, ensure_ascii=False, indent=2) + '\n'
    else:
        source = packet['source']
        lines = [f"GhostWire {source['checkpoint']} r{source['source_revision']} | runtime {source['runtime_version']}",
                 f"Patch author tag: {source['patch_author_tag']}"]
        for key, value in packet.items():
            if key == 'source':
                continue
            lines.append('\n' + key.replace('_', ' ').upper())
            if key == 'read':
                for ref in value:
                    location = ref['path']
                    if 'line' in ref:
                        location += ':' + str(ref['line'])
                        if 'through_line' in ref:
                            location += '-' + str(ref['through_line'])
                    lines.append('  ' + location + (' | ' + ref['find'] if 'find' in ref else ''))
            elif isinstance(value, list):
                lines.extend('  - ' + (v if isinstance(v, str) else json.dumps(v, ensure_ascii=False)) for v in value)
            else:
                lines.append(value if isinstance(value, str) else json.dumps(value, ensure_ascii=False))
        result = '\n'.join(lines) + '\n'
    if len(result.encode('utf-8')) > MAX_PACKET_BYTES:
        raise ContextError('Packet exceeds 8192 UTF-8 bytes; narrow the route, do not truncate rules')
    return result


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    choice = parser.add_mutually_exclusive_group()
    choice.add_argument('--list', action='store_true', help='open tasks and available areas (default)')
    choice.add_argument('--task', help='exact registered task ID')
    choice.add_argument('--area', help='explicit area for a new issue')
    parser.add_argument('--json', action='store_true')
    parser.add_argument('--verify-source', action='store_true', help='hash the clean embedded inventory; summary only')
    args = parser.parse_args(argv)
    try:
        packet = build_packet(task_id=args.task, area=args.area, verify=args.verify_source)
        output = render(packet, args.json)
    except ContextError as error:
        # ContextError messages are fixed strings, never source/input contents.
        print(f'TASK CONTEXT FAIL: {error}; no packet emitted.', file=sys.stderr)
        return 1
    except (OSError, ValueError, KeyError, TypeError):
        # Avoid accidentally printing file contents, private paths or exception text.
        print('TASK CONTEXT FAIL: invalid selection, route or source inventory. Check --help and the selected index; no packet emitted.', file=sys.stderr)
        return 1
    print(output, end='')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
