# Third-party notices and dependency provenance

The four application binaries use the C++ standard library, platform APIs and
the pinned static Botan subset below. They do not link doctest, GUI, database
or networking implementations.

## Botan (runtime)

Version 3.13.0, official release 2026-08-13, BSD-2-Clause, copyright 1999-2026
The Botan Authors. The complete required license/disclaimer is preserved in
third_party/botan/LICENSE.txt and installed with application distribution notices.
The original signed source distribution and unmodified upstream-generated
Linux/Windows/macOS minimized amalgamations are bundled. No external DLL is added.

Official archive: https://botan.randombit.net/releases/Botan-3.13.0.tar.xz

SHA-256: `12f5a8358890bbee82edfe9d2e7769b0a610b6dd0e0698aea13d20a675d84620`.
Release revision: `2a81eef56c96c237e590c27f9a75e60317c9c700`.
Detached signature verified using release public-key fingerprint
`621DAF6411E1851C4CF9A2E16211EBF1EFBADFBC`; key and hash fetched from the same
official HTTPS service, not independently certified. Per-file generated hashes
are in third_party/botan/pins.json and checked by CMake. Offline regeneration
procedure: third_party/botan/README.md and tools/crypto/generate_botan.py.

Modules requested: system_rng, chacha20poly1305, ed25519, sha2_32,
locking_allocator and their upstream dependency closure. Generic CPU, no assembly.
The same exact provider is built in both existing dependency modes. This explicit
runtime vendoring decision, current advisory/license review, alternatives and
historical external-audit limits are in docs/decisions/ADR-006-CRYPTO-PROVIDER.md.
Bundled upstream vector seeds and the release public key are public fixtures,
not user secrets or operational signing credentials.

## doctest (test-only)

Version 2.4.12, upstream https://github.com/doctest/doctest,
commit `1da23a3e8119ec5cce4f9388e91b065e20bf06f5` (tag v2.4.12).
The unmodified header and MIT license are included under third_party/doctest.

| File | SHA-256 |
|---|---|
| third_party/doctest/doctest/doctest.h | 94029a7d32da24a56249658147dbd2b33ff0b9ed665295cbbaf19aafff5b0ced |
| third_party/doctest/LICENSE.txt | 0fe0b331fa1513dcce8604ff1fa925f32d1cea17d8aeb1c2471fad40d291adc5 |

Raw files were obtained from the exact upstream commit, not reconstructed.
CMake verifies the header checksum when the vendored provider is selected.
The vcpkg route resolves doctest 2.4.12 at baseline
`9e593bb18ea69cc5095e012465dcd675a822ed0d` (2026.07.29), with its
vcpkg-cmake 2024-04-23 and vcpkg-cmake-config 2026-07-21 build helpers.
Those helpers and vcpkg itself are build tools, not bundled application code.

GitHub Actions checkout is pinned to
`11d5960a326750d5838078e36cf38b85af677262` (v4 as resolved during preparation).
Runner images and system compilers still evolve; CI must record their versions.
These pins establish a reproducible starting point, not a claim that every tool
is the latest release or free of vulnerabilities. Reassess advisories before
publishing a release. Other future candidate libraries are not shipped here.
