# Validation policy

The current measured results are in docs/checkpoints/GW-A01-HF01.md; GW-A00.md
and GW-A01.md are preserved historical evidence. This file defines
how to reproduce and interpret them, not additional passed tests.

CMake warnings-as-errors applies to project code; vendored doctest is a SYSTEM
include. HF01 CTest registers one doctest executable and eight real subprocess
contracts for each of four roles, plus a Windows-only PowerShell tooling test.
That is 33 Linux/macOS registrations and 34 Windows registrations. Unit cases cover version/result, bounded
configuration, paths, UTF-8, lifecycle/cancellation, log redaction/concurrency,
CLI precedence and exact exit codes. Subprocess tests include help, version,
valid defaults, unknown flag, invalid config, missing file, complete bootstrap
and Unicode paths through actual native argv/config handling.

A01 adds graph ordering/rejection/bounds, partial-start failure/exception rollback,
dependency retention, repeated/reentrant shutdown, context/ownership lifetimes,
stop-token forwarding, deadline overflow/shared budgets, actual joined-worker
timeout/retry and synchronized resource admission. A01 had 65 Linux cases / 589
assertions and 64 Windows cases / 587 assertions because its symlink traversal
test was POSIX-only. HF01 retains all these cases and adds two portable Unicode
cases and a Windows-only native UTF-16 case. Linux now passes 67 cases / 640
assertions; Windows also expects 67 cases but has not been executed for HF01.
Controlled service fixtures
are test code only. No production subsystem is represented by a pretend service.

The contention workload uses four threads with 1,000 admission attempts each;
it checks capacity invariants, not throughput. The real deadline test uses a
completion gate so 10 ms must expire before cleanup can finish, verifies return
within a generous 2 s watchdog, then releases the gate and retries with 2,000 ms.
Deterministic fake-clock tests prove the shared deadline arithmetic without
relying on scheduler timing. CTest applies a 30 s unit-suite timeout and
15 s per original CLI subprocess contract; Unicode/tooling cases get 30 s.

HF01 core fixtures preserve fs::temp_directory_path() and create Unicode/spaces
below it. Native fs::path values are used for filesystem I/O and path_to_utf8
only for CLI/config text. Test on the real non-ASCII Windows profile with its
original TEMP/TMP; do not apply the old ASCII workaround. The Windows tooling
case exercises actual cmd.exe UTF-16 environment transport with Unicode/spaces
and shell metacharacters, failed setup, CTest discovery and architecture checks.
Its fixtures never invoke the fake tool files used solely to test discovery.

Run Debug and Release for a foundation change. On a supported compiler run
ASan/UBSan and record any environment restrictions rather than suppressing errors.
A test process must finish within its CTest timeout; no-tests is an error.
Run `python3 tools/check_boundaries.py` and `python3 tools/format.py --check`.
clang-tidy is opt-in with GW_ENABLE_CLANG_TIDY; absence is reported as unverified.

On this Linux executor, default LeakSanitizer cannot inspect process tasks.
The explicitly separate `ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1`
run passes; it is not a passing leak check and is not baked into presets or CI.
GCC ThreadSanitizer passed in A01; it was not rerun for HF01's fixture/tooling-only
changes. The limited ASan/UBSan suite was rerun for HF01 and all 33 checks passed.
No sanitizer finding was suppressed. Run full leak checks on a supported host.

Windows: validate the actual MSVC/SDK, Known Folder, Unicode argv and tool
selection in both ordinary and Developer PowerShell. macOS: validate
AppleClang, user directories, symlink behavior and host architecture. Hosted
CI is useful but does not substitute for the user's Windows 10/VS2026 host.

The owner reports A01 Windows portable builds/CTest pass with the TEMP workaround.
HF01 Windows and its PowerShell checks remain unexecuted here. All three current
PowerShell files parsed with Tree-sitter PowerShell 0.26.4 / Tree-sitter 0.26.0;
this is a grammar check, not PowerShell 5.1 or MSVC runtime validation. Parser
packages are local validation tools, not project/runtime dependencies. Windows
vcpkg doctest configuration is still unresolved. Follow the HF01 Windows checklist
and record tool versions, exact archive checksum, test counts and unmodified temp
environment before owner acceptance. Full LSan, clang-tidy and hosted CI stay open.

Future checkpoints must add interop, fuzzing, storage fault injection and
privacy leakage tests as real functionality arrives. No synthetic benchmark
number is a product throughput/latency guarantee. Add workload definitions and
hardware/environment metadata before publishing performance claims.
