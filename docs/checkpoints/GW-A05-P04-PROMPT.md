# Proposal only — GW-A05-P04 maintained CBOR implementation comparison

**Not authorized by this file.** Requires owner's independent P03 review/acceptance and reproducible Windows P02/P03 acceptance.

Use the exact latest accepted cumulative ZIP, validate CRC and manifest before work. Read ADR-009, ADR-010, P03 findings and the GhostWire Master Contract. Keep runtime 0.4.1 unless code changes are explicitly authorized; never add sockets, authentication, remote authorization, encrypted messaging, key persistence, discovery, GUI or AI.

- Obtain a **pinned and verified upstream** QCBOR or TinyCBOR release (review exact tree hash and all license obligations). Maintain a fully offline reproducible Windows/Linux/macOS source distribution and build; no downloads during normal builds.
- Independently compare all frozen positive/negative vectors and high-volume generated cases using that actual upstream implementation. Do not confuse valid generic CBOR with the stricter GhostWire schema: register strict canonical and closed-field validation explicitly, rejecting duplicates, unknown types/keys, nonpreferred widths and capabilities.
- Audit allocation ownership, frame size, deadline/stop responsiveness, thread boundary, rollback on allocation faults, error redaction, arbitrary malformed binary, ambiguous mappings, integer overflow, 32/64-bit and platform-specific behavior. Fix only found security defects, with independent regression vectors.
- Re-evaluate replacing the hand-authored schema cursor with an upstream wrapper and document both security footprint and ABI/test effects. Do not change A04 signing bytes, profile trust, community authority or displayed aliases.
- Run and record source integrity, GCC/Clang and MSVC Debug/Release, macOS/Clang, sanitizers, sustained libFuzzer, mutation/interop and exact Windows acceptance where available. Treat unavailable checks as gates, not passes.
- Deliver cumulative single-root source ZIP, embedded SHA-256 manifest, external checksum, changed-file inventory, code/security findings, actual commands/results and next checkpoint proposal. Stop pending owner review. No GitHub writes.
