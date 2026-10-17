# ADR-008: Isolated profiles and scoped device authority

## P03 scoped update

P03 implementation clarification (2026-10-16): deleting an owned membership
also invalidates that profile's matching self-authority trust handles. This
prevents an older accepted set from regaining recognition once the latest owned
record is gone. Other profiles are unaffected; explicit later re-pinning remains
a new owner decision. This refines deletion without changing the accepted
identity hierarchy, signed representation or deferred recovery scope.


Date: 2026-10-15. Status: **accepted by the owner for bounded A04-P02 implementation**
after independent architecture review and source-integrity verification.
Origin: GW-A04-P01-IDENTITY-DESIGN; that delivery was documentation only.
Implementation: GW-A04-P02-IDENTITY-DEVICE-TRUST, separately subject to review
and native Windows acceptance. This decision is not a network authentication
protocol or production security certification. The owner explicitly allowed
progress without a personal macOS host; HF01 hosted native evidence is separate.

## Accepted decisions

1. Give each local profile an independent root and independently generated device
   keys. Keep its contacts, trust decisions, memberships and private state separate.
2. Within each community, use a separate membership authority key and separate
   device keys. Do not send the global profile/device keys or a certificate linking
   them to that community by default. A community sees its own stable membership.
3. Treat short numerical IDs and names as nonunique labels. Address records by
   full typed cryptographic identifiers and explicit scope; never merge by label.
4. Use an authority-signed, versioned complete device set for bounded local device
   authorization. Checking a signature, recognizing a device, proving current
   possession, granting community membership and granting an action are separate.
5. Implement A04 initially as explicit, volatile headless operations. Device
   replacement/revocation are supported within the session. Root recovery and
   root-key continuity require a later approved design and protected storage.

The owner approved this identity model, including independent community keys
and the limited recovery promise, before authorizing P02. These decisions do
not approve persistent formats or a remote authentication/messaging protocol.
The subsequent interactive-interface request is implemented as an explicit local
CLI session using the same volatile services; it adds no hidden profile creation,
key store, listener or future-checkpoint capability.

## Existing requirements

The master contract requires local cryptographic identity, distinct root/device
lifetimes, contact verification, independent hosting and no hardware/IP identity.
The owner additionally requires multiple isolated profiles, distinct credentials
for joined servers and independent people when human-facing IDs coincide.
Those requirements are retained without asking the owner to restate them.

The discussion of two independent proof/signature keys did not settle their exact
roles. The recommendation is an authority key that authorizes replaceable device
keys. A device must later prove possession through an approved channel-bound
authentication protocol. Do not invent two signatures on every login, or call
two secrets stored together two-factor authentication. A different requirement
for two simultaneous independent proofs must receive its own threat-model review.

## Principals and identifiers

| Concept | Meaning and authority |
|---|---|
| Local profile handle | Process-local owner/slot/generation capability; never sent to peers or derived from a display name. Generation overflow fails; a deleted slot cannot revive an old handle. |
| Profile identity | Full domain-separated SHA-256 identifier of that profile's root public key. Share only for an explicitly chosen contact use. It identifies a key, not a unique human. |
| Profile device | Separate key authorized by that profile root; no serial numbers or machine fingerprint. One machine can own devices for multiple profiles. |
| Community identity | Full identifier of the community authority public key, pinned through an explicit later trust/enrollment flow. An endpoint or server name is not this identity. |
| Membership identity | Full identifier scoped to the pinned community identity and a new membership authority public key. A community's admission/ban state remains separate. |
| Membership device | Independent key authorized by the membership authority for that one community. A copy of a public membership ID gives no signing capability. |
| Human alias | 1–20 ASCII digits, exact bytes including leading zeros, nonunique. Unicode display names are separately bounded to 256 UTF-8 bytes; no name grants authority. |

Local membership lookups use (local profile handle, community identity,
membership identity). Remote membership lookups use (community identity,
membership identity). Public root/device bindings are not indexes across all
profiles. Full public keys are retained alongside hashes and compared; a detected
identifier/key conflict fails closed rather than overwriting an existing entry.

Example: two users both display 12345 in the same community. They have distinct
membership keys and membership IDs. A search may return both; an invitation,
message, ban or permission change must select the full intended membership.
Neither can read the other's chat or inherit its role. A later UI may display a
short fingerprint for disambiguation, but must retain the full ID internally.
Changing an alias retains the membership, permissions and bans. Creating a new
key is a new principal; this architecture does not solve Sybil ban evasion.

## Key hierarchy and disclosure

| Key role | Generated and owned by | Signs / authorizes | Default disclosure |
|---|---|---|---|
| Profile root R | Explicit profile creation | Profile device-set record | Public R only to explicitly selected contacts; never automatic community login |
| Profile device D | That profile, independent generation | Future approved device operations | Only the relevant contact/device relationship |
| Membership authority M | That profile for one pinned community C | Membership device-set record scoped to C | Public M and its membership ID to C |
| Membership device K | That profile for (C, M, local device) | Future approved community device operations | Public K and M's scoped authorization to C |

R/D and M/K use the same authority/device concept in different scopes. They are
independently generated with the accepted provider, not derived from aliases,
passwords, IPs, hardware or each other. Do not convert Ed25519 keys into another
algorithm or reuse signing keys as AEAD/vault keys. Existing crypto handle types
remain behind role-specific identity owners; no arbitrary signing oracle or key
export is exposed by the identity service.

The profile-to-membership mapping stays local. No R-to-M or D-to-K linking
certificate is sent by default. A04 does not yet enroll devices across machines
or synchronize M. Later device enrollment needs an authorized protected transfer;
copying a public device-set record alone never transfers private-key ownership.

Per-community credentials reduce a stable cross-community cryptographic link.
They cannot hide IP/timing, reused aliases, content, operator collusion or an
explicitly shared profile fingerprint. Community endpoint changes preserve a
membership only after its same pinned authority is verified. A new community key
does not silently inherit old membership trust. Privacy routing remains A06.

## Trust, possession and permission

An incoming self-signed root or device set is untrusted input. Parsing and a valid
signature may produce a structurally verified record, never an automatic trusted
profile or an authenticated connection. The verifier takes the expected authority,
scope and locally selected trust context independently from the record.

Local trust states are unknown, explicitly pinned, independently
verified, blocked and conflict. A signature cannot move unknown to trusted. A
trusted local user action pins an exact fingerprint; out-of-band comparison may
mark it independently verified. An already trusted binding cannot be replaced
by a matching alias, a remote assertion or a newer untrusted key. Blocking denies
use without deleting that person's local keys or data.

Device recognition additionally requires the latest accepted device set, current
time validity and an active local profile context. Later authentication still
requires fresh possession/channel binding, and later AUTH-PRE must approve the
requested action/resource. A community server can deny admission or remove a
membership but cannot gain a profile root, private DM access or local wipe power.

## Revocation, rotation and recovery

Use a complete bounded device set, signed by its authority, with a strictly
increasing revision. Replacing a device removes its old public key and adds a
new independently generated key in one staged update. Removal immediately denies
new operations under the old device in this local session. Previously issued
public signatures and copied plaintext cannot be erased.

Retain the highest accepted revision in each local trust context. Older sets
are rejected even if signatures and timestamps remain valid. An identical set
at the same revision is idempotent; different content at that revision marks
conflict and denies use until an explicit local resolution. Untrusted data cannot
clear the conflict or force eviction of the revision state. Complete newer sets
may skip revision numbers because they contain the entire current authorization.
Revision overflow is an error, not wraparound. Remote distribution is absent in
A04; an offline verifier can only enforce revocations it has learned.

Revocation is explicitly scoped to one authority/device set. Revoking profile
device D does not revoke its independent community keys K. Keep the
local D-to-K mapping private. A later lost-device workflow must fan out authorized
revocations to every affected membership and report each delivery/freshness
limit. A04 exposes scoped revocation only and must never label it a completed
network-wide device revocation. A stolen authority key is a different, more
serious event than a stolen device key.

Root or membership-authority replacement changes its cryptographic identity.
It does not inherit contacts, roles, bans, membership, prior verification or
history automatically. A later continuity design must distinguish routine
rotation with the old authority available from compromise/loss. A04's bounded
implementation explicitly defers root continuity and recovery rather
than presenting an unsupported operation as successful.

For now, no recovery phrase, hidden master key, server reset, plaintext backup or
social recovery is introduced. Session shutdown loses volatile private keys.
Root loss without authorized protected backup is unrecoverable. Before A11-PRE
enables persistence, review encrypted root/membership backup, device enrollment,
restoration of revision state, rollback resistance and lost/compromised root
policy. Recovery of one membership must not merge it with another alias holder.

## Canonical public data and implementation boundary

[IDENTITY_DESIGN.md](../architecture/IDENTITY_DESIGN.md) specifies a reviewable
device-set signing frame, bounded validation order, ownership and error contracts.
The frame is public application authorization data signed by existing pure
Ed25519. It is not a new primitive, network handshake, E2EE protocol or selected
A05 wire format. A future wire codec may carry it only after its own review.

Public cryptographic records may occupy ordinary memory under sign_public.
Private labels, profile handles, contact graph, root-to-community mappings,
recovery material and message contents are excluded. Public records still reveal
relationships to intended recipients; they must not enter ordinary logs, caches
or unauthenticated discovery. The CLI's explicit public show/list commands may
be captured by the terminal or a caller's redirection; they never export private
keys. No identity state is written by GhostWire to disk.

## Alternatives and trade-offs

| Alternative | Assessment |
|---|---|
| Short ID as account key | Reject: collisions/copies could merge authority or data. |
| One root/device identity exposed to every server | Simpler, but creates an unnecessary shared correlator and weakens server separation. |
| Only one signing key per profile | Does not provide separate replaceable device authority. |
| M plus a globally reused D | Still links the device across communities. Prefer scoped K. |
| Both authority and device sign every routine request | Keeps authority online and adds complexity without an established independent-factor benefit; not proposed. |
| Derive all credentials from one seed now | Introduces a KDF/recovery/export design absent from A03; independent generation keeps this pass within its provider. |
| Persist keys temporarily until the vault arrives | Reject under A11-PRE. |

The extra membership key pairs increase private-handle use and future backup
complexity. Global quotas and lazy explicit creation bound this cost. The design
does not promise one person per identity, universal revocation freshness,
anonymous activity, account recovery or protection from an unlocked compromised OS.

## References reviewed on 2026-10-15

- [RFC 8032](https://www.rfc-editor.org/rfc/rfc8032), Ed25519 and its encoding/test
  vectors. We retain A03's pure Ed25519; a message prefix is not Ed25519ctx.
- [RFC 8725 sections 3.8–3.12](https://www.rfc-editor.org/rfc/rfc8725), issuer,
  audience and purpose-specific validation. Its JWT guidance informs the
  checks by analogy; GhostWire does not adopt JWT or claim RFC 8725 conformance.
- [NIST SP 800-63B-4](https://pages.nist.gov/800-63-4/sp800-63b.html), distinct
  authentication factors. No AAL or multi-factor claim is made for this design.
- Master contract sections 07–16; ADR-003, ADR-006; A03-HF01 handover.

These references do not certify this application composition. Independent
security review remains required before production identity/authentication claims.
