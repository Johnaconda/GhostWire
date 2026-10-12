#!/usr/bin/env python3
"""Source dependency check. Extend explicitly when adding modules."""
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
allowed_core = {'core/error.hpp', 'core/result.hpp', 'core/resource_limits.hpp'}
errors = []
for folder in ['src/platform', 'include/ghostwire/platform']:
    for path in (root / folder).rglob('*'):
        if path.suffix not in {'.cpp', '.hpp'}:
            continue
        for name in re.findall(r'#include\s+"ghostwire/([^"]+)"', path.read_text()):
            if not name.startswith('platform/') and name not in allowed_core:
                errors.append(f'{path.relative_to(root)} includes forbidden {name}')
for folder in ['src/core', 'src/platform', 'include/ghostwire/core', 'include/ghostwire/platform']:
    for path in (root / folder).rglob('*'):
        if path.suffix not in {'.cpp', '.hpp'}:
            continue
        if re.search(r'#include\s*[<"](?:SDL|Qt|wx/|imgui|ghostwire/ui)', path.read_text()):
            errors.append(f'{path.relative_to(root)} imports a desktop dependency')
for error in errors:
    print(error, file=sys.stderr)
print('Boundary check: ' + ('FAIL' if errors else 'PASS'))
raise SystemExit(bool(errors))
