# GW-DEV-CONTEXT-BOOTSTRAP — 2026-11-02

Patch author tag: **GPT-6.1-Sol-Max** (owner-provided label, not inferred model metadata).
Source revision 1; runtime remains **0.4.1 / GW-A04-P03-IDENTITY-HARDENING**.

## Result and startup

The owner requested a first-read source guide so workers start with their task,
not every source/history file. Read [START-HERE.md](../../START-HERE.md) and
[AGENTS.md](../../AGENTS.md) once, then:

```bash
python3 tools/development/task_context.py --list
python3 tools/development/task_context.py --task GW-APP-PUBLIC-RECORD-EXCHANGE
```

For a new issue, `--area cli` (or one of 10 listed areas) returns triage context;
register a bounded task before claiming readiness. On Windows substitute the
configured Python executable or `py -3.13`. No new dependency or CMake target.

Packets show current source/author, exact registered task/module/prerequisite
states, initial write scope, selected code and contract locators, scope-expansion
triggers and check suggestions. Markdown sections have calculated line ranges;
function declarations have exact line locators. The tool prints no source bodies,
whole registries, history or vendor code. `--json` supports worker tooling.

Rules and gates are always included. Both output formats fail above 8192 UTF-8
bytes rather than silently truncating safeguards. Unknown tasks never silently
fall back to another task/area. Completed tasks are review-only, and blocked or
deferred tasks retain their gates; none propose implementation writes or repeated
checks. Area triage is not an assignment or authorization. Inspect actual code,
callers/tests and the named contracts; expand and update the route when shared
API, ownership, wire or security implications require it. Routes are an index,
not a complete dependency-analysis proof.

`--verify-source` hashes every **listed** source file in a clean extraction and
prints a summary. It rejects changed hashes, stale manifest identity, unsafe
paths and symlink components. Verify the external ZIP SHA-256 separately; neither
step proves provenance, finds untracked files nor constitutes a security audit.
The CLI has no network, builds, command execution, file writes or model calls;
Python import caching is disabled for this command. It runs independently of cwd.

## Measured startup text

UTF-8 byte counts, deterministically measured from the verified parent and this
source; not model-specific token counts or a total-task token benchmark:

| Read set | Bytes |
| --- | ---: |
| Previously prescribed eight startup documents | 176,161 |
| START-HERE.md + active AGENTS.md | 7,943 |
| Next app task packet, plain output without optional verification | 3,740 |
| New entry guides + that selected-task packet | 11,683 |
| Same task packet as JSON | 4,785 |

This selected-task initial text is **93.4% smaller** than the prior mandatory
bundle. A task chosen with `--list` adds its short status output; optional source
verification adds a summary. Task-specific code/contracts still need reading,
so this is not a claim of 93.4% fewer total tokens. It does not shrink platform
system/conversation context or guarantee every external agent obeys instructions.
[CONTEXT_BASELINE.json](../development/CONTEXT_BASELINE.json) records the eight
parent-file sizes and hashes for reproduction. Old instructions/map are archived
under `docs/development/history/` with rebased Markdown links. Their historical
read-all instructions are superseded by this newer owner-requested workflow;
substantive design/security contracts remain binding and unchanged.

## Cumulative scope and evidence

Input: GW-APP-P02-MEMBERSHIP-LIFECYCLE revision 2 ZIP,
SHA-256 `45fa5488149b548b1bad58ecd06362ba7587636b84508954c5b0a8a4c200d73e`.
Verified parent archive checksum, CRC and all 315 listed source hashes before
isolated extraction. All 315 parent files remain in this cumulative delivery.
132 protected files (apps/src/include/tests/third_party/.github/cmake, root CMake,
presets/vcpkg manifests and full master contract) are byte-identical to the parent.
Application behavior and original runtime/build/test/dependency inputs are preserved.

Changed parent files: AGENTS.md, BUILD.md (source note), README.md, TODO.md,
current-development-map.md, docs/CHANGELOG.md, docs/COMPLETED.md,
docs/development/MODULE_REGISTRY.json, STRUCTURE_RULES.md, WORKER_QUEUE.json,
WORKER_INTEGRATION.json and tools/packaging/package_source.py.
New files: START-HERE.md, this report, TASK_CONTEXTS.json, CONTEXT_BASELINE.json,
the two historical entry documents and tools/development/task_context.py plus
its focused test_task_context.py. Packager requires START-HERE.md and emits
this checkpoint's parent/revision/author metadata and matching .zip.sha256.

Actual local checks (Python 3.12.14 / Linux):

- Task context: **14/14 tests PASS**, covering all 10 areas and 21 tasks in both
  formats, section selectivity, gate/completed behavior, unknown IDs, traversal/
  symlinks, output overflow, clean/tampered/stale inventory, cwd independence and
  no source-write side effects. No source body is emitted by a packet.
- Existing module registry: **13/13 PASS**; CLI validates **26 modules / 21 tasks**,
  one ready app task, two blocked tasks and deferred gates unchanged.
- Existing effort forecast **7/7 PASS**, Windows documentation **3/3 PASS**;
  dependency boundary check **PASS**. Total Python test cases: **37/37 PASS**.
- Protected-source comparison: **132/132 unchanged**; no parent file removed.
- Source package: pinned upstream validation, CRC, entry count and embedded
  per-file SHA-256 verification; clean-extraction context/manifest verification.

No C++ rebuild, Windows/macOS execution, hosted workflow, external review or
independent agent is claimed for this tooling-only patch. Single coordinator;
no delegated workers, remote publication or owner acceptance. Engineering
confidence **0.91** describes this bounded implementation, not project completion
or security certification.

## Remaining work

**GW-APP-PUBLIC-RECORD-EXCHANGE** remains ready, unimplemented and unassigned:
explicit bounded offline A04 public-record exchange, with full authority/owner
scope and existing verify/pin/accept semantics. A05 native macOS/guard confirmation,
independent parser review and owner disposition remain open; A06 is deferred.
AUTH-PRE possession/channel binding/commit-time authorization and A11-PRE protected
storage remain unimplemented. P02 r1 owner Windows evidence remains source-bound;
P02 r2 native confirmation is not inferred from it. Official completion is null,
accepted effort floor 11/110 (~10%), next agreed Windows gate 20% (not reached).
