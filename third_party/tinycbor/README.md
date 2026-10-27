# TinyCBOR source gate (not yet installed)

The upstream TinyCBOR v7.0 release is *not* bundled at this checkpoint.
`tools/protocol/tinycbor_pins.json` identifies the fixed upstream commit and
12 original source files, including its MIT LICENSE, by SHA-256.

The owner may separately supply a ZIP or extracted directory from that exact
release, then run `python tools/protocol/import_tinycbor.py --source <PATH>`
for verification only, or add `--install` to stage verified test-only sources into an atomically published `verified/` directory.
No normal CMake build, CI check or executable performs a download. Importing
source is not equivalent to an independent code or license compliance audit.

Public-network protocol use remains prohibited. Default builds continue to use
the existing restricted codec and do not import or link TinyCBOR. When the
verified upstream source is installed, an explicitly opt-in interop target can
be enabled separately. None of these steps adds a new cryptographic protocol.


## Optional explicit pinned fetch (A05-P04-HF02)

For an **operator-invoked** public GitHub HTTPS download (never part of CMake
or a regular build), run from the repository root:

```
python tools/protocol/fetch_tinycbor.py --acknowledge-network --output tinycbor-v7.0-pinned.zip
python tools/protocol/import_tinycbor.py --source tinycbor-v7.0-pinned.zip --install
```

The fetch requires a previously nonexistent output path, exactly the pinned
commit URL, bounded transfer, and all twelve SHA-256 pin matches before saving.
Delete the downloaded local ZIP after importing if it is not needed. Existing
files are never overwritten. To run test-only interop, follow the A05-P04-HF02
checkpoint notes. The fetch utility requires network access only when expressly
invoked; the shipped source has no automatic download dependencies.
