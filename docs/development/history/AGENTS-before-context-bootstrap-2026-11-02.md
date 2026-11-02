# Historical root document — superseded

This preserves the prior AGENTS.md for source history; it is not startup authority.
Original SHA-256: `09a5fd49d7896a447c785a64fb68a5583236c49c6c99fcce17f9b2fc2274aeb3`. Markdown links are rebased;
plain repository paths still resolve from the repository root. Read only for a task
that needs this history.

---

## Current authority — prompt navigation revision 2, 2026-11-01

Owner reported P02 revision 1 portable Windows x64 Debug and Release CLI builds
(31/31 each), cli-identity-session 1/1 each, and automatic menu launch. MSVC
19.51.36260.0 / VS2026 Developer PowerShell 18.10.3 / Python 3.13.13. Their archive
was renamed source-6.1Sol.zip; no host archive checksum was posted. These scoped
results are not a full suite, A05 acceptance or a revision 2 Windows pass.
Manual entry exposed a real UI trap: back/quit were consumed as invalid aliases.

P02 revision 2 fixes this in the CLI controller. Standalone quit/exit close from
every prompt; back/cancel/menu/.. discard pending input and return to the main
menu; help explains the current field without applying it. Numeric aliases remain
1-20 digits, including zero. Guided fields reserve navigation words; complete
commands still support literal word names. Services and terminal modes are unchanged.
The shell's later PSReadLine cursor-rendering exception is not a claimed app fix.

Input revision 1 ZIP SHA-256:
`16ffe8c4d5e326eac44a0925c273e25d2ed0763d64bb022a82541d9a294de469`.
All 315 source hashes verified before isolated extraction. Only session .cpp/.hpp
and the existing integration test change runtime/test behavior. Linux Debug and
Release CLI builds and 9/9 scoped CTests each pass. Revision 2 native platforms
are unrun; no full rerun is requested. See the updated existing P02 report.

Patch author tag: GPT-6.1-Sol-Max, supplied by the owner. Every future patch
delivery must display its known author label in the response, checkpoint report,
archive filename and packaging metadata, with matching SHA-256 sidecar filename
and content. Do not infer unavailable model metadata or present the label as a
cryptographic attestation. Current default archive includes r2-source-GPT-6.1-Sol-Max.
No independent agents, remote writes, sockets, persistence or A05/AUTH-PRE advance.
Runtime stays 0.4.1; official completion null; accepted effort floor about 10%.

## Historical authority — local membership lifecycle delivery, 2026-11-01

The owner asked to continue from GW-APP-P01-LOCAL-CONSOLE. Its cumulative ZIP
SHA-256 `faf190b398f30f7ab9e0315687b3fde7f25526982bcf10b1e7e0dd5bd9d2f081`
and all 314 source hashes were verified before extracting an isolated P02 copy.
GW-APP-P02-MEMBERSHIP-LIFECYCLE revision 1 wires existing A04 operations into
guided membership preparation, membership label/device management, profile rename,
confirmed profile/membership deletion and explicit console-managed trust cleanup.

All pending mutations bind existing local handles. Profile/membership deletion
requires exact delete confirmation; forgetting trust requires exact forget and
discloses loss of revision history. Cancellation or EOF before completion or
confirmation applies no pending mutation. A submitted membership preparation can
retain its explicit community pin on service failure, matching prior behavior.
Deleting an owned profile must not erase another profile's independent trust pin.
Those remaining public pins are visible through trusts and explicitly forgettable.

Only apps/cli/identity_session.cpp/.hpp and its existing integration test change
runtime/test behavior. Services, public contracts, terminal routing, core, codec,
crypto, dependencies, CMake and CI remain unchanged. No independent agents ran.
See docs/checkpoints/GW-APP-P02-MEMBERSHIP-LIFECYCLE.md for actual scoped Linux
Debug/Release evidence and Windows reproduction. Native Windows/macOS are unrun;
no immediate Windows acceptance repeat is requested. User continuation is not
native execution evidence. Runtime remains 0.4.1 / GW-A04-P03-IDENTITY-HARDENING.

All A05/native/external-review, AUTH-PRE and A11-PRE boundaries remain binding.
No sockets, remote authorization, private persistence, GUI or remote publication.
Official completion stays null; accepted effort floor remains 11/110 (about 10%).
The next proposed local task is bounded offline public-record exchange using
existing A04 APIs, explicitly unimplemented/unassigned. No trust may be inferred
from an alias, copied key or record alone. Historical sections below are retained.

## Historical authority — local console delivery, 2026-11-01

The supplied app-development handover authorizes bounded local usability work
on the existing volatile identity experience. GW-APP-P01-LOCAL-CONSOLE revision 1
continues from the exact A05-P04-UPSTREAM-INTEROP revision 6 ZIP, SHA-256
`24dde54c4862edd79083dd13393e8cf3df0989dff82c4e11d6d4d72457c2eb95`;
all 311 source hashes were verified before edits. Runtime identity stays 0.4.1 /
GW-A04-P03-IDENTITY-HARDENING; source checkpoint and runtime identity differ.

Argument-free CLI launch opens the real local console only when both input and
output are terminals. Explicit --identity-session still accepts redirected
commands; --bootstrap explicitly selects one-shot startup. A numbered menu,
guided create/select, selected-label status and selected-list marker use existing
services/handles. No profile is created on menu launch or incomplete/cancelled
creation. See docs/checkpoints/GW-APP-P01-LOCAL-CONSOLE.md for Linux Debug/Release
and terminal evidence, Windows reproduction and remaining limits.

This is a local app capability delivery, not A05 acceptance or an A06 advance.
Keep every A05/native/external-review, AUTH-PRE and A11-PRE gate below. No sockets,
remote authorization, private persistence, GUI, dependency change or remote write
is authorized. One coordinator implemented this pass; no independent agents.
Native Windows/macOS confirmation is pending, grouped with the next agreed gate;
do not request an immediate repeat of earlier Windows acceptance. The provisional
accepted floor remains 11/110 points (about 10%); official completion stays null.
Deliver one cumulative ZIP and checksum. Older authority sections are historical.

## Historical authority — A05-P04 fuzz invariants revision 6, 2026-10-31

Continue from the verified HF19 clean cumulative source and supplied continuation
handover. Actual pinned TinyCBOR v7.0 source is now bundled and the existing native
GCC and owner revision 2 MSVC x64 Debug/Release comparisons pass. The owner authorized
continuing the bounded A05 parser review. Revision 4 rejects a caller-created valueless
envelope before encoder dereference; runtime 0.4.1, wire bytes and trust rules remain.
See current-development-map.md and docs/checkpoints/GW-A05-P04-UPSTREAM-INTEROP.md.
Revision 5 enables TinyCBOR in existing platform CI jobs and adds the explicit
Windows configure/all switch. Fresh portable Linux Debug builds all targets and
passes 54/54 tests. No hosted jobs or new native Windows/macOS runs were executed.
Revision 6 strengthens the existing optional fuzz entrypoint with canonical
round-trip and admission-release checks, active under NDEBUG. Two bounded Clang
ASan/UBSan campaigns complete 37,513,997 executions; minimized corpus comparison
and existing upstream checks pass. Leak checking remains host-blocked. Production
source still matches revision 4; CI matches revision 5. No new harness or normal
build target is added. Keep remote publication owner-controlled.
Historical HF09 owner Windows acceptance does not cover this checkpoint.
Owner revision 1 Windows Debug failed C4245/C2220 in the test probe flags; revision 2
uses explicit unsigned operands. Owner revision 2 Windows Debug/Release builds and
both targeted tests pass on 2026-10-29, including the 4,023-case corpus. Revision 3
records that evidence only. Revision 4's encoder guard and one existing-suite regression
are Linux-validated; prior Windows evidence does not cover this new source change.
The coordinator review is not an independent external audit. No immediate Windows
rerun is requested; native confirmation joins the next agreed checkpoint.
Next: native macOS evidence, independent parser review and explicit A05
decision. No A06, AUTH-PRE integration, persistence or remote writes in this pass.
Worker names below and in historical integration records are not active agents.
Deliver one cumulative source archive, without installer overlays or extra harnesses.

## Historical authority — A05-P04-HF09 MSVC test-hook exception-contract fix, 2026-10-27

Owner verified x64 MSVC and HF08 CMake configuration. MSVC then reported
C4297 / C2220 for the HF08 `extern "C"` allocation hook under `/EHsc`.
HF09 corrects only its test-only declaration and definition to C++ linkage
with explicit `noexcept(false)`, preserving all fault assertions and production
protocol code. See `docs/checkpoints/GW-A05-P04-HF09.md`. Linux regression
is not Windows acceptance; VS2026 Debug/Release 10% gate remains owner pending.
No A06, network parser approval, dependency changes, key persistence or GitHub
write. Historical authority below is preserved.

## Current authority — A05-P04-HF08 allocation fixture Windows recovery, 2026-10-27

HF07's user-run Windows Debug result was 49/50, with an allocation-fault
fixture timeout. This bounded candidate removes the process-wide `operator
new` override only in the test executable and exercises the same two real
codec byte-buffer allocation boundaries via a private test compilation.
The original process-global override regression is preserved Linux-only.
Runtime 0.4.1 unchanged. Linux evidence does NOT establish Windows hang
resolution; the VS2026 Debug/Release 10% gate needs owner rerun. A05 upstream
TinyCBOR and independent parser review remain open. No A06/network exposure,
private-key persistence, auth, GUI or GitHub write. See
`docs/checkpoints/GW-A05-P04-HF08.md`; historical authority below remains.

## Current authority — A05-P04-HF03 limited parser interoperability tooling, 2026-10-19

Owner authorized continuing A05 security work without waiting for Windows tests;
subjective confidence for admitted changes must exceed 0.80. This pass corrects
an upstream-verdict verification gap and a dangling-symlink importer publication
edge case. Actual pinned TinyCBOR C compilation is still unavailable here;
**do not claim upstream interoperability or network-parser approval**. No
A04 signing, volatile identity, remote authorization, networking, GUI or private
persistence changes. Runtime 0.4.1 and no GitHub writes. See
`docs/checkpoints/GW-A05-P04-HF03.md`. Prior authority is historical.

## Current authority — A05-P04-HF02 bounded security review continuation, 2026-10-19

Owner requested continued scoped development, with per-change engineering
confidence above 0.80 and without blocking on local Windows acceptance. This
batch adds an opt-in (never automatic) immutable-commit TinyCBOR source fetch,
bounded test-only stdin probes and expanded frozen interop vectors. These are
**not** upstream-maintained parser execution, an independent audit, network
authorization or evidence of native Windows/macOS validation. Preserve all A04
identity/security invariants and A05 v1 wire bytes; do not implement A06 or
bypass A11-PRE/AUTH-PRE. See docs/checkpoints/GW-A05-P04-HF02.md. Runtime
0.4.1; no GitHub writes. Historical authority sections below are preserved.

## Current authority — A05-P04-HF01 source-worker batch, 2026-10-19

Owner requests continued bounded TODO-driven development without requiring new personal Windows acceptance for every candidate. Engineering confidence of each admitted task must exceed 0.80, but qualitative confidence is not security certification or platform test evidence. This batch hardens offline importer, in-memory A05 public codec and a non-executing worker-delivery merger; no autonomous coding-agent launch was available. Do not broaden into A06, network parsing, persistence, AUTH-PRE or E2EE. Pinned TinyCBOR actual parser execution and independent review remain required. See docs/checkpoints/GW-A05-P04-HF01.md and WORKER_QUEUE.json. Runtime 0.4.1; no GitHub writes. Older authority sections below remain historical.

## Current authority — A05-P03 codec hardening candidate, 2026-10-18

The owner requested further development and parallel work after P02 delivery but
has not supplied P02 Windows 41/41 evidence. P03 is therefore a bounded review
candidate, not a retroactive claim of P02 acceptance. See GW-A05-P03.md.
No independent AI worker spawning capability was available; isolated Linux
build/test/differential jobs were executed concurrently and merged. Preserve
A04 signing and all volatile trust invariants. Runtime 0.4.1, no network exposure,
A06, AUTH-PRE bypass, vault, GUI or AI features. No GitHub writes. Stop after
P03 for owner review. Older sections below document prior authority.

## Current authority — A05-P02 implementation candidate, 2026-10-18

The owner supplied A05-P01 Windows Debug/Release 40/40 evidence and instructed
continuation to A05-P02. The exact accepted P01 package SHA-256 is
`a1fcbe5839be69ce897ae2ba41d370b633c50ecf01a4db24aad48eb8186e060a`.
P02 implements only a strict in-memory public CBOR codec (1/2/10) without
sockets, new identity behavior, key persistence, authentication, E2EE, GUI or
AI features. Read ADR-010 and GW-A05-P02; the restricted decoder is a security
review candidate, not authorization for network exposure. Runtime is 0.4.1.
Do not implement P03/A06 without new acceptance; no GitHub changes allowed.
Older sections below are historical.

# GhostWire coding-agent contract

## Current authority — A05-P01 design review candidate, 2026-10-17

The owner accepted GW-A04-P03 and expressly authorized only
GW-A05-P01-PROTOCOL-SCHEMA-VERSIONING-DESIGN. Verified accepted P03 input
SHA-256 `aa52cb195c16a765b2118d62c7cb4b2a1e0a97a89831c9cd229390ea674e0ff4`;
its tracked git tree exactly equals published `main` commit
`01e70b317ff33527be991b0674dc6f93cccd4d1f`. Hosted Linux, Windows
and macOS jobs for that commit completed successfully (2026-10-17).
A05-P01 is documentation/schema/test-plan design only, not an activated codec.
Keep runtime 0.4.1 / A04-P03, preserve A04 public signing bytes and every
A11-PRE/AUTH-PRE boundary, and do not implement networking or private storage.
See ADR-009, PROTOCOL_SCHEMA.md, PROTOCOL_TEST_PLAN.md and the P01 report.
Stop after P01 for owner design review. Old authority sections are historical.

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
