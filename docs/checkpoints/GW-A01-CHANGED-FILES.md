# GW-A01 changed-file inventory

Compared by SHA-256 against `GHOSTWIRE-GW-A00-FOUNDATION-BOOTSTRAP-source(1).zip`
(`d07d38825ceb2c73a9ddb0eee429d42e8b85ad64439ba08fd7079399021d0025`).

**15 added, 32 modified, 0 deleted, 53 unchanged source files.**
The cumulative package contains 100 source files plus generated `SOURCE_MANIFEST.json`.
The manifest is regenerated for A01 and records every source file hash; it excludes
its own hash. The ZIP checksum is supplied externally to avoid recursion.

## Added

| File | Change |
|---|---|
| `docs/architecture/CORE_LIFECYCLE.md` | Complete API/module ownership, threading, deadlines and resource contracts. |
| `docs/architecture/FUTURE_FEATURES.md` | Unimplemented owner proposals for web, AI, training, media and GPU policy. |
| `docs/checkpoints/GW-A01-CHANGED-FILES.md` | This input-to-delivery inventory. |
| `docs/checkpoints/GW-A01.md` | Input provenance, implementation, actual validation and limits. |
| `docs/checkpoints/GW-A02-PROMPT.md` | Proposed next checkpoint, pending owner acceptance and authorization. |
| `docs/decisions/ADR-004-CORE-LIFECYCLE.md` | Record A01 architecture decisions and C++20 source review. |
| `include/ghostwire/core/application_context.hpp` | Immutable owned snapshots and explicit borrowed facilities. |
| `include/ghostwire/core/clock.hpp` | Monotonic clock abstraction and bounded deadline contract. |
| `include/ghostwire/core/resource_admission.hpp` | Thread-safe admission and move-only lease API. |
| `include/ghostwire/core/service.hpp` | Service identity, hooks and shutdown lifetime contract. |
| `include/ghostwire/core/service_manager.hpp` | Owned graph, deterministic lifecycle and bounded reports. |
| `src/core/clock.cpp` | Deadline validation, saturation and remaining-time arithmetic. |
| `src/core/resource_admission.cpp` | Synchronized slots/bytes, CPU policy, promotion and RAII release. |
| `src/core/service_manager.cpp` | Graph validation, rollback, reverse cleanup, retry and destruction. |
| `tests/unit/lifecycle_tests.cpp` | 39 new lifecycle/admission/concurrency/failure cases. |

## Modified

| File | Change |
|---|---|
| `AGENTS.md` | Current authorized checkpoint and explicit stop/acceptance boundary. |
| `ARCHITECTURE.md` | Implemented ownership, shutdown and resource contracts. |
| `BUILD.md` | Current version behavior, admission/config meanings and host commands. |
| `CMakeLists.txt` | Version 0.1.0; compile new core sources and lifecycle tests. |
| `README.md` | Current checkpoint, implemented capabilities and report links. |
| `ROADMAP.md` | A01 status/evidence; future proposals without sequence changes. |
| `SECURITY.md` | Current development maturity and validation scope. |
| `TODO.md` | A01 completed scope and explicit unresolved native/leak/analysis gates. |
| `cmake/build_version.hpp.in` | Compiled A01 checkpoint identity. |
| `docs/CHANGELOG.md` | Append A01 change record; preserve A00 entry. |
| `docs/COMPLETED.md` | Append implementation and measured validation, not owner acceptance. |
| `docs/architecture/DEPENDENCY_GRAPH.md` | Runtime ownership graph; unchanged target DAG. |
| `docs/architecture/FUTURE_INTERFACES.md` | Distinguish implemented core from future interfaces. |
| `docs/architecture/MODULE_MAP.md` | Map current lifecycle/admission modules and ownership. |
| `docs/development/GITHUB_SETUP.md` | Current archive provenance; commands remain unexecuted remotely. |
| `docs/development/VALIDATION.md` | Expanded coverage and honest sanitizer/native limitations. |
| `docs/security/BOUNDARY_MAP.md` | A01 nonsecret core boundary; no new private authority. |
| `docs/security/SECURITY_INVARIANTS.md` | Map lifecycle, budgets, redaction and ownership evidence. |
| `docs/security/THREAT_MODEL.md` | Trusted services, cooperative cleanup and resource accounting limits. |
| `examples/config/ghostwire.conf` | Explain real admission use without changing schema/keys. |
| `include/ghostwire/core/application.hpp` | Own context/admission/service graph; composition and deadline APIs. |
| `include/ghostwire/core/application_lifecycle.hpp` | Starting/stopping state transitions. |
| `include/ghostwire/core/error.hpp` | Structured reasons and service/dependency IDs; exit codes unchanged. |
| `include/ghostwire/core/resource_limits.hpp` | Value-only resource policy/budget contracts. |
| `src/core/application.cpp` | Managed real paths lifecycle, startup/cleanup integration and logging. |
| `src/core/bootstrap.cpp` | A01 capability/help text; CLI behavior preserved. |
| `tests/integration/cli_contract.cmake` | Update expected checkpoint identity only. |
| `tests/unit/core_tests.cpp` | Update expected checkpoint identity only; A00 tests retained. |
| `tools/check_boundaries.py` | Remove stale A00 label; validation rules unchanged. |
| `tools/packaging/package_source.py` | A01 archive name, manifest checkpoint and deterministic date. |
| `tools/windows/dev-windows.ps1` | Remove stale A00 wording from offline-test diagnostic only. |
| `vcpkg.json` | Project version/description only; baseline/dependency pins unchanged. |

## Preserved baseline records

No source file was deleted. The master contract, A00 checkpoint report, A00
changed-file report and original A01 prompt are byte-for-byte unchanged. All
platform implementations, four composition roots, config/parser/logger/path
implementations, vendored dependency/license bytes, compiler-warning options,
CMake presets and CI definitions are retained. No new dependency was added.
The exact A00 input archive remains the external provenance record.
