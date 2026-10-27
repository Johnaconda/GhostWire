# GhostWire implementation roadmap

> **Current as of 2026-10-27 — accepted `GW-A05-P04-HF09` Windows 10% build/test foundation.**
> This roadmap is the dependency and acceptance contract for A00–A25. For actionable functions, ownership and proposed worker assignments, see [current-development-map.md](current-development-map.md). Structured registry records live under [`docs/development/`](docs/development/).

## Current baseline and delivery gates

- **A00–A04:** Accepted portable foundation (build/lifecycle/platform/Botan 3.13.0/volatile identity and device trust); not persistent or connected.
- **A05:** **PARTIAL** strict, in-memory, bounded public CBOR v1.0 schema/codec with security/fault tests. HF09 corrected the MSVC exception-contract fault test; Windows Debug **66/66 build + 50/50 CTest**, and Release **66/66 build + 50/50 CTest** (owner-operated, 2026-10-27). Linux GCC incremental Debug and Release full suites **52/52 each**. HF09 was not a fresh Linux full-build claim.
- **Windows 10% acceptance:** Build/test milestone **ACCEPTED**. The A05 network-input security decision is **not** accepted. Original HF09 archive and its registries predate the owner results; machine-readable acceptance entries must be synchronized in a subsequent documentation/checkpoint commit.
- **A05 gates still open:** Real pinned upstream TinyCBOR execution/comparison and independent parser/security review with a written decision. No untrusted network exposure or A06 listener work before approval.
- **Authoritative source:** Cumulative `GHOSTWIRE-GW-A05-P04-HF09-MSVC-EXCEPTION-CONTRACT-source.zip`; [HF09 checkpoint](docs/checkpoints/GW-A05-P04-HF09.md). GitHub push/tag status is **not verified** by these files.
- **Progress accounting:** Provisional `11/110` weighted-foundation effort ≈ **10%**; overall completion remains `null` in the registry until an approved measurement model. Engineering confidence, test pass count, project completion and milestone acceptance remain separate.

## Stage plan (dependency-ordered, not release dates)

| Stage | Modules / prerequisites | Status | Next verified outcome |
|---|---|---|---|
| S0 — Foundation | A00–A04 | ACCEPTED within volatile scope | Preserve portable build, isolated identity and bounded crypto contracts |
| S1 — Protocol security | A05 | PARTIAL / security review BLOCKED | Actual pinned TinyCBOR interop, resource audit and explicit parser decision |
| S2 — Security prerequisites | AUTH-PRE; A11-PRE | DESIGN TODO | Basic least-privilege authorization and reviewed minimum protected key store |
| S3 — Transport & discovery | A06, A07 | DEFERRED / TODO | Authenticated bounded privacy policy and signed opt-in discovery, only after gates |
| S4 — Private messaging | A08–A12 | TODO | Reviewed E2EE, crash-safe protected state, mailboxes, contacts and vault controls |
| S5 — Communities | A13, A14, A21 | TODO | Community roles/channel runtime, reviewed group encryption, moderation |
| S6 — Desktop interface | A17, A18 | TODO | Real events/accessibility and working service-backed UI, no dummy buttons |
| S7 — Extended capabilities | A15, A16, A19, A20, A22 | TODO | File transfer, federation, media, agent participation and secure extensions |
| S8 — Qualification | A23–A25 | TODO | Recovery drills, independent audit, cross-platform release/benchmark evidence |

## Dependency-ready task queue and proposed agent lanes

| Priority | Work | Owner status | Gate / verification |
|---|---|---|---|
| 1 | `GW-A05-UPSTREAM-INTEROP` — verified pinned upstream parser comparison | **Unassigned**, proposed `agent-interop-01` | Real upstream build, all frozen/generated differential controls |
| 2 | `GW-A05-PARSER-DECISION` — independent security review | **Unassigned**, proposed `agent-security-01` | External interoperability evidence, explicit reviewed retain/replace decision |
| 3 | `GW-DEV-TOOLCHAIN` — x64 MSVC/Python/CTest fail-fast | **Unassigned**, proposed `agent-tooling-01` | Detect x86 compiler and missing 8 Python tests before acceptance |
| 4 | `GW-DEV-REGISTRY-SYNC` — Windows HF09 evidence synchronization | **Unassigned**, proposed `agent-registry-01` | Align `MODULE_REGISTRY.json`, worker records, checkpoint evidence and this map |
| 5 | `AUTH-PRE`, `A11-PRE` — scoped prerequisite designs | **Unassigned**; design only | Authorize and review before persistent key state or network listener |
| Blocked | `A06` transport implementation | **Not assignable** until gates pass | A05 parser approval and AUTH-PRE; no premature networking |

No independent agent is verified as currently running. Proposed names are scheduling labels, not active-worker claims. For code being changed, use simple task/status/owner comments; put unresolved TODOs only where real functionality remains missing. Use [the development map](current-development-map.md) and the JSON registries to avoid conflicting assignments.

## Historical checkpoint notes (not current gating statements)

- **2026-10-17, A05-P01:** Design-only proposal after A04-P03 Windows 40/40; architecture and ADR-009 precede the implemented codec.
- **2026-10-18, A05-P02/P03:** Strict isolated codec introduced, then hardened with differential/negative tests. Historical source snapshots described Windows acceptance as pending; they are superseded by the native HF09 50/50 evidence.
- **2026-10-19 onward, A05-P04/HF series:** Offline importer/comparison guard, bounded fault injection and Windows compiler/runtime portability recovery. These preparations do *not* constitute actual upstream TinyCBOR interoperability or third-party audit.

## Required ordering gates
- **A05 network-input gate:** A verified, pinned real upstream TinyCBOR comparison and an independent parser/security review with explicit disposition are mandatory **before** A06 transport/network parsing. An offline proxy, synthetic test or successful CTest run is not sufficient.
- **A11-PRE:** a minimum reviewed protected key store must precede persistent A04 identities and A08 session/history state. Full vault UI/policy stays A11/A12.
- **AUTH-PRE:** basic action/resource authorization must precede any A06 listener or A13 admin operation. A21 adds moderation workflows.
- **Protocol-library gate:** reviewed implementation, license/toolchain feasibility and durable state model before A08/A14. Rust FFI may be justified; custom crypto is not a fallback.
- **UI feasibility gate:** accessible native text/input/IME, GPU/idle design and actual event wiring before A17 expands into a full desktop.

A02/A03 plus the protected-store slice may need iteration before A04 persistence. Keep volatile tests explicit; never ship temporary plaintext persistence.

## Checkpoints
| ID | Checkpoint | Dependencies | Planned source | Acceptance | State |
|---|---|---|---|---|---|
| GW-A00 | Repository & Build Foundation | none | apps; src/core; src/platform; cmake; tools | Four targets, strict CLI/config, redacted logging, explicit paths and lifecycle, Debug/Release/CTest, docs and cumulative ZIP. | Foundation preserved through owner-accepted HF01; prior HF02 hosted native evidence recorded separately |
| GW-A01 | Core Lifecycle & Services | A00 source; explicit owner continuation authorization | include/ghostwire/core; src/core | Owned context/clock/services; deterministic dependency startup and rollback; reverse cleanup with cooperative cancellation/deadlines; bounded admission and diagnostics. | Implemented; Linux evidence and owner-reported Windows portable Debug/Release with TEMP workaround. HF01 original-TEMP Windows acceptance passed; cooperative limits accepted. HF02 supplied repository macOS26 CI passes; no owner-local vcpkg resolution inferred. |
| GW-A02 | Platform Abstractions | A01 | include/ghostwire/platform; src/platform | Prove native paths, secure-storage capabilities, owned-process and stop-event APIs on Windows/Linux/macOS; unsupported states explicit. | Owner-accepted; cumulative HF01 native hosted checks now pass on all three OSes. Local full LSan, clang-tidy and older-platform qualification remain separate. |
| GW-A03 | Cryptographic Provider | Accepted A02 | include/ghostwire/crypto; src/crypto | Pinned Botan, bounded volatile RNG/AEAD/public signatures/hash, move-only clearing handles and real lifecycle; no identity/persistence/protocol. | Implemented through accepted HF01. Owner Windows 37/37 each; hosted Windows/macOS 37/37 and Linux 38/38, including default sanitizer workload. Independent crypto review remains pending. |
| GW-A04 | Identity & Device Trust | A03; accepted ADR-008; A11-PRE for persistence | include/ghostwire/identity; src/identity; composition adapter; CLI session | Isolated volatile profiles, scoped membership/device keys, bounded public records, explicit trust, device rotation/revocation and alias isolation. | P03 implemented and accepted by owner; runtime 0.4.1; volatile only. |
| GW-A05 | Protocol Schemas & Versioning | Accepted A04 public identity contracts | schemas; future include/ghostwire/protocol; src/protocol | Canonical versioned codecs with bounded fields and negotiation; reject malformed/unsupported input; corpus/fuzz/interop fixtures. | **PARTIAL:** HF09 strict bounded in-memory codec and test-only interop guards; **owner Windows Debug/Release 50/50 accepted**. Real upstream TinyCBOR comparison and independent parser review **OPEN**; A06 unauthorized. |
| GW-A06 | Transport & Privacy Policies | A05; AUTH-PRE | include/ghostwire/transport; src/transport | Authenticated bounded connection API; fail-closed direct/protected/privacy-first capabilities; timeout/backpressure and DNS/ICE/address-leak harness. | Planned |
| GW-A07 | Autonomous Discovery | A04; A05; A06 | include/ghostwire/discovery; src/discovery | Signed expiring records, bounded queries/caches, manual and independent bootstrap peers; poisoning/Sybil/expiry/leak tests; opt-in LAN. | Planned |
| GW-A08 | Encrypted Direct Messaging | A04; A05; A06; A11-PRE; protocol-library approval | include/ghostwire/messaging; src/messaging | Reviewed async E2EE implementation with multi-device design; atomic protected ratchet state; upstream/interop/replay/crash tests; FS/PCS claims reviewed. | Blocked pending crypto protocol and vault gates |
| GW-A09 | Offline Delivery & Mailboxes | A08; storage quotas | src/messaging; src/storage; apps/server | Ciphertext mailbox retention, authorization, dedup/acks/retry/expiry and disk/bandwidth quotas; no delivery claim without reachable storage. | Planned |
| GW-A10 | Friends & Contact Verification | A04; A07; A08; A11-PRE | include/ghostwire/friends; src/friends | Independent private contact graph, explicit trust/key-change warnings and verification UX; no community prerequisite. | Planned |
| GW-A11 | Local Encrypted Vault | A02; A03; A11-PRE accepted | include/ghostwire/vault; src/vault | Complete encrypted history/search/cache/backups, migration, atomic writes, lock/recovery; inspect disk/temp/WAL/journals and crash behavior. | Planned; minimum slice pulled forward |
| GW-A12 | Vault Protection & Device Privacy | A11; A04 | src/vault; src/platform; tests/security | Local configurable retry/backoff/recovery, rollback/DoS tests, deliberate erasure scope, no remote wipe, local-only capabilities; no offline-counter claim. | Planned |
| GW-A13 | Community Server Runtime | A06; A09; AUTH-PRE | apps/server; include/ghostwire/communities; src/communities | Headless local/dedicated community identity, invites/password policy, roles/channels, backup and lifecycle; admin not E2EE member by default. | Planned |
| GW-A14 | Groups & Membership Encryption | A04; A08; A11; A13; MLS-library approval | include/ghostwire/groups; src/groups | Reviewed MLS/equivalent epochs, join/leave/revocation/history policy; membership and replay interop; removed users cannot decrypt future epochs. | Planned |
| GW-A15 | Encrypted File Distribution | A06; A11; A14; storage quotas | include/ghostwire/files; src/files; src/storage | Authenticated encrypted chunks/manifests, resumability, cooperative seeding, scoped access/password wrapping, large-file safe arithmetic and traversal/DoS/leak tests. | Planned |
| GW-A16 | Federation & Portability | A13; A14; A15 | include/ghostwire/federation; src/federation | Authenticated versioned host exchange and encrypted export/import preserving identity/trust; migration and conflict/partition tests. | Planned |
| GW-A17 | Desktop Platform Shell | A01; A02; A11; UI feasibility approval | apps/desktop; ui/framework; ui/rendering | Real native window/event loop, accessible input/text/IME, DPI and clipping; working commands first; capture capability diagnostics and idle invalidation tests. | Planned |
| GW-A18 | Messaging & Community UI | A10; A12; A13; A14; A17 | ui/components; ui/workspaces; ui/themes | Wire actual services to virtualized histories/workspaces; privacy/lock/key warnings; no inert feature buttons; accessibility and two-user acceptance. | Planned |
| GW-A19 | Voice, Video & Media | A06; A14; A18 | include/ghostwire/media; src/media | Reviewed codec/media encryption and route policy; devices, mute/permission state, jitter/backpressure, relay-only ICE leakage and real call tests. | Planned |
| GW-A20 | AI Agent Framework | A14; A18; AUTH-PRE | include/ghostwire/ai; src/ai | Optional local/remote providers, explicit agent membership/context/tool scopes, revocation, provider disclosure and injection/privacy tests. | Planned |
| GW-A21 | Permissions & Moderation | AUTH-PRE; A13; A20 | src/permissions; include/ghostwire/moderation; src/moderation | Extend base authorization with accountable conduct rules, scoped evidence, quarantine/rate limits/review/audited bans/appeals; no sensitive-trait punishment. | Planned |
| GW-A22 | Plugins & Extensions | A21; facade/ABI security review | include/ghostwire/plugins; src/plugins | Versioned capability bridge, sandbox/quotas, installation trust and revocation; malicious plugin cannot bypass routing/vault/AI scopes. | Planned |
| GW-A23 | Resilience & Recovery | A09; A11; A16; A19 | tests/integration; tests/security; src/core; src/vault | Partition/reconnect/disk-full/crash/clock-skew/key-loss restore drills; developer-disappearance acceptance with independent peers/relays/storage. | Planned |
| GW-A24 | Cross-Platform Security Audit | A03–A23 relevant features | tests/security; docs/security; dependency inventory | Independent review, fuzz/interop/penetration evidence, dependency/license audit, host-specific leak/storage/capture gaps resolved or explicitly limited. | Planned |
| GW-A25 | Performance & Release Baseline | A23; A24; all release blockers closed | benchmarks; tools/packaging; docs/release | Defined workloads on ordinary hardware; idle/UI/IO memory/backpressure evidence; installer/update provenance and three-OS release/restore acceptance. | Planned |

## Developer-disappearance acceptance (A23)

Use an isolated network/host allowlist that blocks all original-developer
infrastructure and official bootstrap/update endpoints. Start an existing client
and an independent community host; unlock authorized local history, restore an
encrypted backup, connect to known peers, use separately operated relay/discovery
nodes and communicate on an explicitly enabled LAN. Verify protected mode never
falls back to direct when relays disappear. Queue ciphertext to independent
mailboxes, take the recipient offline, and prove eventual retrieval or expiry.
With every peer/storage node offline, show honest unavailability. Record packet
captures with consent and redact evidence. No official account, activation or
mandatory developer key may be required. A00's no-network bootstrap is only a
small prerequisite, not this end-to-end acceptance test.

## Performance acceptance progression

At A01 the admission contention test runs four threads with 1,000 attempts each;
controlled clocks and an owned real worker verify cancellation, shared deadlines
and retry cleanup. These are correctness checks, not throughput benchmarks.
At A08/A11 define message/history and persistence workloads. At A15 define file
sizes/chunk counts and interruption patterns. At A17/A18 measure event latency,
idle scheduling, redraws, allocations and virtualization. At A19 measure defined
media sessions. A25 consolidates measured budgets on consumer/server hardware;
it must not invent throughput promises before these workloads exist.

## Proposals beyond the accepted sequence

[FUTURE_FEATURES.md](docs/architecture/FUTURE_FEATURES.md) records the proposed
local web gateway, native model runtime, Hugging Face import, optional training
workers, AI permissions, media studio and GPU budgets. These are unimplemented
backlog requirements mapped to existing gates; no checkpoint is reordered or
new dependency adopted by recording them. A03 adopts only its explicitly reviewed
crypto dependency; none of these future product features is implemented.

HF01 repairs A01 fixture encoding and Windows development tooling; it does not
add an A02 platform service or reorder any checkpoint. Current report and native
acceptance steps are in docs/checkpoints/GW-A01-HF01.md and its Windows checklist.

## Current handover — HF09 / 2026-10-27

- The first Windows 10% **build/test** acceptance gate is closed: Debug and Release 66/66 compile, 50/50 CTest, including the previously timed-out allocation-fault test (0.03 s both configurations).
- Continue A05 through **real pinned upstream TinyCBOR interop** and independent parser/security disposition. Block A06 until A05 and AUTH-PRE gates are approved.
- In parallel, improve CMake/VS x64 compiler detection and mandatory Python-backed test registration; the wrong VS terminal silently selected 32-bit MSVC, and Python absence reduced test registration to 42 rather than the expected 50. The owner subsequently supplied Python 3.13.13 and complete 50/50 results.
- Preserve isolated cryptographic local profiles and per-community membership keys; no global identity registry, alias-based merge, unapproved persistent keys, or network listeners.
- At next integration, synchronize `docs/development/MODULE_REGISTRY.json`, `WORKER_QUEUE.json` and HF09 acceptance evidence with the post-archive owner results; report actual active workers only. The [development map](current-development-map.md) is the human-readable live overview.
- Owner Windows acceptance is usually at verified ~20% next milestone, not every small hotfix. A critical security correction may require targeted verification earlier.

Earlier A03/A04 handovers remain in their linked checkpoint documents; they are historical and no longer the current development stop point.

## Historical A05-P04 partial candidate (2026-10-19)

The opt-in pinned TinyCBOR interoperability/import gate and allocation-fault
checks are prepared as a **partial** source candidate, documented in
`docs/checkpoints/GW-A05-P04.md`. The upstream source remains unvendored and
maintained-parser interoperability remains blocked until the exact upstream
release is supplied and tested. A05 networking authorization is unchanged.
