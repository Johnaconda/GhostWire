# ADR-009 — Canonical protocol serialization and versioning (proposed)

**Checkpoint:** GW-A05-P01-PROTOCOL-SCHEMA-VERSIONING-DESIGN  
**Date:** 2026-10-17  
**State:** Proposed for owner review; not an implemented or deployed wire format  
**Supersedes:** No earlier identity-signing frame or cryptographic decision.

## Context and decision

GhostWire needs identical, bounded, unambiguous public wire representations across
Windows, Linux and macOS. Its existing A04 `GhostWire.DeviceSet.v1` frame is an
already implemented and signed **public identity record**, not an A05 message
frame. Its bytes, suite 1 (pure Ed25519), signature and verification rules stay
unchanged. A new wire envelope must not cause a self-signed record to be trusted,
a live device to be authenticated, or a community principal to be admitted.

**Propose** RFC 8949 CBOR with a *strict GhostWire deterministic subset* and an
unsigned 32-bit big-endian transport-independent length prefix. All envelope and
body map keys are nonnegative integers and sorted by their CBOR encoded byte
representation per RFC 8949 §4.2.1. Encode each integer and length in its shortest
CBOR form; definite lengths only; reject duplicate or out-of-order map keys,
indefinite forms, tags, floats, negative integers, simple values, unassigned
fields, trailing bytes and multiple top-level items. Text is forbidden in the
v1.0 core frame. Byte strings are opaque, never parsed as nested CBOR unless a
future reviewed message schema explicitly says so. This is a protocol-specific
subset; generic valid CBOR is *not* necessarily valid GhostWire CBOR.

The four-byte prefix is not a cryptographic domain separator. It is a length for
framing only, never trusted before bounds checking. A05-P02 must independently
review and pin a maintained CBOR codec with tested strict-mode behavior, or
implement only a minimal profile-specific decoder after explicit review; this
ADR **does not** authorize an ad-hoc generalized serialization framework or
creation of original authentication/E2EE algorithms.

## Alternatives

| Candidate | Determinism and safety | Dependency/compatibility | Decision |
|---|---|---|---|
| RFC 8949 CBOR with strict subset | Deterministic encoding is specified; strict rejection and independent byte vectors possible; untrusted parser still needs hard limits | Compact binary; versioned numeric field registry; CDDL can document model; future maintained implementation must be selected | **Proposed** |
| Protocol Buffers | Deterministic output is explicitly *not* canonical across versions/implementations and unknown-field behavior; extra canonicalization rules needed before signing | Mature tooling, but generators/runtime add footprint; unknown-field retention complicates security-critical semantics | Reject for exact signed wire bytes |
| JSON with canonicalization (e.g. JCS) | Possible deterministic text, but duplicate keys, Unicode normalization, numbers, and input parser variants need additional reviews | Easy inspection, larger payloads and parser surface | Reject for compact hostile-input core frames |
| Hand-designed TLV/fixed binary | Can be deterministic but becomes an application-specific new encoding with extensive parser/revision burden | Small dependency, larger self-maintenance/interop risk | Reject beyond the fixed 4-byte length prefix |
| FlatBuffers/Cap'n Proto | Fast access and useful schema tools, but validated untrusted payloads and canonical signing profile still need domain-specific review | More complex builders/ABI/toolchain dependency | Defer; not selected |

CBOR selection is conditional on an independently examined decoder implementation,
upstream maintenance/license review, pinned version and cross-platform tests.
Do not auto-accept generic CBOR, unknown fields or a schema that admits
semantically equivalent encodings. Representation is not encryption or proof.

## Wire and security decisions

1. v1.0's core supports only envelope version **major 1, minor 0**. Field and
   kind registries are closed for this version. Unsupported versions and kinds
   fail closed with static error categories. The decoder never interprets a new
   security-critical field by guessing or by dropping it.
2. A bounded, *unauthenticated* capability offer/selection can describe only
   a potential wire version and feature set. It cannot authenticate a peer,
   authorize an action, set a privacy downgrade policy or bind a connection.
   An eventual reviewed handshake must bind both offers, selected algorithms,
   roles, nonces, negotiated features and the selected version to a fresh
   authenticated transcript/channel; until then negotiation grants no trust.
3. Future authentication proofs, signed content and encrypted containers get
   distinct kinds and domains. They remain **reserved/rejected** at v1.0 and
   require separate accepted designs, key-purposes, replay/clock/state rules,
   crypto-suite registry and security review. Do not equate A04 device-set suite
   1 with a transport or E2EE cipher suite.
4. A routing hint is neither identity nor permission. Relays can change it;
   applications must never select a trusted principal from a hint, alias, IP
   address, short fingerprint or asserted community. No public root-to-membership
   mapping is introduced on the wire by default.
5. No compression, CBOR sequences, nested unbounded structures or implicit
   conversions are permitted. No runtime/service/identity mutation occurs from
   merely decoding a frame; semantic verification belongs to an explicit caller.

## References (design inputs, not certification)

- [RFC 8949 §4.2.1](https://www.rfc-editor.org/rfc/rfc8949.html#section-4.2.1), deterministic CBOR and preferred serialization.
- [RFC 8610](https://www.rfc-editor.org/rfc/rfc8610), CDDL representation.
- [Protocol Buffers: not canonical](https://protobuf.dev/programming-guides/serialization-not-canonical/).
- Accepted [ADR-008](ADR-008-IDENTITY-AND-DEVICE-TRUST.md), identity-scope separation.
- Accepted [ADR-006](ADR-006-CRYPTO-PROVIDER.md), primitive provider only.

**Approval gate:** owner design acceptance and P02 implementation authorization.
