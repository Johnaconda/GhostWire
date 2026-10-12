# Architecture authority

Checkpoint authority is `docs/development/MASTER_CONTRACT.md` plus the supplied
current source and the explicit decisions below. This is a clean-room project;
no TASE source, renderer, database, identity or protocol has been migrated.

HF01 is a scoped A01 recovery: test fixtures convert native paths to UTF-8 only
at CLI/config boundaries and retain native paths for filesystem operations.
Production path validation and the runtime dependency graph are unchanged.
Windows development-tool environment transfer uses explicit Unicode decoding;
tooling helpers and their subprocess checks do not add an application process API.

## What exists at A01

The four composition roots retain the A00 strict CLI, configuration, logging,
paths and one-shot behavior. Application now owns an immutable ApplicationContext,
a dependency-aware ServiceManager, cooperative stop state and resource admission.
The real path initialization runs through the manager in every executable.
Graph validation precedes side effects; failed startup rolls back; shutdown is
reverse-ordered, deadline-aware and retryable while retaining live dependencies.

See [core lifecycle contracts](docs/architecture/CORE_LIFECYCLE.md) and
[ADR-004](docs/decisions/ADR-004-CORE-LIFECYCLE.md) for ownership, states, deadlines,
diagnostics and resource semantics. There are no production workers, domain
services, GUI, networking, cryptography or persistent private data.

`ghostwire_core` and `ghostwire_platform` are static libraries. “Shared core”
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
4. OS environment reads happen during bootstrap, before future workers start.
   No module changes process environment during normal operation.
5. UI, networking, vault and crypto ownership are separate future service
   domains. Their public headers must not expose native OS or renderer types.
6. A result containing Error is a failure, never a success-shaped stub. Errors
   must contain static diagnostic descriptions and bounded structural counters,
   not raw peer input, exception text or paths.
7. No private object is serializable through generic logging/config APIs.

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

A01 creates no production worker pool and performs no background I/O. Test
fixtures own real joining threads to verify cancellation and timeout behavior. Future networking
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
