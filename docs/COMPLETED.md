# Completed implementation records

Updated: 2026-10-12. These records concern implemented work and the stated evidence,
not user acceptance of the entire checkpoint. Windows/macOS acceptance remains
open in TODO. No future crypto, network or UI feature is complete.

| ID | Implemented work | Validation evidence |
|---|---|---|
| GW-A00-CORE | Four composition roots; reusable C++20 core/platform targets; Result/Error, version, config, paths, logger and lifecycle | Linux Debug/Release builds; 26 unit cases and 28 process contracts per suite |
| GW-A00-CONFIG | Bounded parser, schema 1, duplicate/unknown/numeric/UTF-8 checks, explicit precedence, no-write checking | Unit and per-role invalid/valid/missing config tests |
| GW-A00-LOG | Typed JSON-lines, default/private-key redaction, bounded escaped fields, mutex serialization and output failures | Redaction/injection/bounds/concurrent write/stream failure tests |
| GW-A00-PATHS | OS directory adapter, explicit role roots, Unicode paths, collision/symlink rejection and explicit initialization | Linux resolution/UTF-8/collision/symlink tests; Windows/macOS branches unexecuted |
| GW-A00-LIFECYCLE | RAII cleanup, explicit startup/shutdown, terminal states and standard cancellation source/token | Transition/idempotence/failure/cancellation tests; no long-running daemon |
| GW-A00-BUILD | CMake/Ninja presets, pinned vcpkg manifest, exact vendored doctest fallback | vcpkg Linux Debug/Release; vendored Debug and sanitizer profiles |
| GW-A00-POLICY | Module graph, future header contracts, threat/boundary/invariant maps, ADRs, detailed roadmap/TODO and agent rules | Source/design consistency review; no security certification claimed |
| GW-A00-TOOLS | Windows and POSIX entry points, boundary/format checks, source packager and three OS CI definitions | POSIX builds/tools run; Windows script/native branches and hosted CI unverified |

The A00 records above remain historical. Its complete measured commands and
limits are in [GW-A00.md](checkpoints/GW-A00.md). The owner subsequently authorized
A01 from that exact source; this does not certify unexecuted A00 native gates.

## GW-A01 implementation, 2026-10-12

| ID | Implemented work | Validation evidence |
|---|---|---|
| GW-A01-CONTEXT | Owned immutable config/path snapshots, explicit logger/clock/admission references and standard stop token | Snapshot isolation, application composition and dependency/context lifetime tests |
| GW-A01-GRAPH | Bounded unique ownership, deterministic order, missing/duplicate/cycle rejection and frozen lifecycle graph | Registration permutations, graph limits, no initialization on invalid graph, invalid/reentrant transitions |
| GW-A01-CLEANUP | Partial-start rollback, reverse cleanup, retained dependencies, retry and separate startup/shutdown errors | Controlled failure/throw fixtures, failed service cleanup, reverse destruction, repeated shutdown |
| GW-A01-CANCEL | Explicit stop ownership, one monotonic shutdown deadline, bounded retry and joined work | Fake-clock shared budget/overrun and real jthread cancellation/10 ms timeout/retry; no detached work |
| GW-A01-RESOURCE | Active/queued/declared-byte reservations, CPU policy and RAII backpressure | Capacity/overflow/move/promotion/cancellation tests and four-thread, 4,000-attempt contention case; TSan unit suite |
| GW-A01-INTEGRATION | Managed real filesystem initializer shared by all four executables | A00 process/config/path/logging regressions; 29 CTest checks in Debug/Release and ASan/UBSan with leaks disabled |
| GW-A01-HANDOVER | Module contracts, ADR-004, future backlog, current TODO/roadmap, report and source manifest | Source/package inventory and fresh extracted build evidence in GW-A01.md |

The suite now has 65 unit cases / 589 assertions, plus 28 process contracts.
Native acceptance, full LeakSanitizer and clang-tidy remain open. Implementation,
executed validation and user acceptance are distinct; A02 remains proposed only.

## GW-A01 HF01 recovery, 2026-10-12

The owner subsequently reported A01 Windows portable Debug/Release passing
29/29 CTest and 64/64 unit cases with process-local ASCII TEMP/TMP. The one-case
Linux/Windows difference is the existing POSIX-only symlink test. This accepts
A01 functionality and stated cooperative limits, not the Unicode workaround as
a permanent fix or Windows vcpkg as validated.

| ID | Implemented recovery | Evidence / remaining gate |
|---|---|---|
| HF01-UTF8 | Correct fixture native/UTF-8 boundaries without application validator changes | Linux Debug/Release, 67 cases / 640 assertions; original Windows temp remains owner gate |
| HF01-CLI | Four Unicode argv/config/bootstrap/no-write/negative subprocess tests | All 33 Linux CTest checks pass, including prior lifecycle regressions |
| HF01-TOOLS | Unicode environment transfer, matching CTest, x64 diagnostics and -Fresh | PowerShell grammar check passed; 18 Windows-only checks implemented but not run here |
| HF01-HANDOVER | Report, changed-file inventory, original-TEMP checklist and updated A02 prompt | Cumulative package validation in GW-A01-HF01.md; A02 remains unstarted |
