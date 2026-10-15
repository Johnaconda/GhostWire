# GW-A03-HF01-MACOS-RECOVERY

Date: 2026-10-14. Version: 0.3.1. Status: recovery candidate; new Windows/macOS
acceptance pending. No A04 implementation or remote write is included.

## Authority and scope

The owner supplied A03 Windows acceptance, published main, and requested
continuation. Read-only inspection of that push found real macOS regressions,
which this bounded recovery addresses before identity work. Authoritative input:
GHOSTWIRE-GW-A03-CRYPTOGRAPHIC-PROVIDER-source.zip, SHA-256
`b3d3d39e1a369185a1309e41213568f88ef3b212c80898bb577334b0616f3c64`.
CRC, safe paths and all 158 source-manifest entries passed before fresh extraction.
Main `95c9128ea5438663b14038c66f4986dfb4b90279` matches every source byte.
The generated manifest is correctly absent from Git. A final read-only branch
check found the same commit. No source is reconstructed or owner change removed.

[A03 host acceptance](GW-A03-HOST-ACCEPTANCE.md) records the owner's actual
Windows 10/VS 2026 Debug/Release 37/37 each under original TEMP/TMP, four binary
version checks and CLI help. [Hosted evidence](GW-A03-HOSTED-ACCEPTANCE.json)
records all nine jobs at that exact commit: Linux and Windows pass, macOS fails.
Historical checkpoint reports/prompts remain byte-identical to the input.

## Changes and security reasoning

Darwin can return EPERM when group signaling finds only zombies. A02's monitor
intentionally keeps the exited leader unreaped while cleaning its group, so
normal completion could be reported as process_control_failed. Apple's source
confirms the errno mechanism; the original CI assertions did not print errno.
The eleven failed platform cases and successful descendant case fit this
diagnosis. Native execution of the repair remains necessary.

The adapter now distinguishes that narrow completed state from a live denied
target. Under its existing ownership mutex it confirms the exact leader's exit
without reaping, then requests up to 64 native records for its owned group.
Every returned member must be a zombie and the leader must be present. Live,
missing, malformed, incomplete, oversized or failed results remain failures.
Linux retains its original signal/error behavior. No generic EPERM suppression,
post-reap group signaling, foreign-PID control or detached work is introduced.
See [ADR-007](../decisions/ADR-007-MACOS-EXIT-CLEANUP.md).

Separately, the crypto lifecycle fixture now canonicalizes its trusted native
temporary base, following the existing core/platform fixtures. This handles
macOS /var -> /private/var without weakening production symlink rejection.
The unchanged A03 binary fails the same lifecycle assertion on Linux when TMPDIR
is a symlink. The fixed test passes that layout and creates only empty application
directories. Production UTF-8/config/path behavior is unchanged.

Added tests exercise repeated completed children with capture/discard, exact exit
codes, failure snapshots, repeated stop/termination/wait and native reaping; a new
Unix CTest runs the real crypto lifecycle case under a symlink TMPDIR and verifies
its selector actually executed. All prior tests remain. Native handles stay in
process.cpp; public headers, target dependencies, compiler warnings, dependency
pins and portable/vcpkg presets are unchanged. Version/checkpoint metadata and
its assertions identify this distinct recovery candidate.

No private state, profile, identity, key, process record, socket, telemetry or
cloud service is persisted/introduced. Native query records are temporary and
never logged. Existing error descriptions remain fixed. A11-PRE and AUTH-PRE
are unchanged. Botan sources, primitives, memory/nonce contracts and fault
isolation are unchanged; passing tests is not an external security audit.

## Actual local validation

Ubuntu 24.04.3 x64, GCC 13.3.0, CMake 4.4.4, Ninja 1.13.2.git.kitware.jobserver-pipe-1,
Python 3.12.14 and clang-format23.1.3. Project warnings remain errors.

| Executed check | Result |
|---|---|
| Input SHA/CRC/safe paths/158 manifest hashes and exact main reconciliation | PASS |
| Unmodified A03 crypto lifecycle under symlink TMPDIR | Reproduced failure: exit 1 at Application::start, 3/4 assertions |
| Clean portable Debug configure/build/CTest | PASS, 38/38 |
| Clean portable Release configure/build/CTest | PASS, 38/38 |
| ASan/UBSan, explicitly detect_leaks=0 and halt_on_error=1 | PASS, 38/38 after relinking the generated core test binary |
| Default sanitizer crypto runner including LSan | 20 cases/1449 assertions pass, then LSan fatal ptrace/runtime error; exit 1, NOT a passing leak check |
| clang-format, module boundaries, JSON/Python syntax and local Markdown links | PASS |
| Fresh packaged source Release configure/build/CTest | PASS, 38/38; final compilation/build/test inputs byte-identical |
| Final ZIP hashes/CRC/membership/history and source-only contents | PASS; 166 source files plus generated manifest |

Full successful Linux unit runner counts (unique cases, excluding the deliberate
additional lifecycle re-run in crypto-symlink-temp):

| Runner | Cases | Assertions |
|---|---:|---:|
| core-unit | 67 | 640 |
| platform-unit | 19 | 279 |
| crypto-unit | 20 | 1449 |
| crypto-faults | 7 | 67 |
| crypto-system-entropy-failure | 2 | 22 |
| Total | 115 | 2457 |

The full runners have zero skips. The extra integration entry intentionally
selects one lifecycle case; the other 19 crypto cases still run in crypto-unit.
Windows expected registrations remain 37; macOS increases to 37; Linux increases
to 38. Expected counts are not native outcomes.

Commands actually used from the extracted recovery project, with local tools on PATH:

```bash
cmake --preset portable-debug --fresh
cmake --build --preset portable-debug -j1
ctest --preset portable-debug --verbose
cmake --preset portable-release --fresh
cmake --build --preset portable-release -j1
ctest --preset portable-release --verbose
cmake --preset portable-sanitize --fresh
cmake --build --preset portable-sanitize -j1
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 ctest --preset portable-sanitize --verbose
# Separate process with ASAN_OPTIONS/LSAN_OPTIONS unset:
out/build/portable-sanitize/bin/ghostwire-crypto-tests --no-colors
python3 tools/format.py --check
python3 tools/check_boundaries.py
python3 tools/packaging/package_source.py --output ../../a03-hf01-validation/staged-source.zip
```

The first complete sanitizer run passed 37/38; core-unit could not execute because
the build environment produced its executable with mode 0644. This matches the
previously recorded executor artifact issue. Only that generated binary was
removed and relinked from the same source; no chmod or source workaround was used.
The final relinked sanitizer suite passed 38/38. The default LSan process separately exited 1 after reporting its fatal runtime
limitation; leak suppression is confined to the explicitly labeled test run.
No preset or hosted workflow disables leaks.

## Limits and review requirements

The new Darwin branch has source review but no native compilation/runtime result
in this environment. Its candidate status must remain until Intel and ARM64
Debug/Release pass. Windows retesting is also required for this new archive;
accepted A03 Windows logs are not relabeled as HF01. No hosted recovery run has
been triggered. The owner controls all publication.

The bounded Darwin fallback may conservatively fail for an over-capacity
zombie-only group or unavailable/denied sysctl; it never turns an ambiguous native
result into successful cleanup. Native kernel calls and trusted-child group
limitations remain as in A02. This does not provide hostile-process containment.

Owner-local Windows vcpkg/doctest, local full LSan and clang-tidy remain unresolved.
A03's hosted Windows vcpkg and default sanitizer successes are separately scoped
positive evidence. HF01 does not repeat unchanged upstream Botan's 485,213 checks,
regeneration/crypto audit, TSan or a vcpkg build; earlier results remain historical.
No independent cryptographic audit is claimed. The minor legacy-console help
banner rendering issue is recorded for a later output pass; UTF-8 inputs stay strict.

## Delivery and continuation

GHOSTWIRE-GW-A03-HF01-MACOS-RECOVERY-source.zip is cumulative A00 through this
recovery, with a generated SOURCE_MANIFEST and external SHA-256 sidecar. It excludes
builds, caches, executables, credentials and temporary logs. The signed Botan
tar.xz/asc are preserved source dependencies. Only documentation, validation excerpts and the generated inventory changed after
the successful extracted build; every compilation/build/test input was compared
byte-for-byte before the final repack. The complete delta is in
[GW-A03-HF01-CHANGED-FILES.md](GW-A03-HF01-CHANGED-FILES.md).

[GW-A03-HF01-HANDOVER.md](GW-A03-HF01-HANDOVER.md) supplies native acceptance
commands and the updated A04 continuation. The original A04 prompt is retained.
Identity design review is the next pass after recovery review; no root/device,
alias, recovery or per-server credential design is silently treated as approved.
No A04 implementation or persistence is included. Stop after this recovery.
