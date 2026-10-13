# A02 changed-file inventory

The ZIP is the entire cumulative project, not a patch. This inventory compares
actual bytes with the accepted HF01 archive and supplied GitHub commit
`713750550d6fa7bbde7200c494554d1c4f9e008f`. See GW-A02-INPUT-EVIDENCE.md for
provenance. All historical checkpoint reports and the master contract are retained.
SOURCE_MANIFEST.json is regenerated in the ZIP and excludes its own hash.
This inventory lists itself; its final hash appears in that source manifest.

## Relative to Accepted HF01 archive

Added: 19. Modified: 28. Removed: 0.

### Added

- `docs/architecture/PLATFORM_ABSTRACTIONS.md`
- `docs/checkpoints/GW-A01-HF01-HOST-ACCEPTANCE.md` — preserved supplied GitHub bytes
- `docs/checkpoints/GW-A01-HF02-CHANGED-FILES.md` — preserved supplied GitHub bytes
- `docs/checkpoints/GW-A01-HF02.md` — preserved supplied GitHub bytes
- `docs/checkpoints/GW-A02-CHANGED-FILES.md`
- `docs/checkpoints/GW-A02-INPUT-CI.json`
- `docs/checkpoints/GW-A02-INPUT-EVIDENCE.md`
- `docs/checkpoints/GW-A02-NATIVE-ACCEPTANCE.md`
- `docs/checkpoints/GW-A02.md`
- `docs/checkpoints/GW-A03-PROMPT.md`
- `docs/decisions/ADR-005-PLATFORM-OWNERSHIP.md`
- `include/ghostwire/platform/process.hpp`
- `include/ghostwire/platform/process_stop.hpp`
- `include/ghostwire/platform/secure_storage.hpp`
- `src/platform/process.cpp`
- `src/platform/process_stop.cpp`
- `src/platform/secure_storage.cpp`
- `tests/fixtures/platform_child.cpp`
- `tests/unit/platform_tests.cpp`

### Modified

- `.github/workflows/macos.yml` — preserved supplied GitHub bytes
- `AGENTS.md`
- `ARCHITECTURE.md`
- `BUILD.md`
- `CMakeLists.txt`
- `README.md`
- `ROADMAP.md`
- `SOURCE_MANIFEST.json`
- `TODO.md`
- `cmake/build_version.hpp.in`
- `docs/CHANGELOG.md`
- `docs/COMPLETED.md`
- `docs/architecture/DEPENDENCY_GRAPH.md`
- `docs/architecture/FUTURE_INTERFACES.md`
- `docs/architecture/MODULE_MAP.md`
- `docs/development/GITHUB_SETUP.md`
- `docs/development/VALIDATION.md`
- `docs/security/BOUNDARY_MAP.md`
- `docs/security/SECURITY_INVARIANTS.md`
- `docs/security/THREAT_MODEL.md`
- `include/ghostwire/core/error.hpp`
- `src/core/bootstrap.cpp`
- `src/core/config.cpp` — preserved supplied GitHub bytes
- `src/core/logger.cpp` — preserved supplied GitHub bytes
- `tests/integration/cli_contract.cmake`
- `tests/unit/core_tests.cpp`
- `tools/packaging/package_source.py`
- `vcpkg.json`

### Removed

None.

## Relative to Supplied GitHub commit

Added: 17. Modified: 24. Removed: 0.

### Added

- `SOURCE_MANIFEST.json`
- `docs/architecture/PLATFORM_ABSTRACTIONS.md`
- `docs/checkpoints/GW-A02-CHANGED-FILES.md`
- `docs/checkpoints/GW-A02-INPUT-CI.json`
- `docs/checkpoints/GW-A02-INPUT-EVIDENCE.md`
- `docs/checkpoints/GW-A02-NATIVE-ACCEPTANCE.md`
- `docs/checkpoints/GW-A02.md`
- `docs/checkpoints/GW-A03-PROMPT.md`
- `docs/decisions/ADR-005-PLATFORM-OWNERSHIP.md`
- `include/ghostwire/platform/process.hpp`
- `include/ghostwire/platform/process_stop.hpp`
- `include/ghostwire/platform/secure_storage.hpp`
- `src/platform/process.cpp`
- `src/platform/process_stop.cpp`
- `src/platform/secure_storage.cpp`
- `tests/fixtures/platform_child.cpp`
- `tests/unit/platform_tests.cpp`

### Modified

- `AGENTS.md`
- `ARCHITECTURE.md`
- `BUILD.md`
- `CMakeLists.txt`
- `README.md`
- `ROADMAP.md`
- `TODO.md`
- `cmake/build_version.hpp.in`
- `docs/CHANGELOG.md`
- `docs/COMPLETED.md`
- `docs/architecture/DEPENDENCY_GRAPH.md`
- `docs/architecture/FUTURE_INTERFACES.md`
- `docs/architecture/MODULE_MAP.md`
- `docs/development/GITHUB_SETUP.md`
- `docs/development/VALIDATION.md`
- `docs/security/BOUNDARY_MAP.md`
- `docs/security/SECURITY_INVARIANTS.md`
- `docs/security/THREAT_MODEL.md`
- `include/ghostwire/core/error.hpp`
- `src/core/bootstrap.cpp`
- `tests/integration/cli_contract.cmake`
- `tests/unit/core_tests.cpp`
- `tools/packaging/package_source.py`
- `vcpkg.json`

### Removed

None.

## Implementation areas

- Platform process/stop/capability contracts and private native RAII implementations.
- Core fixed error reasons and one-shot bootstrap cancellation forwarding.
- Actual compiled child fixture and independent platform unit runner; old tests kept.
- CMake native feature checks/test targets and A02 version/packager metadata.
- Preserved HF02 macOS workflow, byte conversions, owner README and host records.
- Current architecture/security/TODO/roadmap/changelog/validation, ADR-005, source
  provenance, native acceptance and proposed A03 prompt.

No file removed. Dependency pins, build presets, warning options, HF01 Windows
tooling, production path conversion and the future-feature backlog are retained.
Build outputs, binaries, temporary fixtures, credentials and .git are not packaged.
