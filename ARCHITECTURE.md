# Architecture authority

## P03 hardening addendum

P03 retains the P02 dependency graph, provider, ownership and volatile model.
Deleting a locally owned membership also invalidates matching self-authority
trust in the owning profile, preventing old accepted records from regaining
recognition after the latest owned revocation record is removed. Other profiles'
trust remains independent. See docs/checkpoints/GW-A04-P03.md.


Current pass: A04-P02 implements the accepted identity/device architecture in
[ADR-008](docs/decisions/ADR-008-IDENTITY-AND-DEVICE-TRUST.md) and
[IDENTITY_DESIGN](docs/architecture/IDENTITY_DESIGN.md). The owner accepted P01's
design and authorized bounded volatile implementation. New runtime validation
and native acceptance are recorded separately in
[GW-A04-P02](docs/checkpoints/GW-A04-P02.md); P02 is not yet owner-accepted.

A03-HF01 corrects Darwin cleanup of exited owned groups and a symlinked test
TEMP base. Owner Windows and all hosted native checks now pass; see
[HF01 acceptance](docs/checkpoints/GW-A03-HF01-ACCEPTANCE.md). See also
[ADR-007](docs/decisions/ADR-007-MACOS-EXIT-CLEANUP.md) and the
[recovery report](docs/checkpoints/GW-A03-HF01.md). Earlier checkpoint reports
remain historical; current A03 owner/CI acceptance is recorded in its addendum.

Checkpoint authority is `docs/development/MASTER_CONTRACT.md` plus the supplied
current source and the explicit decisions below. This is a clean-room project;
no TASE source, renderer, database, identity or protocol has been migrated.

HF01 is a scoped A01 recovery: test fixtures convert native paths to UTF-8 only
at CLI/config boundaries and retain native paths for filesystem operations.
Production path validation and the runtime dependency graph are unchanged.
Windows development-tool environment transfer uses explicit Unicode decoding;
tooling helpers and their subprocess checks do not add an application process API.

## What exists through A04-P02

The four composition roots retain the A00 strict CLI, configuration, logging,
paths and ordinary one-shot behavior. The CLI additionally offers an explicitly
requested interactive identity session. Application owns an immutable ApplicationContext,
a dependency-aware ServiceManager, cooperative stop state and resource admission.
The real path initialization runs through the manager in every executable.
Graph validation precedes side effects; failed startup rolls back; shutdown is
reverse-ordered, deadline-aware and retryable while retaining live dependencies.

See [core lifecycle contracts](docs/architecture/CORE_LIFECYCLE.md) and
[ADR-004](docs/decisions/ADR-004-CORE-LIFECYCLE.md) for ownership, states, deadlines,
diagnostics and resource semantics. A02 adds real owned process monitoring, an
explicit native-stop registration and secure-storage API capability discovery.
See [platform contracts](docs/architecture/PLATFORM_ABSTRACTIONS.md) and
[ADR-005](docs/decisions/ADR-005-PLATFORM-OWNERSHIP.md). A03 adds a real lifecycle-owned
Botan provider for bounded volatile randomness, AEAD, public-message signatures
and hashing. See [crypto contracts](docs/architecture/CRYPTO_PROVIDER.md) and
[ADR-006](docs/decisions/ADR-006-CRYPTO-PROVIDER.md).

A04-P02 adds `ghostwire_identity`: bounded owner-thread profile, membership,
device and trust state using the existing provider. Each profile owns an
independent root/device pair; each prepared community membership owns a separate
authority/device pair. Complete canonical signed device sets, explicit pins,
revision/conflict handling, expiry, scoped replacement and revocation are real
operations. Identity recognition grants neither a remote session nor an action.
No GUI, networking, persistent private data or root recovery is implemented.

`ghostwire_composition` owns the lifecycle adapter above core and identity. It
injects CryptoService and borrows its provider only while the dependency is
running. Identity closes before crypto. The CLI session uses this composition;
ordinary bootstrap never creates an identity or trust pin. Identity does not
link core implementation, and ApplicationContext gains no domain-service lookup.

`ghostwire_core`, `ghostwire_platform`, `ghostwire_crypto`, `ghostwire_identity`,
`ghostwire_composition` and the pinned Botan
subset are static libraries. “Shared core”
means common code used by all executable roles, not a shared binary ABI. This
avoids an unnecessary exported ABI/DLL deployment commitment at bootstrap.
If dynamic linkage becomes necessary, introduce exports and ABI testing in a
separate accepted change; do not silently convert these contracts into a plugin ABI.

## Ownership rules

1. Composition roots own service lifetimes. Core has no process-global registry.
2. Application owns its context, stop source, admission and service manager.
   Context owns const config/path snapshots and explicitly borrows core facilities.
   Logger and any injected clock must outlive Application. Lifecycle/graph methods
   are owner-thread only; stop requests and admission are thread-safe. Services
   are destroyed before context, in reverse dependency order.
3. Logger borrows its output stream; a mutex serializes complete bounded records.
   Public field values must be reviewed fixed metadata. Arbitrary content goes
   through a sensitive key and is redacted. It is not a secret detection engine.
4. OS environment reads happen during bootstrap or explicit process launch.
   Process launch defaults to replacement; optional inheritance snapshots must
   not race POSIX environment mutation. No module changes the parent process
   environment during normal operation.
5. CryptoService owns CryptoProvider. Identity and future consumers receive explicit references
   and declare lifecycle dependencies; they join before provider destruction.
   Move-only secret allocations can outlive the provider through bounded shared
   accounting. UI, networking and vault remain future service domains. Public
   headers expose no backend, native OS or renderer types.
6. A result containing Error is a failure, never a success-shaped stub. Errors
   must contain static diagnostic descriptions and bounded structural counters,
   not raw peer input, exception text or paths.
7. No private object is serializable through generic logging/config APIs.
8. IdentityService owns every volatile profile, authority/device key and trust
   context. Handles bind service ownership, slot and generation; profile scope
   is explicit. No process-global active profile or alias-based authority exists.
   Immutable public snapshots may cross threads but confer no signing capability.
   Service/provider/clock lifetimes are joined by the composition owner.
9. Identity uses a trusted caller's UTC VerificationTime for validity and an
   injected monotonic Clock for deadlines. Time revalidation, forgetting trust,
   resolving conflicts and deleting local profiles require explicit owner calls.
   Receiving a public record cannot erase local private keys or clear conflicts.

## Dependency and permission maps

[Dependency graph](docs/architecture/DEPENDENCY_GRAPH.md) defines compile-time
edges; the [boundary map](docs/security/BOUNDARY_MAP.md) defines data access.
A compile-time dependency never grants authority to plaintext or keys.
[Module map](docs/architecture/MODULE_MAP.md) separates implemented code from
future services. [Future interfaces](docs/architecture/FUTURE_INTERFACES.md)
defines proposed headers without providing misleading compilable security stubs.

## Resolved risks

- A layered product sketch is not a linear include hierarchy: platform adapters
  sit below core services, and protocol schemas remain independent of storage/UI.
- Full vault features retain A11, but minimum reviewed key protection is a gate
  before persistent identity and any saved A08 ratchet/message state.
- Base authorization is required before A06/A13; A21 adds richer moderation,
  not the first permission checks. AI is scoped from its first checkpoint.
- Direct/protected/privacy-first describe policies, not current capabilities.
  Future routing must fail closed if the chosen mode is unavailable; a relayed
  socket alone does not prevent DNS, ICE, media, file or AI endpoint leaks.
- Offline delivery and bootstrap discovery need reachable, independently chosen
  infrastructure. No developer infrastructure is required; no infrastructure
  at all is not a feasible promise.
- Numerical aliases are display/routing conveniences, not trust anchors or
  globally unique identifiers without a defined namespace. Device keys and
  community identities are distinct from root keys and hardware capabilities.
- Deleting a local file cannot revoke someone else's copy. Lockout counters are
  not offline password protection. Network events cannot delete vault keys.
- SDL3 may provide platform/GPU primitives later; retained UI, accessibility,
  international text/IME and capture behavior need separate feasibility gates.
- C++20 is the host language. Library choice cannot be constrained to an unsafe
  homegrown crypto design merely to avoid a Rust FFI later.

## Thread and performance contract

A01 creates no production worker pool. A02 Process owns one joined monitor per
explicitly launched child for bounded pipe I/O and reaping. The bootstrap creates
no child/monitor. A03 crypto is synchronous with one nonblocking admission slot
per provider and no queue/worker. Secret handles cannot be mutated concurrently
with use. A04 identity likewise has one synchronous admitted operation, no queue
and no workers. Its counts, record bytes, input limits and cooperative operation
deadline are bounded; provider limits may reject earlier. All mutations stage
their publication, and cleanup releases only the owned volatile state.
Restricted native stop callbacks latch state; only owner-thread
dispatch calls C++ cancellation callbacks. Test fixtures verify real cleanup. Future networking
owns bounded event loops; each vault owns a serialized transaction executor;
crypto/file jobs run on bounded workers and return immutable results. UI and
platform event mutation stay on their UI thread. Shutdown rejects new work,
requests cancellation, drains within deadlines, flushes permitted durable state,
then destroys dependents before dependencies. Failed cleanup retains dependencies
until retry or RAII destruction. Deadlines are cooperative: arbitrary blocking
callbacks/destructors cannot be safely preempted in-process. No background task
may outlive its owned service. Admission bounds declared slots/bytes, not total
process RSS or GPU usage. No product throughput benchmark is claimed.

Changes to security, schemas or the core dependency graph require a full
relevant regression pass, negative-case tests and updated threat-model entries.
A unit test is never sufficient evidence of cryptographic production readiness.

The [future feature backlog](docs/architecture/FUTURE_FEATURES.md) records local
web, native AI, model import, training and media proposals without implementing
them or changing the accepted checkpoint sequence.
