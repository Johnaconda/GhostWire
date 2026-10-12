# ADR-003: Protect keys and authority before persistence and networking

Status: architecture decision for future checkpoints; implementation pending.

Keep the requested A00–A25 IDs for continuity, but use dependency gates rather
than blindly treating numbering as a complete execution order.

- A03 selects a reviewed primitive provider. No identity key is written before
  A02 secure-storage capability and an A11 minimum key-store slice are ready.
- A04 may validate ephemeral identity behavior while persistence is gated.
  It must not introduce temporary plaintext key files for convenience.
- A08 requires durable, atomic, encrypted ratchet/message state when persistence
  is enabled; the minimum A11 slice must be accepted first.
- Base permission checks precede A06 listeners and A13 communities. A21 adds
  moderation workflows and appeals, not first-time access control.
- A20 receives scoped AI identity and tool authority at introduction. Remote
  plaintext disclosure requires explicit user authorization and provider choice.

Signal-family protocols and MLS are candidates, not interchangeable primitives.
Use reviewed implementations, test vectors, persistence/rollback analysis and
interop evidence. The current libsignal implementation has a Rust/toolchain and
AGPL licensing impact (see official repository). A C++ adapter to libsodium is
not a substitute for an asynchronous, multi-device messaging protocol.
If no candidate satisfies maintenance, security, licensing and platform gates,
report A08 blocked; never fill the gap with a convenient original protocol.

Full A11/A12 vault policy work includes cooldown/rollback/DoS tests. Neither
server login failures nor malformed network packets may cause local key erasure.
Software retry counters do not control offline copies of ciphertext.
