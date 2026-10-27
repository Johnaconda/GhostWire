## Current — HF09 MSVC exception contract (2026-10-27)

Owner Windows 10 x64 HF08: correctly selected `Hostx64/x64/cl.exe`, passed
`portable-debug` configuration, but build failed at the throwing test-only
`extern "C"` hook (C4297 promoted by `/WX`). No CTest was executed.
HF09 changes that hook's C++ exception contract, not production wire bytes.
Linux GCC Debug focused fault test passes, and full Debug regression 52/52
passes against the HF08 verified build cache with HF09's two modified
translation units recompiled. Release and other detailed results are in
`../checkpoints/GW-A05-P04-HF09-VALIDATION.json`; owner Windows tests remain
**pending**, as do TinyCBOR real upstream and security review. Do not infer
MSVC success from Linux.

## Current — HF08 allocation-fault portability recovery (2026-10-27)

HF07 source archive SHA-256 `e770ab342de5f23f60f41cb3ad9233f78fad623c1a8ec1fb3f1034e3d14f9f71` and all **282** internal hashes verified. HF07 Linux baseline targeted allocation-fault CTest PASS 1/1, while owner Windows Debug HF07 yielded 49/50 and a 15-second allocation fixture timeout. HF08 uses separately compiled, test-only codec allocation fault injection to avoid global allocator override; production source behavior is unchanged and the same preflight, failures and recovery behavior are checked. A separate Linux-only `protocol-global-allocation-linux` regression retains the original process-wide allocation-count check. Full local GCC and Python results are in `../checkpoints/GW-A05-P04-HF08-VALIDATION.json`. Native Windows HF08 acceptance remains unrun. Real TinyCBOR upstream comparison and independent parser review remain uncompleted. No release of A06.

# Validation policy and current recovery evidence

## A05-P01 design-only execution evidence, 2026-10-17

Accepted P03 archive: SHA-256 and all 200 per-file source-manifest hashes
verified; extracted tracked files reproduce published Git tree
`0062e94ad243fc1da79b1d69ed99d9091c3bce6c`. All GitHub Actions
platform workflows for main `01e70b3` completed green, per live API query.
Local GCC 14.2 Linux portable Debug **and Release**: configure/build PASS,
CTest **41/41 PASS each**. Compilation interrupted by bounded executor windows
was resumed and completed; Release showed a vendor Botan compiler warning.
JSON fixture consistency checker: 2 positive + 12 negative descriptions PASS
for prefix lengths/schema pointers only, **not protocol decoding**.
P01 adds documentation/fixtures/packaging changes, no C++ or CMake diffs.
Executed Python/boundary checks and ZIP source integrity are recorded in
GW-A05-P01.md; owner Windows tests have not been run on this P01 package.


## Current P03 validation

Linux Debug/Release: 41/41 CTest each, 167 cases / 4,383 assertions each.
ASan/UBSan with leaks disabled: 41/41. Full LSan remains ptrace-blocked.

P02 supplied Windows evidence is in ../checkpoints/GW-A04-P02-HOST-ACCEPTANCE.md.
Use ../checkpoints/GW-A04-P03.md for P03 actual runs and limitations. Never reuse
P02's 40/40 Windows passes as P03 execution evidence. Historical sections below
retain their original source scope.


## Current update — A04-P02, 2026-10-16

Linux Debug and Release: **41/41 CTest each**. ASan/UBSan with leak detection
disabled: **41/41**. Full local LSan remains blocked by ptrace. Each normal
configuration passed 166 unit cases / 4,325 assertions; native P02 acceptance
remains pending.

The P01 design is owner-accepted. The implementation candidate adds actual
volatile identities, complete signed device sets, explicit trust and the opt-in
CLI identity session. [P02 results](../checkpoints/GW-A04-P02.md) are authoritative
for its executed Linux Debug/Release/sanitizer checks and coverage limitations.
The new suite retains all earlier tests; the compiled checkpoint assertion is
updated to 0.4.0 / GW-A04-P02-IDENTITY-DEVICE-TRUST. No warning flag is weakened.

Native Windows acceptance uses the exact single-line commands in
[P02 Windows instructions](../checkpoints/GW-A04-P02-WINDOWS-ACCEPTANCE.md).
New native Windows/macOS and hosted CI results remain unverified. Previous HF01
results below are historical and cannot validate this new implementation.
The owner controls publication; no remote workflow was dispatched.

## Historical P01/HF01 update — 2026-10-15

[HF01 acceptance](../checkpoints/GW-A03-HF01-ACCEPTANCE.md) records the owner's
Windows 37/37 per configuration and all nine successful hosted native job logs
at 1f92c553, including macOS ARM64/Intel 37/37 each and Linux default sanitizers
38/38. Personal Mac availability is not required to inspect native hosted tests.
Older-OS coverage, owner-local vcpkg, local LSan and clang-tidy remain separate.

A04-P01 changes only documentation and source-package metadata. Its validation
checks source preservation, document links/structured evidence, proposed field
lengths, module boundaries, Python syntax, ZIP reproducibility and manifest
integrity. Existing runtime/build/test bytes are compared with the exact tested
HF01 baseline. No new C++ build or identity test is claimed; the 26-case identity
plan is future acceptance work. See [the P01 report](../checkpoints/GW-A04-P01.md).

## Historical HF01 delivery status

Current source: A03-HF01. [Recovery results](../checkpoints/GW-A03-HF01.md)
record Linux Debug/Release 38/38 each and remaining candidate-native checks.
[A03 owner/hosted acceptance](../checkpoints/GW-A03-HOST-ACCEPTANCE.md) records
Windows owner 37/37 twice, hosted Windows/Linux success, and four macOS failures.
No original source report is rewritten into a later success. Expected HF01 counts
are Linux38, macOS37 and Windows37; only executed counts are validation results.
The new Unix symlink-TEMP integration intentionally selects one lifecycle case;
its other 19 crypto cases run in the full crypto-unit entry, not silently omitted.

The historical A03 measured matrix and commands are in [GW-A03](../checkpoints/GW-A03.md):
Linux portable Debug/Release and vcpkg Debug 37/37 each, 114 cases / 2369 assertions,
separate leak-disabled ASan/UBSan 37/37 and TSan five unit runners 5/5. The provider
is instrumented too. Default LSan failed at runtime after assertions passed;
clang-tidy, native A03 Windows/macOS and hosted A03 CI remain open. Source pin,
regeneration, negative dependency rejection and production fault-symbol isolation
are recorded separately. No A04 authorization follows from these results.
The pinned upstream minimized-module suite linked to GhostWire's built static
Botan passed 485,213 upstream checks; excluded modules/online/long tests are not
covered. This does not replace independent cryptographic review.

[A03 native acceptance](../checkpoints/GW-A03-NATIVE-ACCEPTANCE.md) gives exact
owner commands and expected counts: Windows 37, macOS 36. The Windows vcpkg issue
is still unresolved. A02 owner Windows acceptance is now recorded in
[the addendum](../checkpoints/GW-A02-HOST-ACCEPTANCE.md).

The retained sections below describe the **historical A02 validation policy and
then-current evidence**, including gates that have since received that Windows
acceptance. The measured A02 matrix is in [GW-A02](../checkpoints/GW-A02.md). Earlier
reports remain historical records. [Input evidence](../checkpoints/GW-A02-INPUT-EVIDENCE.md)
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
