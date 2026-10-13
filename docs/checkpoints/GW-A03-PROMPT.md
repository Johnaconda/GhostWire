# Proposed continuation prompt: GW-A03

**Proposal only. Stop after A02. Do not execute this prompt until the owner has
reviewed/accepted A02, supplied Windows host acceptance and explicitly authorized
GW-A03.** Identity-system discussion is not authorization for identity or network work.

CHECKPOINT: GW-A03-CRYPTOGRAPHIC-PROVIDER

AUTHORITATIVE INPUT: the accepted cumulative
`GHOSTWIRE-GW-A02-PLATFORM-ABSTRACTIONS-source.zip`, or a newer explicitly accepted
A02 recovery archive. Obtain the actual expected SHA-256 from the owner's
delivery/sidecar and verify before extraction. If a repository is also supplied,
pin its exact commit and reconcile actual differences; preserve owner changes.
Never reconstruct source from memory or silently reset working functionality.

Read AGENTS.md, MASTER_CONTRACT.md, CHECKPOINT_RULES.md, ARCHITECTURE.md, ROADMAP.md,
TODO.md, DEPENDENCY_GRAPH.md, MODULE_MAP.md, CORE_LIFECYCLE.md,
PLATFORM_ABSTRACTIONS.md, SECURITY_INVARIANTS.md, THREAT_MODEL.md, ADR-003/004/005,
the A00/A01/HF01/HF02 historical records and GW-A02.md/native acceptance evidence.
Establish actual native outcomes; authorization does not turn pending checks into passes.

Implement only the bounded cryptographic-provider foundation already sequenced
at A03. Before adding a provider, review current primary documentation, maintained
release/advisory status, exact dependency pin, license and platform build support.
Record the selected backend and bounded primitive set in a new ADR. Use a
maintained reviewed dependency through the existing dependency policy; do not
invent primitives, a messaging protocol or an unreviewed fallback. If provider
choice changes the accepted architecture/language/security contracts, resolve
that concrete decision with the owner before implementation.

Required work after authorization:

1. Define an explicitly owned, headless provider facade using typed errors and
   fixed redacted diagnostics. Keep backend/native types private and dependencies
   within the accepted DAG. Bound all buffers/work and document thread/lifetime rules.
2. Implement checked OS/provider cryptographic randomness with testable failure;
   never fall back to timestamps, std::rand or predictable seeds. Use upstream
   recommended APIs for the approved primitive set and key/nonce/input bounds.
3. Use move-only secret ownership, explicit key purpose and controlled access;
   prevent accidental logging/copy/serialization. Document clearing, allocation,
   memory-lock/dump limitations honestly and surface unavailable capabilities.
   A02 API presence does not authorize secret persistence or imply protected memory.
4. Add actual upstream known-answer/negative tests, malformed sizes, authentication
   failure with no released plaintext, entropy/allocation failure paths where the
   provider permits injection, cancellation/budget behavior and secret lifetime
   tests. Keep crypto integration tests meaningful and the provider test seam explicit.
5. Preserve all prior CLI/lifecycle/admission/Unicode/toolchain/process/stop tests.
   Run available Debug/Release, CTest, provider tests, sanitizers and static checks.
   Record executed host/versions and blocked native validation separately.
6. Update architecture/module contracts, invariant map/threat model, dependency
   notices/license/pins, TODO/roadmap/changelog/completed records, validation,
   changed-file inventory/source manifest and a proposed A04 handover with gates.
7. Deliver the full cumulative source ZIP, exact SHA-256, concrete commands/results,
   security limits and remaining acceptance items. Stop after A03 for independent
   review and native acceptance. Do not publish/push remotely without authorization.

Out of scope: persistent profiles, account/device identities, root-key files,
ratchet/session state, vault storage, listeners, routing/discovery, messaging,
GUI, AI inference/training and model execution. A11-PRE must precede persistent
A04/A08 state; AUTH-PRE must precede A06 listeners/A13 administration. Any needed
minimum gate is a separately approved slice, not implicit A03 authorization.
No production encryption claim follows merely from passing primitive tests.

Carry forward A02 native acceptance, owner Windows vcpkg/doctest, clang-tidy,
full A02 LeakSanitizer and any blocked concurrency checks until concrete evidence
closes each one. Earlier HF02 hosted runs apply only to their pinned commit.
