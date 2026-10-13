# Validation policy and current A02 evidence

The measured A02 matrix is in [GW-A02](../checkpoints/GW-A02.md). Earlier reports
remain historical records. [Input evidence](../checkpoints/GW-A02-INPUT-EVIDENCE.md)
separates owner HF01 acceptance and supplied HF02 hosted results from new A02 tests.
Implementation, execution evidence and owner acceptance are different states.

## Regression coverage

CMake warnings-as-errors applies to project code; vendored doctest is a SYSTEM
include. No warning flags or accepted test cases are weakened. A02 registers two
unit executables plus eight real subprocess contracts per role, and the existing
Windows PowerShell test: **34 Linux/macOS, 35 Windows** CTest registrations.
Windows/macOS A02 counts are expected coverage, not execution evidence.

The existing core suite retains bounded config/version/results, strict UTF-8,
Unicode native paths, role isolation, exact CLI exits, no-write validation,
privacy-aware logging, service graphs/failure rollback/reverse cleanup, repeated
shutdown, context lifetimes, cancellation, shared monotonic deadlines, joined
work and admission contention. Linux still has 67 core cases / 640 assertions.

The new platform suite uses an actual compiled child fixture. Linux has 18 cases /
191 assertions, including Unicode exe/cwd/argv/env, spaces/empty/quotes/backslashes,
literal shell metacharacters, invalid UTF-8/options/missing native resources,
nonzero exits, flood/discard/zero output budgets, timeout/cancellation ownership,
force/repeated/concurrent waits, RAII reaping, ordinary descendants, native stop
owner dispatch/reinstall/restoration, environment isolation, foreign descriptors,
SIGCHLD ownership and a real A01 child-owning service. Windows adds foreign HANDLE
isolation in place of three POSIX-only cases. Its actual A02 case/assertion counts
must come from native logs. Capability tests cover unsupported/disabled/cancel/
invalid-policy behavior and native API presence on supported systems; no vault.

Fixtures retain native temp paths, including ö/spaces, and use explicit UTF-8
conversion only at text boundaries. Never substitute an ASCII TEMP/TMP to obtain
acceptance. Windows platform tests own a console only when no shared console is
present. Native process-control failures must fail the test rather than become a
silent skip. CTest's suite watchdogs and real handshake deadlines bound fixtures;
these do not prove hard kernel cleanup deadlines.

## Actual local commands

Run from project root with compiler/CMake/Ninja on PATH. A02 local evidence uses
serial builds because an observed parallel-link environment issue intermittently
left binaries at mode 0644; serial clean rebuild restored valid output without
source or warning-policy workarounds. This is not a native-platform result.

```bash
cmake --preset portable-debug
cmake --build --preset portable-debug --parallel 1
ctest --preset portable-debug --verbose
cmake --preset portable-release
cmake --build --preset portable-release --parallel 1
ctest --preset portable-release --verbose
# Requires pinned VCPKG_ROOT; no silent fallback to vendored:
cmake --preset linux-debug
cmake --build --preset linux-debug --parallel 1
ctest --preset linux-debug --verbose
cmake --preset portable-sanitize
cmake --build --preset portable-sanitize --parallel 1
# Actual default local attempt: unit assertions passed, LSan runtime failed:
ctest --preset portable-sanitize -R '^core-unit$'
# Separate limited evidence, never embedded in presets/CI:
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 ctest --preset portable-sanitize --verbose
cmake -S . -B out/build/portable-tsan -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DGW_TEST_DEPENDENCY=vendored \
  '-DCMAKE_CXX_FLAGS=-fsanitize=thread -fno-omit-frame-pointer' \
  -DCMAKE_EXE_LINKER_FLAGS=-fsanitize=thread
cmake --build out/build/portable-tsan --target ghostwire-tests ghostwire-platform-tests ghostwire-platform-fixture --parallel 1
ctest --test-dir out/build/portable-tsan -R '^(core|platform)-unit$' --output-on-failure
python3 tools/check_boundaries.py
python3 tools/format.py --check
```

Test-only replacement environments explicitly forward ASAN_OPTIONS, UBSAN_OPTIONS
and TSAN_OPTIONS when the test runner receives them, so instrumented children
honor the requested run policy. They set no default suppression/disable policy.
Production environment replacement/inheritance is unaffected.

## Interpretation and remaining checks

Local default LeakSanitizer failed to inspect /proc task state after core assertions
passed. The separately recorded leak-disabled ASan/UBSan success is not a passing
leak audit. TSan covered both complete unit executables (including real instrumented
children), not all role CLI subprocess contracts. Prior HF02 default sanitizer CI
passed, but it predates the new A02 monitor/process implementation.

clang-tidy is opt-in via GW_ENABLE_CLANG_TIDY; it is unavailable in this executor.
Do not report it as passed based on strict compilation or the boundary script.
A02 Windows10/MSVC19.51 and macOS native runtime are unexecuted here. The prior
hosted MSVC19.44 success does not close the owner's local vcpkg/doctest issue.
Use [native acceptance](../checkpoints/GW-A02-NATIVE-ACCEPTANCE.md) and retain
original temp settings. New A02 CI was not triggered and no remote changes made.

Source JSON/YAML, local Markdown links, packaging CRC/source hashes and a fresh
archive extraction/build must be checked before delivery. Do not ship build/cache
outputs, credentials, temporary fixtures or .git. Future crypto/storage/network
checkpoints need their own upstream vectors, fault injection, interop/fuzzing and
privacy leakage tests; no current unit result establishes product readiness.
