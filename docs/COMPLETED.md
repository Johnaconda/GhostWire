## Agent startup context — 2026-11-02

`GW-DEV-CONTEXT-BOOTSTRAP` implements task-first startup routing: short guides,
10 area routes, exact task/dependency/contract locators, bounded output and optional
clean-manifest hashing. Tests cover all registered selections and failure paths.
Patch author tag: **GPT-6.1-Sol-Max**. This is development tooling, with no new
application functionality, owner acceptance or project completion credit.
[Evidence](checkpoints/GW-DEV-CONTEXT-BOOTSTRAP.md).

## Prompt navigation correction — P02 revision 2, 2026-11-01

`GW-APP-PROMPT-NAVIGATION` is implemented and Linux-validated: all guided prompts
support explicit return/quit/help navigation, numeric alias guidance is clearer,
and canonical data/confirmation semantics are preserved. Fresh Debug/Release CLI
builds 31/31 and scoped CTests 9/9 each pass, including 19 prompt contexts and the
owner reproduction. Default packaging includes owner-provided author tag
**GPT-6.1-Sol-Max** and matched checksum naming. Owner Windows revision 1 scoped
build/workflow results are recorded separately; revision 2 native execution is
pending. No new full-suite, security approval or 20% acceptance is inferred.
[Evidence](checkpoints/GW-APP-P02-MEMBERSHIP-LIFECYCLE.md).

## Local membership lifecycle — 2026-11-01

`GW-APP-MEMBERSHIP-LIFECYCLE` is implemented and Linux-validated in Debug/Release:
guided preparation, membership label/device/public-record panel, profile rename,
confirmed local deletion and explicit trust cleanup. Existing scoped CTests pass
11/11 each, including identity 41 cases / 1799 assertions and faults 11 / 127.
Two additional terminal workflows per configuration pass, with no persisted files.
The integration checks unchanged rename keys, fresh recreated authorities, independent
profile/member/device/trust scopes, bounded input and incomplete/confirmation EOF.
Native Windows/macOS and owner acceptance of this delivery remain pending; no A05,
AUTH-PRE or 20% acceptance is implied. [Evidence](checkpoints/GW-APP-P02-MEMBERSHIP-LIFECYCLE.md).

## Local console usability — 2026-11-01

`GW-APP-LOCAL-CONSOLE` is implemented and Linux-validated in Debug and Release:
automatic terminal launch, numbered navigation, guided create/select,
cancellation, selected-label status and explicit one-shot --bootstrap. Existing
34 scoped CTests and 14 host terminal/redirection scenarios pass per configuration;
core reports 67 cases / 643 assertions. No files are persisted by the exercised
workflows. This is completion of a local usability task, not owner acceptance,
native Windows/macOS validation, A05 approval or the 20% milestone. See
[delivery evidence](checkpoints/GW-APP-P01-LOCAL-CONSOLE.md).

## A05 bounded fuzz properties — revision 6, 2026-10-31

`GW-A05-FUZZ-INVARIANTS` is locally complete: the existing entrypoint checks
canonical bytes, decoded values and admission recovery. Clang ASan/UBSan runs
complete 18,969,914 and 18,544,083 executions, with successful independent replay
of the minimized corpus and frozen controls. Strict GCC/Clang compilation and two
intentional fault controls validate the target itself. This is bounded Linux
evidence with leak detection disabled, not A05 approval or an external audit.
See [checkpoint evidence](checkpoints/GW-A05-P04-UPSTREAM-INTEROP.md).

## A05 CI upstream coverage — revision 5, 2026-10-31

`GW-A05-CI-INTEROP` is prepared and locally checked. All existing platform jobs
explicitly enable the pinned upstream comparison; Windows selects the new
configure/all switch, and CI logs preserve successful comparison counts.
Fresh portable Linux Debug builds 79/79 steps and passes 54/54 tests. Workflow
structure and Bash syntax checks pass. Hosted execution, native PowerShell and
macOS evidence remain unrun. This completes CI wiring, not the native acceptance
or independent review gates. See [checkpoint evidence](checkpoints/GW-A05-P04-UPSTREAM-INTEROP.md).

## A05 encoder state guard / internal review — revision 4, 2026-10-30

`GW-A05-ENCODE-STATE` is implemented and Linux-validated: a valueless caller-created
body now returns `malformed_frame` without a null dereference; admission remains
reusable. One existing-suite regression covers the failure and recovery. Protocol
unit tests pass 14/14 (2,494 assertions), Release CTest 54/54, targeted Debug 5/5
and scoped ASan/UBSan 3/3; the 4,023-case corpus results are unchanged.
This completes the narrow guard, not A05, independent external review or new
native Windows/macOS acceptance. See
[checkpoint evidence](checkpoints/GW-A05-P04-UPSTREAM-INTEROP.md).

## Upstream interoperability revision 3 — 2026-10-29

Record owner revision 2 VS2026/MSVC 19.51.36260.0 x64 Debug/Release success:
both targeted tests pass 2/2 in each configuration, with the same 4,023-case
counts as GCC. C4245/C2220 is resolved; upstream C5287 remains nonfatal.
Evidence and package metadata only; code, dependencies, build settings and
tests are unchanged. No repeat Windows run is requested. Native macOS evidence
and independent parser review/owner decision remain open.
See [checkpoint evidence](checkpoints/GW-A05-P04-UPSTREAM-INTEROP.md).
Earlier pending statements below describe their original deliveries.

## Upstream interoperability revision 2 — 2026-10-29

Correct the owner-reported MSVC C4245/C2220 in the test-only TinyCBOR probe:
convert validation enums to uint32_t before OR-ing, preserving all mask bits.
Original failure reproduced with GCC sign-conversion errors; corrected Debug,
Release and scoped ASan/UBSan interop pass. Native Windows rerun is pending.
See [checkpoint evidence](checkpoints/GW-A05-P04-UPSTREAM-INTEROP.md).

## A05 actual upstream comparison — 2026-10-29

Bundled the exact 12 pinned TinyCBOR v7.0 sources and ran the existing native
GCC Debug/Release 4,023-case comparison and stdin-bounds checks. Fixed source
packaging and optional build language/sanitizer/vendor-header integration.
Production behavior remains unchanged; Windows/macOS and independent security
approval remain open. See [evidence](checkpoints/GW-A05-P04-UPSTREAM-INTEROP.md).

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
