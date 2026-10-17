# A04-P02 identity module contracts

## P03 deletion clarification

Explicit local membership deletion invalidates matching self-authority trust
handles in that profile and releases their retained record/trust accounting.
Otherwise removing the latest owned revocation record could expose an older
accepted self-record again. Other authority contexts and other local profiles
remain untouched. A new explicit pin/accept after forgetting is a new local
trust decision; no permanent blacklist or cross-profile revocation is claimed.


Status: P01 design accepted by the owner; bounded volatile implementation in
P02. Read [ADR-008](../decisions/ADR-008-IDENTITY-AND-DEVICE-TRUST.md) for the
accepted product decisions and [P02 report](../checkpoints/GW-A04-P02.md) for
actual validation and limits. P01 itself added no runtime code. P02 implementation
acceptance and native Windows testing remain distinct from design acceptance.

## Module and lifetime

`ghostwire_identity` depends on `ghostwire_crypto` and header-only value
contracts (Result, Clock and resource values). It must not link core's Application
implementation, permissions, a vault, networking or UI. Permissions may consume
immutable identity values later; identity verification does not consume a future
permissions implementation. This refines the earlier broad future dependency
table and avoids an identity/permissions cycle.

A separate `ghostwire_composition` adapter links core plus identity and implements the
existing Service interface. Composition roots explicitly own/register it with a
CryptoService dependency. The adapter borrows the explicitly injected CryptoService,
obtains its provider only after start, and stops before it. Neither core nor
ApplicationContext gains a generic identity getter. Bootstrap registration changes
must be explicit, reviewed and keep ordinary startup free of identity creation.

| Implemented contract | Ownership, purpose and failure boundary |
|---|---|
| identity_id.hpp | Distinct fixed-size root/community/membership/device IDs; no implicit conversion between them or from an alias. Local handles include owning service identity and generation. |
| identity_record.hpp | Immutable, bounded public DeviceSet and DeviceEntry values; canonical encode/decode into caller-owned bounded public buffers. |
| identity_error.hpp | Enum-only errors and fixed descriptions; no caller text, key/alias/endpoint values or native exception strings. |
| identity_service.hpp | Owns private role-specific keys, profile containers and bounded local trust state; explicit create/select/revoke/delete operations; no ambient active profile. |
| identity_record.hpp verification functions | Record verification with independently supplied expected authority/scope/time and cancellation; cannot enroll a member or mutate trust implicitly. No separate verification placeholder header. |
| composition/identity_service_adapter.hpp | Core Service lifecycle, fixed diagnostics, explicit provider/admission/clock borrowing; no worker, queue or file store. |
| apps/cli/identity_session.hpp | Explicit local interactive composition; UTF-8 input, bounded 4096-byte command lines, no command-history or private-state files. |

All state mutations and high-level operations are serialized on the owner thread.
Immutable bounded public values may cross threads. request_stop and its owned
stop token may cross threads; the owner must join consumers before destruction.
Synchronous operations observe cancellation before/after provider calls and
before one commit point. Cancellation after that commit may race success, as in
A03. A failed operation must release staged keys/reservations and publish no
partial profile/device/revision update. Do not retry busy in a loop.

The service factory publishes a running owner only after successful creation;
its lifecycle is running -> stopping -> stopped. The adapter has its own
pre-start state and cannot restart after stop/failure. Failed start releases
partial state and remains unusable. No new operations begin while
stopping. Clear owned private handles during cleanup after consumers stop; retain
dependencies until cleanup completes. Repeated shutdown is safe. Cleartext keys
never pass through config, logger, JSON or generic serialization. Memory locking,
swap/dump and compromised-process limits remain exactly those of A03.

## Typed scopes and IDs

Identifiers are full 32-byte SHA-256 values computed through the provider over a
fixed ASCII domain, a single zero separator, then fixed-width public values:

| Type | Domain string | Remaining bytes, in order |
|---|---|---|
| ProfileIdentityId | GhostWire.ProfileRoot.v1 | root public key (32) |
| CommunityId | GhostWire.CommunityRoot.v1 | community public key (32) |
| MembershipId | GhostWire.Membership.v1 | CommunityId (32), membership authority public key (32) |
| ProfileDeviceId | GhostWire.ProfileDevice.v1 | ProfileIdentityId (32), device public key (32) |
| MembershipDeviceId | GhostWire.MembershipDevice.v1 | MembershipId (32), device public key (32) |

Domains and their null separator are literal bytes, not C++ object memory.
Never truncate for lookup/authentication, hash an alias into authority, compare
IDs of different types or treat an ID as proof of possession. Full keys/scopes
are checked alongside IDs; conflict is an error. No random record contains an
authority that automatically becomes trusted. A local profile handle is separate
from every public identifier and is never signed or exported.

## Public DeviceSet signing frame (version 1)

A complete allowlist makes local revocation and gap handling explicit without
an unbounded certificate chain. One profile root signs profile devices. One
membership authority signs devices for that membership and community. No device
may delegate more devices merely because it is present in this set.

| Field | Bytes | Required value / validation |
|---|---:|---|
| Domain | 23 | ASCII GhostWire.DeviceSet.v1 followed by exactly one zero byte |
| Version | 2 | Unsigned big-endian 1 |
| Suite | 2 | Unsigned big-endian 1 = A03 pure Ed25519 |
| Authority kind | 1 | 1 profile, 2 membership; all others rejected |
| Scope | 32 | All zero for profile; exact expected CommunityId for membership |
| Authority public key | 32 | Exact expected root/membership key, never selected solely from received input |
| Revision | 8 | Unsigned big-endian, 1..UINT64_MAX; overflow cannot roll over |
| Valid from | 8 | Unsigned big-endian UTC seconds |
| Valid until | 8 | Unsigned big-endian UTC seconds, exclusive |
| Device count | 2 | Unsigned big-endian, 0..32 |
| Device entries | 48 each | Public key (32), valid from (8), valid until (8), in that order |
| Signature | 64 | Pure Ed25519 over every preceding byte; not itself in the signed message |

The authoritative domain literal is ASCII `GhostWire.DeviceSet.v1` followed
by exactly one zero byte: 23 bytes. The signed header is 118 bytes, before
entries. The full frame is exactly 182 + 48 * device_count bytes, including
the signature: 182 bytes empty, 1718 bytes with 32 devices. Reject all other
lengths. Fixed buffer sizes and checked parser bounds avoid input-driven allocation.
Integers use explicit byte encoding. Never sign a struct, host-endian integer,
JSON string, formatted timestamp, locale-dependent text or prehash substituted
for the declared signature input. This prefix does not change Ed25519 into
Ed25519ctx. No algorithm negotiation, extensions, compression or nested frames.

Entries are strictly lexicographically sorted by their 32 public-key bytes;
duplicates and unsorted input are rejected rather than normalized after receipt.
Each interval is nonempty and contained within the enclosing interval. Empty
sets are allowed to revoke every device. A signer constructs canonical bytes;
a receiver must reject any size mismatch, trailing data or alternate encoding.
Device-purpose follows authority kind; a profile set cannot authorize membership
devices and vice versa. Aliases, profile-to-membership mapping, private labels,
contacts and permissions are absent from this public frame.

This is an identity signing representation only. A04 has no socket,
message envelope, network parser integration, enrollment or login handshake.
Do not assign A05 wire negotiation fields or claim remote authentication here.

## Verification and authorization order

1. Check operation/service state, profile handle owner/generation, bounded size,
   cancellation and admission before allocation or cryptographic work.
2. Check exact domain/version/suite/kind, exact length, sorted unique entries,
   timestamp/revision ranges and scope against the independently selected context.
3. Match the full expected authority key and computed typed identifier. The
   caller's expected authority must come from explicit trust input, never alias
   lookup or an auto-selected self-signed record.
4. Verify the signature with A03's provider. Produce only a verified record value;
   this step alone changes no trust state and proves no current live possession.
5. For explicit record acceptance, check the local pin/block/conflict state and
   revision. Stage any change, recheck cancellation, then atomically update the
   record/high-water mark on the owner thread. Equal bytes/revision are idempotent;
   conflicting equal revision blocks the context. An old valid signature cannot
   lower the revision. Failed/unknown records cannot consume retained trust slots.
6. For device recognition, recheck the current accepted record, validity, block
   state, requested scope and exact device key at the point of use. A saved
   verification result is not a durable authorization capability. Later protocol
   possession checks and action/resource grants remain mandatory separately.

Future A05/A06 network adapters must add bounded challenge lifetime/replay state,
audience and channel binding using a reviewed protocol. No reusable public
signature may be treated as a login proof. Arbitrary URLs embedded in identity
input never cause lookups, filesystem access or network fetches.

## Time and revision policy

Use the existing monotonic Clock for work/deadline checks. Take a separate explicit
UTC-seconds VerificationTime from the trusted composition caller for record
validity; never reinterpret steady_clock values as wall time. Test with injected
values. The range is 0..253402300799 seconds; arithmetic uses checked
subtraction after ordering checks, independent of native time_t width.

Default issued lifetime: 30 days, maximum accepted lifetime: 90 days. Require
valid_from <= now < valid_until, with no positive expiry grace. The verifier's
configured ceiling must be at most 90 days. Future-dated records are not yet
valid. Device intervals obey the same point-in-time check. These are application
defaults, not claims that a clock or expiration alone prevents replay.

Retain the largest observed trusted UTC value per service session. A backward
step exceeding five seconds returns time_untrusted; smaller steps use the retained
maximum. Recover only via explicit local time revalidation, never by resetting
revision state from received data. Forward jumps can expire credentials; they
never restore revoked devices. Rechecking time cannot make an old revision new.

High-water marks and conflict decisions live only in memory in A04. No restart
rollback resistance, synchronized revocation or indefinite offline validity is
claimed. Explicitly forgetting a trust context invalidates its local handles and
verification; reintroducing it requires new local approval. Do not silently evict
trust/revision state when admission fills.

## Profile isolation, key use and trust operations

Every operation takes its explicit profile context and expected scope. There is
no process-global current profile or fallback to another unlocked profile. Local
selection may return a generation-checked view, not raw key pointers. Deleting a
profile invalidates its handles and clears only its owned keys; other profiles
continue. This is logical separation within a trusted process, not an OS sandbox.

Explicitly create a profile with a root and initial profile device. Explicitly
create one membership authority/device pair for a selected pinned community.
The second operation prepares credentials only, not community admission. No
private association is inserted into a public record. Generate each key through
the provider and reject same-key role reuse inside a profile. Root/device keys
are separate from future session/vault/encryption keys. Imported raw private
keys, generic arbitrary-message signing and export are not A04 service APIs.

Only authority-owned operations may issue or update a device set. Device handles
cannot create a new authority, change trust pins or delegate enrollment. Device
rotation stages the replacement key and newer set, then commits them together.
Revocation/rotation APIs name the exact authority scope. Removing a profile
device does not alter independent membership-device sets. The private mapping
between them can support a later explicit lost-device workflow; no partial
local update is reported as global revocation. Test both the scoped denial and
the continued validity of unaffected scopes.
Remote revocation information may change local recognition but never destructively
delete locally owned keys, files or another profile. Future local cleanup requires
an explicit owner operation; memory clearing during shutdown is ordinary RAII.

## Resource and error policy

Defaults and P02 ceilings: 8 live local profiles; 32 devices per authority; 32 prepared
memberships per profile; 128 private key handles across the identity service;
256 accepted public trust contexts; 2 MiB retained public record bytes; 8 KiB
input envelope ceiling; one synchronous operation in progress, zero queued work.
The exact DeviceSet length remains stricter than the envelope ceiling. These
individual maxima are not simultaneously guaranteed: global handle/byte and
application admission must also fit. A03's provider budgets can reject earlier.
Counts/bytes can be lowered within ServiceLimits, not raised above these P02
ceilings. All size/count multiplications/additions are checked; failure leaves
old state. The adapter holds one application active reservation for declared
public-record bytes plus 32 bytes per private seed. This is logical admission,
not an allocation or measurement of total object overhead, provider workspace,
allocator overhead, CPU or process RSS.

Monotonic work deadline default 100 ms, configurable 1..5000 ms; inspect before
and after bounded crypto and before commit. It cannot preempt OS RNG/crypto.
Clock, provider and budget objects outlive the service. No new executor or job
framework, sleeping retry or detached thread is warranted by A04.

Error categories: invalid_argument, invalid_state, invalid_handle, wrong_scope,
malformed_record, unsupported_version, unsupported_suite, invalid_signature,
untrusted_authority, blocked, not_yet_valid, expired, time_untrusted,
stale_revision, conflicting_revision, unauthorized_device, revision_exhausted,
resource_exhausted, busy, cancelled, deadline_exceeded and crypto_failure.
Raw provider errors may map to fixed nested enums; arbitrary strings may not.
Diagnostics expose counts and service state only; persistence_available stays
false. No alias, ID, fingerprint, record, key material or membership graph is logged.
The local console separately allows explicit show/list output of selected public
values and labels. Such output may remain in terminal capture; it is not a log
redaction bypass or permission to persist private state.

## Service and console surfaces

IdentityService exposes explicit create/list/rename/delete profile operations,
community-key pinning and independently keyed membership preparation; membership
lookups retain profile and community scope. It supports add/rotate/revoke devices,
renewed complete records, explicit authority pins, independent-verification and
block flags, record acceptance, conflict resolution, forgetting and current-device
recognition. There is no root replacement, backup, recovery or arbitrary-message
signing entry point.

`ghostwire-cli --identity-session [--data-root ABS_PATH]` constructs the real
Application, CryptoService and IdentityServiceAdapter. Ordinary bootstrap stays
one-shot and creates no profile. Session input selects local handles explicitly;
the selection is owned by that console, not a global current-profile facility.
Its help command is the authority for the available console subset of the API.
The composition uses OS UTC as the explicitly selected trusted time source;
this is not authenticated network time, and service rollback detection remains
active. Quit/end-of-input performs ordered cleanup. Abrupt process termination
does not promise a completed cleanup callback or protection from OS dumps.

## Gates

A11-PRE still blocks private profiles, trust/session state, credentials, keys,
backup/export and recovery persistence. AUTH-PRE still blocks privileged listeners
and remote actions. No gateway, discovery, DM/group protocol, GUI, AI or media
implementation is included. [A04 test plan](../checkpoints/GW-A04-TEST-PLAN.md)
defines negative cases for the implementation. The [P02 report](../checkpoints/GW-A04-P02.md)
maps them to actual tests and records executed outcomes; a planned case or
historical HF01 pass is not P02 validation. Stop after P02 for independent review
and Windows host testing; no P03 or A05 work follows automatically.
