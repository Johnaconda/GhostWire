# P03 exact changed-file inventory

Baseline: accepted P02 cumulative ZIP SHA-256
`e755c9b994541dbf66e061d799b31621b2f91034cda7741d1473699426783b59`.

**6 added, 23 modified, 171 unchanged, 0 deleted** source files.
Cumulative package: **200 source files**, plus regenerated `SOURCE_MANIFEST.json`.
The generated manifest is packaging metadata, outside these source counts.

## Added

- `docs/checkpoints/GW-A04-P02-HOST-ACCEPTANCE.md`
- `docs/checkpoints/GW-A04-P03-CHANGED-FILES.md`
- `docs/checkpoints/GW-A04-P03-VALIDATION.json`
- `docs/checkpoints/GW-A04-P03-WINDOWS-ACCEPTANCE.md`
- `docs/checkpoints/GW-A04-P03.md`
- `docs/checkpoints/GW-A05-PROMPT.md`

## Modified

- `AGENTS.md`
- `ARCHITECTURE.md`
- `BUILD.md`
- `CMakeLists.txt`
- `README.md`
- `ROADMAP.md`
- `TODO.md`
- `apps/cli/identity_session.cpp`
- `cmake/build_version.hpp.in`
- `docs/CHANGELOG.md`
- `docs/COMPLETED.md`
- `docs/architecture/DEPENDENCY_GRAPH.md`
- `docs/architecture/IDENTITY_DESIGN.md`
- `docs/architecture/MODULE_MAP.md`
- `docs/decisions/ADR-008-IDENTITY-AND-DEVICE-TRUST.md`
- `docs/development/VALIDATION.md`
- `docs/security/SECURITY_INVARIANTS.md`
- `docs/security/THREAT_MODEL.md`
- `src/identity/identity_service.cpp`
- `tests/integration/cli_contract.cmake`
- `tests/unit/core_tests.cpp`
- `tests/unit/identity_service_tests.cpp`
- `tools/packaging/package_source.py`

## Scope and preservation

Behavioral repair: membership deletion invalidates matching self-trust in the
owning profile. A single meaningful regression adds 58 assertions. Other runtime
source changes identify version 0.4.1/P03 and update the interactive banner;
no new command/API, dependency, network capability or persistent format.

All prior checkpoint reports/prompts, master contract, third-party sources,
CI workflows, presets, vcpkg configuration and Windows scripts are byte-identical
to P02. Current documentation adds native evidence and explicit hardening scope.
No build binaries/caches, temporary logs, credentials or private state included.
