# Proposed GW-A05-P01 — protocol schema and versioning design

This is a proposal only. Begin after the owner reviews/accepts P03 and explicitly
authorizes A05. Use the accepted cumulative P03 ZIP and its delivered SHA-256;
verify CRC, safe paths and every SOURCE_MANIFEST entry before extraction. Preserve
all accepted source and native evidence; never reconstruct from conversation.

Read AGENTS, MASTER_CONTRACT, CHECKPOINT_RULES, architecture/dependency maps,
ADR-006/008, identity module contracts, P02 host evidence, P03 report, TODO,
roadmap and security invariants. Reconcile newer owner-published source read-only.

Produce a bounded protocol-schema/versioning design before implementation:

- Separate transport envelopes, routing metadata, signed public identity values,
  future authentication proofs and eventual encrypted payloads. A valid DeviceSet
  is not login, live key possession, membership admission or action permission.
- Define exact byte representation, version/suite/type handling, length ceilings,
  integer rules, unsupported/unknown behavior and compatibility policy. Evaluate
  existing serialization choices against small dependency and bounded parsing
  requirements; document the decision before adding a dependency.
- Define caller ownership, cancellation/resource limits, structural validation
  order, fixed diagnostics and negative vectors. Avoid ambiguous encodings,
  input-driven allocations and recursive/compressed unbounded payloads.
- Keep the core/crypto/identity/protocol dependency graph acyclic. Preserve the
  accepted isolated-profile and independent-community identity model.
- Specify testable future codec acceptance criteria (fixed independent vectors,
  truncation/mutation/unknown-version/limit failures) and implementation scope.

No socket/listener, transport, discovery, login/challenge protocol, E2EE design,
private persistence, GUI, AI or media implementation in this design pass.
A11-PRE and AUTH-PRE remain gates. Do not claim protocol security from structural
schema validity. No custom cryptographic primitive or messaging protocol.

Deliver cumulative design source ZIP, checksum, exact changed files, reviewable
schema/ADR and A05 implementation prompt. Do not bump runtime for a docs-only
pass. Stop for owner design acceptance; no automatic implementation or A06.
The owner controls every push, PR, tag, merge and hosted workflow dispatch.
