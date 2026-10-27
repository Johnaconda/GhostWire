# HF09 changed files relative to verified HF08

- `src/protocol/wire_codec.cpp` — test-only hook declaration uses C++ linkage,
  explicitly `noexcept(false)`; no production code path changes.
- `tests/fixtures/protocol/allocation_fault.cpp` — matching test-only throwing
  function definition with C++ linkage, explanatory comment.
- `docs/checkpoints/GW-A05-P04-HF09.md` — MSVC C4297 evidence and reasoning.
- `docs/checkpoints/GW-A05-P04-HF09-WINDOWS-10PCT-GATE.md` — native acceptance.
- `docs/checkpoints/GW-A05-P04-HF09-VALIDATION.json` — exact evidence/limits.
- `docs/checkpoints/GW-A05-P04-HF09-LINUX-DEBUG-CTEST.txt` — full-suite regression.
- `docs/checkpoints/GW-A05-P04-HF09-LINUX-RELEASE-CTEST.txt` — if executed,
  full-suite regression.
- `AGENTS.md`, `docs/CHANGELOG.md`, `docs/development/VALIDATION.md`,
  `docs/development/MODULE_REGISTRY.json`, `docs/development/WORKER_QUEUE.json` —
  current authority and pending gates.
- `tools/packaging/package_source.py` — checkpoint ID, output archive filename.
- `SOURCE_MANIFEST.json` — regenerated inside cumulative source ZIP.

HF08 historic files and evidence are kept intact. No CMake warning or timeout
relaxations, dependencies, wire ABI, identity/security rules, A06, or remote writes.
