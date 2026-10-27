# Proposed GW-A05-P02 — strict versioned wire codec implementation

**Authorization:** NOT granted by this file. Begin only after the owner accepts
GW-A05-P01 design and explicitly authorizes P02. The exact P01 cumulative ZIP,
manifest and SHA-256 must be verified against any newer published `main` before
editing. Preserve all historical implementation and test artifacts.

## Required read-first contracts

`AGENTS.md`, `docs/development/MASTER_CONTRACT.md`, `CHECKPOINT_RULES.md`,
`docs/decisions/ADR-006-CRYPTO-PROVIDER.md`, `ADR-008-IDENTITY-AND-DEVICE-TRUST.md`,
`ADR-009-CANONICAL-PROTOCOL-SERIALIZATION.md`,
`docs/architecture/PROTOCOL_SCHEMA.md`, `schemas/ghostwire-wire-v1.cddl`,
`docs/security/PROTOCOL_TEST_PLAN.md`, existing DeviceSet encoding/tests,
`docs/security/SECURITY_INVARIANTS.md`, `THREAT_MODEL.md`, roadmap and TODO.

## Bounded implementation goal

Create a real C++20 `ghostwire_protocol` library with strict canonical
encoding/decoding of a bounded in-memory v1.0 CBOR envelope. Review and pin
an appropriately maintained library (license/platform/security), including
its parser allocation and noncanonical-input behavior; if unsuitable, record
an ADR amendment **before** a restricted manual codec is considered. No
hardcoded security-shaped success stubs, generic extensible runtime registry,
or runtime crypto/identity implementation changes without a separate recovery
scope. Dependency DAG must remain acyclic and public headers must not expose
platform/backend/internal parser types.

Implement only version offer/select and public DeviceSet carrier kinds 1/2/10;
respect exact bytes and validators in the P01 schema. Strict 4-byte length
prefix, CBOR canonical map ordering/preferred encodings, complete length
consumption and explicit fixed errors. Reject all unsupported versions,
capabilities, fields, kinds and future auth/E2EE containers. Decoder returns
only immutable public values and never mutates profiles, trust or keys.
Ownership, cancellation, memory budgets, checked arithmetic and staged
publication are mandatory; no unbounded memory, CPU or recursion from inputs.

Implement independent golden vectors, negative/fault/resource checks,
fuzz harness and cross-platform determinism per PROTOCOL_TEST_PLAN; report actual
results. Add an isolated composition demonstration only if entirely in-process
and it cannot be misrepresented as remote authentication/secure messaging.
CLI flags, sockets, listeners, relays, discovery, remote handshake, encryption,
persistent credentials, vault, GUI, AI, files, voice/video and A06 are out of
scope. Maintain A11-PRE and AUTH-PRE gates.

## Delivery and stop condition

Run scoped and full relevant regression, architecture/boundary and integrity
checks on available hosts. Produce cumulative single-root ZIP with SHA-256,
source manifest, diff inventory, actual platform/test results and exact Windows
10 / VS2026 PowerShell acceptance. Distinguish local test execution, hosted CI,
and owner acceptance. Preserve runtime metadata except legitimate implementation
checkpoint changes. No GitHub mutation. **Stop after P02** for independent
owner acceptance; do not begin A05-P03 or A06.
