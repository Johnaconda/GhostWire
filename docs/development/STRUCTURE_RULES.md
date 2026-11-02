# GhostWire task/module development rules — HF05

Derived from the agreed Steel Oath V2 workflow without overriding GhostWire's existing
Master Contract, A05 security gates, or strict identity isolation.

1. Verify the latest cumulative source manifest and ZIP SHA-256 before any change.
2. Read START-HERE.md and AGENTS.md once. Use `tools/development/task_context.py --task TASK_ID`
   to inventory only that task's functions, callers, tests, TODOs and direct dependencies.
   Follow returned contract sections. Expand for shared APIs/security/ownership changes and
   record why; the routing index is not proof of completeness. Do not reload unchanged history,
   vendor code or the full master contract at every startup. This owner-requested workflow
   supersedes blanket startup reading in the older master-contract AI guide; its substantive
   design/security contracts and all gates still apply.
3. Skip implementation that is already correctly implemented and verified.
4. Assign each dependency-ready task a distinct file ownership set. Isolate task branches/worktrees;
   merge only after checksums, conflict detection, task review, and change-scoped tests.
5. Actual separate AI coding agents may be used only if available. Multiple shell jobs are
   *parallel validation*, not multiple independent AI workers.
6. `MODULE_REGISTRY.json` records module dependencies, readiness, evidence, blocking conditions
   and owner acceptance independently of code implementation. Validate with
   `python tools/development/module_registry.py --json` and
   `python tools/development/test_module_registry.py`.
7. A completed task must have concrete evidence and a reasoned engineering confidence greater than
   0.80. This estimate is NOT a probability or security certification. Missing platform tests remain
   explicitly unrun, and user acceptance is tracked separately.
8. Project percentage stays `null` until a complete weighted functional module model is agreed.
   Count of complete tasks, implementation confidence, passing tests and checkpoint number are
   **not** interchangeable with project completion. Windows user acceptance normally occurs at
   agreed ~10 percentage-point gates, with extra gates for security/data loss.
9. Security gates remain binding. Actual pinned TinyCBOR GCC and owner MSVC interoperability is recorded
   in GW-A05-P04-UPSTREAM-INTEROP; native macOS evidence and independent parser review remain
   separate. No A06 networking, private-key persistence, privileged remote authorization
   or claims of secure messaging follow from passing this comparison.
10. Deliver a full cumulative source ZIP, SHA-256, source manifest, changed-file inventory,
    exact tests actually run, limitations and next dependency-ordered work queue. Do not push
    GitHub changes without explicit owner permission.

The module registry is a dependency/evidence tool, not runtime authorization, a service
launcher or a replacement for architecture review.

Task routing lives in `TASK_CONTEXTS.json`. Its packets are read-only and capped at
8192 UTF-8 bytes (including rules/gates); over-budget/stale routes fail, never silently
drop safeguards. Use `--area AREA` to triage a new issue and register its bounded task.
Completed/blocked/deferred packets do not propose implementation writes or repeated
checks. `--verify-source` hashes the embedded listed inventory programmatically on a
clean extraction; verify the external ZIP checksum separately. Do not claim actual
token savings from document byte counts or control over platform-injected context.
