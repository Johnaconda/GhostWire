# A03-HF01 changed-file inventory

Baseline: delivered A03 archive SHA-256 `b3d3d39e1a369185a1309e41213568f88ef3b212c80898bb577334b0616f3c64`, exactly matching main `95c9128ea5438663b14038c66f4986dfb4b90279`.

21 modified, 8 added, 137 unchanged, 0 removed. SOURCE_MANIFEST.json is regenerated separately and is not counted as a source file.

Every previously delivered checkpoint report/prompt and third-party file is byte-identical. No application header/API, dependency pin, preset or workflow changed.

## Modified

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
- `docs/architecture/PLATFORM_ABSTRACTIONS.md`
- `docs/development/VALIDATION.md`
- `docs/security/SECURITY_INVARIANTS.md`
- `src/platform/process.cpp`
- `tests/integration/cli_contract.cmake`
- `tests/unit/core_tests.cpp`
- `tests/unit/crypto_tests.cpp`
- `tests/unit/platform_tests.cpp`
- `tools/packaging/package_source.py`
- `vcpkg.json`

## Added

- `docs/checkpoints/GW-A03-HF01-CHANGED-FILES.md`
- `docs/checkpoints/GW-A03-HF01-HANDOVER.md`
- `docs/checkpoints/GW-A03-HF01-VALIDATION-EXCERPTS.txt`
- `docs/checkpoints/GW-A03-HF01.md`
- `docs/checkpoints/GW-A03-HOST-ACCEPTANCE.md`
- `docs/checkpoints/GW-A03-HOSTED-ACCEPTANCE.json`
- `docs/decisions/ADR-007-MACOS-EXIT-CLEANUP.md`
- `tests/integration/crypto_temp_path.cmake`

## Functional delta

- Production: bounded Darwin zombie-only group cleanup in process.cpp; version/checkpoint metadata.
- Tests: canonical trusted crypto TEMP base, Unix symlink-TEMP CTest, repeated completed-child cleanup and updated checkpoint assertions.
- Records: owner/published CI acceptance, recovery design/report, current TODO/roadmap, native checklist and A04 handover.
- Packaging: distinct cumulative recovery name/version and regenerated source manifest.
