# Pinned Botan runtime source

Botan 3.13.0, release 2026-08-13, BSD-2-Clause. Official source:
https://botan.randombit.net/releases/Botan-3.13.0.tar.xz

The source distribution SHA-256 is
`12f5a8358890bbee82edfe9d2e7769b0a610b6dd0e0698aea13d20a675d84620`.
It matched the official sha256sums.txt. The bundled detached signature verified
with release-key.txt, fingerprint `621DAF6411E1851C4CF9A2E16211EBF1EFBADFBC`.
The key was obtained from the same official HTTPS site; not an independent
web-of-trust certification. pins.json records revision, requested modules and all
generated file hashes. CMake checks the original archive and selected source.

The tar.xz is intentional source provenance and permits completely offline
regeneration; it is not a build cache. It includes upstream public test vectors
and example test keys, never GhostWire operational keys. The bundled release key
is public. No private signing credential is shipped.

## Regeneration

On a POSIX development host with Python 3.12+:

```bash
python3 tools/crypto/generate_botan.py --check
# Deliberate source regeneration, followed by review and validation:
python3 tools/crypto/generate_botan.py
python3 tools/crypto/generate_botan.py --check
```

Run from the GhostWire root. No download, patch or compiler is involved in
generation. The script checks the archive SHA, safe extraction paths and selects
only regular files/directories (an unused upstream CI symlink is omitted). It
runs the exact upstream configure.py for Linux/GCC, Windows/MSVC and macOS/Clang,
generic CPU, assembly disabled, minimized static amalgamation. Requested modules:
system_rng, chacha20poly1305, ed25519, sha2_32, locking_allocator; upstream resolves
their dependency closure, including SHA-512 and public-key base interfaces.
Windows explicitly selects crypto_ng/BCrypt rather than rtlgenrandom.

A fixed unused /opt/ghostwire-botan prefix prevents temporary host paths from
entering generated output. The script never installs to it. All three platforms'
files must compare byte for byte. Do not edit generated cryptographic source.
No Python is required for normal CMake/MSVC builds; the amalgamations are bundled.

Both GhostWire dependency modes use these exact static sources. vcpkg remains
the existing test-dependency route; its baseline and doctest pin are unchanged.
No extra Botan runtime DLL is required. System runtime dependencies remain.
Generic CPU builds deliberately omit hardware acceleration. See ADR-006 and
CRYPTO_PROVIDER.md for selection, public-message signing and memory limitations.
