# TinyCBOR — pinned, bundled, test-only

`verified/` contains the 12 original files selected by
`tools/protocol/tinycbor_pins.json` from Intel TinyCBOR v7.0, immutable commit
`6442e749ca811e24afad1551338a45c75e09808f`. Their original MIT license and file
notices are preserved. No upstream source bytes are modified.

The existing explicit fetch/import utilities obtained these sources on
2026-10-29. Actual GCC Debug/Release execution of the maintained C parser is
recorded in `docs/checkpoints/GW-A05-P04-UPSTREAM-INTEROP.md`. Source provenance
and successful interoperability are not an independent security audit.

Default CMake builds do not compile or link this dependency. Only the explicit
`GW_A05_ENABLE_TINYCBOR_INTEROP=ON` test configuration enables it, after verifying
every pin. CMake generates static-export and version headers in the build tree.
No normal build, test or product executable downloads anything.

From the source root, verify the bundled files without modifying them:

```text
python tools/protocol/import_tinycbor.py --source third_party/tinycbor/verified
```

For an independent, explicitly invoked acquisition of the same upstream revision:

```text
python tools/protocol/fetch_tinycbor.py --acknowledge-network --output out/tinycbor-v7.0-pinned.zip
python tools/protocol/import_tinycbor.py --source out/tinycbor-v7.0-pinned.zip
```

The output archive must not already exist. `--install` is needed only when the
verified directory is absent; the importer never overlays an existing directory.
The source packager revalidates the full pinned subset and includes its license.
Public-network parsing remains blocked pending separate platform/security review
and owner approval. No product identity, authorization or transport uses TinyCBOR.
