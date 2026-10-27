# GW-A05-P04-HF02 — Changed-file inventory

Source baseline: `GW-A05-P04-HF01-SCOPED-PARALLEL-INTEGRATION`
(SHA-256 `856562b80244c7545eac6718c9385cb958caddfed5e335e67ab7696ccf415e24`).

## Modified

- `AGENTS.md` — active scoped authority and security caveats
- `CMakeLists.txt` — independent test probes and fetch-guard CTest registrations
- `README.md` — latest checkpoint status, no runtime version increase
- `TODO.md` — current dependencies and task statuses
- `docs/development/WORKER_QUEUE.json` — HF02 task ownership and >0.80 subjective confidence
- `tests/interop/tinycbor_probe.cpp` — bounded stdin and oversize error behavior
- `tests/fixtures/protocol/probe.cpp` — bounded stdin and resynchronization
- `third_party/tinycbor/README.md` — documented explicit pinned fetch procedure
- `tools/packaging/package_source.py` — checkpoint name and package output
- `tools/protocol/differential_check.py` — frozen-design-vector corpus coverage

## Added

- `docs/checkpoints/GW-A05-P04-HF02.md`
- `docs/checkpoints/GW-A05-P04-HF02-VALIDATION.json`
- `docs/checkpoints/GW-A05-P04-HF02-CHANGED-FILES.md`
- `tests/interop/bounded_hex_line.hpp`
- `tools/protocol/fetch_tinycbor.py`
- `tools/protocol/test_tinycbor_fetch.py`
- `tools/protocol/test_probe_bounds.py`

## Removed

- None. The generated `SOURCE_MANIFEST.json` is regenerated when packaging.
