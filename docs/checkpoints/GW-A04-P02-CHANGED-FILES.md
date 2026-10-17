# P02 changed-file inventory

Compared byte-for-byte with the accepted P01 cumulative archive (SHA-256
`38fbfceccd352bc9b95990dc2b605c3e7e4212b21e3057b72bc34e7b484d6fc3`).

**20 added, 26 modified, 148 unchanged, 0 deleted** source files.
Current package: 194 source files plus regenerated `SOURCE_MANIFEST.json`.
The generated manifest is additional packaging metadata, not counted as source.
The archive SHA-256 is delivered externally to avoid a self-referential digest.

## Added

- `apps/cli/identity_session.cpp`
- `apps/cli/identity_session.hpp`
- `docs/checkpoints/GW-A04-P02-CHANGED-FILES.md`
- `docs/checkpoints/GW-A04-P02-VALIDATION.json`
- `docs/checkpoints/GW-A04-P02-WINDOWS-ACCEPTANCE.md`
- `docs/checkpoints/GW-A04-P02.md`
- `docs/checkpoints/GW-A04-P03-PROMPT.md`
- `include/ghostwire/composition/identity_service_adapter.hpp`
- `include/ghostwire/identity/identity_error.hpp`
- `include/ghostwire/identity/identity_id.hpp`
- `include/ghostwire/identity/identity_record.hpp`
- `include/ghostwire/identity/identity_service.hpp`
- `src/composition/identity_service_adapter.cpp`
- `src/identity/identity_record.cpp`
- `src/identity/identity_service.cpp`
- `tests/integration/identity_session.cmake`
- `tests/unit/identity_fault_tests.cpp`
- `tests/unit/identity_lifecycle_tests.cpp`
- `tests/unit/identity_record_tests.cpp`
- `tests/unit/identity_service_tests.cpp`

## Modified

- `AGENTS.md`
- `ARCHITECTURE.md`
- `BUILD.md`
- `CMakeLists.txt`
- `README.md`
- `ROADMAP.md`
- `TODO.md`
- `apps/cli/main.cpp`
- `cmake/build_version.hpp.in`
- `docs/CHANGELOG.md`
- `docs/COMPLETED.md`
- `docs/architecture/DEPENDENCY_GRAPH.md`
- `docs/architecture/FUTURE_INTERFACES.md`
- `docs/architecture/IDENTITY_DESIGN.md`
- `docs/architecture/MODULE_MAP.md`
- `docs/checkpoints/GW-A04-TEST-PLAN.md`
- `docs/decisions/ADR-008-IDENTITY-AND-DEVICE-TRUST.md`
- `docs/development/VALIDATION.md`
- `docs/security/BOUNDARY_MAP.md`
- `docs/security/SECURITY_INVARIANTS.md`
- `docs/security/THREAT_MODEL.md`
- `src/core/bootstrap.cpp`
- `tests/integration/cli_contract.cmake`
- `tests/unit/core_tests.cpp`
- `tools/check_boundaries.py`
- `tools/packaging/package_source.py`

## Preserved baseline

All baseline source files remain present. The master contract, earlier checkpoint
reports/prompts, ADR-001 through ADR-007, vendored dependencies, presets and
Windows toolchain scripts are unchanged. Current architecture/security/status
records and the A04 test-plan addendum explicitly distinguish this implementation
from P01 design acceptance and earlier native validation. Build products, caches,
private data, credentials and temporary validation logs are excluded.
