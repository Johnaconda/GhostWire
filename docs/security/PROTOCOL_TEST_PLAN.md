> **P03 candidate security gate:** Added a bounded C++ test probe and
> seeded independent Python-vs-C++ differential checker plus a strict
> impossible-offer regression. The Python reference is not QCBOR/TinyCBOR;
> external maintained implementation interoperability, allocation injection,
> native MSVC/macOS and independent network-exposure security review remain
> separate unfulfilled requirements. See GW-A05-P03.md.

> **P02 implementation evidence:** The C++ test `protocol-unit` exercises the
> frozen P01 examples, additional invalid types, admission/deadline/stop quotas,
> route limits, signed A04 carrier isolation and deterministic 15,000-input
> mutation corpus. The independent Python AST-based test-only checker verifies
> the P01 fixture contents. Both tools are not external code audits; hosted
> Windows/macOS and sustained fuzz checks still require execution.

# A05 wire codec: required negative/interop acceptance plan

**State:** design-only P01 specification; no A05 codec tests have run.
Future A05-P02 must record case-level executed evidence, failure categories,
resource ceilings and independent implementations. See
[the proposed schema](../architecture/PROTOCOL_SCHEMA.md) and
[`v1-design-vectors.json`](../../tests/fixtures/protocol/v1-design-vectors.json).

## Test strategy / independence

- Freeze reviewed v1.0 byte examples as **external fixtures**, with a manual
  independent byte/layout review before implementation. Do not generate the
  only expected bytes with the production encoder then call that an oracle.
- Build separate encoder and decoder tests. Cross-check with a separately
  maintained independent RFC 8949 decoder/encoder configured for GhostWire
  restrictions, or a test-only reference implementation checked separately.
  Do not claim CBOR validity alone establishes GhostWire profile validity.
- Use exact binary comparison for golden vectors on Windows MSVC Debug/Release,
  Linux GCC/Clang and macOS Clang. Architecture-native endianness must not affect
  bytes. Check native 32-/64-bit size arithmetic where feasible.
- Fuzz both outer prefix framing and strict CBOR with mutation, structure-aware
  generation, corpus minimization and recorded seeds. Test ASan/UBSan and
  constrained allocations; use a limit that terminates expensive inputs.
  Observing no crash in a bounded run is not a proof of memory safety.
- Verify that parser failure cannot alter DeviceSet trust/revision, identity
  recognition, authorization, key lifetime, quotas or local storage.
  Any added authenticated state must be separately reviewed later.

## Mandatory matrix (P02 exit criteria)

| ID | Check | Required outcome |
|---|---|---|
| W01 | Two fixed frame golden bytes for offer/select | Exact encode and decode matches fixture; version 1.0 only |
| W02 | Envelope key ordering changes without semantic changes | Reject noncanonical |
| W03 | Duplicate key in any nested map | Reject, never first/last-wins |
| W04 | Map key encoded nonpreferred (e.g. `0x18 0x00`) | Reject |
| W05 | Integer or collection length encoded nonpreferred | Reject |
| W06 | Indefinite map, array or byte string | Reject |
| W07 | Break, tag, floats (including NaN), negatives, booleans/null or UTF-8 text | Reject forbidden type |
| W08 | More than one top-level value or trailing bytes | Reject; no ignored suffix |
| W09 | 0, 65537 and 0xffffffff length prefixes | Reject before large allocation |
| W10 | 1..3 prefix bytes; 4-byte prefix but short payload | Incomplete, no publication |
| W11 | N=65536 complete bounded input | Accept structure if valid and budget fits; reject otherwise within limits |
| W12 | Nested depth 5, arrays 65, map keys 17, items 513 | Resource-limit rejection without recursive blowup |
| W13 | Large CBOR definite-length claim vs remaining bytes | Reject without allocation proportional to claim |
| W14 | Overflow in N+4, length×element, offset+length and quota accounting | Reject using checked arithmetic |
| W15 | Envelope missing required key or wrong type | Reject malformed |
| W16 | Unknown major/minor or reserved kind 20/30 | Fail closed with version/kind error |
| W17 | Nonempty/unsorted/duplicate capability IDs | Reject unsupported or noncanonical |
| W18 | Unsupported version pair, duplicate or unsorted version proposals | Reject; no fallback |
| W19 | Selection not offered by local caller | Reject at caller negotiation boundary |
| W20 | Offer/selection modified by simulated MITM | No privileged or authenticated state; later handshake must fail without transcript proof |
| W21 | Stale/unknown crypto suite in future container | Unsupported; never silently use A04 suite 1 as cipher |
| W22 | Routing token shorter/longer than 16 bytes, hop>16 | Reject |
| W23 | Routing hint changed by relay | No change in identity/trust/authorization scope |
| W24 | Two equal human aliases in separate profiles/memberships | No cross-principal merge or lookup via alias |
| W25 | DeviceSet carrier 181, 182, 1718, 1719 bytes | Enforce range; A04 semantic length/count still independent |
| W26 | Signed DeviceSet bytes carried unchanged | Bit-for-bit equality; A04 signature verification exactly unchanged |
| W27 | Invalid signature, authority mismatch, stale revision, expired DeviceSet | A04 rejects on explicit verification; codec alone never accepts trust |
| W28 | Inactive trust pin or deleted membership self-trust | No revival via transport decode or cached trust |
| W29 | Multiple frames with different profile/community contexts | No ambient/profile-global principal selection |
| W30 | Cancellation at prefix, map, bytes and precommit boundaries | Failure atomicity, reservations released; postcommit race documented |
| W31 | Operation deadline, busy slot and quota failure | Explicit fixed error; no retry spin or unbounded queue |
| W32 | Malformed input with binary secrets or arbitrary text | No untrusted bytes/addresses/exception text in error/logs |
| W33 | Repeated hostile frames and fuzz corpus | Bounded memory/work; no stuck worker, leak or unbounded cache |
| W34 | Unknown minor optional/noncritical extension | Reject under 1.0; later versions need reviewed registration |
| W35 | Replay of identical offered selections and public records | No authentication from frame; A04 high-water and expiry enforced on acceptance |
| W36 | Windows/Linux/macOS independent builds, signed char/endian/Unicode path differences | Same exact bytes, error classes and bounds |

## Future security-gate tests (not executable/claimable in P02)

- AUTH-PRE: deny-by-default action and resource matrix; verify no alias, route
  token or valid public signature grants a permission.
- Authenticated transport: bind offers, selections, suite identifiers,
  freshness, roles and channel context to a reviewed transcript; active MITM
  downgrade, splice, reflection, cross-protocol and replay tests.
- A08/A14: independently vetted DM/MLS provider, upstream interop vectors,
  authenticated replay/epoch/durable state with A11-PRE crash and rollback
  tests. Do not write or claim a new ratchet in the wire codec.
- Privacy: leak and metadata tests across direct/protected/privacy-first
  transport modes, endpoints, relays, discovery and optional federation.

## Acceptance records

For every executed test retain: precise source checkpoint, compiler and OS,
backend/library pins, random seed/corpus revision where applicable, command,
number of tests/assertions, exit codes, sanitizer flags, observed resource peak
and actual failures. Never mark any planned test as passed simply because this
plan exists. Independent external security review remains separate.
