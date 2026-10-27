# ADR-010 — A05-P02 finite-schema codec review and controlled exception

**Checkpoint:** GW-A05-P02-STRICT-VERSIONED-WIRE-CODEC  
**Status:** Implementation candidate, not owner accepted / not security audited  
**Supersedes:** Only the *proposed implementation dependency* part of ADR-009; RFC 8949 deterministic subset and the A04 signing contract remain intact.

## Decision and review

ADR-009 preferred an independently reviewed, maintained CBOR implementation and
required an amendment before making a profile-specific decoder. The alternatives
reviewed for A05-P02 were:

* **QCBOR 1.6.1** (BSD-3-Clause; upstream tag `v1.6.1`; OpenEmbedded source
  revision `930708bb86481e88879eb1d87fd4d664f1d69503`): maintained C
  implementation, broad tags/types/maps support, configurable encoding and
  decoding, but it still requires explicit application restrictions for v1
  and a fully vendored, pinned source review/portability audit. Version 2 was
  available only in prerelease at the time of review. Relevant sources:
  https://github.com/laurencelundblade/QCBOR and
  https://layers.openembedded.org/layerindex/recipe/464941/.
* **Intel TinyCBOR v7.0** (MIT): maintained full-featured C parser with canonical
  validation capabilities, but also broader parser grammar and recursion paths
  than this finite wire registry requires. Some reports of validation issues
  remained open. Relevant sources: https://github.com/intel/tinycbor and
  https://github.com/intel/tinycbor/issues.

No full vendored source audit, CI integration or upstream negative vector
comparison was performed here. The build environment cannot download upstream
source distributions, and silently introducing unverified dependencies would
break the project's self-contained/offline source contract. This is **not** a
claim that the libraries are generally unsuitable for other applications.

**Controlled exception:** implement only the fixed A05 v1.0 grammar (three
registered kinds, fixed map keys, definite collections, no unknown extensions)
as a non-recursive C++20 cursor with known maximum depth and bounded value
storage. It is NOT a generic CBOR parser, an independently audited codec, or
permission to implement a proprietary security protocol. This exception must
be reviewed before any network-exposed use. A future approval may replace it
with a pinned maintained CBOR library without changing the public v1 bytes.

## Enforced constraints and residual risks

* Four-byte network-order length with N in 1..65536; exactly one frame.
* Deterministic shortest CBOR integer/length representation; maps contain only
  sorted one-byte integer keys 0..23; no recursion or field skipping.
* Only `version_offer` (1), `version_select` (2) and `public_device_set` (10).
  Future auth/E2EE kinds 20/30 and all unregistered versions/capabilities fail.
* Input borrowed; result owns immutable-to-caller public values. Only A04 may
  validate device signatures, scope, authority, revision, freshness and trust.
* One explicit caller-owned atomic admission slot; deadline/cancellation,
  per-operation byte/work caps and staged publication. No memory proportional
  to unchecked CBOR claims. Decoder-owned device copy <=1718 bytes, framing
  <=65540 bytes; under the planned 131072-byte decoded allocation ceiling.
* Fixed error enumerations. No key/path/payload bytes exposed through errors.
* Tests use pre-existing P01 binary fixtures plus a test-only independently
  written Python CBOR AST implementation. Independence does not replace
  security review, interop against an upstream third-party implementation or
  production fuzzing on all three platforms.

**Gate:** Public-network input to this codec remains blocked until the owner
reviews this exception and the protocol security review requirements. No
remote authentication, transport negotiation or authorization results from P02.
