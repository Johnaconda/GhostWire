# Future interface contracts

Status: design proposals only. No header listed here is implemented unless explicitly present in the current module map. Paths are relative to `include/ghostwire`.

For **every header in each group**, the eight numbered fields below form its contract. The header table supplies its specific responsibility and additional invariant. Later implementation must refine method-level types and failure handling in its checkpoint ADR; these are not fabricated compilable APIs.

## Core services — GW-A01 (implemented subset)

1. **Ownership:** Application owns the context and service manager. Service registrations carry explicit dependencies; observers borrow scoped handles.

2. **Lifetime:** Registered objects remain owned through cleanup and reverse-order destruction; even partially started services receive cleanup. Jobs cannot outlive their service.

3. **Thread safety:** Registry mutation is on the owner thread; immutable snapshots may cross threads; stop_token is thread-safe.

4. **Input validation:** Reject duplicate registrations, missing dependencies, dependency cycles and invalid transition/deadline values.

5. **Errors:** Structured missing_service, duplicate, cycle, cancelled, deadline and initialization failures; unwind already-started services.

6. **Security invariants:** No ambient root key, raw vault or unchecked network authority in a global registry. One cooperative shutdown deadline; failed cleanup retains dependencies.

7. **Platform constraints:** No desktop types. Later OS stop events enter an adapter; C++20 stop tokens remain the portable API.

8. **Dependencies:** A01 core values and platform bootstrap; A03 additionally composes the real crypto service through an acyclic dependency. No UI or transport implementation dependency.


| Proposed header | Header-specific responsibility / refinement |
|---|---|

| `core/application_context.hpp` | Explicit config/path/clock/log facades; read-only runtime snapshots, no global getter. |

| `core/service_registry.hpp` | Own registrations, ordered startup and reverse shutdown; reject partial success. |

| `core/cancellation.hpp` | Prefer std::stop_token directly; introduce a wrapper only if a scoped deadline contract requires it. |

| `core/runtime_info.hpp` | Local version/capability/health snapshot; no serial numbers, endpoints or secret dumps. |

| `core/job_system.hpp` | Future bounded admission, cancellation and completion queue; no fire-and-forget detached threads. |



A01 implementation mapping: application_context.hpp exists; service_manager.hpp
and service.hpp implement the proposed registry responsibility; clock.hpp owns
deadlines; std::stop_source/token are used directly without cancellation.hpp.
Structural diagnostics are ServiceSnapshot/ShutdownReport, not a fabricated
runtime_info.hpp. resource_admission.hpp implements bounded reservations. A
job_system.hpp, domain diagnostics service and executor remain future work.
[CORE_LIFECYCLE.md](CORE_LIFECYCLE.md) supersedes the core proposal's broad wording.

## Platform adapters — GW-A02 implemented subset / GW-A17 future

1. **Ownership:** Application owns runtime adapters and exact process/secure-store handles; native resources are RAII-managed.

2. **Lifetime:** Handles are move-only and close on destruction; secure-store sessions lock before UI/session teardown.

3. **Thread safety:** UI/platform event functions stay on owner thread; asynchronous process/store operations use explicit executors.

4. **Input validation:** Validate absolute paths, permissions, ownership proof, executable/argv separation, buffer sizes and capability requests.

5. **Errors:** Unsupported, permission_denied, unavailable, invalid_handle and OS-category errors; never echo raw secrets or paths.

6. **Security invariants:** Only explicitly owned processes may be stopped. Secure storage fails closed; capability inspection stays local.

7. **Platform constraints:** Windows Known Folder/DPAPI or equivalent, macOS Keychain, Linux reviewed secret-service/file policy need real host tests. No native types in headers.

8. **Dependencies:** core value contracts and OS APIs; no UI widget or protocol implementation.


| Proposed header | Header-specific responsibility / refinement |
|---|---|

| `platform/platform_runtime.hpp` | Explicit init/shutdown and platform stop-event ownership. |

| `platform/process.hpp` | Spawn/observe owned processes; never shell-evaluate peer input or kill arbitrary PIDs. |

| `platform/secure_storage.hpp` | Capability-returning sealed-key operations; report unavailable instead of plaintext fallback. |

| `platform/hardware_capabilities.hpp` | Coarse CPU/RAM/GPU/codec features with explicit optional disclosure; no stable fingerprint. |

| `platform/capture_protection.hpp` | Supported/active/unavailable state and documented best-effort window protection. |



A02 mapping: process.hpp owns real child operations; process_stop.hpp implements
explicit stop registration instead of a generic platform_runtime registry.
secure_storage.hpp reports API presence only and implements no sealed-key operation.
The broader secure-store/session, hardware, capture and UI contracts above remain
future proposals. [PLATFORM_ABSTRACTIONS.md](PLATFORM_ABSTRACTIONS.md) is the
implemented ownership, lifetime, threading, validation, error and resource contract.

## Cryptographic provider — GW-A03 implemented subset

The numbered design proposal below is retained for traceability. The implemented
contract in [CRYPTO_PROVIDER.md](CRYPTO_PROVIDER.md) supersedes it: no KDF,
revocation, backup/export, queued jobs or extra platform entropy facade exists.
crypto_provider.hpp includes checked random generation directly, avoiding a
second owner in secure_random.hpp. key_handle.hpp and crypto_error.hpp are real.
Thread-safe nonblocking admission returns busy; callers own/join executors.
Key reset is memory cleanup, not identity/device revocation. Native RNG/memory
adapters belong to the pinned Botan implementation. No further proposed header
is a compilable success-shaped stub.

1. **Ownership:** Provider owns reviewed backend resources; secret material is kept behind move-only key handles.

2. **Lifetime:** Key handles expire/revoke explicitly and clear provider-owned buffers where supported; no accidental copy/serialization.

3. **Thread safety:** Immutable public values can cross threads; provider/key use follows upstream thread-safety constraints through bounded jobs.

4. **Input validation:** Algorithm/suite allowlists, nonce/key lengths, authenticated associated data, KDF limits and RNG status are validated.

5. **Errors:** Authentication failure, invalid_parameters, entropy_unavailable, unsupported_suite, allocation and revoked_handle; no plaintext on failure.

6. **Security invariants:** No homegrown primitives. No predictable RNG fallback. Distinct key purposes, reviewed memory handling and authenticated outputs.

7. **Platform constraints:** Entropy/secure memory capability varies by OS; inability to lock memory or protect keys must be surfaced honestly.

8. **Dependencies:** core values, platform secure memory/entropy and one approved provider; not messaging/UI.


| Proposed header | Header-specific responsibility / refinement |
|---|---|

| `crypto/crypto_provider.hpp` | Reviewed backend facade; selected primitives are not a full messaging protocol. |

| `crypto/key_handle.hpp` | Opaque scoped handle; usage purpose and revocation; export only through authorized backup policy. |

| `crypto/secure_random.hpp` | Checked OS/provider CSPRNG; fills bounded buffers or fails. |

| `crypto/crypto_error.hpp` | Safe provider errors independent of backend ABI. |



## Identity and trust — GW-A04 / GW-A10

1. **Ownership:** Identity service owns root/device trust state; contacts receive immutable public records and scoped verification results.

2. **Lifetime:** Root, authorized device and signed record lifetimes are distinct; revoked devices cannot get future authorized state.

3. **Thread safety:** Serialized identity mutations; public snapshots are immutable; signature work on bounded crypto executor.

4. **Input validation:** Validate canonical encoding, signatures, key type, record version, expiry, authorization chain and namespace-scoped alias.

5. **Errors:** Invalid_signature, untrusted, revoked, stale_record, conflict, unsupported_version and protected_store_unavailable.

6. **Security invariants:** Alias/display name is not identity. Server bans reference crypto identity policy; fresh identities remain a Sybil risk. No hardware serial authentication.

7. **Platform constraints:** Persistent keys require the minimum protected key-store gate; backups/recovery cannot assume one OS keystore is portable.

8. **Dependencies:** crypto, permissions, protocol values, protected key store; no community membership prerequisite.


| Proposed header | Header-specific responsibility / refinement |
|---|---|

| `identity/identity_id.hpp` | Canonical public-key-derived identity reference using reviewed encoding/hash decisions. |

| `identity/identity_record.hpp` | Signed versioned public metadata with bounded alias/display fields. |

| `identity/device_identity.hpp` | Root-authorized device credential, expiry, revocation and least privilege. |

| `identity/identity_service.hpp` | Create/authorize/revoke/backup through key handles; no secret export convenience. |

| `identity/contact_verification.hpp` | Out-of-band verification and changed-key warnings that cannot be silently dismissed by a peer. |

| `identity/trust_record.hpp` | Explicit local trust decision with evidence/provenance and scoped persistence. |

| `identity/key_rotation.hpp` | Authenticated rotation chain and recovery conflict rules; old/compromised keys cannot silently replace trust. |



## Vault and recovery — GW-A11-PRE / GW-A11 / GW-A12

1. **Ownership:** Vault owns database key, wrapped-key metadata, serialized transaction executor and encrypted stores per local profile.

2. **Lifetime:** Locked/unlocked sessions are scoped; handles expire on lock; identity keys and DB keys have distinct lifecycles.

3. **Thread safety:** Database writes serialized; readers use scoped snapshots; unlock KDF runs on bounded workers without mutating UI.

4. **Input validation:** Validate format/KDF versions and resource bounds before allocation; authenticate wrappers; check paths, permissions and restore provenance.

5. **Errors:** Locked, wrong_credential, corrupted, unsupported_format, quota, rollback_detected, recoverable_lockout and key_store_unavailable.

6. **Security invariants:** No plaintext temp/index/backup fallback. Atomic durable crypto-state updates. Remote failure counters never erase local keys; offline guessing unaffected by software lockouts.

7. **Platform constraints:** Test SQLCipher temp/WAL/journal behavior and platform key stores. Secure deletion cannot guarantee removal from SSD snapshots or copied keys.

8. **Dependencies:** crypto, platform secure_storage/filesystem and reviewed DB; no network callbacks with deletion authority.


| Proposed header | Header-specific responsibility / refinement |
|---|---|

| `vault/vault.hpp` | Scoped transaction API; key-handle boundary and atomic ratchet/history state. |

| `vault/vault_policy.hpp` | Versioned local storage, lock, timeout, backup and retention choices. |

| `vault/vault_access_controller.hpp` | Only local authorized unlock/lock sessions; rate limiting distinct from remote login. |

| `vault/unlock_attempt_policy.hpp` | Configurable retry threshold (example 5), cooldown/backoff and recoverable lockout; destruction off by default. |

| `vault/vault_recovery.hpp` | Deliberate restore or crypto-erasure workflow with replicated-key inventory and rollback limits. |



## Protocol values and codecs — GW-A05

1. **Ownership:** Value objects own bounded byte buffers; codecs do not own sockets, database handles or private keys.

2. **Lifetime:** Decoded messages are immutable and may outlive a parser only through owned storage.

3. **Thread safety:** Pure codecs reentrant; no mutable global schema registry.

4. **Input validation:** Validate version, length, canonical encoding, field ranges and unknown-field policy before dispatch; enforce per-type limits.

5. **Errors:** Unsupported_version, malformed, too_large, invalid_field and unknown_mandatory_capability; no downgrade through parse errors.

6. **Security invariants:** Authenticated protocol binding and replay domains are explicit; do not conflate project version with wire version.

7. **Platform constraints:** Endian, integer-width and Unicode rules are fixed by schema, not compiler ABI or native struct layout.

8. **Dependencies:** core value contracts and approved codec only; crypto operations live above the codec.


| Proposed header | Header-specific responsibility / refinement |
|---|---|

| `protocol/protocol_version.hpp` | Major/minor compatibility and explicit negotiation rules; no protocol version invented in A00. |

| `protocol/message_envelope.hpp` | Bounded routing envelope separated from encrypted payload; no unauthenticated trusted metadata. |

| `protocol/capabilities.hpp` | Negotiated supported operations with policy constraints; unknown required capability fails. |



## Transport and route privacy — GW-A06

1. **Ownership:** Transport service owns connections, listeners, route tokens and per-peer/global resource reservations.

2. **Lifetime:** Route and authenticated session scoped handles close before service destruction; callbacks cannot resurrect closed routes.

3. **Thread safety:** I/O executor owns sockets; immutable results post to services; no direct UI or vault mutation.

4. **Input validation:** Validate endpoint encoding, peer authentication, route permissions, timeout/size budgets and negotiated protocol.

5. **Errors:** Policy_unavailable, connection_refused, unauthenticated, timeout, cancelled, quota and unsupported; no silent direct fallback.

6. **Security invariants:** Every text/file/discovery/media/AI connection goes through route policy. Relays see hop metadata; protected mode never advertises direct addresses to the remote peer.

7. **Platform constraints:** NAT/ICE/DNS/TLS/QUIC behavior must be tested per OS; relay-only media needs verified candidate filtering.

8. **Dependencies:** protocol, permissions, core resources, platform I/O, approved transport library; no vault decryption.


| Proposed header | Header-specific responsibility / refinement |
|---|---|

| `transport/transport.hpp` | Bounded send/receive and explicit session ownership. |

| `transport/endpoint.hpp` | Sensitive validated endpoint type; ordinary logging redacts it. |

| `transport/connection_policy.hpp` | Authenticated purpose, peers, budgets and permitted connection operations. |

| `transport/relay_route.hpp` | Authorized relay chain and observable hop trust model; no universal anonymity claim. |

| `transport/privacy_policy.hpp` | Direct/protected/privacy-first with fail-closed capabilities across subsystems. |



## Autonomous discovery — GW-A07

1. **Ownership:** Discovery service owns bounded caches and queries; signed records are immutable values.

2. **Lifetime:** Records expire; queries have cancellation/deadlines; no infinite rebroadcast or unbounded historical cache.

3. **Thread safety:** Discovery executor owns mutable cache; verification tasks return immutable results.

4. **Input validation:** Verify signature, identity/device authorization, expiry/clock skew, endpoint policy, record size and query budget.

5. **Errors:** Untrusted, expired, poisoned, unavailable, quota and cancelled; lack of bootstrap is not account lockout.

6. **Security invariants:** No chat plaintext, public direct endpoint leakage in protected mode, or identity keyed by IP. Independent/manual peers must work.

7. **Platform constraints:** LAN discovery is opt-in and policy-scoped; interfaces/DNS/multicast differ by OS; privacy-first may disable LAN advertisement.

8. **Dependencies:** protocol, transport policy, identity signature verification, bounded storage.


| Proposed header | Header-specific responsibility / refinement |
|---|---|

| `discovery/discovery_service.hpp` | Bounded queries, deduplication, backoff and authenticated reachability. |

| `discovery/reachability_record.hpp` | Signed short-lived device/relay reachability, not a public contact-graph dump. |

| `discovery/bootstrap_config.hpp` | User-chosen independent bootstrap/manual peers; no mandatory official nodes. |



## Application services and content — GW-A08–GW-A19 / GW-A21

1. **Ownership:** Each service owns its domain state and receives narrow injected facades; peers/hosts cannot obtain generic vault access.

2. **Lifetime:** Session, membership, transfer and media handles expire/cancel/revoke explicitly; queued work has bounded retention.

3. **Thread safety:** Per-service executor owns mutation; storage transactions serialize; UI receives immutable views.

4. **Input validation:** Validate authorization, membership epochs, content bounds, replay IDs, path/filename safety, quotas and durable state versions.

5. **Errors:** Denied, locked, stale_epoch, replay, integrity_failure, unavailable, cancelled and quota; do not expose unauthenticated plaintext.

6. **Security invariants:** Reviewed DM/group protocols, E2EE participant-owned keys, private metadata protection, route-policy compliance and no execution of attachments.

7. **Platform constraints:** Database/codec/media availability and sandbox behavior need per-OS validation. No automatic plaintext or direct-connect fallback.

8. **Dependencies:** identity, permissions, crypto protocol adapters, protocol, vault, storage and transport policy as specified by the module DAG.


| Proposed header | Header-specific responsibility / refinement |
|---|---|

| `messaging/messaging_service.hpp` | Asynchronous send/receive with atomic protected ratchet state and local history. |

| `messaging/offline_mailbox.hpp` | Authorized ciphertext-only retention, expiry, dedup and quota; delivery requires reachable storage. |

| `friends/friend_service.hpp` | Contacts independent of community membership; explicit trust/verification and private graph. |

| `groups/group_service.hpp` | Reviewed MLS/equivalent membership epochs; removal affects future access, not copied history. |

| `communities/community_service.hpp` | Headless hosting with scoped roles/invites; admin has no automatic member E2EE keys. |

| `federation/federation_service.hpp` | Versioned host-to-host identity/permission boundaries and portable export/import. |

| `storage/content_store.hpp` | Quota-reserved ciphertext content addressing; avoid revealing plaintext hashes unintentionally. |

| `files/file_transfer.hpp` | Chunk integrity, resumability and routing; safe large length arithmetic and inert filenames. |

| `media/media_session.hpp` | Authorized voice/video with policy-constrained candidate exchange, jitter/codec limits and encryption decisions. |

| `permissions/permission_service.hpp` | Deny-by-default action/resource scopes; minimum facade required before network listeners. |

| `moderation/moderation_service.hpp` | Conduct-based rules, scoped evidence, audited proportional actions, human review and appeals. |

| `diagnostics/diagnostics_service.hpp` | Opt-in redacted snapshots; never a generic dump of private application state. |



## AI and plugins — GW-A20 / GW-A22

1. **Ownership:** User-approved agent/plugin instance owns scoped context and its own credentials; host retains vault/root keys.

2. **Lifetime:** Context grants expire/revoke; provider calls and tools carry request-scoped tokens; retained context follows explicit retention policy.

3. **Thread safety:** Isolated process or bounded executor; no direct UI/service-graph mutation, no unrestricted plugin pointer to host objects.

4. **Input validation:** Validate identity/membership, scope/resource/time bounds, provider destination, content allowlist and tool arguments before execution.

5. **Errors:** Permission_denied, grant_expired, provider_unavailable, route_policy_unavailable, quota and cancelled; no hidden provider fallback.

6. **Security invariants:** E2EE-reading agents are explicit members. Remote plaintext disclosure is informed and scoped. Message/tool text cannot grant authority.

7. **Platform constraints:** Local inference accelerators and plugin sandbox availability differ; expose unavailable capabilities without fingerprinting users.

8. **Dependencies:** permissions, identity, scoped application facades, transport privacy policy and bounded resources; no direct crypto/vault backdoor.


| Proposed header | Header-specific responsibility / refinement |
|---|---|

| `ai/agent_identity.hpp` | Separate agent identity and optional group membership credentials. |

| `ai/agent_permissions.hpp` | Explicit readable conversations/files, providers, actions and expiry. |

| `ai/provider_interface.hpp` | Local versus remote provider boundaries, cancellation and disclosure state. |

| `ai/agent_context.hpp` | Minimal authorized snapshot with provenance/retention; no all-history default. |

| `ai/tool_permissions.hpp` | Per-invocation allowlist and consequential-action authority independent of model output. |

| `ai/moderation_interface.hpp` | Scoped conduct evidence and accountable suggestions/actions with human review. |

| `plugins/plugin_host.hpp` | Versioned capability bridge and isolation level; no stable in-process ABI promised yet. |


