# A05-P04-HF03 changed files

Baseline: A05-P04-HF02, SHA-256 `ea0a758a93393c836d548ebd3e61216ef48365591c6fd401ad83baedae291f22`.

## Modified
- `AGENTS.md`, `README.md`, `TODO.md` — current scoped authority and gate status
- `CMakeLists.txt` — offline upstream-comparison guard CTest
- `tools/protocol/differential_check.py` — reject known generic CBOR noncanonical cases from actual upstream verdicts
- `tools/protocol/import_tinycbor.py` — refuse preexisting/dangling symlink publication target, factor testable publication helper
- `tools/protocol/test_tinycbor_import.py` — publication regression cases
- `tools/packaging/package_source.py` — cumulative HF03 identity
- `docs/development/WORKER_QUEUE.json` — worker queue checkpoint evidence

## Added
- `tools/protocol/test_interop_gate.py` — offline fail-closed upstream-verdict test harness
- `docs/checkpoints/GW-A05-P04-HF03.md`
- `docs/checkpoints/GW-A05-P04-HF03-VALIDATION.json`
- `docs/checkpoints/GW-A05-P04-HF03-CHANGED-FILES.md`

## Removed
- None; production codec, wire bytes and A04 source unchanged.
