# GhostWire — current development map

Start at [START-HERE.md](START-HERE.md), then use the task packet. This is a current
snapshot; archived checkpoint history is not a mandatory startup read.

## Source and current delivery

- Source: **GW-DEV-CONTEXT-BOOTSTRAP revision 1**, 2026-11-02.
- Patch author tag: **GPT-6.1-Sol-Max**, owner-provided label.
- Verified parent: GW-APP-P02-MEMBERSHIP-LIFECYCLE r2,
  SHA-256 `45fa5488149b548b1bad58ecd06362ba7587636b84508954c5b0a8a4c200d73e`.
- Current change: short agent entry rules and bounded task-specific context routing,
  locator/check selection and clean-inventory verification. No app runtime change.
- Runtime identity: **0.4.1 / GW-A04-P03-IDENTITY-HARDENING**.
- [Delivery evidence](docs/checkpoints/GW-DEV-CONTEXT-BOOTSTRAP.md).

## Working application and next task

The CLI owns real volatile profiles, community membership credentials, devices and
local trust. P02 r2 fixes guided help/return/quit navigation; aliases remain numeric,
labels are not authority and membership preparation is not admission. Keys/trust
are lost on exit. Desktop/server/relay are bootstrap shells.

**GW-APP-PUBLIC-RECORD-EXCHANGE** is dependency-ready, unassigned and unimplemented:
bounded offline public-record exchange using existing A04 verify/pin/accept APIs,
explicit full authority/owner scope, no automatic trust. Get its exact read set:

```bash
python3 tools/development/task_context.py --task GW-APP-PUBLIC-RECORD-EXCHANGE
```

No workers are delegated in this delivery. Completed tasks remain recorded in the
registry; the default packet lists only ready, blocked and deferred work.

## Gates and evidence

A00–A04 foundations are verified within their recorded scope; A05 is a partial
offline codec. Native macOS and source-specific guard confirmation, independent
parser review and owner disposition remain open. A06 networking is deferred.
AUTH-PRE fresh possession/channel binding/commit-time authorization and A11-PRE
protected private storage remain unimplemented. No messaging/GUI/persistence claim.

Owner Windows P02 **r1** Debug/Release CLI build/workflow passes are preserved in
[the P02 report](docs/checkpoints/GW-APP-P02-MEMBERSHIP-LIFECYCLE.md); r2 native
confirmation remains unrun. This tooling change does not request a Windows rebuild.
Official project completion is null; accepted effort floor **11/110 (~10%)**;
next agreed Windows gate **20%**, not reached by adding development documentation.

## Indexes and optional history

- [Module/task registry](docs/development/MODULE_REGISTRY.json): dependencies and readiness.
- [Task routes](docs/development/TASK_CONTEXTS.json): initial code/contract read sets.
- [Worker queue](docs/development/WORKER_QUEUE.json): actual ownership, no fabricated agents.
- [Previous map](docs/development/history/development-map-before-context-bootstrap-2026-11-02.md)
  and [previous agent history](docs/development/history/AGENTS-before-context-bootstrap-2026-11-02.md):
  preserved historical details; consult only for a relevant evidence question.
