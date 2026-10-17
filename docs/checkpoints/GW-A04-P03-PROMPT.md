# Proposed GW-A04-P03 — independent acceptance and scoped hardening

Execution requires an explicit owner instruction after reviewing A04-P02.
This file does not authorize P03, A05 or any other implementation checkpoint.

Use the owner's accepted cumulative
GHOSTWIRE-GW-A04-P02-IDENTITY-DEVICE-TRUST-source.zip and delivered checksum.
Verify CRC, safe paths and every SOURCE_MANIFEST hash before a fresh extraction.
Never reconstruct from conversation, replace the tree with an earlier archive,
or overwrite owner changes. Reconcile newer owner-published source read-only.

Read AGENTS, MASTER_CONTRACT, CHECKPOINT_RULES, ADR-008, IDENTITY_DESIGN,
the P02 report/changed inventory/Windows acceptance instructions, test plan,
security invariants, TODO and roadmap. P01 design was accepted independently;
that is separate from P02 runtime acceptance. Inspect the actual native Windows
10 / VS2026 Debug/Release results and interactive session results supplied by
the owner. If hosted CI is available for the exact source, inspect its native
Windows/Linux/macOS jobs. The owner has no personal Mac; do not demand one.

First record evidence and perform independent code/security review of the
volatile profile/membership/DeviceSet/trust implementation. Address concrete
findings within this approved scope only, with meaningful regression tests.
Focus on ownership and stale capabilities, distinct community credentials,
alias isolation, atomic failures, canonical parsing, explicit pins, revision
conflicts, time/revocation at point of use, bounded memory/work and cleanup.
Recheck the CLI's explicit public disclosures and no private persistence.

Do not treat public signature validity as live possession, membership admission
or action permission. No networking, challenge/login protocol, GUI, keyfiles,
vault, root continuity, recovery, enrollment synchronization, AI or media work
is authorized by this review prompt. A11-PRE and AUTH-PRE remain mandatory.

Run change-scoped tests for narrow repairs; full Debug/Release and available
sanitizers for shared identity/crypto/lifecycle changes. Record unavailable
native platforms, local full LSan, clang-tidy and the owner's vcpkg issue
honestly. Never copy historical pass counts into new evidence.

Deliver any approved P03 recovery as a full cumulative source ZIP with SHA-256,
exact changed-file inventory, actual results and refreshed host instructions.
If review finds no code change needed, report that instead of inventing a patch.
Only then propose the separately gated GW-A05 protocol-schema design. Stop for
owner acceptance. No GitHub push, PR, merge, tag or workflow dispatch: publication
belongs to the owner.
