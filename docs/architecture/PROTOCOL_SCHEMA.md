> **A05-P02 implementation note (2026-10-18):** The previously proposed v1.0
> schema now has a bounded C++20 strict in-memory implementation for kinds
> 1/2/10. See [GW-A05-P02](../checkpoints/GW-A05-P02.md) and
> [ADR-010](../decisions/ADR-010-RESTRICTED-WIRE-CODEC-P02.md).
> P01-specific 'not implemented' statements below are historical design
> records; they are not permission to use this as an authenticated network
> interface. Authentication/E2EE/transport remain absent.

# GhostWire protocol schema v1.0 — design contract (A05-P01)

**Original A05-P01 design state:** proposed; no codec, connection, listener, discovery, remote
login, relay, messaging or persistent protocol state exists. **Runtime stays
0.4.1.** See [ADR-009](../decisions/ADR-009-CANONICAL-PROTOCOL-SERIALIZATION.md)
for the serialization decision and [`schemas/ghostwire-wire-v1.cddl`](../../schemas/ghostwire-wire-v1.cddl)
for the machine-readable draft skeleton. Written constraints here are normative
for the proposed profile where they are stricter than CDDL alone.

## 1. Separate representations and trust boundaries

| Object | Wire role / audience | Security classification | Trust implication |
|---|---|---|---|
| Frame length | Transport-neutral frame delimiter | Untrusted public size | No authority |
| Envelope (version/kind) | Dispatch and compatibility | Untrusted public metadata | No authorization |
| Routing hint | Per-hop opaque route token, optional hop budget | Untrusted and potentially correlating | Never an identity or permission |
| Capabilities offer/selection | Compatibility candidates | Unauthenticated advisory until future transcript binding | Never establishes authenticated negotiation |
| Public DeviceSet | Existing A04 signed frame, copied **unchanged** as byte string | Public but relationship-revealing; signed by A04 authority | Valid signature only proves signer key; no pin, membership, possession or permission |
| Future possession/authentication proof | Reserved kind 20 | Authentication-related; design not specified | Cannot be processed under v1.0 |
| Future encrypted container | Reserved kind 30 | Intended confidential payload; no format/guarantee yet | Cannot be processed under v1.0 |
| Eventual permission decision | **Not** a wire field | Local/server authorization | AUTH-PRE must independently gate action/resource |

Existing public record IDs are 32-byte full domain-separated identifiers as in
IDENTITY_DESIGN.md. DeviceSet verification requires an independently selected
expected authority, scope, trusted time, accepted record revision and explicit
local trust; receiving a frame supplies none of these. Profile root/device
keys and per-community membership/device keys are **not** interchangeable.
Aliases and local profile handles do not occur in v1.0 envelopes. No globally
shared account ID, hardware fingerprint or public root-to-membership binding.

## 2. Framing, type system and canonical bytes

Each frame is `uint32_be(N) || CBOR_bytes[N]`, where `1 <= N <= 65536`.
N includes the whole encoded envelope and excludes the four-byte prefix.
Exactly one top-level definite-length CBOR map; no trailing bytes. A stream
reader must reject N=0, N>65536 and truncated/overflowing reads *before* body
allocation. Partial frames are not accepted as completed messages. Datagram
transport is not currently designed; a future adapter must preserve boundaries.

The decoder permits only unsigned CBOR integer (major type 0), definite byte
strings (2), definite arrays (4) and definite maps (5). No text strings,
negative integers, tags, floats, bool/null/undefined, indefinite lengths,
break markers or CBOR sequences at v1.0. Integers, map/array sizes and byte-string
lengths use RFC 8949 preferred shortest encodings. CBOR integer multi-byte
arguments are network-big-endian as specified by RFC 8949. Map keys are unsigned
integers 0..23, encoded as **one byte**, strictly increasing. This restricted
registry yields the same order as RFC 8949 deterministic byte sorting. Keys are
unique at every nesting level; duplicate or out-of-order key is a fatal parse
error, not first/last-wins. Never silently normalize a noncanonical message.

Envelope (CBOR map) required keys:

| Key | Value | Meaning |
|---|---|---|
| 0 | unsigned 1 | Major version |
| 1 | unsigned 0 | Minor version |
| 2 | unsigned 1, 2 or 10 | Message kind |
| 3 | definite map | Kind-specific body |
| 4 | optional definite map | Untrusted routing hint, not identity |

No unlisted keys are valid at 1.0. Presence/absence is significant; no implicit
nil/default substitution. The encoder emits fields in key order; omit key 4
unless explicitly supplied. Empty top-level body is not valid for the defined
kinds. Do not serialize C++ object layouts, native-endian integers, a locale,
formatted timestamps, or an identity record's internal object memory.

The optional routing map has required key 0: *exactly 16 opaque bytes*,
randomly scoped to one route/context by a future routing owner, and optional
key 1: unsigned hop hint in 0..16. A routing token must not be derived from a
stable identity, profile key, membership ID, alias or device fingerprint; it
has no authenticity, global-uniqueness or anonymity claim. No peer-supplied
endpoint or address strings are defined. Routing hints may be changed in transit.

## 3. Kinds and bodies: registry 1.0

| Kind | Name | Body map fields | Allowed v1.0 meaning |
|---:|---|---|---|
| 1 | `version_offer` | key 0: array of 1..8 unique `[major, minor]` pairs in increasing numeric order; key 1: array of 0..32 unique ascending unsigned capability IDs | Advisory supported combinations; v1.0 implementation emits `[[1,0]]` and empty capabilities |
| 2 | `version_select` | key 0: exactly `[major,minor]`; key 1: array of 0..32 unique ascending capability IDs | Advisory selection; caller must compare with *its own* prior offered and local values; not authorization |
| 10 | `public_device_set` | key 0: one byte string containing exactly one A04 DeviceSet frame, 182..1718 bytes, without encoding changes | Structural carrier only; identity service separately verifies bytes, scope, pin and revision |
| 20 | `auth_proof` | **RESERVED — no admissible body** | Reject; not a login/challenge design |
| 30 | `encrypted_container` | **RESERVED — no admissible body** | Reject; not an AEAD/DM/MLS design |

`version_offer` and `version_select` require a version entry/pair supported by
this implementation; v1.0 only `[1,0]`. Any nonempty capability list is **not
supported by v1.0**, even if syntactically valid as a list: reject until a
separate registry maps IDs to reviewed behavior. Duplicate entries and
unsorted arrays fail before any semantic action. Unknown kinds reject. The
presence of reserved kind 20/30 always rejects, even if it carries valid CBOR.

`public_device_set` only wraps bytes. The independent identity validator
retains its **A04** domain `GhostWire.DeviceSet.v1\0`, suite=1 Ed25519,
big-endian record fields, canonical device sorting, exact length and explicit
expected authority. CBOR does not replace or re-sign A04 bytes. In particular,
the encoded public key or asserted scope is never sufficient to choose a trusted
profile, community or local authorization state.

## 4. Versioning, unknown fields, capabilities and suites

- Major changes denote incompatible or security-sensitive representation
  changes; they require a separate reviewed decoder/version gate, never a
  fallback parse. Minor additions may introduce **explicitly registered**
  non-critical optional fields, but only for a later supported minor version.
  v1.0 rejects every unknown key and minor>0. An explicitly optional future
  field may be skipped only after version negotiation, bounds validation and
  a published interpretation classifying it as non-security-critical.
- Reserved field numbers 16..23 are not a catch-all extension escape hatch:
  they are rejected until a later schema assigns them. Security-critical
  fields are always required/understood before activation. A parser shall not
  carry unknown fields into signatures and then validate re-encoded known
  fields. It must reject or keep *exact* canonical bytes under a reviewed rule.
- For a future authenticated negotiation, bind complete byte-exact offers,
  selected `(major, minor)`, cipher/authentication suite identifiers,
  capabilities, parties/roles, freshness and channel context to the verified
  transcript. An unbound offer/selection is attacker-editable, so versions
  exchanged under this draft **must not be treated as downgrade-resistant**.
  If an authenticated downgrade check cannot be performed, prohibit privileged
  operations; never silently reconnect under weaker privacy policy.
- `DeviceSet.suite=1` is the existing pure Ed25519 signature suite **inside the
  A04 record**, not a protocol AEAD, transport cipher suite, E2EE or login
  algorithm. The **future** suite registry starts unassigned in v1.0; unknown,
  retired or unsanctioned suites fail closed. Suite numbers must be explicit and
  versioned, not dynamically inferred from key length.
- No replay prevention is possible from decoding alone. Future authenticated
  sessions must have reviewed per-context replay/freshness state, limits,
  epochs and expiry. The A04 DeviceSet monotone revision/validity rules are
  separately preserved, not generalized to all frames.
- Version migrations operate on explicitly decoded, validated value objects,
  never re-interpret raw old bytes in place. Old signing domains and old pinned
  trust do not silently transfer to a new identity or community principal.

## 5. Resource, validation and ownership contract

**Limits per candidate frame** (hard ceilings, not defaults to be enlarged by
untrusted input): frame CBOR <=65536 bytes; prefix exactly 4; nesting depth
<=4 containers; <=16 key/value pairs in any map; <=64 entries in any array;
<=512 total decoded items; any byte string <=49152 bytes; A04 DeviceSet <=1718;
version offers <=8, caps <=32 and routing tokens exactly 16 bytes. The
smaller of per-field and overall limits wins. Use overflow-checked
size_t/uint32 arithmetic before addition, multiplication or indexing. Reject
large claimed lengths without reserving large buffers or recursively allocating.

**Order for future P02 decoder**:

1. Caller chooses a bounded input slice, deadline/stop token, work/admission
   budget and semantic scope. Check service state/stop/deadline; reserve a
   bounded operation slot before expensive work. No raw input logging.
2. Read exactly four bytes; compare unsigned N with maximum and remaining
   caller budget before allocation. Require complete frame length; reject
   truncation/trailing complete-frame bytes in single-frame decode.
3. Traverse bytes with a checked cursor; at each token first verify remaining
   length/recursion/item and work budgets. Check preferred lengths/order and
   forbidden major types before constructing values. Do not call unbounded
   generic recursive parsers or inflate/compress data.
4. Validate the envelope required/optional keys, versions, registered kind and
   known field types. Validate kind-specific cardinality, duplicates, numeric
   ranges and exact A04 record-carrier size; reserve future kinds as errors.
5. Create only a staging value owned by the caller, not a trust or service
   mutation. Recheck cancellation/deadline before a single publication of a
   fully validated value. Input buffers remain caller-owned/borrowed only
   until decode returns; published value owns bounded copies where needed.
6. **Outside the codec**, a separately authorized owner selects expected
   community/profile authority and calls A04 record verification. Later
   connection proof and action permission require AUTH-PRE and reviewed
   protocol. No external input alone may delete keys or modify local trust.

Maximum *new decoded-state allocation* budget: <=131072 bytes per admission,
including staging and copies, and no more than one working frame per operation.
Use a pre-reserved fixed parser stack (max depth 4), no input-size-driven
`std::vector::reserve` prior to bounds checks, no worker queue, and explicit
lease release on every failure. A streaming transport will additionally have
per-peer cumulative quotas, buffer backpressure, timeouts and admission limits;
A05-P01 creates none of those mechanisms. Cancellation before commit aborts with
no published value; cancellation racing after commit may observe success, and
must be documented. No parser exception text, raw bytes, keys, endpoints,
aliases, or user-controlled strings enter ordinary error/log messages.

Proposed fixed diagnostic classes: `incomplete_frame`, `frame_limit`,
`unsupported_version`, `unsupported_kind`, `unsupported_capability`,
`noncanonical_encoding`, `malformed_frame`, `resource_limit`, `cancelled`,
`deadline_exceeded`, `operation_busy`. These are *proposed enum values only*;
no public C++ ABI/API or Error mapping has been implemented.

## 6. Later C++20 integration boundaries (NOT implemented)

A pure `ghostwire_protocol` library should own strict CBOR frame encode/decode
and immutable public result values, with no transport, vault or identity private
key ownership. `identity` keeps the existing provider and A04 record checks;
composition/application coordinates the two through explicit dependency
injection. Proposed direction: `apps / composition -> protocol` and
`apps / composition -> identity -> crypto`; `protocol -> only public
core header-only Result/resource types` where useful. **Never** add
`identity -> protocol -> identity` or `core -> protocol -> core` cycles. Parser
functions accept borrowed `std::span<const std::byte>` and caller-owned budgets;
returned objects own their public byte buffers. No global registry,
unbounded executor, background state, network socket, or security-shaped stub.

C++ API names, compiler dependencies and exact CBOR library are P02 review
subjects; this document is deliberately *not* a compilable pseudo-API.

## 7. Acceptance and limits

The [test plan](../security/PROTOCOL_TEST_PLAN.md), [golden examples](../../tests/fixtures/protocol/v1-design-vectors.json)
and [P02 prompt](../checkpoints/GW-A05-P02-PROMPT.md) control later
implementation. Cross-platform byte-equality, signed record-carrier fidelity,
malformed-input rejection, expiry/revision preservation, fail-closed negotiation,
parser resource use and fuzzing require actual tests in P02. Passing these
**cannot** prove anonymity, remote authentication, E2EE, traffic confidentiality,
permission correctness or resistance to compromised endpoints.
