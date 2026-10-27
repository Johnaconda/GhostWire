# HF08 changed-file inventory relative to verified HF07

## Runtime-equivalent test implementation

- `src/protocol/wire_codec.cpp` — private, compile-time test-only injection point at each existing byte-buffer allocation boundary; production build has no hook.
- `tests/fixtures/protocol/allocation_fault.cpp` — remove process-wide operator new/delete overrides; use scoped deterministic fault injection, preflight/lease/input/roundtrip regressions.
- `tests/fixtures/protocol/allocation_fault_global_linux.cpp` — preserve HF07's original process-wide fault and preflight assertion on Linux as an independent stricter regression.
- `CMakeLists.txt` — compile the original codec source directly into the fault test under a *private* target macro; retain 15-second watchdog.

## Evidence, module state and package metadata

- `docs/checkpoints/GW-A05-P04-HF08.md` — diagnosis and limits.
- `docs/checkpoints/GW-A05-P04-HF08-VALIDATION.json` — observed Linux results and native gaps.
- `docs/checkpoints/GW-A05-P04-HF08-LINUX-DEBUG-CTEST.txt` — full Linux Debug CTest output (52/52).
- `docs/checkpoints/GW-A05-P04-HF08-LINUX-RELEASE-CTEST.txt` — full Linux Release CTest output (52/52).
- `docs/checkpoints/GW-A05-P04-HF08-WINDOWS-10PCT-GATE.md` — VS2026 test commands.
- `docs/checkpoints/GW-A05-P04-HF08-CHANGED-FILES.md` — this inventory.
- `docs/development/MODULE_REGISTRY.json` — scope-specific completed Linux task and blocked native acceptance; project percent unchanged.
- `docs/development/WORKER_QUEUE.json` — dependency queue, confidence, remaining gates.
- `docs/development/VALIDATION.md` — current validation addendum.
- `AGENTS.md`, `docs/CHANGELOG.md` — current scope authority and history.
- `tools/packaging/package_source.py` — new cumulative checkpoint metadata and deterministic archive name.
- `SOURCE_MANIFEST.json` — regenerated per-file integrity manifest, packaged with source ZIP.

No network/runtime schema, A04 identity/private-key, Botan/TinyCBOR dependency,
PowerShell, GUI or A06 change. No GitHub push.
