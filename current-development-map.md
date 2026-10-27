# GhostWire — Current Development Map

> **Living project map · 2026-10-27 · baseline `GW-A05-P04-HF09-MSVC-EXCEPTION-CONTRACT`**  
> **Purpose:** See what works, what is missing, what is blocked, which functions/files need attention, and who actually owns each task.  
> **Placement:** Repository root, `current-development-map.md`. This is a planning/coordination document, **not** executable code or a replacement for the authoritative JSON registries.

## 0. How to read this map

- **VERIFIED:** Implemented with recorded tests and owner acceptance where stated.
- **PARTIAL:** Some real code and tests exist; whole module is not approved.
- **TODO:** Work identified, not implemented.
- **READY:** Dependencies allow the scoped work to begin; assignment is a separate question.
- **ASSIGNED:** Someone explicitly accepted ownership; does not imply code has started.
- **IN_PROGRESS:** Work actually underway in a task/worktree.
- **REVIEW:** Code delivered but needs integration/security/owner acceptance.
- **BLOCKED:** A prerequisite, external source or required review is missing.
- **DEFERRED:** Intentionally not authorized yet.
- **PROPOSED:** Candidate work; neither authorized nor assigned.

**Ownership is literal.** A named workstream or `owner` in an old JSON record describes its *historical responsibility*, not a currently running agent. At this snapshot, **no independent AI coding worker is confirmed active**. The assistant acts as the planning/implementation coordinator when the user starts a development turn; the user is the Windows acceptance owner. Proposed workers below are **not** claimed to be running.

## 1. Baseline and acceptance evidence

| Item | Current status |
|---|---|
| Authoritative source | `GHOSTWIRE-GW-A05-P04-HF09-MSVC-EXCEPTION-CONTRACT-source.zip` |
| Source ZIP SHA-256 | `5ebfa176a50ba19609b6bdb56d3d67c34fda440e87306956d4a43b17ae2279b2` |
| Embedded source manifest | **295/295 SHA-256 entries verified** in the HF09 archive |
| Runtime version | **0.4.1**; no new production network capability |
| Windows 10% gate | **Owner accepted** on 2026-10-27: Windows 10, VS2026, MSVC 19.51 x64; Debug **66/66 build, 50/50 tests**; Release **66/66 build, 50/50 tests** |
| Former Windows `protocol-allocation-fault` timeout | **Resolved in HF09 native acceptance**; test passed in **0.03 s** in Debug and Release |
| Linux HF09 | GCC Debug **52/52**, Release **52/52** in recorded incremental full suites; not an HF09 fresh-clean-build claim |
| Provisional weighted floor | **11/110 = 10%** for owner-accepted A00–A04 under the *unapproved provisional* effort model |
| Official project completion field | Remains **`null`** in `MODULE_REGISTRY.json` until methodology/criteria are agreed |
| GitHub `main` | **Push not verified by this map**. HF09 is eligible as accepted source baseline; do not claim a push/tag occurred without a Git confirmation |
| Security posture | **Not ready for untrusted network inputs or secure-messaging claims**; actual pinned TinyCBOR interoperability and independent parser/security review remain open |

**Registry-sync TODO:** HF09 archive predates the owner's Windows success. Its `GW-A05-WINDOWS-10PCT-HF09` task and HF09 validation documents still say Windows testing is pending. Update *those evidence records* at the next documentation/checkpoint commit. Do not edit the old accepted ZIP retroactively. Also distinguish the 10% **Windows validation milestone** from approval of the incomplete A05 protocol module.

## 2. High-level development stages (dependency-aware, not fixed calendar dates)

This is a *visual roadmap*, not permission to skip an earlier dependency. Parts of later stages may be designed in parallel but cannot ship early.

| Stage | Modules | Current stage state | Main outcome |
|---|---|---|---|
| **S0 — Portable foundation** | A00 Foundation; A01 Core lifecycle; A02 Platform; A03 Crypto; A04 Identity/device trust | **VERIFIED / accepted** | Portable C++20 scaffold, bounded services, OS adapters, pinned Botan, volatile isolated cryptographic profiles |
| **S1 — Wire protocol security** | A05 Protocol schemas/codec | **PARTIAL; security gates BLOCKED** | Canonical bounded CBOR, real independent upstream differential results, reviewable parser decision |
| **S2 — Prerequisite safety** | Early **A11-PRE** protected-key-store slice; **AUTH-PRE** base authorization | **TODO / prerequisites** | Prevent insecure identity persistence and unauthorized network actions; full A11 vault remains a larger module |
| **S3 — Connectivity** | A06 Transport/privacy; A07 Autonomous discovery | **A06 DEFERRED; A07 TODO** | Authenticated bounded transport and privacy controls, then signed opt-in discovery |
| **S4 — Private communication** | A08 E2EE direct messaging; A09 Offline delivery; A10 Friends; A11 full Vault; A12 Vault protection | **TODO** | Independently reviewed E2EE, protected state, ciphertext mailboxes, private contacts |
| **S5 — Communities** | A13 Community server; A14 Group encryption; A21 Permissions/moderation expansion | **TODO** | Community roles/channels, membership epochs, auditable moderation (on top of AUTH-PRE) |
| **S6 — Usable desktop** | A17 Desktop shell; A18 Messaging/community UI | **TODO** | Real window/events, accessible text, actual service wiring, no inert interface buttons |
| **S7 — Advanced capabilities** | A15 Encrypted files; A16 Federation; A19 Voice/video; A20 Agent framework; A22 Plugins | **TODO** | Optional extended capabilities with scoped data/privacy and per-agent authorization |
| **S8 — Hardening/release** | A23 Resilience; A24 Cross-platform audit; A25 Performance/release | **TODO** | Recovery, security qualification, performance budgets, release readiness |

**Dependency notes:** The full roadmap requires reviewed key storage before persistent A04/A08 state, authorization before A06 listeners/A13 admin, a reviewed E2EE/MLS library before A08/A14, and UI feasibility before desktop expansion. The simplified module DAG and the narrative `ROADMAP.md` do not express every same dependency (notably some A10/A17/A20/A21 gates): **reconcile them explicitly before scheduling those modules; never infer that omitted registry edges grant implementation permission.**

### 2a. Entire module inventory (A00–A25)

| Module | Implementation | Next action / dependency |
|---|---|---|
| A00 Repository/build | VERIFIED | Preserve known-good portable presets, source hash and CI contracts |
| A01 Lifecycle/services | VERIFIED | Keep cleanup/admission regressions |
| A02 Platform abstractions | VERIFIED | Preserve Unicode, native process and stop behavior |
| A03 Crypto provider | VERIFIED | Independent crypto review remains release-level work |
| A04 Identity/device trust | VERIFIED **volatile scope only** | Persistent identities require A11-PRE |
| A05 Protocol | PARTIAL | Verified upstream TinyCBOR + independent parser review |
| A06 Transport/privacy | DEFERRED | A05 approved, AUTH-PRE; no network listener yet |
| A07 Discovery | TODO | A04+A05+A06; signed/expiring records |
| A08 Direct E2EE | TODO | A04+A05+A06+A11-PRE + reviewed E2EE protocol library |
| A09 Offline mailboxes | TODO | A08, quotas and reachable storage |
| A10 Friends/trust UI | TODO | A04, discovery/communication and protected store; reconcile DAG |
| A11 Local encrypted vault | TODO | Protected-store minimum first; full crash-safe encrypted state later |
| A12 Device/vault protection | TODO | A11 and explicit local recovery semantics |
| A13 Community runtime | TODO | A06, A09, base authorization |
| A14 Group membership encryption | TODO | A08, A13, reviewed MLS/equivalent, protected state |
| A15 Encrypted file transfer | TODO | A06, A11, A14 and quotas |
| A16 Federation/portability | TODO | A13, A14, A15; check roadmap/registry edge consistency |
| A17 Native desktop shell | TODO | A01/A02 plus UI feasibility and secure state integration |
| A18 Messaging/community UI | TODO | Working messaging/community services and A17 |
| A19 Voice/video/media | TODO | A06, A14, A18 and privacy-safe media routing |
| A20 Agent framework | TODO | Agent identity/context scopes, permission base and applicable messaging/UI services |
| A21 Advanced moderation | TODO | AUTH-PRE first, then community/member/agent policy as required |
| A22 Plugins/extensions | TODO | Secure plugin facade + UI, permissions and reviewed boundaries |
| A23 Recovery/resilience | TODO | Durable services, atomic state and actual failure/recovery tests |
| A24 Security audit | TODO | Network, E2EE, key management, membership and cross-platform review |
| A25 Performance/release | TODO | Working feature integration, A23/A24 qualification |

## 3. Current concrete functions, files and work items

### 3.1 Existing, verified implementations — **do not rewrite**

| Source ownership | What exists now | Status |
|---|---|---|
| `src/core/*`, `include/ghostwire/core/*` | Config/bootstrap, service graph/lifecycle, clock, logger, cancellation/admission limits | VERIFIED A00/A01 |
| `src/platform/*`, `include/ghostwire/platform/*` | Native process/stop lifecycle, paths, arguments, OS capability probes | VERIFIED A02 |
| `src/crypto/crypto_provider.cpp`, `include/ghostwire/crypto/*` | Pinned Botan provider and bounded volatile crypto/secret handles | VERIFIED A03 scope |
| `src/identity/*`, `src/composition/identity_service_adapter.cpp` | Isolated volatile profiles, public DeviceSet, device trust/membership and lifecycle | VERIFIED A04 scope |
| `apps/cli/identity_session.cpp` | Explicit local, bounded identity-session workflow | VERIFIED A04 scope |
| `src/protocol/wire_codec.cpp`, `include/ghostwire/protocol/wire_codec.hpp` | `encode_frame`, `decode_frame`, `selected_was_offered`, admission, budgets, deadline/cancel; bounded public-only wire v1.0 | PARTIAL **A05 module**, but current codec tests verified |
| `tests/fixtures/protocol/allocation_fault.cpp` | MSVC-compatible *test-only* injected allocation failure; stronger global allocator regression retained Linux-only | VERIFIED HF09 Windows + Linux |
| `tools/protocol/*` | Pinned TinyCBOR fetch/import guards, C++/Python differential checker and test probes | Tools VERIFIED; **actual upstream comparison NOT YET EXECUTED** |

### 3.2 Next work queue, in dependency order

| ID / priority | Owner **now** | Proposed workstream | Files/functions to work on | Done only when… |
|---|---|---|---|---|
| **GW-DOC-WIN-HF09** · small / READY | **Unassigned** | `project-coordination` | `docs/development/MODULE_REGISTRY.json`, `WORKER_QUEUE.json`, HF09 acceptance evidence | Windows gate says **accepted**, full 50/50 x2 evidence recorded, JSON validation passes; A05 security gate stays open |
| **GW-A05-UPSTREAM-INTEROP** · critical / READY for verified-source acquisition, otherwise BLOCKED | **Unassigned** (historical registry label: `protocol-review`) | `protocol-interop` | `tools/protocol/fetch_tinycbor.py`, `import_tinycbor.py`, `differential_check.py`; test-only upstream build/probe in `tests/interop` | Fetch **pinned** upstream TinyCBOR, verify all pinned hashes, build original upstream C library, execute real positive/negative/frozen/generated vectors on Windows/Linux; distinguish generic canonical CBOR from GhostWire schema checks |
| **GW-A05-PARSER-DECISION** · critical / BLOCKED | **Unassigned** (historical `security-review`) | `security-review` | `docs/decisions/`, `docs/security/PROTOCOL_TEST_PLAN.md`, `src/protocol/wire_codec.cpp`, tests/fuzz | Independent review of length arithmetic, resource ceilings, exceptional paths, corpus/rejection behavior; written retain/replace decision with threats and evidence; requires real upstream results |
| **GW-DEV-TOOLCHAIN** · suggested / READY | **Unassigned** | `build-tooling` | `CMakePresets.json`, `CMakeLists.txt`, `tools/windows/dev-windows.ps1`, Windows tooling tests | x86 compiler rejected clearly; missing Python-backed tests cannot silently reduce suite to 42; robust x64 VS CMake/CTest/Python detection; no hardcoded user paths; Windows regression verified at next gate |
| **GW-AUTH-PRE** · design-only until scope approved | **Unassigned** | `authorization-design` | Proposed `include/ghostwire/authorization/*`, service/action tests; no implementation path committed | Explicit least-privilege action/resource model, identity-vs-alias isolation, denial and revocation tests, security review **before** A06/A13 |
| **GW-A11-PRE** · design-only until scope approved | **Unassigned** | `protected-storage-design` | `include/ghostwire/platform/secure_storage.hpp`, proposed protected key-store interfaces, crash/permissions tests | Threat-modeled protected store with atomicity and no plaintext fallback; separately accepted **before** private-key persistence |
| **GW-A06-TRANSPORT** · DEFERRED | **Unassigned** | `transport` **not activated** | Proposed `include/ghostwire/transport/*`, `src/transport/*` | Begin only after A05 parser acceptance and AUTH-PRE approval; independent privacy/leak/backpressure/security tests |

**Do not create empty placeholder classes to mark a task as done.** The exact candidate function signatures for A06/A11/A20 should be designed at the relevant dependency gate; names above are work areas, not claims that those functions exist.

### 3.3 Suggested parallel tasks (when actual workers are available)

| Lane | Proposed alias | Independent files | Can run now? |
|---|---|---|---|
| 1 — external interop | `agent-interop-01` | `tools/protocol/`, `tests/interop/` | **Yes**, once verified upstream access is available; no production codec changes without review |
| 2 — Windows tooling | `agent-tooling-01` | `tools/windows/`, presets/toolchain tests | **Yes**, while A05 interop runs; CMake edits require coordinated ownership |
| 3 — threat review | `agent-security-01` | `docs/security/`, `docs/decisions/` | **Review prep yes**; final decision waits for lane 1 and *independent* security review evidence |
| 4 — coordination/evidence | `agent-registry-01` | `current-development-map.md`, `docs/development/*` | **Yes**, sync completed Windows acceptance now |
| 5 — transport | `agent-transport-01` | Future `src/transport/` | **No**, blocked; do not assign implementation yet |

The aliases above are **PROPOSED, not currently running agents**. Separate branches/worktrees, one file owner per change, independent review, deterministic tests, merge and verified manifest are required. If true parallel agents are unavailable, label the work *coordinator-authored* rather than pretending parallel models ran.

## 4. Source comment convention (only for files being changed)

Use small comments on the touched header, implementation file, or immediately before a pending function. Track *task ID + status + factual owner*; do **not** plaster `TODO` across already completed code.

```cpp
// GW-TASK: GW-A05-UPSTREAM-INTEROP
// GW-STATUS: TODO           // ASSIGNED | IN_PROGRESS | REVIEW | VERIFIED | BLOCKED
// GW-OWNER: unassigned     // change only after a real worker accepts the task
// GW-NEXT: Verify actual pinned upstream-C comparison before parser approval.
```

For function-specific follow-ups:

```cpp
// TODO(GW-A05-PARSER-DECISION): Review rejection/resource proof against verified
// upstream results before authorizing untrusted network input.
```

On completion: replace stale TODO comments with a short `VERIFIED` reference or remove temporary coordination comments; record evidence in the registry and checkpoint docs. The **registry remains authoritative for machine-readable status**, the Markdown map is the human-readable overview, and source comments point to a concrete task.

## 5. Operational policy and quality gates

1. Begin every task with the accepted source and a verified `SOURCE_MANIFEST.json`; inventory code, TODOs and tests before making changes. Do not restart finished modules.
2. Assign a bounded change to an actual owner, with isolated source/worktree; keep the assignment table truthful as workers start, stop, or hand off.
3. Write missing functionality only; test normal/failure/cancel/boundary paths. Preserve strict profile isolation, per-membership cryptographic authority and alias-collision separation.
4. Require **engineering confidence >0.80** with named evidence before integrating; this **never substitutes** for native Windows, external interoperability or independent security audit.
5. Record changed files, test counts/platforms, known limits, security decisions and dependencies. Regenerate manifest and cumulative source ZIP only when delivering an accepted development checkpoint.
6. GitHub `main` is for accepted baselines; use `develop`/scoped branches or an agreed equivalent for work in progress. Do not push without the owner's instruction.
7. Prefer owner-operated Windows acceptance at the next **20% verified weighted milestone**; additional critical security checks are permitted. A05 review is required before A06, regardless of percentage.
8. Do not implement live networking, persistent private keys, E2EE claims, privileged remote authorization, GUI stubs, or A06 transport merely because a roadmap row exists.

## 6. Suggestions / architectural backlog (not assignments)

- **Automated stage dashboard:** Generate parts of this Markdown from `MODULE_REGISTRY.json` and `WORKER_QUEUE.json` at checkpoint time; keep manual narrative for priorities and product choices.
- **Strict test registration:** If Python is required for the checkpoint security suite, fail CMake configuration with an actionable diagnostic instead of reporting a misleading 42/42 success.
- **One-click Windows tooling:** Discover Visual Studio x64 developer environment, bundled CMake/CTest and real Python under the *current account*; reject Windows Store execution aliases as a discovered Python interpreter. Avoid permanent user-machine changes by default.
- **Protocol interoperability evidence:** Store upstream commit ID, pinned file hashes, executable build commands, frozen-vector verdicts and machine-readable comparison results alongside parser review.
- **Agent-to-agent design:** Treat future agents as separately authorized participants with scoped community membership, read/understood/continuing acknowledgements and request/reply semantics. Keep model training/evaluation/mutation in **CppDnnGui**, outside GhostWire's transport layer.
- **Early security threat review:** Authorization and protected storage can be *designed* alongside current A05 work without prematurely committing runtime network/storage implementation.

## 7. Canonical documentation links and update checklist

- [Full roadmap](ROADMAP.md) · [Architecture authority](ARCHITECTURE.md) · [Security invariants](docs/security/SECURITY_INVARIANTS.md)
- [Machine module registry](docs/development/MODULE_REGISTRY.json) · [Worker queue](docs/development/WORKER_QUEUE.json) · [Integration ownership](docs/development/WORKER_INTEGRATION.json)
- [Effort model](docs/development/PROJECT_EFFORT_FORECAST.json) · [Structure rules](docs/development/STRUCTURE_RULES.md) · [HF09 checkpoint](docs/checkpoints/GW-A05-P04-HF09.md)

**At every accepted delivery:**

- [ ] Verify baseline source and manifest before editing.
- [ ] Update this map's date, baseline and stage changes.
- [ ] Mark only *real* active workers; distinguish proposed vs historical owners.
- [ ] Keep `MODULE_REGISTRY.json` and `WORKER_QUEUE.json` in sync with evidence.
- [ ] List changed functions/files and concrete tests, including blocked ones.
- [ ] Preserve security gates; avoid inventing readiness percentages or release ETA.
- [ ] Package **one cumulative source** with updated manifest and SHA-256 at the actual milestone; confirm GitHub commit/tag before referencing it as pushed.

---

*Initial map authored from the actual HF09 cumulative source manifest, roadmap and JSON registries, plus the owner's subsequent Windows 50/50 Debug and 50/50 Release acceptance logs. This new Markdown file does not alter the accepted HF09 source ZIP or claim new code/tests ran.*
