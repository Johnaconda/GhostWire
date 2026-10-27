# GW-A01 — Core Lifecycle & Services

Checkpoint: **GW-A01-CORE-LIFECYCLE-SERVICES** · Version **0.1.0** · C++20.
Implementation/validation date: **2026-10-12**. A01 implementation is complete
for the stated scope. Linux results below are executed evidence. Owner acceptance
and Windows/macOS native validation remain pending. **Stop after A01.**

## Authoritative baseline and inspection

Input: `GHOSTWIRE-GW-A00-FOUNDATION-BOOTSTRAP-source(1).zip`.
Expected and measured SHA-256:

```text
d07d38825ceb2c73a9ddb0eee429d42e8b85ad64439ba08fd7079399021d0025
```

Extracted into an isolated working tree after verifying SHA-256, ZIP CRCs and
safe member paths. All 85 file hashes in the input SOURCE_MANIFEST.json matched.
Read the entire repository inventory, first-party implementation, tests/build
tools and documentation, including AGENTS.md, the full MASTER_CONTRACT.md,
ARCHITECTURE.md, ROADMAP.md, TODO.md, DEPENDENCY_GRAPH.md, GW-A00.md,
GW-A01-PROMPT.md and SECURITY_INVARIANTS.md. The bundled upstream doctest header
and notice were verified by the existing exact-hash dependency mechanism.

Built the untouched A00 portable Debug baseline: **29/29 CTest checks**, with
**26 unit cases / 195 assertions** passing. A00 supplied working configuration,
redacted logging, platform paths and one-shot lifecycle, not domain services.
The owner explicitly authorized A01 from this input. No source was reconstructed
from conversation memory, reset or silently removed. MASTER_CONTRACT.md, the A00
checkpoint/changed-file reports and original GW-A01-PROMPT.md remain unchanged.

## Implemented functionality

| Area | Actual behavior |
|---|---|
| Application context | Application owns immutable copies of config/paths/role, stop source, default monotonic clock, admission controller and service manager; Logger and optional test clock have explicit borrowed lifetimes. No global registry or hidden lookup. |
| Owned graph | unique_ptr registration, copied dependency IDs, maximum 64 services and 16 dependencies per service. Lowest-ready-ID topological order is deterministic. Missing dependencies, duplicate registrations, invalid declarations and cycles fail before hooks. |
| Startup/rollback | Dependency-order start; each attempted service, including partial failure/throw, is tracked for cleanup. Later services do not start. Rollback uses reverse dependency order and retains original startup error separately from cleanup errors. |
| Shutdown | Admission closes, standard cancellation is requested, then reverse hooks share one monotonic deadline. Default 5,000 ms; valid range 0..60,000 ms. Incomplete services and their dependencies remain alive; independent branches may finish. Retry touches only unfinished hooks. |
| Errors/diagnostics | Structured code/reason/service/dependency fields and bounded copied snapshots/reports. Arbitrary service error text and exception text are discarded. Reentrant/invalid transitions fail. Complete repeated shutdown returns the cached result. |
| Resource admission | Bounded active/queued slots and declared-memory reservations; move-only RAII leases, atomic queued-to-active promotion, immediate rejection/backpressure, cancellation and terminal close. Overflow-safe accounting and synchronized concurrent use. |
| Composition policy | Existing worker/queue config now supplies admission limits. Active capacity respects 20% default CPU headroom; memory defaults to 64 MiB of declared reservations. Policy/timeout are C++ composition options; config schema stays 1. |
| Executables | All four targets run the same managed lifecycle with a real owned filesystem initializer. CLI flags, validation, paths, privacy-aware logging and exits 0/2/3/4/70 are preserved; version/checkpoint text advances to A01. |

No production background worker or scheduler was needed. Test fixtures use
owned std::jthread and condition-variable waits to prove real cancellation/join
behavior. No detached thread, polling loop, fake network/crypto service or new
dependency was introduced. Target DAG and warning policy remain unchanged.

Detailed ownership, thread, error, resource and platform contracts are in
[CORE_LIFECYCLE.md](../architecture/CORE_LIFECYCLE.md); rationale is in
[ADR-004](../decisions/ADR-004-CORE-LIFECYCLE.md). The
[changed-file inventory](GW-A01-CHANGED-FILES.md) compares exact input bytes.

## Executed build and test evidence

Host: Linux x64, Ubuntu 24.04.3 LTS, GCC 13.3.0, CMake 4.4.4,
Ninja 1.13.2.git.kitware.jobserver-pipe-1, Python 3.12.14,
clang-format 23.1.3. The existing vcpkg baseline remains
`9e593bb18ea69cc5095e012465dcd675a822ed0d`; doctest remains exactly 2.4.12.
Local toolchain/build caches are excluded from the source delivery.

| Configuration/check | Actual outcome |
|---|---|
| Untouched input, portable-debug | Build passed; 29/29 CTest; 26 cases / 195 assertions |
| A01 linux-debug, vcpkg | Configure/build passed; 29/29 CTest; 65 cases / 589 assertions |
| A01 linux-release, vcpkg | Configure/build passed; 29/29 CTest; 65 cases / 589 assertions |
| A01 portable-sanitize, default environment | Build passed; default CTest failed 29/29 because LeakSanitizer cannot inspect process tasks here; not counted as a pass |
| Same ASan/UBSan build, explicit leak-disabled run | 29/29 CTest passed with ASAN_OPTIONS=detect_leaks=0 and UBSAN_OPTIONS=halt_on_error=1; no reported address/undefined-behavior finding |
| GCC ThreadSanitizer, separate vendored Debug build | Build passed; complete unit executable passed 65 cases / 589 assertions; no race report; subprocess suite not claimed under TSan |
| Formatting and module boundaries | Both checks passed; no warning suppression or boundary allowlist relaxation |
| JSON/YAML and local documentation links | 3 JSON and 6 YAML files parsed; 35 Markdown files inspected, all 32 local links resolve |
| Fresh packaged-source portable-release | Fresh extraction configured/built successfully; 29/29 CTest; 65 cases / 589 assertions |
| Windows 10 x64 / VS2026 MSVC | Not executed; owner host acceptance required |
| macOS x64 / ARM64 | Not executed on either architecture |
| clang-tidy / hosted CI | clang-tidy unavailable; hosted workflows not run; no remote created/pushed |

Reproduction from the project root, with a bootstrapped pinned VCPKG_ROOT and
CMake/Ninja/compiler on PATH:

```bash
export VCPKG_DISABLE_METRICS=1
bash tools/dev.sh all linux-debug
out/build/linux-debug/bin/ghostwire-tests --no-colors
bash tools/dev.sh all linux-release
out/build/linux-release/bin/ghostwire-tests --no-colors
cmake --preset portable-sanitize
cmake --build --preset portable-sanitize --parallel 2
ctest --preset portable-sanitize
# This executor blocks LeakSanitizer. Separate, explicitly limited evidence:
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 ctest --preset portable-sanitize
python3 tools/format.py --check
python3 tools/check_boundaries.py
```

The default sanitizer failure includes `Can't open /proc/.../task for reading`
and `LeakSanitizer has encountered a fatal error` / ptrace inspection hints.
The unit assertions themselves passed before leak-runtime failure. Full leak
checking is still required on an unrestricted host; no leak setting was changed
in presets or CI. Separate TSan commands actually executed:

```bash
cmake -S . -B out/build/portable-tsan -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DGW_TEST_DEPENDENCY=vendored \
  '-DCMAKE_CXX_FLAGS=-fsanitize=thread -fno-omit-frame-pointer' \
  -DCMAKE_EXE_LINKER_FLAGS=-fsanitize=thread
cmake --build out/build/portable-tsan --parallel 2
out/build/portable-tsan/bin/ghostwire-tests --no-colors
```

The 29 CTest registrations comprise one unit executable and seven subprocess
contracts for each of four roles. The unit suite adds **39 cases**, preserving
the original **26**, for **65 cases / 589 assertions** in total. No test skipped.

| Required coverage | Concrete tests/evidence |
|---|---|
| Dependencies and invalid states | Registration permutations; missing edge before side effects; duplicate consumed ownership; self/disconnected cycles; registration limits; reentrancy; shutdown-before-start |
| Partial initialization and rollback | Failed/throwing service cleanup plus prior services; reserved work released; reverse order; sanitized exceptions; original failure distinct from failed rollback |
| Reverse/repeated shutdown | Dependencies retained after stop error; independent branch cleanup; retry without repeated successful hooks; cached complete outcome; outstanding lease rejects false clean completion |
| Cancellation and deadlines | Pre/during-start stop; admission closed before callback; shared fake-clock 10/3/0 ms budget; callback overrun; negative/zero/large timeout; max-time saturation; real owned worker wake/join |
| Real timeout and lifetime | Worker deliberately held behind completion gate; 10 ms timeout returns before 2 s watchdog with dependencies/lease retained; gate release and 2,000 ms retry joins; reverse object destruction while context alive |
| Resource limits | CPU headroom/unknown CPU/policy bounds; active/queued/byte caps; promotion; move/release after controller destruction; cancellation/close; size_t overflow; four threads x 1,000 attempts |
| A00 behavior | Exact CLI exits/flags/config and no-write checks; role paths/Unicode/symlinks; logging redaction/injection/concurrency/output failures; actual managed directory startup and cleanup |

These are correctness tests, not CPU utilization, RSS enforcement or throughput
benchmarks. Native scheduling and platform branches still require host evidence.

## Security and known limits

- No listener, GUI, identity, cryptography, discovery, messaging, AI runtime or
  training, private persistence, cloud dependency or telemetry was added.
  A11-PRE still gates private state and AUTH-PRE still gates listeners/admin.
- Services are trusted in-process code. IDs are ordering labels, not authority
  or access-control tokens. Core/module separation is not a security sandbox.
- Shutdown deadlines are cooperative. A blocking hook, synchronous stop callback,
  filesystem operation or destructor cannot be safely preempted by this C++ API.
  Overruns are reported after control returns; destructors may block to join
  residual work. Work is never detached or freed with live dependencies merely
  to claim a hard deadline. Hard isolation requires future owned processes.
- Resource reservations count declared work, not allocations or measured CPU.
  Headroom does not guarantee utilization; leases do not enforce process RSS or
  GPU limits. There is no scheduling fairness guarantee, executor or payload queue.
- Lifecycle/graph/snapshot access is owner-thread only. Only documented stop and
  admission operations are thread-safe. Logger, stream and injected clock must
  outlive Application; clocks must be monotonic and share the deadline domain.
- A00 directory creation still leaves empty directories on failure/shutdown and
  is not race-proof secure storage. No keys may be placed there before A11-PRE.
- Diagnostic arrays and registration limits are fixed. OOM/fatal process failure
  cannot be made generally recoverable; service destructors must not throw.
- LSan, clang-tidy, Windows/macOS native acceptance and hosted CI remain open as
  tracked in TODO. Passing Linux tests is not an external security audit.

## Documentation, future scope and acceptance

Architecture, dependency graph, module/interface contracts, ADR, invariants,
threat/boundary records, TODO, roadmap, changelog and completed records now
distinguish implemented A01 behavior, measured Linux evidence and pending owner
acceptance. Historical A00 checkpoint records and the supplied A01 specification
remain intact. No accepted checkpoint ID/order has been changed.

[FUTURE_FEATURES.md](../architecture/FUTURE_FEATURES.md) records local browser
sessions/gateway, native model backends, model import/license/compatibility,
isolated custom model code, optional separate Python training, scoped AI/E2EE
permissions, advanced media and GPU policy. All are unimplemented proposals.
The [A02 prompt](GW-A02-PROMPT.md) is a proposal requiring explicit authorization
after the owner's Windows acceptance. It does not start A02.

Owner reproduction:

```powershell
.\tools\windows\dev-windows.ps1 -Action all -Configuration Debug
.\tools\windows\dev-windows.ps1 -Action all -Configuration Release
.\out\build\windows-msvc-release\bin\ghostwire-tests.exe --no-colors
```

Also perform the fresh-PowerShell, Unicode/spaces paths, version/help and exit
checks in BUILD.md. Acceptance is pending, not implied by this delivery.

## Final source verification

The allowlisted packager produced 100 source files plus SOURCE_MANIFEST.json.
Relative to the authoritative input: 15 added, 32 modified, 0 deleted and 53
unchanged source files; the generated manifest is replaced for A01. All archive
member CRCs and manifest SHA-256 hashes were checked. Historical master/A00/A01
prompt bytes, warning policy and dependency registry configuration were verified
against the input. JSON/YAML syntax and local Markdown file links passed.

A fresh extraction, without any reused build cache, successfully ran:

```bash
bash tools/dev.sh all portable-release
out/build/portable-release/bin/ghostwire-tests --no-colors
```

Result: 29/29 CTest checks and 65/65 unit cases, 589/589 assertions. Only this
report's verification wording changed after that build. All non-Markdown source,
build, script, test and dependency bytes in the final ZIP were compared with the
tested extraction. The final archive was regenerated and its CRCs, every manifest
hash and exact input-to-output file set were verified again.

The delivery is `GHOSTWIRE-GW-A01-CORE-LIFECYCLE-SERVICES-source.zip` with the full
cumulative project, this report, inventory, updated TODO/roadmap, future backlog
and proposed A02 prompt. Its SHA-256 is supplied with the delivery and generated
`.zip.sha256` sidecar, not embedded recursively inside the archive. Build outputs,
binaries, toolchains, .git, caches, credentials and temporary files are excluded.
