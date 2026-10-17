# Changelog

## 0.4.1 — GW-A04-P03-IDENTITY-HARDENING — 2026-10-16

- Record owner-supplied P02 Windows Debug/Release 40/40 each and console launch;
  distinguish the older A03 screenshot and uncaptured manual command workflow.
- Repair local membership deletion so stale self-trust cannot revive a revoked
  device; preserve unrelated authority contexts and independent profiles.
- Add focused deletion/handle/resource regressions and retain the complete suite.
- Refresh runtime/banner/package metadata, native acceptance and gated A05
  design handover. No dependency, wire representation or persistence changes.
- Actual P03 results and remaining gates are in checkpoints/GW-A04-P03.md.


## 0.4.0 — GW-A04-P02-IDENTITY-DEVICE-TRUST — 2026-10-16 (acceptance candidate)

- Continue the verified cumulative P01 source after owner design acceptance;
  preserve earlier implementation, dependency pins, Unicode behavior and reports.
- Implement typed public identities and explicit owner/generation handles,
  isolated volatile profiles, independently generated community membership/device
  keys and collision-safe labels. No implicit profile or global selection.
- Implement canonical bounded Ed25519 DeviceSets, independently selected
  authority/scope/time validation, explicit trust pins, revision/conflict handling,
  point-of-use recognition, scoped rotation/revocation and renewed records.
- Bound counts, retained record bytes and operation input; stage mutations across
  crypto/cancellation/deadline failures. Keep UTC validity separate from monotonic
  work deadlines and preserve explicit local time revalidation.
- Add a separate lifecycle composition adapter with an injected CryptoService,
  declared admission reservation and identity-before-crypto cleanup.
- Add opt-in `ghostwire-cli --identity-session` for real profile/membership/device
  interaction, explicit local trust/block/recognition commands and public display.
  Preserve ordinary four-role help/version/config/bootstrap behavior.
- Add identity, isolated fault, lifecycle and interactive CLI regressions. Actual
  executed results and limitations are recorded in GW-A04-P02.md; new native
  Windows/macOS execution and independent implementation acceptance remain gates.
- Update contracts, status/security records and Windows handover. No persistent
  private keys, profile/trust files, root recovery, networking, GUI or future
  checkpoint implementation. A11-PRE/AUTH-PRE remain mandatory. Stop after P02;
  the owner retains all GitHub publication control.

Historical headings below retain their status at delivery. The owner later
accepted P01's design; that acceptance is not P02 implementation validation.

## GW-A04-P01-IDENTITY-DESIGN — 2026-10-15 (design review; runtime stays 0.3.1)

- Record HF01 owner Windows Debug/Release 37/37 and exact 166-file reconciliation
  with main 1f92c553. Inspect all nine successful hosted jobs, including native
  macOS Intel/ARM64 Debug/Release 37/37 and Linux default sanitizers 38/38.
- Prepare proposed ADR-008, identity/module contracts and 26 implementation
  acceptance scenarios: isolated profiles, independent per-community credentials,
  collision-safe aliases, explicit trust, bounded device sets, revocation and
  honest recovery limits. Design approval remains pending.
- Refine future dependency direction to prevent core/identity and
  identity/permissions cycles. Update status, threats, invariants and next prompt.
- Update source-package metadata only; preserve all runtime sources, CMake,
  dependencies, CLI versions, tests, historical reports and original A04 prompt.
  No executable identity feature, new wire protocol or private persistence.

Historical headings below retain their acceptance status at delivery. Later
acceptance is recorded in GW-A03-HF01-ACCEPTANCE.md.

## 0.3.1 — GW-A03-HF01-MACOS-RECOVERY — 2026-10-14 (native acceptance pending)

- Record owner A03 Windows Debug/Release 37/37 each with original TEMP/TMP and
  exact main reconciliation. Hosted Linux/Windows pass; macOS failed 34/36 in
  all four ARM64/Intel Debug/Release jobs. Historical reports remain unchanged.
- Handle Darwin's zombie-only process-group EPERM with bounded group inspection;
  preserve live permission failures, unreaped leader ownership and fixed errors.
- Canonicalize the crypto lifecycle fixture's trusted temporary base. Production
  filesystem validation is unchanged. Add a Unix symlink-TEMP integration test
  and a repeated completed-child cleanup regression.
- Preserve both dependency modes, warning policy, CLI behavior and A11-PRE/AUTH-PRE.
  No A04 functionality or remote write. Updated native checklist and handover.

## 0.3.0 — GW-A03-CRYPTOGRAPHIC-PROVIDER — 2026-10-13 (owner acceptance pending)

- Continue the verified owner-accepted A02 source (125 manifest hashes); preserve
  owner changes and historical reports; record A02 Windows 35/35 Debug/Release
  acceptance on win10dev with original TEMP/TMP. No remote write.
- Select Botan 3.13.0 after maintenance/advisory/license/platform and alternative
  review; bundle signed source and reproducible, pinned static generic subsets
  for Linux, Windows and macOS. Ordinary offline builds need no Python/Botan DLL.
- Add explicitly owned bounded OS randomness, RFC 8439 ChaCha20-Poly1305, RFC 8032
  pure Ed25519 for public messages and SHA-256. No algorithm-string API/protocol.
- Add enum-only failures, move-only purpose-specific keys/clearing buffers,
  shared lifetime accounting, input/workspace quotas, immediate busy admission
  and cooperative stop tokens. Document nonce uniqueness and memory limitations.
- Integrate a real owned CryptoService in all four one-shot roles. Preserve CLI,
  config/exit/path/Unicode/privacy contracts and existing portable/vcpkg modes.
- Add upstream vectors, exhaustive small-vector tampering, ownership/cleanup,
  cancellation/concurrency, isolated faults and real Linux OS entropy failure.
  No test fault symbols are present in production targets.
- Record complete Linux Debug/Release and vcpkg Debug regressions, limited
  sanitizer evidence and blocked native/leak/static-analysis gates in GW-A03.md.
- Update contracts, invariant/threat maps, TODO/roadmap, native acceptance,
  source inventory and A04 proposal. A11-PRE and AUTH-PRE remain mandatory.

No identity, persistent key/profile/vault, networking, messaging, GUI or AI is
implemented. No A04 work or GitHub publication. Historical entries below retain
their then-current acceptance status; A02's later acceptance has its own addendum.

## 0.2.0 — GW-A02-PLATFORM-ABSTRACTIONS — 2026-10-12 (acceptance pending)

- Continue verified accepted HF01 and reconcile supplied GitHub commit 7137505;
  preserve HF02 macOS CI/byte-conversion repairs, owner README and prior records.
- Add owned native process launch, explicit cwd/environment, argv without shell
  interpolation, bounded stream capture, cancellable/deadline waits and RAII cleanup.
- Add Windows job/handle isolation and POSIX group/descriptor/reaping ownership,
  with a joined monitor and explicit graceful versus forced stop.
- Add explicit process-stop registration and owner-thread cancellation dispatch;
  integrate safe points into all four existing one-shot bootstraps.
- Add truthful DPAPI/Keychain API-presence probes; Linux unsupported, policy-disabled
  unavailable, failures distinct, and persistence_ready false throughout.
- Add real child/descendant, Unicode/quoting, failure, environment, lifetime,
  deadline, native signal/console, concurrent-wait and child-service tests.
- Preserve C++20, warning policy, portable/vcpkg presets and all prior tests;
  require safe POSIX spawn facilities instead of unsafe fallback inheritance.
- Record actual Linux evidence, prior input CI, new native acceptance gates,
  platform contracts, ADR-005, inventory and proposed A03 handover. No A03 work.


## 0.1.2 — GW-A01-HF02-MACOS-CI-RECOVERY — 2026-10-12 (candidate)

- Scoped GitHub publication/CI repair atop accepted HF01; not A02 work.
- Moved macOS CI from unsupported AppleClang 15/libc++ on macos-14 to
  macos-26 ARM64 and Intel with a C++20 std::stop_source preflight.
- Made char-to-unsigned-char control checks explicit in three core loops without
  relaxing -Werror or changing validation behavior.
- Recorded owner HF01 Windows Debug and Release (34/34 CTest each, original
  TEMP/TMP), newly published GitHub CI evidence and remaining macOS gates.
- Updated README/agent/roadmap/TODO and source packaging checkpoint metadata.
- macOS CI acceptance remains pending until fresh hosted jobs succeed.


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
