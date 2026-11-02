# GhostWire — start here

This is the first-read guide for a new worker. Read this and [AGENTS.md](AGENTS.md)
once, then load only your task's context. Do not start by reading the whole source
tree, every report, or the complete master contract.

## What exists

GhostWire is a C++20 communication application in foundation development. The
working interface is `ghostwire-cli`: volatile profiles, independent community
credentials, device records and explicit local trust. Membership preparation is
not admission. Desktop/server/relay are bootstrap shells. Networking, messaging,
GUI and private persistence are not implemented. The offline A05 codec is still
subject to native/review/owner gates. Runtime version remains 0.4.1.

## Choose the read set before reading code

From the source root, with Python 3.9+ (the owner uses Python 3.13 on Windows):

```bash
python3 tools/development/task_context.py --list
python3 tools/development/task_context.py --task GW-APP-PUBLIC-RECORD-EXCHANGE
```

On Windows use the configured Python executable or `py -3.13` with the same
arguments, avoiding the WindowsApps shim. For a new unregistered issue use
`--area cli` (or an area shown by `--list`), then register a bounded task.

The packet gives the current source identity, task/dependency status, intended
write set, relevant file/section locators, scope-expansion triggers and checks.
It does not execute checks, authorize a blocked feature, or substitute for actual
code inspection. JSON output is available with `--json`; packets are bounded to
8192 UTF-8 bytes and never dump whole source files or worker history.

Read the named declarations/functions and direct callers. Follow their relevant
tests/contracts. If a shared API, lifetime, permission, serialization or storage
meaning changes, expand into that dependency and record why. Otherwise defer
unrelated modules. `rg -n` and targeted line ranges are preferable to large `cat`
calls. Do not reread unchanged contracts already available in the current task.

## Trust the source and retain the boundaries

The incoming cumulative ZIP and its checksum establish the baseline. After clean
extraction, add `--verify-source` to check the embedded hashes programmatically;
only the result enters model context. A modified working tree will fail that
check until repackaged. Source hashes are not an independent security audit.

[AGENTS.md](AGENTS.md) carries the mandatory ownership/security/publication rules.
[current-development-map.md](current-development-map.md) is a short status map.
The module registry is authoritative for ready/blocked/completed task state;
`TASK_CONTEXTS.json` is its read-route index. Update both when a task's scope changes.
History remains in checkpoint reports and `docs/development/history/`; open it
only to resolve a task-specific question. The full master contract remains
available by section rather than being mandatory startup reading.

## Finish the task

Implement the missing capability, run the listed change-scoped checks, update
the factual records and deliver cumulative source with an author tag and matching
SHA-256. No Windows recompile is required for this development-tool-only change.
The current ready application task is still offline public-record exchange;
this guide does not implement it or unlock networking/storage gates.
