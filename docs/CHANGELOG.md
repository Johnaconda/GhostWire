# Changelog

## 0.1.1 — GW-A01-HF01-WINDOWS-UNICODE-RECOVERY — 2026-10-12

- Continued the exact supplied A01 cumulative archive; recorded owner-reported
  Windows Debug/Release success with the ASCII TEMP workaround and open gates.
- Replaced six fixture path.string() conversions with the existing UTF-8 boundary
  helper and native path values; every core fixture now includes ö and spaces
  beneath the host's original temporary directory. No UTF-8 validation relaxation.
- Added CLI/config/default-temp and invalid-UTF-8 regressions, native Windows
  UTF-16 round trip and four real executable Unicode subprocess tests.
- Added matching CMake/CTest discovery, CTest override, Unicode-safe VsDevCmd
  environment transfer, x64 compiler/linker/LIB diagnostics and explicit -Fresh.
  TEMP/TMP and permanent user/machine environment/execution policy stay unchanged.
- Added dependency-free Windows tooling checks to CTest; grammar parsed here,
  native execution pending. Linux Debug/Release/vcpkg Debug and ASan/UBSan with
  leak detection disabled pass 33 CTest checks; 67 unit cases / 640 assertions.
- Preserved Windows vcpkg mode and doctest pin; its reported configure failure is
  unresolved. Updated acceptance checklist, handover, TODO and A02 proposal.

No runtime implementation defect was found in the inspected production path
adapters; their source and A01 lifecycle remain unchanged. Stop after HF01.

## 0.1.0 — GW-A01-CORE-LIFECYCLE-SERVICES — 2026-10-12

- Continued the verified A00 archive with immutable application configuration/path
  snapshots, explicitly owned facilities and constructor-injected dependencies.
- Added deterministic owned service registration, bounded dependency validation,
  partial-start rollback, reverse shutdown, retry and structured diagnostics.
- Integrated the real A00 directory initializer into every executable's service
  lifecycle; kept command options, schema 1, exit codes and privacy-aware logging.
- Added cooperative standard stop tokens, injectable monotonic deadlines and
  retained dependency ownership when cleanup is unfinished. No detached work.
- Added thread-safe active/queued/declared-memory admission, CPU headroom policy,
  immediate backpressure and move-only RAII reservations. No worker framework.
- Added 39 unit cases (65 total / 589 assertions), including real owned-worker
  cancellation, deadline retry, concurrent admission and A00 regressions.
- Linux vcpkg Debug/Release and ASan/UBSan with leak detection disabled pass
  all 29 CTest checks; TSan passes the unit suite. Full LSan is blocked by this
  environment. Windows, macOS, clang-tidy and hosted CI remain unverified.
- Updated architecture/contracts/security records and future proposal backlog;
  included the cumulative source manifest, changed-file inventory and A02 proposal.

No GUI, listener, messaging, crypto, discovery, inference, training, telemetry or
remote publication added. A11-PRE and AUTH-PRE remain gates. A01 stops for owner
Windows host acceptance; A02 is not authorized by its proposal's presence.

## 0.0.1 — GW-A00-FOUNDATION-BOOTSTRAP — 2026-10-11

- Created clean-room C++20 core/platform libraries and four distinct console targets.
- Added bounded configuration, typed errors, numeric project version, UTF-8 path
  handling, OS user directories, explicit bootstrap lifecycle and cancellation.
- Added structured redacted logging and actual unit/process exit-code tests.
- Added CMake/Ninja Debug/Release profiles, vcpkg 2026.07.29 baseline and exact
  doctest 2.4.12 fallback with upstream source/notice/hash verification.
- Added Windows tool detection with explicit overrides and preserved vcpkg root;
  POSIX build tools; formatting/boundary checks; pinned three-platform CI files.
- Recorded license choice, trust/data boundaries, future interface designs,
  security prerequisites, detailed A00–A25 roadmap/TODO and A01 handover.
- Added deterministic source packaging, embedded file inventory and SHA-256 sidecar.

No remote repository created/pushed. No GUI, network, identity, crypto, vault,
message, media or AI provider implementation was added. Native Windows/macOS
and full leak detection remain acceptance gates, not passing test claims.
