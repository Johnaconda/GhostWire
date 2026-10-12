# GW-A01-HF01 changed-file inventory

Compared by SHA-256 against
`GHOSTWIRE-GW-A01-CORE-LIFECYCLE-SERVICES-source(1).zip`
(`0c8ded6652ec0d799cfcfde0789160e3ed5b821ad5f797c1562bae12088aed5d`).

**6 added, 19 modified, 0 deleted, 81 unchanged source files.**
The cumulative package contains 106 source files plus generated
`SOURCE_MANIFEST.json`. The manifest is regenerated for HF01 and records every
source file hash, excluding its own hash. The ZIP checksum is supplied externally.

## Added

| File | Change |
|---|---|
| `docs/checkpoints/GW-A01-HF01-CHANGED-FILES.md` | This exact input-to-delivery inventory. |
| `docs/checkpoints/GW-A01-HF01-WINDOWS-ACCEPTANCE.md` | Owner-reported A01 evidence and original-TEMP HF01 host checklist. |
| `docs/checkpoints/GW-A01-HF01.md` | Provenance, diagnosis, implementation, actual validation and open gates. |
| `tests/integration/unicode_paths.cmake` | Real Unicode argv/config/bootstrap/negative tests for every executable. |
| `tests/windows/tooling-tests.ps1` | Windows-native tool/environment regression checks, without Pester. |
| `tools/windows/tooling.ps1` | Testable tool discovery, Unicode environment transport and architecture diagnostics. |

## Modified

| File | Change |
|---|---|
| `AGENTS.md` | HF01 authorization, host acceptance boundary and explicit stop before A02. |
| `ARCHITECTURE.md` | Existing native/UTF-8 boundary clarified; runtime DAG and ownership unchanged. |
| `BUILD.md` | Ordinary PowerShell discovery, portable acceptance, -Fresh, architecture diagnosis and unresolved Windows vcpkg. |
| `CMakeLists.txt` | Version 0.1.1; register four Unicode subprocess tests and Windows-only tooling checks. |
| `README.md` | Current checkpoint, acceptance commands and recovery-report links. |
| `ROADMAP.md` | Record conditional A01 Windows evidence and pending HF01 acceptance; sequence unchanged. |
| `TODO.md` | Recovery implementation/evidence and original-TEMP/vcpkg/native gates. |
| `cmake/build_version.hpp.in` | Compiled HF01 checkpoint identity. |
| `docs/CHANGELOG.md` | Append 0.1.1 recovery record; preserve historical entries. |
| `docs/COMPLETED.md` | HF01 implementation distinguished from Windows acceptance. |
| `docs/checkpoints/GW-A02-PROMPT.md` | Continue from HF01 only after host acceptance and explicit authorization; carry open gates. |
| `docs/development/VALIDATION.md` | New encoding/tooling coverage, actual counts and platform/sanitizer limits. |
| `docs/security/SECURITY_INVARIANTS.md` | Add strict UTF-8 regression evidence; no invariant weakened. |
| `include/ghostwire/platform/filesystem.hpp` | Comment clarifying native filesystem paths versus UTF-8 text; no API change. |
| `tests/integration/cli_contract.cmake` | Expected checkpoint identity only. |
| `tests/unit/core_tests.cpp` | Six encoding fixes, native path I/O, Unicode temporary fixtures, strict UTF-8 and Windows round-trip regressions. |
| `tools/packaging/package_source.py` | HF01 archive name and manifest checkpoint. |
| `tools/windows/dev-windows.ps1` | Compose discovered tools, preserve TEMP/TMP, diagnose x64 mismatches, use explicit tool paths and -Fresh. |
| `vcpkg.json` | Project version only; dependency versions and registry baseline unchanged. |

## Preserved baseline

No source file was deleted. All `src/`, `apps/` and vendored dependency bytes are
unchanged. All lifecycle tests, compiler-warning settings, build presets, CI
definitions and dependency registry configuration are retained. The platform
header edit is documentation only. The master contract, A00 checkpoint/inventory,
A01 checkpoint/inventory and original A01 prompt remain byte-for-byte unchanged.
No new runtime or test-library dependency was added. The updated Windows tooling
uses the PowerShell/.NET and cmd.exe environment already available on Windows.
