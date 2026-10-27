## A05-P03 validated subset (candidate, not owner accepted)

Codec advisory helper now verifies the exact registered local offer and selection.
Added test-only C++ probe and deterministic Python-vs-C++ differential harness,
without linking an external CBOR library to production. Actual test results,
platform limitations and required follow-up are documented in
[GW-A05-P03](checkpoints/GW-A05-P03.md).

# Completed implementation records

## A05-P01 documentation-only proposal (no implemented protocol)

The owner accepted A04-P03 and authorized A05-P01 **design**. ADR-009,
PROTOCOL_SCHEMA.md, CDDL and prospective test vectors/plan are prepared for
review, not yet owner-accepted. No wire codec, runtime version update, network
feature or persistence was implemented. Existing implementation remains P03
0.4.1; do not count this entry as completed protocol functionality.


## P03 scoped hardening

P02 Windows Debug/Release 40/40 each and console launch are supplied evidence;
see [host record](checkpoints/GW-A04-P02-HOST-ACCEPTANCE.md). P03 reviews the
implementation and repairs membership deletion/self-trust invalidation. Actual
validation: Debug/Release and ASan/UBSan with leaks disabled, 41/41 each;
167 unit cases / 4,383 assertions per normal configuration. Details in [P03 report](checkpoints/GW-A04-P03.md); native P03
acceptance remains pending. No A05 functionality is included.


## Current addendum — 2026-10-16

HF01 Windows owner acceptance and exact-source hosted Linux/Windows/macOS
validation are now recorded in
[GW-A03-HF01-ACCEPTANCE.md](checkpoints/GW-A03-HF01-ACCEPTANCE.md). This supersedes
earlier pending native statements for the tested HF01 source and environments.
All nine hosted jobs passed; local LSan/clang-tidy and independent review stay separate.

A04-P01 completed a design deliverable and was subsequently accepted by the
owner after independent architecture review and source-integrity verification.
Its historical [report](checkpoints/GW-A04-P01.md) records documentation-only
validation. P02 implements the approved bounded volatile model and advances the
runtime to 0.4.0. Linux Debug/Release and ASan/UBSan (leak detection disabled)
passed 41/41 CTest each. Full LSan remains host-blocked. Details are recorded in
[GW-A04-P02.md](checkpoints/GW-A04-P02.md); independent implementation review and
native Windows acceptance remain pending.

| ID | P02 implemented work | Evidence and limits |
|---|---|---|
| A04-P02-IDENTITY | Explicit volatile profiles and independent profile/community authority-device keys; typed IDs and owner/generation handles | Identity service/record test mapping and executed results in P02 report; logical process isolation only |
| A04-P02-RECORD | Bounded canonical signed DeviceSet, explicit authority/scope/time validation | Pure Ed25519 through accepted provider; no handshake, live possession or permission claim |
| A04-P02-TRUST | Explicit pins, independent-verification state, block/conflict/revision handling and point-of-use recognition | No automatic trust or eviction; no durable revision state or network revocation distribution |
| A04-P02-DEVICES | Scoped replacement/removal and complete renewed device sets | Independent membership devices survive profile-device revocation; root continuity/recovery deferred |
| A04-P02-LIFECYCLE | Separate composition adapter; identity stops before its injected CryptoService | Owned synchronous operations, bounded counts/bytes, cooperative deadlines and cancellation |
| A04-P02-CONSOLE | Opt-in CLI session driving real lifecycle-owned identity operations | No private persistence, key import/export, GUI or network; Windows interaction requires native acceptance |

This table records implementation, not owner acceptance or a passing native
test claim. P02 stops for review; no P03 or A05 work is authorized by this record.

Updated: 2026-10-13. These records concern implemented work and the stated evidence,
not user acceptance of the entire checkpoint. Earlier entries retain their historical scope; later addenda supersede their
then-pending acceptance statements. Current gates are tracked in TODO. No
persistent store, network or graphical UI feature is complete.

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

## Subsequent acceptance and A02 implementation, 2026-10-12

The owner accepted HF01 Windows10/VS2026 portable Debug/Release, 34/34 each with
original TEMP/TMP. The supplied HF02 GitHub commit 7137505 has directly inspected
Linux/Windows/macOS CI success. Historical pending statements above reflect the
then-current checkpoints, not the latest evidence. See GW-A02-INPUT-EVIDENCE.md.

| ID | Implemented work | Executed evidence / limits |
|---|---|---|
| A02-PROCESS | Owned child/group/job, explicit argv/cwd/env, bounded capture, cancellation/deadline waits and joined cleanup | Linux real-child tests; native Windows/macOS A02 gates pending |
| A02-STOP | Restricted callback latch and explicit owner cancellation dispatch | Linux native delivery/restoration/reinstall and duplicate/owner checks; Windows native branch unexecuted |
| A02-CAPABILITY | DPAPI/Keychain API probe, unsupported Linux result, disabled/failure semantics | Linux unsupported/disabled/cancel tests; never authorizes persistence |
| A02-INTEGRATION | Existing one-shot roots forward stop into Application; real child-service fixture | Linux Debug/Release 34/34 each; 67 core + 18 platform cases, 831 assertions |
| A02-HANDOVER | Platform contracts/ADR-005, source provenance/CI evidence, current records, native checklist and A03 proposal | Complete measured matrix and source archive verification in GW-A02.md |

A02 is implemented with local evidence, **not owner accepted**. No A03 provider,
identity, private persistence, network, GUI or AI implementation was added.


## Owner A02 acceptance and A03 implementation — 2026-10-13

Owner accepted A02 Windows clean portable Debug/Release 35/35 each, platform,
Unicode/tooling and CLI launch, original TEMP/TMP, on the win10dev account.
See GW-A02-HOST-ACCEPTANCE.md. Earlier A02 pending statements above are historical.
A03 was explicitly authorized from that archive; it is not yet owner-accepted.

| ID | Implemented work | Executed evidence / limits |
|---|---|---|
| A03-PROVIDER | Reviewed Botan 3.13.0 pin, signed source, three generic static amalgamations and deterministic regeneration | Source/signature/hash checks, offline regeneration, corrupt-source/version rejection; native Windows/macOS build pending |
| A03-PRIMITIVES | Bounded OS RNG, ChaCha20-Poly1305, public-message Ed25519 and SHA-256 | RFC/upstream vectors, tampering/invalid sizes/signatures, OS entropy and isolated fault tests |
| A03-OWNERSHIP | Move-only clearing buffers and purpose-specific keys; bounded shared accounting | Move/reset/provider-outliving, allocation rollback and wipe observer tests; memory locking remains best effort |
| A03-CANCEL | Synchronous immediate admission, stop tokens and nonblocking close | Cancellation, deterministic in-flight stop/busy, contention and ThreadSanitizer unit evidence |
| A03-INTEGRATION | Lifecycle-owned CryptoService in all four roots; exact prior CLI/schema/exits retained | Linux portable Debug/Release and vcpkg Debug 37/37 each, 114 cases / 2369 assertions; no production test fault symbols |
| A03-HANDOVER | Contracts/ADR-006, current records, native checklist, inventory and proposed A04 | Actual complete matrix and archive verification in GW-A03.md; independent review/owner acceptance still required |

ASan/UBSan passes the full suite with leak detection disabled; default LSan fails
in this executor. Native A03 Windows/macOS, owner Windows vcpkg, full leaks,
clang-tidy and hosted A03 CI remain open. Primitive correctness is not identity,
protocol, persistent-storage or E2EE acceptance. Stop after A03.


## A03 acceptance and scoped HF01 recovery — 2026-10-14

Owner A03 Windows10/VS2026 portable Debug/Release passed 37/37 each with original
TEMP/TMP; all four binaries launched for version reporting. Exact main 95c9128
matches all delivered source bytes. Hosted Linux/Windows passed; macOS failed
34/36 in each ARM64/Intel configuration. See GW-A03-HOST-ACCEPTANCE.md.

HF01's crypto fixture now resolves its trusted TEMP base; the added Linux
symlink-TEMP regression fails on old A03 and passes on the candidate. A repeated
real-child cleanup regression and full Linux Debug/Release 38/38 pass. Darwin
zombie-only group handling is implemented with bounded inspection and remains
native-unvalidated. It is not listed as completed native recovery. The precise
sanitizer/package outcomes and remaining gates are in GW-A03-HF01.md.
No identity implementation, private storage, networking or remote publication
is included; A04 design remains pending owner review.
