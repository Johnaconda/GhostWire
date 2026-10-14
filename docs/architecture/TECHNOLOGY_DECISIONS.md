# Technology decisions and source review

Initial review 2026-10-11; crypto decision updated 2026-10-13 in ADR-006.
“Candidate” means no adoption or security claim. Recheck upstream maintenance, licensing, advisories and
platform support at the checkpoint that actually adopts a dependency.

| Area | Current decision | Rationale / remaining gate |
|---|---|---|
| Host language | C++20 | User toolchain; no second language at bootstrap. ADR-001 compares alternatives. |
| Build | CMake >=3.25, Ninja, vcpkg manifest | Explicit profiles and pinned ports; vendored exact test fallback. |
| Test framework | doctest 2.4.12 | Small upstream test-only header; exact SHA checked offline. |
| Core linkage | Static reusable libraries | No plugin ABI or DLL deployment promise at A00. |
| Config | Bounded nonsecret key=value schema 1 | Small bootstrap grammar; no unnecessary JSON/TOML runtime dependency. |
| Log | Typed synchronous JSON-lines | Fixed event names and sensitive-key redaction; stdout/stderr only. |
| Crypto primitives | Botan 3.13.0 pinned static subset | A03 owns OS RNG, ChaCha20-Poly1305, pure Ed25519 for public messages, SHA-256 and clearing handles; ADR-006 compares libsodium/OpenSSL. KDF, protocol and protected persistence remain future. |
| DM protocol | Evaluate reviewed Signal-family implementations and established alternatives | Library license, external support, FFI, multi-device and async state consistency need an explicit decision. |
| Group protocol | MLS candidate | RFC 9420 protocol and RFC 9750 architecture; validate authentication/delivery integration and implementations. |
| Vault | SQLCipher candidate | Key management, temp stores, journals, encrypted indexes and atomic ratchet transactions need a prototype and audit. |
| Transport | Deferred to A06 | Compare QUIC/TLS and relay/media integration using real workloads, policy leak tests and library support. |
| Desktop | SDL3/native GPU and retained UI candidate | Accessibility, Unicode shaping/IME, DPI, hit testing and capture need platform proof first. |
| License | MIT for project code | Explicit choice; third-party licenses independent. ADR-002 records trade-offs. |

## Primary sources

- CMake preset format: https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html
  Repository presets are shared; user-specific overrides are ignored by Git.
- vcpkg versioning: https://learn.microsoft.com/en-us/vcpkg/users/versioning
  A registry baseline and exact override pin the test dependency; compiler and
  tool binary versions must also be recorded for reproducibility.
- doctest: https://github.com/doctest/doctest and
  https://github.com/doctest/doctest/tree/1da23a3e8119ec5cce4f9388e91b065e20bf06f5
- libsodium password hashing: https://doc.libsodium.org/password_hashing
  Password KDFs are separate from retry counters; choose and benchmark explicit
  parameters and store their version with the wrapped-key envelope.
- Argon2: https://www.rfc-editor.org/info/rfc9106/
- Signal specifications: https://signal.org/docs/ and implementation:
  https://github.com/signalapp/libsignal . The reviewed repository declares
  AGPL-3.0 and contains Rust with language bindings; see ADR-003 before adoption.
  A protocol specification does not itself supply an approved C++ integration.
- MLS protocol: https://www.rfc-editor.org/rfc/rfc9420.html and architecture:
  https://www.rfc-editor.org/rfc/rfc9750.html . Group epoch security does not solve
  account trust, delivery availability or removal of previously copied plaintext.
- SQLCipher design: https://www.zetetic.net/sqlcipher/design/ and key material:
  https://www.zetetic.net/sqlcipher/database-key-material/ . Application-side
  key lifecycle and temporary store configuration remain essential. Do not
  assume an encrypted database protects every companion file automatically.
- SDL3: https://wiki.libsdl.org/SDL3/FrontPage . Platform/multimedia primitives
  are not a complete accessible retained widget toolkit.
- Windows capture: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowdisplayaffinity
  OS display-affinity controls are limited; they are not DRM or protection from
  an external camera or a compromised operating system.
- License texts and FAQ: https://opensource.org/license/mit ,
  https://www.apache.org/licenses/LICENSE-2.0 ,
  https://www.mozilla.org/en-US/MPL/2.0/FAQ/ .

This source review informs design gates. It is not a dependency security audit,
legal sign-off, protocol certification or a claim of current interoperability.

## Windows toolchain review adjustment

Use the pinned 2026.07.29 vcpkg checkout rather than a pre-VS2026 tool. Microsoft
records initial v145/VS2026 support in its October 2025 releases:
https://devblogs.microsoft.com/cppblog/whats-new-in-vcpkg-october-2025/ .
The selected registry release is documented at
https://github.com/microsoft/vcpkg/releases/tag/2026.07.29 .

The Windows script preserves an explicitly supplied/incoming VCPKG_ROOT across
VsDevCmd initialization, honors an explicit Visual Studio root, and diagnoses
a tool predating VS2026 support. It does not silently change or update the
user's dependency checkout. Owner portable A02 MSVC acceptance is recorded in
GW-A02-HOST-ACCEPTANCE.md; Windows vcpkg and all new A03 native acceptance remain open.
