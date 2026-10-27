# A05-P01 exact changed-file inventory

Baseline: accepted A04-P03 ZIP, 200 source files, SHA-256
`aa52cb195c16a765b2118d62c7cb4b2a1e0a97a89831c9cd229390ea674e0ff4`.

All unlisted previously tracked source bytes remain unchanged. No existing source files deleted.

**11 added, 13 modified, 187 unchanged, 0 deleted** relative to P03.

## Added

- `docs/architecture/PROTOCOL_SCHEMA.md`
- `docs/checkpoints/GW-A05-P01-VALIDATION.json`
- `docs/checkpoints/GW-A05-P01-WINDOWS-ACCEPTANCE.md`
- `docs/checkpoints/GW-A05-P01.md`
- `docs/checkpoints/GW-A05-P02-PROMPT.md`
- `docs/decisions/ADR-009-CANONICAL-PROTOCOL-SERIALIZATION.md`
- `docs/security/PROTOCOL_TEST_PLAN.md`
- `schemas/ghostwire-wire-v1.cddl`
- `tests/fixtures/protocol/v1-design-vectors.json`
- `tools/protocol/check_design_vectors.py`
- `docs/checkpoints/GW-A05-P01-CHANGED-FILES.md`

## Modified

- `AGENTS.md`
- `ARCHITECTURE.md`
- `README.md`
- `ROADMAP.md`
- `TODO.md`
- `docs/CHANGELOG.md`
- `docs/COMPLETED.md`
- `docs/architecture/DEPENDENCY_GRAPH.md`
- `docs/architecture/MODULE_MAP.md`
- `docs/development/VALIDATION.md`
- `docs/security/SECURITY_INVARIANTS.md`
- `docs/security/THREAT_MODEL.md`
- `tools/packaging/package_source.py`

## Scope

Changes are documentation, machine-readable design fixtures, a doc-only
consistency checker and updated deterministic ZIP packaging metadata.
Production C++ sources/headers, all CMake/CTest definitions, dependency
pins, runtime version and existing executable behavior are unchanged.
The regenerated `GhostWire/SOURCE_MANIFEST.json` is source-package metadata,
not counted among source file additions. Existing historical evidence preserved.
