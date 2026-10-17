# Proposed A04-P02 — volatile identity and device-trust implementation

Execution gate: owner approval of ADR-008 and the identity module design. The
owner authorized A04 continuation; the original A04 prompt requires a reviewed
design before implementing identity behavior. This proposed prompt is not that
approval. Do not require the owner to repeat HF01 acceptance or provide a Mac.

Use GHOSTWIRE-GW-A04-P01-IDENTITY-DESIGN-source.zip with its delivered SHA-256 as
the cumulative input. Verify archive/CRC/safe paths and every SOURCE_MANIFEST
hash. Runtime remains 0.3.1 / GW-A03-HF01-MACOS-RECOVERY because P01 changes only
documentation and source-package metadata. Preserve every implementation/test
and historical report. Reconcile any newer owner-published GitHub source read-only.

Read AGENTS, MASTER_CONTRACT, CHECKPOINT_RULES, architecture/dependency/module
maps, TODO/ROADMAP, A04-P01 report, ADR-008, IDENTITY_DESIGN, A04-TEST-PLAN,
A03-HF01-ACCEPTANCE and the original GW-A04-PROMPT. Carry the owner's actual
design amendments forward; never invent approval or restore remembered sources.

## Implement the approved bounded scope

- Add real C++20 typed public identities, explicit isolated volatile profiles,
  independently generated root/device and per-community membership/device keys.
- Implement reviewed canonical public device-set generation/verification,
  explicit trust pins, local recognition, revision/conflict/expiry rules,
  device replacement and revocation. No ambiguous alias-based principal lookup.
- Use the existing pure Ed25519/public-message facade and provider RNG/hash;
  no new primitives, KDF, raw key export, arbitrary signing oracle or protocol.
- Inject provider, clocks/time, cancellation and budgets. Use unique ownership,
  transactional publication, enum errors, bounded state and fixed diagnostics.
- Compose a real lifecycle adapter without a core/identity or
  identity/permissions dependency cycle. Preserve all four bootstrap contracts;
  identity creation must always be an explicit operation.
- Test ID-01..ID-26 with real provider and lifecycle paths. Broaden this plan when
  implementation review uncovers concrete risks. No success-shaped stubs.

Root-key continuity and recovery are explicitly deferred under the proposed
ADR. If owner review instead requires those in A04, settle that scoped design
before implementation. Device rotation must actually work; root rotation must
not be silently substituted with an alias change or inheritance from a new key.

A11-PRE blocks persistence of private profiles, keys, credentials, trust/session
state, recovery/backup and cached private mappings. AUTH-PRE blocks relevant
listeners/actions. No A05 networking schemas, transport, discovery, messaging,
GUI, web gateway, inference/training, remote enrollment or cloud requirement.
No production-ready identity/E2EE claim follows from primitive/unit tests.

## Validation and delivery

Run full supported Debug/Release and relevant sanitizer/security regressions;
retain both dependency paths, warnings, privacy and Unicode boundaries. Update
architecture, security invariants/threats, module/source inventories, TODO,
roadmap, changelog, completed work, actual validation and owner acceptance state.
Choose runtime/checkpoint version metadata only when functionality lands; do
not report this design pass as a new executable release.

Deliver cumulative GHOSTWIRE-GW-A04-IDENTITY-DEVICE-TRUST-source.zip, SHA-256,
changed files, measured commands/results, security limits, Windows acceptance
commands and proposed A05 handover. Stop at A04 for review; no A05 execution.
No GitHub push, PR, merge, tag or workflow dispatch: the owner publishes checkpoints.
