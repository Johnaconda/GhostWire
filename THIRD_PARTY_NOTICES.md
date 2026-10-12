# Third-party notices and dependency provenance

The four application binaries use the C++ standard library and platform APIs.
They do not link doctest or any cryptographic, GUI, database or network library.

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
publishing a release. No future candidate library is licensed or shipped here.
