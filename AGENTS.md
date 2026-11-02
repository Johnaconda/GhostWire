# GhostWire agent entry rules

Read [START-HERE.md](START-HERE.md) once, then load the context for your assigned
task. These are the active repository instructions. The owner's newer task/scope
instructions take precedence. Historical checkpoint instructions describe their
own source and are not startup requirements.

## Read only the task's dependency cone

- Run `python3 tools/development/task_context.py --list` for current tasks, or
  `--task TASK_ID` for its status, exact files, contract sections and checks.
  Use `--area cli` (or another listed area) for a new issue without a task ID.
- Read the returned declarations/functions, direct callers and relevant tests.
  Follow section/line locators, not whole historical reports. Read each unchanged
  contract once per task. Missing context is a reason to inspect a specific
  dependency, not to dump the repository.
- Expand the read/write scope when a shared API, ownership rule, wire meaning or
  security boundary is affected; state the concrete reason and update the route.
  Routing is an index, not proof that other callers are unaffected.
- Completed tasks stay completed unless new evidence warrants a correction.
  Blocked/deferred tasks do not become authorized because a packet can describe
  them. Do not manufacture progress or silently choose a different task.
- Defer `third_party/`, generated builds, old checkpoint reports, history and the
  full master contract unless the selected task actually requires them. Relevant
  master-contract/ADR sections remain binding. Root history is archived under
  `docs/development/history/`.

## Invariants that always apply

- Use the verified cumulative source; never reset it or replace working code with
  stubs. C++20, explicit ownership/RAII, typed errors, bounded resources and the
  dependency DAG remain required. Native OS types stay in adapter .cpp files.
- Main thread owns UI/event state; services own their executors and consumers must
  stop before dependencies. No global registry or success-shaped fake feature.
- Numeric aliases/display names are not authority. Preserve independent profile,
  community/member and device handles/keys; recognition is not possession/login
  or action permission. Never invent crypto or an original messaging protocol.
- A05 native/external parser review and owner disposition remain open. No A06
  listener/network parsing before its gates. AUTH-PRE fresh possession/channel
  binding and commit-time policy/revocation integration remain unimplemented;
  HF19 LocalPolicyTable is candidate data, not authorization.
- A11-PRE protected storage is required before private-key/history persistence.
  Current keys/trust are volatile. No temporary plaintext secret store, remote
  key destruction, privacy downgrade or implicit admin/AI/plugin private access.
- Diagnostics use fixed metadata; do not log raw input, paths, endpoints, keys,
  content or exception text. Private storage work must cover temp files, backups,
  dumps, caches and indexes. Passing tests is not security certification.
- No remote push/PR/merge/tag/dispatch/publication without owner authorization.
  Do not send messages to others. Do not claim agents/reviews that did not run.

## Execution and delivery

Verify the incoming ZIP checksum and embedded inventory before editing. On a clean
extraction, `task_context.py --verify-source --task TASK_ID` checks hashes in code
and prints only a summary; this is not another model read of every file. Keep work
isolated. Inspect actual code/TODOs, implement only the missing piece, then run
change-scoped checks. Broaden regression for crypto/wire/platform/shared-core work.
Avoid duplicate harnesses and repeated suites without a new reason.

Keep the current map, task registry and actual worker ownership factual; add task/
owner/TODO comments in touched source. Update relevant evidence/ADRs when behavior
or decisions change. Distinguish implementation confidence (>0.80), exact-source
validation, owner acceptance and project completion. Official completion remains
null; accepted effort floor is 11/110 (about 10%), next agreed Windows gate 20%.
Do not request Windows acceptance for each small edit.

Use `python3 tools/packaging/package_source.py` for cumulative source and SHA-256.
Preserve history, exclude builds/private state, and verify the archive. Display the
known patch author label in the response, checkpoint report, archive filename and
metadata; checksum filename/content must match. The owner-provided current label
is GPT-6.1-Sol-Max; do not infer unavailable model metadata. Runtime remains 0.4.1.
