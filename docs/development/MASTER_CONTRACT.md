# GHOSTWIRE
## MASTER DEVELOPMENT CONTRACT — FOUNDATION EDITION 1.0

**Project:** GhostWire  
**Initial checkpoint:** `GW-A00-FOUNDATION-BOOTSTRAP`  
**Development model:** Clean-room, cumulative, checkpoint-based  
**Primary language:** C++20  
**Target platforms:** Windows, Linux, macOS  
**Initial development environment:** Windows 10 x64 / Visual Studio 2026 Community / MSVC / CMake / Ninja / vcpkg  
**Project status:** New project — no existing source tree  
**Repository:** Independent GitHub repository  
**Authoritative principle:** The software must remain usable without developer-operated infrastructure.

---

# 00 — INSTRUCTIONS TO ASTRA

You are Astra.

Your role is to act as:

- Principal software architect
- Senior C++ systems engineer
- Cross-platform application engineer
- Security-conscious protocol designer
- Build and tooling engineer
- Development roadmap coordinator
- AI-assisted development contract maintainer

Your objective is to establish a production-oriented foundation for GhostWire.

You are not being asked to implement the entire application in one response.

You are being asked to **create an architecture capable of safely supporting its eventual implementation**.

This includes real source files, build targets, module interfaces, dependency management, documentation, testing infrastructure, architectural boundaries, and detailed development checkpoints.

All future development must be able to continue from this foundation without rebuilding the application architecture.

### Execution requirements

1. Review this entire contract.
2. Identify architecture risks and contradictions.
3. Resolve ordinary implementation choices with documented rationale.
4. Construct the initial source repository.
5. Implement the A00 foundation.
6. Execute every build or test supported by your environment.
7. Document everything that could not be tested.
8. Package cumulative source as a ZIP.
9. Provide build instructions and checkpoint results.
10. Stop after A00 and await acceptance.

Do not claim a test was executed if it was not.

Do not substitute illustrative code for deliverable implementation.

Do not implement future features merely to make the application look more complete.

---

# 01 — PRODUCT DEFINITION

GhostWire is a decentralized, privacy-first, self-hostable communication platform.

It combines aspects of:

- Discord-style communities and chat
- Peer-to-peer private communication
- Cryptographic personal identities
- Self-hosted infrastructure
- Encrypted file distribution
- Cooperative network discovery
- Privacy-preserving connectivity
- Locally encrypted history
- Optional AI agent integration
- Extensible client and server capabilities

GhostWire must not require the original developers to maintain an authentication service, central database, or operational gateway.

If development ceases permanently, existing installations and independently operated networks must remain functional.

### Operational modes

**Desktop Client**

An application users install to communicate, manage friends, join communities, exchange files, and configure privacy settings.

**Local Community Host**

A server running on a user's own computer or network.

**Dedicated Community Server**

A headless server suitable for Linux, Windows, or macOS hosting.

**Relay Node**

An independently operated service that provides connectivity without revealing direct endpoints to the other participating peer.

**Discovery Node**

A cooperative participant that assists in locating signed reachability information.

**Optional Storage Node**

A participant that explicitly volunteers bounded resources for encrypted offline mailboxes or file chunks.

One physical machine may run multiple roles, but role separation and resource ownership must remain explicit.

---

# 02 — FUNDAMENTAL DESIGN PRINCIPLES

The following rules are mandatory:

1. No central account registration requirement.
2. No mandatory developer-controlled infrastructure.
3. No mandatory telemetry or advertising identifiers.
4. No forced remote activation.
5. No mandatory cloud storage.
6. No unauthorized plaintext message collection.
7. No custom cryptographic primitives.
8. No hardware fingerprinting by default.
9. No IP address-based account identity.
10. No uncontrolled internet-wide broadcasting.
11. No silent fallback that defeats a selected privacy policy.
12. No plaintext private-message storage by default.
13. No AI access to private content without authorization.
14. No remote party may trigger destruction of another user's local encryption keys.
15. No dependence on the graphical interface for headless hosting.
16. No server administrator automatically obtains E2EE group keys.
17. No security claims stronger than their actual technical guarantees.
18. No fabricated validation or simulated production functionality.
19. No implicit trust of externally received data.
20. No breaking protocol changes without versioning and compatibility policy.

Performance, security, interoperability, and maintainability are first-class architectural requirements.

---

# 03 — LANGUAGE AND TOOLCHAIN

## Primary implementation

Use C++20.

The user already has a Windows C++ development environment.

Known development environment:

- Windows 10 x64
- Visual Studio 2026 Community
- MSVC toolchain
- CMake
- Ninja
- vcpkg
- PowerShell

Do not assume absolute installation paths.

Implement tool detection and clear diagnostics.

## Rust consideration

Rust is a possible future technology.

However, introducing Rust at A00 would add an unnecessary second toolchain and complicate builds.

Create an architecture decision record comparing:

- C++20
- C++23 adoption later
- Rust
- Mixed C++/Rust architecture

Evaluate memory safety, development complexity, portability, library ecosystem, performance, interoperability, and tooling.

**Default decision: remain C++20 unless there is a concrete, documented reason to change.**

## Core tools

Use:

- CMake
- CMakePresets.json
- vcpkg manifest mode
- Pinned dependency versions
- Git
- GitHub Actions
- CTest
- An established unit-testing framework
- clang-format
- clang-tidy where supported
- Platform-specific sanitizers where available

Do not introduce large frameworks unnecessarily.

---

# 04 — SYSTEM ARCHITECTURE

Separate the project into independent layers.

```text
+------------------------------------------------+
|                 GHOSTWIRE UI                   |
| Desktop / Themes / Workspaces / Preferences     |
+------------------------------------------------+
|              APPLICATION SERVICES              |
| Friends / Communities / Messaging / Files / AI |
+------------------------------------------------+
|                SECURITY LAYER                  |
| Identity / Permissions / Keys / Vault / Trust  |
+------------------------------------------------+
|             COMMUNICATION LAYER                |
| Protocol / Discovery / Transport / Federation  |
+------------------------------------------------+
|               PERSISTENCE LAYER                |
| Local Vault / SQLite / Content Store / Backups |
+------------------------------------------------+
|                 CORE SERVICES                  |
| Lifecycle / Config / Jobs / Logging / Errors   |
+------------------------------------------------+
|               PLATFORM ADAPTERS                |
| Windows / Linux / macOS                        |
+------------------------------------------------+
```

The diagram represents dependency responsibilities, not permission for arbitrary cyclic dependencies.

Define an explicit dependency graph.

Protocol and cryptographic core components must not depend on UI or desktop libraries.

Servers must not require graphics initialization.

Cross-platform implementation differences must be encapsulated.

---

# 05 — REPOSITORY STRUCTURE

Create an organized GitHub-ready project.

Recommended structure:

```text
GhostWire/
│
├── .github/
│   ├── workflows/
│   │   ├── windows.yml
│   │   ├── linux.yml
│   │   └── macos.yml
│   ├── ISSUE_TEMPLATE/
│   └── pull_request_template.md
│
├── apps/
│   ├── desktop/
│   ├── server/
│   ├── relay/
│   └── cli/
│
├── include/
│   └── ghostwire/
│       ├── core/
│       ├── platform/
│       ├── identity/
│       ├── crypto/
│       ├── vault/
│       ├── permissions/
│       ├── protocol/
│       ├── transport/
│       ├── discovery/
│       ├── messaging/
│       ├── friends/
│       ├── groups/
│       ├── communities/
│       ├── federation/
│       ├── storage/
│       ├── files/
│       ├── media/
│       ├── moderation/
│       ├── ai/
│       ├── plugins/
│       └── diagnostics/
│
├── src/
│   └── [matching implemented modules]
│
├── ui/
│   ├── framework/
│   ├── rendering/
│   ├── components/
│   ├── workspaces/
│   └── themes/
│
├── schemas/
│   ├── identity/
│   ├── messaging/
│   ├── discovery/
│   ├── federation/
│   └── files/
│
├── tests/
│   ├── unit/
│   ├── integration/
│   ├── protocol/
│   ├── security/
│   ├── performance/
│   └── fixtures/
│
├── benchmarks/
├── examples/
├── tools/
│   ├── windows/
│   ├── linux/
│   ├── macos/
│   └── packaging/
│
├── cmake/
├── docs/
│   ├── architecture/
│   ├── security/
│   ├── protocol/
│   ├── development/
│   ├── decisions/
│   ├── checkpoints/
│   ├── COMPLETED.md
│   └── CHANGELOG.md
│
├── CMakeLists.txt
├── CMakePresets.json
├── vcpkg.json
├── vcpkg-configuration.json
├── .gitignore
├── .gitattributes
├── .editorconfig
├── .clang-format
├── README.md
├── BUILD.md
├── CONTRIBUTING.md
├── SECURITY.md
├── LICENSE
├── AGENTS.md
├── ARCHITECTURE.md
├── ROADMAP.md
└── TODO.md
```

Use repository-local source, documentation, and generated build directories.

Avoid hundreds of empty placeholder files.

For future modules, document proposed headers and contracts rather than pretending their implementations exist.

---

# 06 — PREDEFINED CORE HEADERS

Implement functional A00 contracts where appropriate.

Design future module contracts separately.

## Core

```text
core/application.hpp
core/application_context.hpp
core/application_lifecycle.hpp
core/version.hpp
core/result.hpp
core/error.hpp
core/logger.hpp
core/config.hpp
core/paths.hpp
core/service_registry.hpp
core/cancellation.hpp
core/resource_limits.hpp
core/runtime_info.hpp
```

Core requirements:

- RAII ownership
- Explicit initialization and shutdown
- Structured errors
- Predictable exit codes
- Log redaction
- Testable configuration
- No uncontrolled global state
- No UI dependencies

## Platform

```text
platform/platform_runtime.hpp
platform/platform_info.hpp
platform/filesystem.hpp
platform/process.hpp
platform/secure_storage.hpp
platform/hardware_capabilities.hpp
```

Platform APIs must not leak Windows, Linux, or macOS native types into platform-independent contracts.

## Identity

```text
identity/identity_id.hpp
identity/identity_record.hpp
identity/device_identity.hpp
identity/identity_service.hpp
identity/contact_verification.hpp
identity/trust_record.hpp
identity/key_rotation.hpp
```

## Cryptography and Vault

```text
crypto/crypto_provider.hpp
crypto/key_handle.hpp
crypto/secure_random.hpp
crypto/crypto_error.hpp

vault/vault.hpp
vault/vault_policy.hpp
vault/vault_access_controller.hpp
vault/unlock_attempt_policy.hpp
vault/vault_recovery.hpp
```

## Network

```text
protocol/protocol_version.hpp
protocol/message_envelope.hpp
protocol/capabilities.hpp

transport/transport.hpp
transport/endpoint.hpp
transport/connection_policy.hpp
transport/relay_route.hpp
transport/privacy_policy.hpp

discovery/discovery_service.hpp
discovery/reachability_record.hpp
discovery/bootstrap_config.hpp
```

All future header proposals must describe:

- Ownership
- Lifetime
- Thread safety
- Input validation
- Errors
- Security invariants
- Relevant platform constraints
- Dependencies

Do not implement cryptographic operations as fake placeholders.

---

# 07 — CRYPTOGRAPHIC IDENTITY

Users must be able to generate an identity locally.

Identity ownership must be cryptographically verifiable.

Provide for:

- Public/private identity keys
- User-facing numerical aliases
- Display names
- Per-device authorization
- Signed identity records
- Contact verification
- Device additions and removal
- Key rotation
- Backup and recovery
- Impersonation resistance
- Explicit trust management

A numerical alias is not a cryptographic identity.

Changing an alias must not silently remove bans or invalidate the underlying trust model.

A device key should be distinct from the user's root identity.

An account should eventually support multiple authorized devices without requiring hardware serial-number tracking.

Do not make a globally correlatable hardware fingerprint the foundation of authentication.

---

# 08 — PRIVATE MESSAGING

Friends and DMs must be independent of community membership.

A user must be able to contact an authorized friend without sharing a community server.

Design for:

- Asynchronous end-to-end encryption
- Secure initial key agreement
- Forward secrecy
- Post-compromise recovery
- Offline message storage
- Message authenticity
- Replay resistance
- Deduplication
- Delivery acknowledgment
- Retry policies
- Expiration
- Local encrypted history

Use an established, appropriately reviewed messaging protocol or implementation.

Do not create an original cryptographic messaging protocol merely for convenience.

Keep message contents separate from transport routing metadata.

---

# 09 — GROUP ENCRYPTION

Private groups must support cryptographic membership control.

Evaluate Messaging Layer Security (MLS) or an equivalent established group-key-management design.

Required concepts:

- Group identifiers
- Member identities
- Membership verification
- Encryption epochs
- Group key updates
- Joining and leaving
- Revocation
- Access to previous history
- Future-message access restrictions

Private group encryption keys must belong to authorized participants.

The server hosting a group must not automatically gain plaintext access.

A removed participant may retain material already decrypted or copied. Do not advertise impossible retroactive erasure.

AI agents must be treated as explicit members when they require E2EE content access.

---

# 10 — LOCAL ENCRYPTED VAULT

Every device must support encrypted local storage.

Protect:

- Message history
- Attachment metadata
- Friend records
- Sensitive account metadata
- Local search indexes
- Authentication secrets
- Recovery material
- Cached private content

Use established encryption and key-derivation mechanisms.

Evaluate SQLCipher or another reviewed approach for encrypted database persistence.

Use appropriately protected device keys and memory-hard password derivation, such as Argon2id, where passwords are involved.

Account identity keys and local database encryption keys must have distinct lifecycles.

Temporary plaintext files, crash dumps, logs, search indexes, and backups must be covered by the threat model.

## Failed unlock protection

Implement a future configurable policy for failed vault unlock attempts.

Example:

```text
Vault security policy:

Maximum failed attempts: 5
Initial response: Temporary lock
Cooldown: Configurable
Progressive backoff: Supported
Local event logging: Redacted
Remote-triggered key deletion: Forbidden
Automatic destructive wiping: Disabled by default
```

Five attempts is a configurable example, not a cryptographic security guarantee.

Important security rules:

1. Repeated unauthorized local unlock attempts can trigger lockout or increasing delays.
2. Invalid server login attempts must not affect a user's local vault keys.
3. Malicious remote parties must not be capable of forcing local data deletion.
4. Software-only counters must not be represented as protection against offline password guessing.
5. Password-strength and key-derivation policies are independent from retry thresholds.
6. Normal lockouts must be recoverable through authorized means.
7. Vault security behavior must be tested against denial-of-service and rollback attacks.

## Cryptographic erasure

Optional user-initiated cryptographic erasure may be supported in the future.

It must:

- Require deliberate authorization.
- Clearly describe irreversibility.
- Account for backups and replicated keys.
- Never be triggered merely by unauthenticated network traffic.
- Never silently erase another user's data.

Concealing locked messages in the interface is allowed but must not be confused with cryptographic deletion.

---

# 11 — HARDWARE PRIVACY

GhostWire may inspect hardware for legitimate local purposes.

Examples:

- CPU architecture
- Logical processor count
- Available RAM
- GPU capabilities
- Supported instruction sets
- Storage capabilities
- Available acceleration features

Such information may improve:

- Client performance
- Media encoding
- Encryption scheduling
- Local AI acceleration
- Resource budgeting
- Battery and power management

However:

**Hardware capability discovery must not become mandatory user fingerprinting.**

Rules:

- Hardware capability information remains local by default.
- No unnecessary hardware serial-number collection.
- No automatic combination of hardware identifiers with IP addresses.
- No central device-tracking database.
- No globally stable device fingerprint exposed to arbitrary community servers.
- Use cryptographic device credentials for authentication.
- Clearly document any necessary remote capability disclosure.

Device identity, hardware diagnostics, and performance telemetry are separate responsibilities.

---

# 12 — IP ADDRESS PRIVACY

IP addresses must not be used as personal identities.

Avoid retaining IP information unnecessarily.

Protect operational IP records at rest and in transit where applicable.

The network stack must recognize that packet routing still requires certain intermediaries to know endpoint addresses.

Encryption alone cannot hide an address from every system responsible for routing traffic.

## Connection modes

**DIRECT**

Allows direct peer connectivity and NAT traversal.

Peers may learn each other's network addresses.

**PROTECTED**

Uses authorized relay infrastructure.

The remote peer should not receive the user's direct IP through this transport.

**PRIVACY-FIRST**

Supports a future stronger anonymity-oriented routing configuration.

Evaluate multi-hop or independently operated privacy networks.

Document limitations and latency costs.

## Privacy invariants

- Never silently downgrade Protected to Direct.
- Never silently downgrade Privacy-first to Direct.
- Apply privacy policies to text, files, discovery, voice/video, and AI connections.
- Prevent known cross-subsystem address leaks.
- Redact network addresses from ordinary logs.
- Minimize retention of operational connection metadata.
- Protect necessary persistent network records through encryption and access control.
- Ensure diagnostics do not accidentally expose sensitive identifiers.
- Treat relay operators as separate trust boundaries.

Connection addresses must not become global user lookup identifiers.

Privacy claims must identify which parties may still observe network metadata.

---

# 13 — AUTONOMOUS NETWORK DISCOVERY

GhostWire should locate authorized peers without requiring the original developers' servers.

Support an architecture involving:

- Signed reachability records
- Short-lived discovery information
- Expiration
- Public-key verification
- Cooperative discovery nodes
- User-configured bootstrap nodes
- Manually configured peers
- LAN discovery
- Cached known contacts
- Authorized relay records

No uncontrolled global broadcast loops.

Use bounded searches, retry delays, deduplication, and network backpressure.

The discovery system must operate independently from chat content.

## Offline recipients

When a user is offline, an authorized mailbox may retain encrypted message envelopes until retrieval or expiry.

Offline delivery requires some reachable storage or relay infrastructure.

Do not claim impossible delivery when all relevant participants and storage nodes are offline.

## Developer disappearance

The following must remain possible without official services:

- Starting the application
- Accessing existing local identity
- Accessing authorized local history
- Running a community server
- Connecting directly to known peers
- Using independently operated relays
- Configuring independent discovery nodes
- Communicating on a LAN
- Restoring encrypted backups

Define an explicit developer-disappearance acceptance test for a future checkpoint.

---

# 14 — SELF-HOSTED COMMUNITIES

Users must eventually be able to create a community through a straightforward configuration workflow.

Support:

- Local hosting
- Dedicated hosting
- Private communities
- Public communities
- Invite codes
- Server passwords where appropriate
- Whitelists
- Roles and permissions
- Text channels
- Voice channels
- Group membership
- Server-specific moderation
- Independent community identity
- Backup and restoration
- Optional migration to another host

Community administrators control community policy.

They do not automatically control members' private cryptographic identities or encrypted DM contents.

The dedicated server must run headlessly.

---

# 15 — ENCRYPTED FILE DISTRIBUTION

Design a torrent-inspired content transport system.

Support:

- Large files
- Encrypted chunks
- Independent integrity verification
- Content-addressed storage
- Resumable transfers
- Cooperative seeding
- Optional password protection
- Member-based access
- Transfer throttling
- Disk quotas
- Bandwidth limits
- Resource budgets
- Safe metadata parsing

Files must remain encrypted on unauthorized intermediary nodes.

Conventional P2P operation can expose peers' IP addresses.

When Protected or Privacy-first mode is active, file transfer routing must obey that policy.

No arbitrary small application-level size ceiling is required, but resource exhaustion safeguards are mandatory.

---

# 16 — AI INTEGRATION

AI is optional.

Future supported providers may include:

- Local inference engines
- Remote AI APIs
- Self-hosted AI services
- Community-operated agents
- User-operated personal assistants

Define future contracts for:

```text
ai/agent_identity.hpp
ai/agent_permissions.hpp
ai/provider_interface.hpp
ai/agent_context.hpp
ai/tool_permissions.hpp
ai/moderation_interface.hpp
```

AI access must follow explicit permission scopes.

An AI agent must not automatically read every chat or decrypted file.

Local and remote processing must be clearly distinguished.

Remote model calls may disclose authorized plaintext to an external provider, and users must be informed.

## Moderation

Support moderation based on defined conduct and actual policy violations.

Potential actions:

- Rate limiting
- Flagging
- Attachment quarantine
- Temporary restrictions
- Human review
- Audited bans
- Appeals

Do not use inferred sensitive personal characteristics or lawful interests as grounds for punishment.

Detection methods may remain confidential, but moderation decisions must be governed by accountable rules and safeguards.

---

# 17 — SCREEN CAPTURE PRIVACY

Design a future cross-platform capture-protection interface.

Windows:

Evaluate OS-supported capture exclusion mechanisms, including `SetWindowDisplayAffinity`, subject to documented limitations.

macOS:

Document the limitations of application-controlled screen capture restrictions.

Linux:

Account for Wayland compositor permissions and the lack of reliable universal protection on X11.

No operating system provides absolute protection against privileged capture, external cameras, or compromised hardware.

Never claim that GhostWire is impossible to screenshot or record.

Provide future privacy diagnostics showing whether capture protection is supported, active, or unavailable.

---

# 18 — DESKTOP AND GUI STRATEGY

Do not build the complete GUI during A00.

Investigate:

- SDL3 platform integration
- Native GPU rendering
- Lightweight retained UI
- Accessible text and input
- Cross-platform font handling
- Efficient scrolling and virtualization
- Theme customization
- Multi-window support
- DPI scaling
- Screen capture privacy hooks

Lessons from earlier native UI development:

- Do not redraw unnecessarily.
- Keep event dispatch deterministic.
- Define input hit-testing and clipping semantics.
- Use explicit main-thread UI ownership.
- Profile before optimizing.
- Avoid expensive per-frame allocations.
- Cache text and geometry where beneficial.
- Ensure buttons and interactive controls are functional before cosmetic refinement.

Desktop rendering must not influence network protocol design.

---

# 19 — SECURITY ENGINEERING REQUIREMENTS

Create:

`docs/security/THREAT_MODEL.md`

Cover at minimum:

- Malicious peers
- Compromised community servers
- Malicious relay operators
- Offline password attacks
- Account impersonation
- Key theft
- Device compromise
- Malicious attachments
- Denial-of-service
- Replay attacks
- Traffic analysis
- Discovery poisoning
- Sybil attacks
- Vault lockout abuse
- Backup theft
- Metadata correlation
- Remote AI data exposure
- Supply-chain compromise

Define security boundaries and attacker capabilities explicitly.

Create:

`docs/security/SECURITY_INVARIANTS.md`

Each security invariant should eventually map to one or more tests.

No cryptographic subsystem may be considered production-ready solely because a unit test passes.

Plan for dependency auditing, fuzzing, protocol interop testing, and independent security review.

---

# 20 — PERFORMANCE AND RESOURCE CONTRACT

GhostWire must support ordinary consumer computers, low-powered servers, and dedicated infrastructure.

Plan for:

- Bounded queues
- Backpressure
- Resource quotas
- Efficient async I/O
- Cancellation
- Bounded worker pools
- CPU headroom
- Predictable memory ownership
- Graceful shutdown
- Large-history virtualization
- Efficient database access
- Incremental synchronization
- Minimal idle CPU usage

Do not assume all logical processors should be consumed.

Define thread ownership for each subsystem.

Use performance benchmarks at checkpoints where real functionality exists.

Do not invent target throughput or latency measurements without establishing realistic test workloads.

---

# 21 — DEVELOPMENT CHECKPOINT ROADMAP

Use the following as the initial milestone chain.

| ID | Checkpoint |
|---|---|
| GW-A00 | Repository & Build Foundation |
| GW-A01 | Core Lifecycle & Services |
| GW-A02 | Platform Abstractions |
| GW-A03 | Cryptographic Provider |
| GW-A04 | Identity & Device Trust |
| GW-A05 | Protocol Schemas & Versioning |
| GW-A06 | Transport & Privacy Policies |
| GW-A07 | Autonomous Discovery |
| GW-A08 | Encrypted Direct Messaging |
| GW-A09 | Offline Delivery & Mailboxes |
| GW-A10 | Friends & Contact Verification |
| GW-A11 | Local Encrypted Vault |
| GW-A12 | Vault Protection & Device Privacy |
| GW-A13 | Community Server Runtime |
| GW-A14 | Groups & Membership Encryption |
| GW-A15 | Encrypted File Distribution |
| GW-A16 | Federation & Portability |
| GW-A17 | Desktop Platform Shell |
| GW-A18 | Messaging & Community UI |
| GW-A19 | Voice, Video & Media |
| GW-A20 | AI Agent Framework |
| GW-A21 | Permissions & Moderation |
| GW-A22 | Plugins & Extensions |
| GW-A23 | Resilience & Recovery |
| GW-A24 | Cross-Platform Security Audit |
| GW-A25 | Performance & Release Baseline |

This roadmap is provisional.

Refine dependencies and milestone ordering where necessary.

In particular, basic vault-backed key protection may need to be implemented before persistent messaging, even if the full vault feature checkpoint appears later.

Never rely on insecure interim key persistence.

Each checkpoint must have explicit acceptance criteria.

---

# 22 — TODO SYSTEM

Create one authoritative `TODO.md`.

Each entry must contain:

- Unique ID
- Title
- Planned source location
- Dependencies
- Implementation status
- Validation status
- Acceptance criteria

Use:

```text
[ ] Planned
[~] Partial / experimental
[x] Implemented and validated
[!] Blocked
```

Track design acceptance separately from runtime validation.

Completed milestones move into `docs/COMPLETED.md`.

Never mark features complete simply because headers, stubs, or source directories exist.

Include testing and documentation work in the roadmap.

---

# 23 — GITHUB DEVELOPMENT POLICY

Prepare the repository for long-term collaborative development.

Recommended branches:

```text
main
develop
feature/*
fix/*
security/*
```

`main` contains accepted checkpoints.

Each accepted milestone should be tagged.

Example:

`GW-A00-FOUNDATION-BOOTSTRAP`

Include:

- Pull-request template
- Issue templates
- Build-status checks
- Code review guidelines
- Security reporting instructions
- Dependency update policy
- Contribution guidelines
- Git ignore rules
- Line-ending rules
- Repository license

Recommend a suitable open-source license, documenting the trade-offs between Apache-2.0, MIT, and MPL-2.0. Record the chosen license explicitly rather than leaving accidental ambiguity.

Do not create or push a remote repository without user authorization.

GitHub Actions should cover Windows, Linux, and macOS.

---

# 24 — AI DEVELOPMENT GUIDELINES

Create a detailed `AGENTS.md`.

Future AI coding agents must:

1. Read this master architecture.
2. Inspect the actual current source.
3. Confirm the authoritative checkpoint.
4. Understand module ownership.
5. Identify change scope.
6. Preserve unrelated behavior.
7. Implement complete functionality within scope.
8. Add relevant tests.
9. Run available validation.
10. Document unsupported test environments.
11. Update TODO and architectural decisions.
12. Produce a cumulative source package.

## Source-of-truth rule

The repository and supplied source archive are authoritative.

Never reconstruct code from memory when source exists.

Never silently replace working implementations with placeholders.

Never reset the project to an earlier checkpoint.

## Delivery requirements

For each implementation pass, provide:

- Checkpoint identifier
- Changed files
- Architectural changes
- Build status
- Test status
- Security considerations
- Known limitations
- TODO changes
- SHA-256 of delivered archive
- Next checkpoint proposal

Prefer change-scoped validation for isolated changes.

Run full regression validation when protocol compatibility, cryptography, schemas, platform foundations, or broad shared architecture changes.

## Handover continuity

Future AI agents must be able to understand:

- What exists
- What works
- What is experimental
- What is unimplemented
- What was validated
- What still requires host testing
- Which contracts must not be violated

No undocumented architectural assumptions.

---

# 25 — IMPLEMENT GW-A00 NOW

This is the only implementation checkpoint currently authorized.

## Required foundation

Create a functional C++20 repository with:

- Shared core library
- Application bootstrap
- Version reporting
- Basic configuration handling
- Error/result model
- Structured logging
- Path initialization
- Graceful startup and shutdown
- Build presets
- Unit-test infrastructure
- Platform abstractions needed by the bootstrap
- Cross-platform CI configuration
- Development tooling
- Architecture documentation

## Executable targets

Create:

```text
ghostwire-desktop
ghostwire-server
ghostwire-relay
ghostwire-cli
```

During A00 these may operate as console executables.

They must have distinct identities and clearly documented purposes.

At minimum, implement useful command-line behavior:

```text
--help
--version
--check-config
```

The exact supported options may differ between targets.

Each executable must return meaningful exit codes.

No executable may pretend to implement live networking before transport exists.

Do not build a fake Discord-style UI for A00.

## Tests

Implement real tests for:

- Version parsing/reporting
- Configuration validation
- Invalid configuration handling
- Filesystem path resolution
- Application lifecycle
- Expected exit codes
- Logging initialization and sensitive-value redaction where implemented

Run available tests through CTest.

## Windows scripts

Provide convenient PowerShell entry points for:

```text
configure
build
test
run
clean
diagnostics
```

Scripts must find or accept the proper Visual Studio toolchain and vcpkg location.

Do not hardcode personal machine paths.

Support Debug and Release.

Do not globally modify Windows execution policies.

## Documentation

At minimum, deliver:

```text
README.md
BUILD.md
ARCHITECTURE.md
ROADMAP.md
TODO.md
AGENTS.md
CONTRIBUTING.md
SECURITY.md

docs/architecture/MODULE_MAP.md
docs/architecture/DEPENDENCY_GRAPH.md
docs/architecture/TECHNOLOGY_DECISIONS.md
docs/security/THREAT_MODEL.md
docs/security/SECURITY_INVARIANTS.md
docs/development/CHECKPOINT_RULES.md
docs/development/VALIDATION.md
docs/checkpoints/GW-A00.md
```

Provide a security boundary map showing which components may access plaintext, private keys, network endpoints, and persistent records.

Provide future header/interface designs in documentation for modules not yet implemented.

Do not create compileable interfaces that imply nonexistent security guarantees.

---

# 26 — A00 ACCEPTANCE CRITERIA

A00 is accepted only when the following are satisfied or clearly reported as unverified.

### Repository

- [ ] Repository structure created
- [ ] Build scripts included
- [ ] Dependencies pinned
- [ ] Source hygiene configured
- [ ] GitHub workflows included

### Compilation

- [ ] CMake configure succeeds in the available environment
- [ ] C++20 shared core compiles
- [ ] Executables link
- [ ] Debug build validated
- [ ] Release build validated

### Runtime

- [ ] CLI help works
- [ ] Version output works
- [ ] Configuration validation works
- [ ] Invalid configuration returns an appropriate error
- [ ] Graceful exit works
- [ ] CTest suite reports actual results

### Architecture

- [ ] Module dependency map documented
- [ ] Header ownership defined
- [ ] Thread ownership policy documented
- [ ] Security boundaries defined
- [ ] Threat model established
- [ ] Future development contracts written
- [ ] Detailed TODO created

### Cross-platform

- [ ] Windows workflow configured
- [ ] Linux workflow configured
- [ ] macOS workflow configured
- [ ] Actual validation status reported separately

### Packaging

- [ ] Cumulative ZIP created
- [ ] ZIP contents verified
- [ ] SHA-256 calculated
- [ ] Build instructions included
- [ ] Known limitations reported
- [ ] Next checkpoint prepared

A missing verification must remain unverified, not be silently counted as a pass.

---

# 27 — FINAL DELIVERABLE

Produce:

**`GHOSTWIRE-GW-A00-FOUNDATION-BOOTSTRAP-source.zip`**

The archive must contain the cumulative source repository.

Include no private credentials, local caches, compiler outputs, or unrelated generated files.

Provide the SHA-256 checksum and actual validation report.

Also provide an initial proposed implementation prompt for:

`GW-A01-CORE-LIFECYCLE-SERVICES`

Do not begin A01 automatically.

---

# 28 — FINAL PROJECT PRINCIPLE

GhostWire is intended to be owned by its users, not controlled by its developers.

The architecture must favor:

- User autonomy
- Privacy
- Open interoperability
- Cryptographic accountability
- Independent hosting
- Long-term maintainability
- Cross-platform portability
- Honest security guarantees
- Predictable performance
- Reproducible development

**Build systems before features. Build trustworthy primitives before complex interactions. Build verified implementations before polished demonstrations.**

A00 should establish a foundation that future development can extend without rewriting the entire application.

Begin the architecture review and implement GW-A00.
