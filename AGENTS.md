# GhostWire coding-agent contract

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
