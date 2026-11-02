# Historical root document — superseded

This preserves the prior current-development-map.md for source history; it is not startup authority.
Original SHA-256: `8e973928bd27642c1bf9fafdd4f4f3b8cc68c0a1c120d9839ec90eb7cb70f8e4`. Markdown links are rebased;
plain repository paths still resolve from the repository root. Read only for a task
that needs this history.

---

# GhostWire — Current Development Map

> **2026-11-01 · GW-APP-P02-MEMBERSHIP-LIFECYCLE revision 2 · runtime 0.4.1 · author tag GPT-6.1-Sol-Max**
> Current coordination summary. Historical checkpoint/worker records remain evidence for their own source only.

## Source and acceptance

- Input: P02 revision 1 ZIP, SHA-256 `16ffe8c4d5e326eac44a0925c273e25d2ed0763d64bb022a82541d9a294de469`; all 315 source hashes verified. The cumulative P01/A05/HF10/HF11/HF19 tree is retained. Attribution tag is owner-provided.
- The last verified GitHub baseline was `89af5eebd1ac796d80b570651b21feef5d6122ae`, older than the input ZIP. This is historical evidence, not a fresh assertion about main. No GhostWire remote repository query, write or publication ran in this pass.
- Current revision 2: Linux GCC Debug/Release CLI builds 31/31 and scoped CLI CTests 9/9 each pass; existing workflow covers navigation across 19 pending contexts and literal/zero data preservation. Revision 2 native execution is pending. [Current evidence](../../../docs/checkpoints/GW-APP-P02-MEMBERSHIP-LIFECYCLE.md).
- Owner revision 1 Windows: VS2026 Dev PS 18.10.3, x64 MSVC 19.51.36260.0 and Python 3.13.13; portable Debug/Release CLI builds 31/31 and identity workflow 1/1 each pass, with live menu launch. Manual input exposed the prompt navigation trap now corrected in revision 2. No host archive checksum, full-suite, successful manual creation/rename or new A05 native evidence is inferred.
- Prior P02 revision 1 Linux: scoped CLI/identity builds and 11/11 CTests each pass, including identity 41 cases / 1799 assertions and faults 11 / 127, plus two terminal workflows each. Backend/source evidence remains source-specific.
- Prior P01 Linux evidence: four roles/core target built, 34 scoped CTests and 14 terminal/redirection scenarios per configuration passed. P02 preserves core/terminal startup and all other roles byte-for-byte; prior native limitations remain. [P01 evidence](../../../docs/checkpoints/GW-APP-P01-LOCAL-CONSOLE.md).
- Historical owner Windows 10% gate: HF09 VS2026 x64 Debug/Release **66/66 build, 50/50 CTest each**, accepted 2026-10-27. This does not accept subsequent source changes.
- Owner Windows revision 2: VS2026/MSVC 19.51.36260.0 x64 Debug/Release builds and both targeted tests **2/2 PASS each**, supplied 2026-10-29. The unsigned-mask correction resolves revision 1's C4245/C2220 failure. Counts match GCC; upstream C5287 remains nonfatal.
- Revision 4 adds a Linux-validated encoder guard and one regression case. Earlier Windows results do not cover this change; native confirmation is deferred to the next agreed checkpoint, not requested immediately.
- Revision 5 enables TinyCBOR in all existing platform CI jobs, including Linux sanitizers. Fresh portable Linux Debug builds all targets and passes 54/54 tests. No hosted run, macOS execution or new Windows execution is claimed.
- Revision 6 strengthens the existing fuzz entrypoint. Two Clang ASan/UBSan campaigns complete 37,513,997 executions without a finding; 1,937 minimized inputs plus 18 frozen controls pass reference/upstream replay, including one separately checked probe-bound rejection. Leak detection remains disabled after a reproduced host inspection failure.
- Actual pinned TinyCBOR v7.0 C implementation now builds and passes the existing **4,023-case** comparison and stdin-bounds check on GCC Debug/Release. ASan/UBSan passes with local leak detection disabled after host teardown failure; unrestricted leaks remain unverified.
- Official project completion remains `null`. Provisional accepted floor stays 11/110 effort points (10%); **20% is not reached**. [Current evidence and retained reproduction commands](../../../docs/checkpoints/GW-A05-P04-UPSTREAM-INTEROP.md).

## Module state and dependency order

| Stage | Actual state | Next dependency |
|---|---|---|
| A00–A04 | Accepted foundation/volatile identity; P02 r1 Windows scoped build/workflow passes; r2 navigation Linux-validated, native pending | Next local record-exchange task; preserve handles, label rules and explicit trust |
| A05 | Offline codec; upstream comparison and bounded fuzz properties evidenced; caller-state encoder failure fixed; native CI prepared | Leak-enabled and native platform evidence, independent review, owner retain/replace decision |
| AUTH-PRE | HF19 64-rule policy table; separate compiled candidate only | After accepted A05: composition-owned identity, fresh possession and commit-time revocation/policy checks |
| A11-PRE | Protected storage prerequisite, unimplemented | Approved key protection, rollback/revocation and crash contracts before persistence |
| A06–A07 | Deferred transport/discovery | Accepted A05 and AUTH-PRE before listeners |
| A08–A16, A21 | Planned/blocked messaging, communities and moderation | Authenticated transport, reviewed E2EE/MLS, protected state and scoped authorization |
| A17–A20, A22 | Planned desktop, media, agents and plugins | Working service integration, UI feasibility and permissions |
| A23–A25 | Planned recovery, audit and release | Implemented runtime capabilities and measured acceptance |

## Current local app task

`GW-APP-PROMPT-NAVIGATION` corrects the native manual finding: navigation is dispatched
before guided data parsing. Back/cancel/menu/.. clears pending work; help keeps the
field; quit/exit closes cleanly. Numeric aliases remain unchanged and visible names
come next. Complete commands retain literal navigation-word names; zero is not
mistaken for a global quit. Confidence 0.94; existing 9 scoped CLI CTests pass in
each Linux configuration. Packaging/report/response include author tag and matched
checksum naming. No terminal mode or PowerShell module/policy is changed.

## Previous membership lifecycle task

`GW-APP-MEMBERSHIP-LIFECYCLE` is implemented with existing A04 APIs:

- Main menu 7 prepares independent membership credentials; 8 manages labels, devices, explicit public record and confirmed removal; 9 renames the selected profile; 10 requests confirmed profile deletion. No community admission is implied.
- Pending operations capture owner/member/trust handles. Cancel/EOF before completion or confirmation applies no pending mutation. Existing preparation failures may retain their explicitly submitted community pin and report that fact.
- Profile deletion clears its owned selection/trust mappings, retaining another profile's independent public pins. `trusts` lists console-managed profile pins and `forget-trust` requires explicit confirmation before discarding pin/revision history.
- Only CLI session code/header and its existing integration test change behavior. Services, public contracts, core/platform, codec, crypto, dependency pins, build targets and CI remain unchanged. No persistent file is added. Confidence is 0.92, separate from accepted progress.

## Previous local console task

`GW-APP-LOCAL-CONSOLE` uses existing lifecycle-owned A04 services:

- Argument-free CLI launch opens the menu only with terminal input/output. Redirection keeps one-shot startup; explicit `--identity-session` accepts command pipes/files and `--bootstrap` selects one-shot startup.
- Numbered create/list/select/status/membership-list/help/quit; guided create/select with cancellation; selected-list marker and explicit selected-label status. Complete existing commands remain available.
- Pending aliases are bounded to 20 bytes, input lines to the existing 4096 bytes, and canonical validation stays in IdentityService. Cancel/EOF before both fields arrive creates no profile. No alias-based selection, automatic trust, implicit public-key dump or persisted file is added.
- No service/dependency, test target, parser harness, GUI or network implementation is added. Confidence is separately 0.92; no accepted module points or 20% claim follows from this task.

## Preserved A05 pass — historical implementation detail

- Preserve HF10's macOS importer fixture, HF11's x64/Python guards and HF19's local-policy candidate; omitted HF12–HF18 overlay experiments stay omitted.
- Bundle 12 unmodified, pin-verified upstream files plus their original MIT license. Normal builds remain offline and do not build or link TinyCBOR.
- Fix exact pinned C-source packaging, explicit C++20 for the opt-in probe, sanitizer instrumentation of upstream C code and vendor-header warning separation. Keep project warnings as errors.
- Revision 2 fixes the test probe's signed enum mask conversion; all validation bits and corpus outcomes are preserved. Through revision 3, production `src/`, `include/` and `apps/` matched HF19.
- Revision 3 records owner Windows results and synchronizes the existing checkpoint/registries; code, dependencies, build settings and tests match revision 2. No repeat Windows run is requested for this evidence update.
- Revision 4 reviews framing/cursor arithmetic, canonical forms, schema rejection, resource/lease cleanup and trust separation. The confirmed finding is `encode_frame` assuming the public variant always holds a body. An explicit guard fixes the reproduced null dereference; one case extends `protocol-unit`, with no new harness, dependency or CMake target delivered.
- Revision 5 closes a CI coverage gap: prior workflows left the opt-in parser comparison disabled. Existing Linux/macOS configure steps now enable it; the Windows helper exposes `-TinyCborInterop` for configure/all and CI selects it. Verbose CTest retains case counts in logs. No jobs, runners, permissions, pins, production code or test cases change.
- Revision 6 adds exact re-encoding, decoded-value stability and admission-release checks to the existing fuzz entrypoint. Checks remain active with NDEBUG; temporary byte-corruption and lease-leak controls both fail as expected. No production fix, new harness, CMake target or dependency change was needed. Optional Linux reproduction is in BUILD.md.
- [ADR-010](../../../docs/decisions/ADR-010-RESTRICTED-WIRE-CODEC-P02.md) recommends retaining the restricted reader for current offline use. This is not security approval or permission for network input.

## Ownership and next capability

One coordinator implemented/reviewed revision 2 in an isolated copy of verified P02 revision 1; independent agents: **none**. No worker remains active at delivery. Existing records retain source-specific native evidence and confidence. Revision 2 native smoke joins the next agreed gate; no immediate full repeat is requested. The later PSReadLine cursor exception remains a separate shell issue. User remains acceptance and publication authority.

Next proposed local task: `GW-APP-PUBLIC-RECORD-EXCHANGE`, dependency-ready but unimplemented/unassigned. Existing A04 verify/pin/accept APIs can support bounded offline public record exchange between sessions with explicit full authority/owner selection. This is not automatic trust, A05 wire-parser approval, remote possession or authorization. Service gates remain `GW-A05-UPSTREAM-NATIVE`, `GW-A05-PARSER-DECISION`, then deferred `GW-AUTH-PRE-COMPOSITION` and A06.

Launch `ghostwire-cli` in a terminal for the local menu; use `--identity-session` explicitly if an IDE exposes pipes. All keys are volatile. Desktop/server/relay remain bootstrap executables. The next service capability after accepted A05 is one real local action path with scoped device/possession/policy checks at commit. No A06 listener, private-key persistence, E2EE claim or remote write follows from this delivery.
