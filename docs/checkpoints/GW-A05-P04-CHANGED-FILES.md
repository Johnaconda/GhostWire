# A05-P04 partial source changed-file inventory

**Updated**

- `CMakeLists.txt` — isolated fault test, optional SHA-256-gated maintained TinyCBOR test target.
- `ROADMAP.md`, `TODO.md`, `THIRD_PARTY_NOTICES.md` — explicit blocked upstream gate and license notice.
- `tools/protocol/differential_check.py` — optional three-way generic-CBOR versus strict-schema comparison.
- `tools/packaging/package_source.py` — partial checkpoint name; revalidate imported upstream before any future inclusion.

**Added**

- `docs/checkpoints/GW-A05-P04.md`, `GW-A05-P04-VALIDATION.json`, `GW-A05-P04-WINDOWS-ACCEPTANCE.md`, `GW-A05-P04-CHANGED-FILES.md`.
- `third_party/tinycbor/README.md` — future test-only provenance/integration policy.
- `tools/protocol/tinycbor_pins.json` — pinned v7.0 upstream SHA-256 catalog.
- `tools/protocol/import_tinycbor.py` — offline verified import, staged atomic publication.
- `tools/protocol/test_tinycbor_import.py` — fail-closed importer security regression.
- `tests/interop/tinycbor_probe.cpp` — isolated maintained-parser test adapter, uncompiled until real source is supplied.
- `tests/fixtures/protocol/allocation_fault.cpp` — process-local allocation-fault regression fixture.

No production C++ codec/identity/crypto implementation changed. The runtime is still 0.4.1. The upstream library itself is **not** present.
