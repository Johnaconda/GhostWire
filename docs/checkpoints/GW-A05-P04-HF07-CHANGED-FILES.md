# HF07 changed files relative to HF06

- `tools/protocol/import_tinycbor.py` — reject normalized raw ZIP names via `orig_filename`.
- `tools/protocol/test_tinycbor_import.py` — generate the malformed backslash case on disk, not through a platform-normalizing ZIP writer.
- `tools/packaging/package_source.py` — reflect the current source checkpoint in package and manifest defaults.
- `docs/development/MODULE_REGISTRY.json` and `WORKER_QUEUE.json` — advance checkpoint reference only; no completion credit or gate change.
- `docs/checkpoints/GW-A05-P04-HF07.md` — findings and validation boundaries.
- `docs/checkpoints/GW-A05-P04-HF07-VALIDATION.json` — actual validation outcomes and remaining limits.
- `docs/checkpoints/GW-A05-P04-HF07-WINDOWS-10PCT-GATE.md` — safe native build/CTest sequence.

No C++ runtime changes, deleted files, or GitHub modifications.
