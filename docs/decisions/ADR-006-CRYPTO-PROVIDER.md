# ADR-006: Bounded volatile cryptography through Botan

Date: 2026-10-13. Status: selected for authorized A03 implementation; independent
review and native A03 acceptance remain separate gates. A02 owner acceptance is
recorded in GW-A02-HOST-ACCEPTANCE.md. No identity or persistence authorization.

## Selection and current review

| Candidate | Evidence and fit | Decision |
|---|---|---|
| Botan 3.13.0 (2026-08-13) | Maintained C++20, BSD-2-Clause, MSVC/Linux/macOS; modular source builds; system RNG exceptions, clearing allocator, AEAD and Ed25519; published historical external evaluation and ongoing upstream side-channel tests | Select the bounded subset below |
| libsodium 1.0.22-stable | Maintained ISC library, established NaCl lineage and published audit; strong high-level AEAD/signature APIs and secure-memory utilities; broad native support | Good future alternative. Its default randombytes_buf API has no error result and uses fatal failure for unavailable entropy. A03 specifically needs recoverable typed entropy failures; replacing its global RNG implementation would complicate that contract |
| OpenSSL 3.5.9 LTS / 4.0.3 (2026-09-29) | Maintained Apache-2.0, reviewed widely deployed code; EVP and RAND return failures; Windows/Linux/macOS; LTS 3.5 support through 2030-04-08 | Not selected for this small facade: substantially larger build/provider surface and Perl/assembler tooling. This is a scope/build decision, not a claim that OpenSSL is unsafe |

Upstream Botan advisories were checked on the date above. 3.13.0 fixes the
August 2026 AutoSeeded_RNG empty-output/reseed issue, 32-bit Scrypt overflow,
OCSP SSRF, X.509 name constraints and FFI/CLI/binding defects. Those APIs are not
exposed here; AutoSeeded_RNG, Scrypt, OCSP, TLS, FFI and network modules are not
enabled. No published unresolved advisory applicable to this selected release
and subset was found in that review; this is time-limited evidence, not a
vulnerability-free guarantee. Recheck before any release/provider upgrade.

The published 3curity/Sirrix evaluation concerns Botan **1.11.18**, not 3.13.0
or GhostWire. It establishes external-review history, not current certification.
Upstream describes constant-time coding and compiler/architecture side-channel
testing, with explicit limits for speculative execution, power and EM attacks.
GhostWire still needs independent review of its facade and compiled targets.

## Dependency and build decision

Pin official Botan-3.13.0.tar.xz, SHA-256
`12f5a8358890bbee82edfe9d2e7769b0a610b6dd0e0698aea13d20a675d84620`,
release revision `2a81eef56c96c237e590c27f9a75e60317c9c700`.
The published SHA matched and GPG verified its detached release signature with
fingerprint `621DAF6411E1851C4CF9A2E16211EBF1EFBADFBC`. The key was obtained
from the same official HTTPS site; this is not independent web-of-trust validation.

Bundle the unmodified signed upstream source distribution, license and three
upstream-generated minimized amalgamations (Windows, Linux, macOS). Use the
documented generic CPU target and disable assembly; x64 and ARM64 use portable
code. Python is needed only for reproducible regeneration, not an ordinary
offline Windows build. Every build verifies the selected generated files and
source-archive pin. No configure-time download, system-library substitution or
silent downgrade. The regeneration script has a compare-only mode.

This extends the existing exact vendoring policy to the first runtime dependency;
CMake/Ninja and the vcpkg dependency manager remain. Both portable and vcpkg
presets compile the **same** pinned static provider. vcpkg still supplies doctest
in its existing route, with its baseline/override unchanged. This deliberate
exception avoids an older registry Botan version or different crypto builds
between the accepted offline route and standard development. It does not claim
the owner's unresolved Windows vcpkg/doctest configuration was repaired.

Static linkage adds no Botan DLL. Windows uses its system BCrypt library; macOS
uses the system CommonCrypto RNG; Linux uses getrandom. Platform code stays in
upstream implementation files. GhostWire headers expose no Botan/native types.
Generic builds trade hardware acceleration for a small, reproducible baseline;
performance and accelerated builds require a future measured review.

## Bounded primitive scope and ownership

Expose only OS-backed random bytes, RFC 8439 ChaCha20-Poly1305 (32-byte key,
12-byte nonce, 16-byte tag), RFC 8032 pure Ed25519 (32-byte seed/public key,
64-byte signature) and SHA-256. No algorithm-name strings, arbitrary DER/PEM
imports, KDF/password policy, key exchange, protocol, identity or storage API.
Callers must provide a **unique nonce for each AEAD encryption under a key**;
there is no durable counter or messaging protocol. Random 96-bit nonces are not
promoted as a general nonce-management solution. Future protocol/storage gates
must choose an explicit reviewed nonce and key lifecycle.

The noncopyable provider owns cancellation, limits and a nonblocking single-call
admission lock. Operations are synchronous and bounded; concurrent work returns
busy rather than joining a hidden queue. Secrets have move-only ownership,
bounded shared accounting and controlled read-only borrowing. AEAD and signing
keys are distinct types with no export, implicit text conversion or serializer.
Provider-native handles remain private. Destruction cannot race callers; keys
and returned secrets can outlive the provider for safe clearing/accounting.

Use Botan's clearing allocation/deallocation and explicit secure_scrub_memory.
The locking allocator is best effort and can fall back to ordinary heap memory;
no all-buffers-locked claim. Stack/register copies, caller copies, dumps, swap,
hibernation, VM snapshots and a compromised endpoint are not comprehensively
protected. The backend internally shares its OS RNG and locked allocator pool;
these are upstream OS-resource implementations, not a GhostWire service registry.

Errors are fixed enums with static descriptions and operation IDs; no provider
exception text, key identifier or input enters diagnostics. Failures and
cancellation never publish partial random output or unauthenticated plaintext.
Private failure injection is compiled into a separate test library only; the
production provider has no entropy-replacement or fault-setting API.

Core composition may depend on crypto; crypto depends only on header-only core
Result values and Botan. This remains acyclic. A real lifecycle adapter owns the
provider, initializes it at ordinary bootstrap and stops it before destruction;
help/version/config/path commands retain their existing no-write behavior.

A11-PRE still gates persistent identities/session state; AUTH-PRE still gates
listeners and privileged remote actions. A03 primitive correctness is not E2EE,
forward secrecy, contact authentication, secure erasure or protocol readiness.

## Primary references reviewed

- https://botan.randombit.net/ and https://botan.randombit.net/news.html
- https://botan.randombit.net/security.html
- https://botan.randombit.net/releases/sha256sums.txt
- https://botan.randombit.net/releases/audit_1.11.18.pdf
- https://botan.randombit.net/handbook/building.html
- https://botan.randombit.net/handbook/api_ref/rng.html
- https://botan.randombit.net/handbook/api_ref/secmem.html
- https://botan.randombit.net/handbook/threat_model.html
- https://doc.libsodium.org/ and https://doc.libsodium.org/generating_random_data
- https://openssl-library.org/source/ and https://openssl-library.org/news/vulnerabilities/
- https://docs.openssl.org/3.5/man3/RAND_bytes/
- https://www.rfc-editor.org/rfc/rfc8439.html
- https://www.rfc-editor.org/rfc/rfc8032.html

Source review includes the exact pinned Botan system_rng, allocator, AEAD and
Ed25519 implementations. See CRYPTO_PROVIDER.md for the precise facade contract,
and GW-A03.md for actual outcomes and remaining acceptance gates.
