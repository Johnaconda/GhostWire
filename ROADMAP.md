# GhostWire implementation roadmap
The A00–A25 IDs and security ordering gates remain unchanged. A02 is owner-accepted: Windows portable Debug/Release 35/35 each with original TEMP/TMP, plus independent Linux acceptance. A03 was explicitly authorized and is implemented with local Linux evidence; native A03 acceptance remains pending. Stop after A03. A04 is a proposal only.
## Required ordering gates
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
| GW-A02 | Platform Abstractions | A01 | include/ghostwire/platform; src/platform | Prove native paths, secure-storage capabilities, owned-process and stop-event APIs on Windows/Linux/macOS; unsupported states explicit. | Owner-accepted; Linux 34/34 and Windows 35/35 per Debug/Release. Native macOS, full local leaks, clang-tidy and new hosted CI remain open. |
| GW-A03 | Cryptographic Provider | Accepted A02 | include/ghostwire/crypto; src/crypto | Pinned Botan, bounded volatile RNG/AEAD/public signatures/hash, move-only clearing handles and real lifecycle; no identity/persistence/protocol. | Implemented; Linux Debug/Release 37/37 each; native A03 review/acceptance pending. |
| GW-A04 | Identity & Device Trust | A03; A11-PRE for persistence | include/ghostwire/identity; src/identity | Generate/verify root and authorized device identities; rotation/revocation/contact proof; alias collision/change tests; no raw key files. | Planned |
| GW-A05 | Protocol Schemas & Versioning | A04 public identity contracts | schemas; include/ghostwire/protocol; src/protocol | Canonical versioned codecs with bounded fields and negotiation; reject malformed/unsupported input; corpus/fuzz/interop fixtures. | Planned |
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

## Current handover

A03 adds the pinned static Botan provider and owned CryptoService, preserving
all accepted core/platform, Unicode, CLI and Windows tooling behavior. See
[A03 evidence](docs/checkpoints/GW-A03.md), [crypto contracts](docs/architecture/CRYPTO_PROVIDER.md)
and the [native checklist](docs/checkpoints/GW-A03-NATIVE-ACCEPTANCE.md).
The [A04 prompt](docs/checkpoints/GW-A04-PROMPT.md) is a proposal, not authorization.
Identity-system design is still an owner decision. A11-PRE remains mandatory
before persistent identities/session state; AUTH-PRE remains mandatory before
relevant listeners and privileged remote actions. No A04 functionality is included.
