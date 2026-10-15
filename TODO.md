# Authoritative TODO
Legend: `[ ]` planned, `[~]` partial/experimental, `[x]` implemented and validated in the stated environment, `[!]` blocked. Design/owner acceptance is tracked separately from implementation and runtime validation. Completed narrow work is recorded in docs/COMPLETED.md; pending platform acceptance stays here.
## Current recovery status — 2026-10-14

This table and the A03 gate updates below supersede earlier checkpoint snapshots.
Historical results remain evidence for their exact source, not for this candidate.

| ID | Work / source | Implementation and validation | Acceptance requirement |
|---|---|---|---|
| GW-A03-OWNER | A03 owner acceptance; docs/checkpoints/GW-A03-HOST-ACCEPTANCE.md | [x] Windows10/VS2026 portable Debug/Release 37/37 each, original TEMP/TMP; all four binary identities | Owner evidence received; do not ask to repeat A03 acceptance |
| GW-A03-REPO | Exact published source and CI reconciliation | [x] All 158 source files match 95c9128; Linux/Windows each 37/37; all four macOS jobs failed 34/36 | Preserve owner changes and pushes; no remote mutation |
| GW-A03-HF01-PROCESS | Darwin zombie-only group cleanup; src/platform/process.cpp; ADR-007 | [~] Candidate implemented; new real-process regression passes on Linux; Darwin-only query unexecuted locally | macOS26 ARM64/Intel Debug/Release 37/37 each; no masking of live permission failures |
| GW-A03-HF01-TEMP | Crypto lifecycle fixture and tests/integration/crypto_temp_path.cmake | [x] Original A03 fails with Linux symlink TMPDIR; fixed Debug/Release passes, production symlink rejection retained | Confirm original macOS TEMP in native runs |
| GW-A03-HF01-EVIDENCE | Full regression, cumulative source and changed inventory | [x] Linux Debug/Release 38/38 each; further measured checks in recovery report | New Windows/macOS review and source-package evidence remain distinct |
| GW-A03-CONSOLE | Legacy Windows help-banner em dash rendering | [ ] Cosmetic issue observed in owner log; CLI/UTF-8 path tests pass | Future console-output pass; do not weaken UTF-8 inputs |
| GW-A04-DESIGN | Identity ADR under existing A04 prompt | [ ] Not approved/implemented; isolated profiles and collision-safe memberships are owner requirements | Resolve credentials, trust, rotation/recovery and public encodings with owner review before implementation |

## Current acceptance gates
| ID | Title | Planned source | Dependencies | Implementation | Validation | Acceptance criteria | Design/owner acceptance |
|---|---|---|---|---|---|---|---|
| GW-A00-WIN | Windows 10 / VS2026 acceptance | tools/windows; src/platform | Cumulative HF01 | [x] Existing foundation accepted through HF01 | Owner Debug/Release 34/34 each, original TEMP/TMP | Preserve Unicode/CLI/tooling coverage in A02 | HF01 accepted; new A02 native gates below |
| GW-A00-MAC | macOS foundation acceptance | src/platform; CI | Supplied HF02 commit | [x] Prior cumulative hosted coverage | macOS26 ARM64/Intel Debug/Release 33/33 each at 7137505 | Preserve coverage; rerun new A02 native branches | Hosted evidence only; physical-host/minimum-OS acceptance separate |
| GW-A00-CI | Execute remote workflows and branch policy | .github; SECURITY.md | Owner-published repository | [x] Prior hosted workflows executed | Linux/Windows/macOS success at 7137505; see A02-INPUT-EVIDENCE | New A02 CI still required; branch protection/private reporting not audited | No repository policy changes made in A02 |
| GW-A00-LEAK | LeakSanitizer on supported host | tests/unit | Sanitizer builds | [~] Prior HF02 default sanitizer workflow passed | 33/33 hosted; A02 local default core run blocked by /proc inspection | Full A02 leak validation remains open | No standalone leak audit inferred |
| GW-A00-TIDY | Run clang-tidy in supported tooling environment | .clang-tidy; CMakeLists.txt | clang-tidy installed | [ ] Integration configured | Not run locally | Record actual static-analysis version/findings; resolve relevant defects | Pending |
| GW-A11-PRE | Minimum encrypted key and session-state store | src/vault; platform/secure_storage.hpp | A02; A03; provider/license review | [ ] Planned | Not run | No raw keys on disk; OS-protected wrapping + bounded KDF; atomic authenticated state and rollback/crash tests before persistent A04/A08 | Architecture decision recorded; implementation unaccepted |
| GW-AUTH-PRE | Base scoped authorization | src/permissions; include/ghostwire/permissions | A01; A04 public principal contract | [ ] Planned | Not run | Deny by default; action/resource/expiry; no admin/agent ambient vault rights; negative matrix before listeners | Architecture decision recorded; implementation unaccepted |
| GW-PROTOCOL-LIB | DM/MLS implementation feasibility | docs/decisions; src/crypto adapters | A03; current upstream review | [!] DM/MLS protocol provider not selected | Not run | Reviewed protocol implementation, maintenance/license/FFI decision, upstream vectors/interop and durable state integration | A03 primitive provider selection does not select a messaging protocol |
| GW-UI-SPIKE | Native accessible UI feasibility | ui/framework; ui/rendering; docs/decisions | A02 | [ ] Planned | Not run | Working hit tests/controls/IME/Unicode/accessibility/DPI and idle profile on three OSes before polish | Candidate SDL3; not selected runtime dependency |

## Implementation and validation work by checkpoint

| ID | Title | Planned source | Dependencies | Implementation | Validation | Acceptance criteria | Design/owner acceptance |
|---|---|---|---|---|---|---|---|
| GW-A01-IMPL | Core Lifecycle & Services | include/ghostwire/core; src/core | Verified A00; explicit owner authorization | [x] Implemented | Linux Debug/Release: 65 cases, 589 assertions; all 29 CTest checks | Owned context; deterministic graph validation; rollback/reverse cleanup; cancellation/deadline retry; bounded admission and diagnostics | Owner A01/HF01 acceptance recorded below |
| GW-A01-EVIDENCE | Core Lifecycle & Services: tests and handover | tests; docs/checkpoints/GW-A01.md; ADR-004 | GW-A01-IMPL | [x] Recorded for executed environment | ASan/UBSan with leaks disabled; TSan unit suite; format/boundaries and package verification | Full cumulative source, changed files, commands, actual limits and A02 proposal; native/leak gates remain below | Owner HF01 Windows portable accepted; prior records preserved |
| GW-A02-IMPL | Platform Abstractions | include/ghostwire/platform; src/platform | Accepted HF01 + supplied HF02 | [x] Implemented and owner-accepted | Linux Debug/Release 34/34; owner Windows Debug/Release 35/35 each, original TEMP/TMP | Preserve owned processes, stop bridging, capability and Unicode regressions | GW-A02-HOST-ACCEPTANCE.md; macOS remains open |
| GW-A02-EVIDENCE | Platform tests and handover | tests; docs/checkpoints/GW-A02.md; ADR-005 | GW-A02-IMPL | [x] Historical report plus acceptance addendum | Verified accepted ZIP and all 125 hashes | Preserve cumulative source and original reports | Owner explicitly authorized A03 only |
| GW-A03-IMPL | Cryptographic Provider | include/ghostwire/crypto; src/crypto; core/crypto_service | Accepted A02 | [x] Implemented, local Linux validated | Debug/Release 37/37 each; 114 cases / 2369 assertions | Pinned Botan; bounded volatile operations; failure, tamper, cancellation, ownership and memory checks | Owner Windows accepted; macOS recovery and independent review remain |
| GW-A03-EVIDENCE | Cryptographic Provider: tests and handover | tests; docs/checkpoints/GW-A03.md; ADR-006 | GW-A03-IMPL | [x] Recorded for executed environment | Actual sanitizer/dependency/source/package evidence in GW-A03.md | Full cumulative ZIP/SHA, inventory, native checklist and A04 proposal | Stop after A03; A04 unstarted |
| GW-A04-IMPL | Identity & Device Trust | include/ghostwire/identity; src/identity | A03; A11-PRE for persistence | [ ] Planned | Not run | Generate/verify root and authorized device identities; rotation/revocation/contact proof; alias collision/change tests; no raw key files. | Proposed; not accepted |
| GW-A04-EVIDENCE | Identity & Device Trust: tests and handover | tests; docs/checkpoints/GW-A04.md; relevant ADRs | GW-A04-IMPL | [ ] Planned | Not run | Add relevant failure/security tests; run available host/build checks; update invariant map, TODO, changes, limits and cumulative ZIP/SHA. | Proposed; not accepted |
| GW-A05-IMPL | Protocol Schemas & Versioning | schemas; include/ghostwire/protocol; src/protocol | A04 public identity contracts | [ ] Planned | Not run | Canonical versioned codecs with bounded fields and negotiation; reject malformed/unsupported input; corpus/fuzz/interop fixtures. | Proposed; not accepted |
| GW-A05-EVIDENCE | Protocol Schemas & Versioning: tests and handover | tests; docs/checkpoints/GW-A05.md; relevant ADRs | GW-A05-IMPL | [ ] Planned | Not run | Add relevant failure/security tests; run available host/build checks; update invariant map, TODO, changes, limits and cumulative ZIP/SHA. | Proposed; not accepted |
| GW-A06-IMPL | Transport & Privacy Policies | include/ghostwire/transport; src/transport | A05; AUTH-PRE | [ ] Planned | Not run | Authenticated bounded connection API; fail-closed direct/protected/privacy-first capabilities; timeout/backpressure and DNS/ICE/address-leak harness. | Proposed; not accepted |
| GW-A06-EVIDENCE | Transport & Privacy Policies: tests and handover | tests; docs/checkpoints/GW-A06.md; relevant ADRs | GW-A06-IMPL | [ ] Planned | Not run | Add relevant failure/security tests; run available host/build checks; update invariant map, TODO, changes, limits and cumulative ZIP/SHA. | Proposed; not accepted |
| GW-A07-IMPL | Autonomous Discovery | include/ghostwire/discovery; src/discovery | A04; A05; A06 | [ ] Planned | Not run | Signed expiring records, bounded queries/caches, manual and independent bootstrap peers; poisoning/Sybil/expiry/leak tests; opt-in LAN. | Proposed; not accepted |
| GW-A07-EVIDENCE | Autonomous Discovery: tests and handover | tests; docs/checkpoints/GW-A07.md; relevant ADRs | GW-A07-IMPL | [ ] Planned | Not run | Add relevant failure/security tests; run available host/build checks; update invariant map, TODO, changes, limits and cumulative ZIP/SHA. | Proposed; not accepted |
| GW-A08-IMPL | Encrypted Direct Messaging | include/ghostwire/messaging; src/messaging | A04; A05; A06; A11-PRE; protocol-library approval | [!] Planned, prerequisites unresolved | Not run | Reviewed async E2EE implementation with multi-device design; atomic protected ratchet state; upstream/interop/replay/crash tests; FS/PCS claims reviewed. | Proposed; not accepted |
| GW-A08-EVIDENCE | Encrypted Direct Messaging: tests and handover | tests; docs/checkpoints/GW-A08.md; relevant ADRs | GW-A08-IMPL | [ ] Planned | Not run | Add relevant failure/security tests; run available host/build checks; update invariant map, TODO, changes, limits and cumulative ZIP/SHA. | Proposed; not accepted |
| GW-A09-IMPL | Offline Delivery & Mailboxes | src/messaging; src/storage; apps/server | A08; storage quotas | [ ] Planned | Not run | Ciphertext mailbox retention, authorization, dedup/acks/retry/expiry and disk/bandwidth quotas; no delivery claim without reachable storage. | Proposed; not accepted |
| GW-A09-EVIDENCE | Offline Delivery & Mailboxes: tests and handover | tests; docs/checkpoints/GW-A09.md; relevant ADRs | GW-A09-IMPL | [ ] Planned | Not run | Add relevant failure/security tests; run available host/build checks; update invariant map, TODO, changes, limits and cumulative ZIP/SHA. | Proposed; not accepted |
| GW-A10-IMPL | Friends & Contact Verification | include/ghostwire/friends; src/friends | A04; A07; A08; A11-PRE | [ ] Planned | Not run | Independent private contact graph, explicit trust/key-change warnings and verification UX; no community prerequisite. | Proposed; not accepted |
| GW-A10-EVIDENCE | Friends & Contact Verification: tests and handover | tests; docs/checkpoints/GW-A10.md; relevant ADRs | GW-A10-IMPL | [ ] Planned | Not run | Add relevant failure/security tests; run available host/build checks; update invariant map, TODO, changes, limits and cumulative ZIP/SHA. | Proposed; not accepted |
| GW-A11-IMPL | Local Encrypted Vault | include/ghostwire/vault; src/vault | A02; A03; A11-PRE accepted | [ ] Planned | Not run | Complete encrypted history/search/cache/backups, migration, atomic writes, lock/recovery; inspect disk/temp/WAL/journals and crash behavior. | Proposed; not accepted |
| GW-A11-EVIDENCE | Local Encrypted Vault: tests and handover | tests; docs/checkpoints/GW-A11.md; relevant ADRs | GW-A11-IMPL | [ ] Planned | Not run | Add relevant failure/security tests; run available host/build checks; update invariant map, TODO, changes, limits and cumulative ZIP/SHA. | Proposed; not accepted |
| GW-A12-IMPL | Vault Protection & Device Privacy | src/vault; src/platform; tests/security | A11; A04 | [ ] Planned | Not run | Local configurable retry/backoff/recovery, rollback/DoS tests, deliberate erasure scope, no remote wipe, local-only capabilities; no offline-counter claim. | Proposed; not accepted |
| GW-A12-EVIDENCE | Vault Protection & Device Privacy: tests and handover | tests; docs/checkpoints/GW-A12.md; relevant ADRs | GW-A12-IMPL | [ ] Planned | Not run | Add relevant failure/security tests; run available host/build checks; update invariant map, TODO, changes, limits and cumulative ZIP/SHA. | Proposed; not accepted |
| GW-A13-IMPL | Community Server Runtime | apps/server; include/ghostwire/communities; src/communities | A06; A09; AUTH-PRE | [ ] Planned | Not run | Headless local/dedicated community identity, invites/password policy, roles/channels, backup and lifecycle; admin not E2EE member by default. | Proposed; not accepted |
| GW-A13-EVIDENCE | Community Server Runtime: tests and handover | tests; docs/checkpoints/GW-A13.md; relevant ADRs | GW-A13-IMPL | [ ] Planned | Not run | Add relevant failure/security tests; run available host/build checks; update invariant map, TODO, changes, limits and cumulative ZIP/SHA. | Proposed; not accepted |
| GW-A14-IMPL | Groups & Membership Encryption | include/ghostwire/groups; src/groups | A04; A08; A11; A13; MLS-library approval | [ ] Planned | Not run | Reviewed MLS/equivalent epochs, join/leave/revocation/history policy; membership and replay interop; removed users cannot decrypt future epochs. | Proposed; not accepted |
| GW-A14-EVIDENCE | Groups & Membership Encryption: tests and handover | tests; docs/checkpoints/GW-A14.md; relevant ADRs | GW-A14-IMPL | [ ] Planned | Not run | Add relevant failure/security tests; run available host/build checks; update invariant map, TODO, changes, limits and cumulative ZIP/SHA. | Proposed; not accepted |
| GW-A15-IMPL | Encrypted File Distribution | include/ghostwire/files; src/files; src/storage | A06; A11; A14; storage quotas | [ ] Planned | Not run | Authenticated encrypted chunks/manifests, resumability, cooperative seeding, scoped access/password wrapping, large-file safe arithmetic and traversal/DoS/leak tests. | Proposed; not accepted |
| GW-A15-EVIDENCE | Encrypted File Distribution: tests and handover | tests; docs/checkpoints/GW-A15.md; relevant ADRs | GW-A15-IMPL | [ ] Planned | Not run | Add relevant failure/security tests; run available host/build checks; update invariant map, TODO, changes, limits and cumulative ZIP/SHA. | Proposed; not accepted |
| GW-A16-IMPL | Federation & Portability | include/ghostwire/federation; src/federation | A13; A14; A15 | [ ] Planned | Not run | Authenticated versioned host exchange and encrypted export/import preserving identity/trust; migration and conflict/partition tests. | Proposed; not accepted |
| GW-A16-EVIDENCE | Federation & Portability: tests and handover | tests; docs/checkpoints/GW-A16.md; relevant ADRs | GW-A16-IMPL | [ ] Planned | Not run | Add relevant failure/security tests; run available host/build checks; update invariant map, TODO, changes, limits and cumulative ZIP/SHA. | Proposed; not accepted |
| GW-A17-IMPL | Desktop Platform Shell | apps/desktop; ui/framework; ui/rendering | A01; A02; A11; UI feasibility approval | [ ] Planned | Not run | Real native window/event loop, accessible input/text/IME, DPI and clipping; working commands first; capture capability diagnostics and idle invalidation tests. | Proposed; not accepted |
| GW-A17-EVIDENCE | Desktop Platform Shell: tests and handover | tests; docs/checkpoints/GW-A17.md; relevant ADRs | GW-A17-IMPL | [ ] Planned | Not run | Add relevant failure/security tests; run available host/build checks; update invariant map, TODO, changes, limits and cumulative ZIP/SHA. | Proposed; not accepted |
| GW-A18-IMPL | Messaging & Community UI | ui/components; ui/workspaces; ui/themes | A10; A12; A13; A14; A17 | [ ] Planned | Not run | Wire actual services to virtualized histories/workspaces; privacy/lock/key warnings; no inert feature buttons; accessibility and two-user acceptance. | Proposed; not accepted |
| GW-A18-EVIDENCE | Messaging & Community UI: tests and handover | tests; docs/checkpoints/GW-A18.md; relevant ADRs | GW-A18-IMPL | [ ] Planned | Not run | Add relevant failure/security tests; run available host/build checks; update invariant map, TODO, changes, limits and cumulative ZIP/SHA. | Proposed; not accepted |
| GW-A19-IMPL | Voice, Video & Media | include/ghostwire/media; src/media | A06; A14; A18 | [ ] Planned | Not run | Reviewed codec/media encryption and route policy; devices, mute/permission state, jitter/backpressure, relay-only ICE leakage and real call tests. | Proposed; not accepted |
| GW-A19-EVIDENCE | Voice, Video & Media: tests and handover | tests; docs/checkpoints/GW-A19.md; relevant ADRs | GW-A19-IMPL | [ ] Planned | Not run | Add relevant failure/security tests; run available host/build checks; update invariant map, TODO, changes, limits and cumulative ZIP/SHA. | Proposed; not accepted |
| GW-A20-IMPL | AI Agent Framework | include/ghostwire/ai; src/ai | A14; A18; AUTH-PRE | [ ] Planned | Not run | Optional local/remote providers, explicit agent membership/context/tool scopes, revocation, provider disclosure and injection/privacy tests. | Proposed; not accepted |
| GW-A20-EVIDENCE | AI Agent Framework: tests and handover | tests; docs/checkpoints/GW-A20.md; relevant ADRs | GW-A20-IMPL | [ ] Planned | Not run | Add relevant failure/security tests; run available host/build checks; update invariant map, TODO, changes, limits and cumulative ZIP/SHA. | Proposed; not accepted |
| GW-A21-IMPL | Permissions & Moderation | src/permissions; include/ghostwire/moderation; src/moderation | AUTH-PRE; A13; A20 | [ ] Planned | Not run | Extend base authorization with accountable conduct rules, scoped evidence, quarantine/rate limits/review/audited bans/appeals; no sensitive-trait punishment. | Proposed; not accepted |
| GW-A21-EVIDENCE | Permissions & Moderation: tests and handover | tests; docs/checkpoints/GW-A21.md; relevant ADRs | GW-A21-IMPL | [ ] Planned | Not run | Add relevant failure/security tests; run available host/build checks; update invariant map, TODO, changes, limits and cumulative ZIP/SHA. | Proposed; not accepted |
| GW-A22-IMPL | Plugins & Extensions | include/ghostwire/plugins; src/plugins | A21; facade/ABI security review | [ ] Planned | Not run | Versioned capability bridge, sandbox/quotas, installation trust and revocation; malicious plugin cannot bypass routing/vault/AI scopes. | Proposed; not accepted |
| GW-A22-EVIDENCE | Plugins & Extensions: tests and handover | tests; docs/checkpoints/GW-A22.md; relevant ADRs | GW-A22-IMPL | [ ] Planned | Not run | Add relevant failure/security tests; run available host/build checks; update invariant map, TODO, changes, limits and cumulative ZIP/SHA. | Proposed; not accepted |
| GW-A23-IMPL | Resilience & Recovery | tests/integration; tests/security; src/core; src/vault | A09; A11; A16; A19 | [ ] Planned | Not run | Partition/reconnect/disk-full/crash/clock-skew/key-loss restore drills; developer-disappearance acceptance with independent peers/relays/storage. | Proposed; not accepted |
| GW-A23-EVIDENCE | Resilience & Recovery: tests and handover | tests; docs/checkpoints/GW-A23.md; relevant ADRs | GW-A23-IMPL | [ ] Planned | Not run | Add relevant failure/security tests; run available host/build checks; update invariant map, TODO, changes, limits and cumulative ZIP/SHA. | Proposed; not accepted |
| GW-A24-IMPL | Cross-Platform Security Audit | tests/security; docs/security; dependency inventory | A03–A23 relevant features | [ ] Planned | Not run | Independent review, fuzz/interop/penetration evidence, dependency/license audit, host-specific leak/storage/capture gaps resolved or explicitly limited. | Proposed; not accepted |
| GW-A24-EVIDENCE | Cross-Platform Security Audit: tests and handover | tests; docs/checkpoints/GW-A24.md; relevant ADRs | GW-A24-IMPL | [ ] Planned | Not run | Add relevant failure/security tests; run available host/build checks; update invariant map, TODO, changes, limits and cumulative ZIP/SHA. | Proposed; not accepted |
| GW-A25-IMPL | Performance & Release Baseline | benchmarks; tools/packaging; docs/release | A23; A24; all release blockers closed | [ ] Planned | Not run | Defined workloads on ordinary hardware; idle/UI/IO memory/backpressure evidence; installer/update provenance and three-OS release/restore acceptance. | Proposed; not accepted |
| GW-A25-EVIDENCE | Performance & Release Baseline: tests and handover | tests; docs/checkpoints/GW-A25.md; relevant ADRs | GW-A25-IMPL | [ ] Planned | Not run | Add relevant failure/security tests; run available host/build checks; update invariant map, TODO, changes, limits and cumulative ZIP/SHA. | Proposed; not accepted |

## HF02 repository/macOS CI repair — supplied input evidence (2026-10-12)

| ID | Work | Current evidence | Acceptance needed |
|---|---|---|---|
| GW-A01-HF01-WIN-ACCEPTED | Owner portable Windows Debug/Release and Unicode tooling | [x] 34/34 CTest each; original TEMP/TMP, no manual workaround; see HOST-ACCEPTANCE | Accepted only for portable Windows 10 x64; owner vcpkg remains separate |
| GW-A01-HF02-CLANG | Explicit unsigned-byte validation in three core loops | [x] Preserved; supplied AppleClang21 CI and current GCC builds pass | Behavior and warnings unchanged |
| GW-A01-HF02-MACOS-CI | C++20 macOS standard library, ARM64/Intel runner coverage | [x] Actual four hosted Debug/Release jobs pass at 7137505 | New A02 native code still requires a rerun |
| GW-A01-HF02-REPO | README, GitHub instructions, checkpoint inventory | [x] Actual supplied commit checked and preserved | Prior repository work is input, not A02 hosted validation |

## Status discipline

No future feature is complete because its header is proposed. A00 records remain
historical evidence. A01 continuation was explicitly authorized, which does not
prove the prior native acceptance gates passed. A01 source and Linux test evidence
are implemented deliverables; owner acceptance and native platform gates remain
open. Do not mark the security architecture
“validated” from bootstrap tests. Move accepted, implemented and validated work
into COMPLETED with evidence; leave unresolved constraints visible here.

## A01 acceptance and follow-up gates

| ID | Remaining work | Status and acceptance evidence required |
|---|---|---|
| GW-A01-WIN | Windows 10 x64 / VS2026 MSVC | Owner passed HF01 portable Debug/Release 34/34 CTest each in normal PowerShell with original TEMP/TMP, plus CLI launch. Previous ASCII-TEMP 29/29 results are historical. |
| GW-A01-MAC | macOS x64 and ARM64 | Initial macos-14 failure is historical; supplied HF02 macOS26/AppleClang21 ARM64/Intel Debug/Release now pass 33/33 each. A02 native changes unexecuted here. |
| GW-A01-LEAK | Full LeakSanitizer | Prior HF02 default hosted ASan/UBSan workflow passed 33/33. A02 default local run remains blocked by /proc task inspection; leak-disabled success is separate. Full A02 leak gate remains open. |
| GW-A01-TIDY | clang-tidy | Tool unavailable here; run configured analysis on a supported compiler/tooling host. Compiler warnings-as-errors and boundary checks passed separately. |
| GW-A01-CI | Hosted workflows | Actual Linux, Windows and macOS runs all passed at supplied commit 7137505; exact job evidence in GW-A02-INPUT-EVIDENCE.md. No A02 run or push performed. |
| GW-A01-ACCEPT | Checkpoint acceptance | Owner accepted A01 functionality/cooperative limits and HF01 Windows portable original TEMP/TMP; A02 authorized. Supplied HF02 repository repairs preserved with prior hosted evidence. No owner-local vcpkg success inferred. |
| GW-FUTURE-PROPOSALS | Web, native AI, model ecosystem, training, AI security, media and GPU policies | Planned only; IDs and scope/gates in docs/architecture/FUTURE_FEATURES.md. No implementation authorization implied. |

## A01 HF01 recovery

The A00/A01 rows above retain their historical evidence; the following records
control the current recovery gate. Source: the owner's 2026-10-12 Windows report
and the exact supplied A01 ZIP, SHA-256 recorded in GW-A01-HF01.md.

| ID | Work | Implementation / validation | Acceptance |
|---|---|---|---|
| GW-A01-HF01-UTF8 | Remove native narrow fixture serialization; retain native fs paths | [x] Implemented; Linux Debug/Release and vcpkg Debug pass 33 CTest checks; 67 unit cases / 640 assertions; ASan/UBSan with leaks disabled | Windows original-TEMP owner validation passed 34/34 each |
| GW-A01-HF01-REGRESS | Unicode profile shape, ö/spaces/default temp, config/CLI, invalid bytes | [x] Two new portable cases, native Windows UTF-16 case, four real executable Unicode tests; production UTF-8 validator unchanged | Windows actually passed 34/34 CTest each (unit case counts not independently posted) |
| GW-A01-HF01-TOOLS | CMake/CTest pairing, explicit overrides, Unicode environment, x64 diagnostics and fresh configure | [x] Source implemented and owner native tests passed; 18 native tooling checks registered | Owner Windows PowerShell 5.1/MSVC execution passed 34/34 CTest each |
| GW-A01-HF01-VCPKG | Windows doctest 2.4.12 configure failure | [!] Unresolved owner report; pins/modes preserved; Linux vcpkg passing does not resolve Windows failure | Capture full configure/buildtree logs and pass Debug/Release + CTest before closure |
| GW-A01-HF01-ACCEPT | Original Windows TEMP/TMP and ordinary-shell acceptance | [x] Owner procedure in checklist; results recorded in GW-A01-HF01-HOST-ACCEPTANCE.md | Windows portable Debug/Release accepted; original TEMP/TMP |

The prior HF02 macOS matrix and hosted runs now have evidence; A02 Windows owner acceptance is now recorded. Native macOS, full local leak
checks, clang-tidy and new hosted CI remain incomplete.
No new runtime scope, private persistence or authorization exception is granted.

## A02 remaining acceptance gates

| ID | Remaining work | Current status |
|---|---|---|
| GW-A02-WIN | Windows10 x64 / VS2026 Debug/Release with original TEMP/TMP | [x] Owner reports clean builds and 35/35 each, platform/Unicode/tooling pass, CLI exit 0; win10dev account |
| GW-A02-MAC | macOS x64/ARM64 Debug/Release | [ ] New 34-check suite including spawn/group/stop/Keychain API probe unexecuted; prior HF02 CI is not A02 evidence |
| GW-A02-CI | Hosted build/test matrix for A02 source | [ ] No remote changes/runs performed; user controls publication |
| GW-A02-LEAK | Default full A02 LeakSanitizer | [!] Local /proc task inspection failure; separate leak-disabled ASan/UBSan passes; supported host required |
| GW-A02-TIDY | clang-tidy | [!] Tool unavailable locally; compiler warnings-as-errors, format and boundaries are separate checks |
| GW-A02-ACCEPT | Independent review and owner Windows acceptance | [x] Owner accepted A02 and explicitly authorized A03; see acceptance addendum |
| GW-A01-HF01-VCPKG-LOCAL | Owner's Windows vcpkg/doctest configuration | [!] Still unresolved on that host; preserve both routes and gather actual buildtree logs |

A02 does not hard-limit child CPU/RSS, sandbox hostile code, guarantee hard-kill
or OS-stall cleanup deadlines, or implement secret persistence. These limits are
explicit in PLATFORM_ABSTRACTIONS.md and do not weaken A11-PRE/AUTH-PRE.

## A03 remaining acceptance gates

| ID | Gate | State |
|---|---|---|
| GW-A03-WIN | Windows10 / VS2026 / win10dev, original TEMP/TMP | [x] A03 clean portable Debug/Release 37/37 each plus CLI identities/help; HF01 retest remains separate |
| GW-A03-MAC | macOS x64/ARM64 | [!] Actual A03 Debug/Release on both architectures: 34/36, platform-unit and crypto-unit failed; HF01 expected 37/37 needs native execution |
| GW-A03-VCPKG-WIN | Owner standard vcpkg/doctest route | [!] Still unresolved; new account and Linux success do not close this issue |
| GW-A03-LEAK | Default sanitizer workload / local LSan | [~] A03 hosted default ASan/UBSan passed 37/37 without suppression; local LSan restriction and exhaustive leak audit remain separate |
| GW-A03-TIDY | clang-tidy | [!] Unavailable locally; warnings/format/boundaries are different checks |
| GW-A03-CI | Hosted A03 workflows | [x] Exact 95c9128 jobs inspected: Linux/Windows pass, macOS fails; HF01 not pushed/run; user owns pushes |
| GW-A03-REVIEW | Independent crypto facade/native review and owner acceptance | [~] Owner Windows acceptance received; native recovery and independent crypto review remain; no production E2EE claim |

Nonce uniqueness, public-message-only Ed25519, best-effort memory locking,
cooperative cancellation and logical (not RSS) budgets are explicit current
contracts. Protocol selection remains unresolved despite primitive-provider
selection. A11-PRE and AUTH-PRE remain mandatory and unimplemented. Identity,
profile collision/namespace decisions and persistent storage are not smuggled
into this checkpoint. Proposed continuation: GW-A04-PROMPT.md.
