# GhostWire coding-agent contract

## Current authority — A04-P03, 2026-10-16

The owner supplied P02 Windows Debug/Release 40/40 each plus identity-console
launch, and instructed continuation. See docs/checkpoints/GW-A04-P02-HOST-ACCEPTANCE.md.
P03 uses the exact P02 archive SHA e755c9b994541dbf66e061d799b31621b2f91034cda7741d1473699426783b59.
Current runtime is 0.4.1 / GW-A04-P03-IDENTITY-HARDENING. This bounded pass
reviews identity and repairs concrete deletion/revocation findings only.
P03 execution/acceptance is separate from supplied P02 evidence. Stop after P03;
A05 is a proposal only. No remote writes, persistent keys, networking or GUI.
Older sections below retain checkpoint context and are superseded on current status.


## Current authority — 2026-10-16

The owner accepted A03-HF01 Windows portable Debug/Release 37/37 each and
authorized continued work without a personal macOS machine. Published main
1f92c5539f7eb3f6f8e3833f27df9384f260251a matches all 166 files of the HF01
ZIP, SHA-256 5d203d63341deeb14f2128e86bb8cf5773c19c7084ef6a83caefdc273c1747a8.
All nine hosted jobs are now inspected: macOS Intel/ARM64 Debug/Release 37/37,
Windows Debug/Release 37/37, Linux Debug/Release and default sanitizers 38/38.
See docs/checkpoints/GW-A03-HF01-ACCEPTANCE.md for exact environments/limits.

The owner subsequently accepted GW-A04-P01-IDENTITY-DESIGN after independent
architecture review and source-integrity verification, and explicitly authorized
GW-A04-P02. Its authoritative cumulative P01 ZIP has SHA-256
38fbfceccd352bc9b95990dc2b605c3e7e4212b21e3057b72bc34e7b484d6fc3.
ADR-008 and IDENTITY_DESIGN are accepted for this bounded volatile implementation;
do not repeat the P01 approval gate. The owner also authorized parallel agent
work and an interactive application interface within this pass.

Current package/runtime: GW-A04-P02-IDENTITY-DEVICE-TRUST / 0.4.0. The new identity
library and separate composition adapter provide volatile profiles, independent
community credentials, signed device sets and explicit local trust. The CLI's
opt-in --identity-session exposes these real services; ordinary four-role
bootstrap behavior remains intact. This is not a network login or community
admission implementation. All keys/trust state are lost at session shutdown.
See docs/checkpoints/GW-A04-P02.md for actual validation and
docs/checkpoints/GW-A04-P02-WINDOWS-ACCEPTANCE.md for native acceptance.
P02 remains subject to independent review and owner Windows testing. Stop after
P02; do not automatically advance to P03 or A05. No persistent private storage,
GUI, networking, messaging or future AI functionality is authorized by this pass.
Do not ask the owner to repeat HF01 acceptance or acquire a Mac. Native hosted
CI is valid evidence for its exact source; older-OS/physical-host coverage stays
separate. Owner-local vcpkg, local LSan, clang-tidy and crypto review remain open.

The earlier acceptance snapshot in item 3 below is retained for traceability
and is superseded by this section. The owner controls every GitHub push. No
remote write, PR, merge, tag or workflow dispatch is authorized by packaging.

## Read before editing

1. Read docs/development/MASTER_CONTRACT.md, ARCHITECTURE.md, ROADMAP.md,
   TODO.md, docs/checkpoints/GW-A00.md and the latest later checkpoint if present.
2. Inspect actual headers/sources, build settings, tests and decisions for the
   affected modules. Confirm the supplied archive/checkpoint/hash.
3. Identify the user's authorized checkpoint and bounded change scope. The owner
   supplied A03 Windows10 / VS2026 / MSVC19.51.36260 portable Debug/Release
   acceptance: 37/37 each, original TEMP/TMP, all four binary identities and CLI.
   Their main commit 95c9128ea5438663b14038c66f4986dfb4b90279 matches all 158 source
   files in delivered A03 ZIP, SHA-256
   b3d3d39e1a369185a1309e41213568f88ef3b212c80898bb577334b0616f3c64.
   The continuation pass found macOS CI failures and repairs only those A03/A02
   regressions as GW-A03-HF01-MACOS-RECOVERY. See GW-A03-HOST-ACCEPTANCE.md and
   GW-A03-HF01.md. Preserve all earlier source/history and user-owned pushes.
   A03 hosted Windows/Linux passed; all four macOS jobs failed 34/36. New HF01
   native Windows/macOS and hosted results require actual evidence. Hosted
   Windows vcpkg success does not close the owner's separate local issue.
   Local LSan, clang-tidy and independent crypto audit remain separate limits.
   Stop after this recovery for review. A04's first pass is identity design under
   GW-A04-PROMPT.md plus GW-A03-HF01-HANDOVER.md; implementation follows design
   approval. Persistence, profiles on disk, vault and networking remain outside
   this pass. A continuation proposal is never authorization by itself.
4. Record the implemented, experimental, planned and blocked boundaries before
   making changes. Existing source is authoritative over memory or old packages.

## Implementation rules

- Never reset to an older checkpoint or replace functioning code with stubs.
- Keep C++20 unless an accepted ADR changes the requirement.
- Preserve the DAG in DEPENDENCY_GRAPH.md. OS types stay in adapter .cpp files.
- Main thread owns UI/platform event state. Services own their executors; worker
  jobs return immutable results and never directly mutate UI or the service graph.
- Use RAII, explicit dependency injection and typed errors. No global registry,
  hidden socket creation, unbounded queue or success-shaped unimplemented method.
- Every operation needs ownership, lifetime, thread model, validation/error,
  security, resource and platform contracts. Implement within the current scope;
  document future contracts rather than adding empty security headers.
- Never invent cryptographic primitives or an original DM/group protocol.
  Maintained reviewed dependencies, interop, licensing and external review gates
  must precede production claims. C++ host preference does not justify weaker crypto.
- No plaintext private persistence pending the minimum vault gate. Never write
  temporary unencrypted keys to make a demo work.
- Do not downgrade privacy policies or grant admins/AI/plugins implicit private
  access. Remote unauthenticated data must never trigger local key destruction.
- Do not log raw input, exception.what(), endpoints, key IDs, content or paths.
  Logger public fields are only fixed reviewed metadata; sensitive is the default.
- Include effects on backups, dumps, temporary files, search indexes and caches
  when changing any private storage behavior.
- No remote repo creation/push/publication without owner authorization.

## Validation and delivery

Use change-scoped checks for isolated changes. Run full relevant regression
checks for crypto, wire schemas, platform foundations or broad shared-core
changes. Test actual failure behavior, not copies of implementation logic.
Use upstream test vectors/fuzzing/interop where appropriate. Never silently skip
unavailable dependencies or report intended tests as run.

For each pass update TODO statuses and evidence, ADRs when decisions change,
CHANGELOG, completed records only for implemented/validated work, and the
checkpoint report. Keep user/design acceptance distinct from execution results.
Include changed files, architecture changes, commands/results, untested
platforms, security impact, known limits, TODO changes and the next checkpoint
proposal. Package the cumulative current tree with
`python3 tools/packaging/package_source.py`; update its checkpoint/name when
advancing. Verify contents and SHA-256; exclude builds/caches/credentials.

Keep master contract continuity. If a requested change contradicts an invariant,
state the concrete conflict and resolve the design before weakening guarantees.
A missing provider or unavailable host is a reported gate, not permission to
fake security or platform validation. User acceptance is required before moving
to the next checkpoint. No automated remote publication is part of packaging.
