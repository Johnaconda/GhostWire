# A03 cryptographic provider contract

Implementation: include/ghostwire/crypto, src/crypto, core/crypto_service.hpp and
src/core/crypto_service.cpp. Selection, alternatives, advisory review and pins:
[ADR-006](../decisions/ADR-006-CRYPTO-PROVIDER.md). Validation:
[GW-A03](../checkpoints/GW-A03.md). These are source contracts, not a stable ABI.

## Ownership and lifetime

CryptoProvider::create returns Result<unique_ptr<CryptoProvider>>. Creation checks
limits, the exact backend version, clearing allocator, actual OS entropy and
required primitive availability. Failure publishes no provider. No implicit
global lookup, worker, queue, identity, file or socket is created.

CryptoService is uniquely owned by Application's service manager (service ID 2).
Ordinary bootstrap registers and initializes it; failure rolls back and returns
exit 70 with fixed diagnostics. Help, version, config checking and path printing
return before provider initialization and retain their no-write contracts.
Future consumers receive explicit constructor references and declare a dependency
on cryptographic_provider_service. They must stop and join before this service.
No registry accessor is added to ApplicationContext.

SecretBuffer, AeadKey and SigningKey are move-only. Keys expose valid/reset only,
with separate types for encryption and signing. No key export, implicit string,
streaming or generic serialization exists. Import copies exactly 32 bytes into
clearing storage; the caller remains responsible for its original copy.
SecretBuffer::borrow returns a read-only span whose lifetime ends on reset,
move-assignment or destruction. A borrow grants access, not continued ownership.
Do not race a handle's mutation/destruction with its use. Immutable handle use
may cross providers; accounting remains charged to the provider that allocated
it. Shared accounting survives provider destruction until the last buffer clears.

## Primitive and validation boundary

| Operation | Exact contract |
|---|---|
| random / generated keys | Botan System_RNG, OS entropy only; output is clearing SecretBuffer or purpose-specific key; zero-byte random succeeds without allocation/entropy after creation |
| ChaCha20-Poly1305 | RFC 8439, 32-byte key, exactly 12-byte nonce, 16-byte tag; output payload followed by tag; AAD/nonce supplied separately |
| Pure Ed25519 | RFC 8032, 32-byte seed and public key, 64-byte signature; sign_public/verify_public accept public messages only |
| SHA-256 | Bounded input and fixed 32-byte digest; not password hashing or a MAC |

**Every AEAD encryption under a key requires a unique nonce.** The caller owns
that obligation. A03 supplies no persistent counter, replay policy, envelope,
nonce generator, identity format or messaging protocol. Random 96-bit values
are not presented as a general nonce-management policy. Future persistence and
protocol work must explicitly design and review nonce/key lifetimes.

Malformed lengths, oversized inputs and invalid/moved-from handles fail before
backend processing. Decryption uses private clearing workspace; only successful
authentication followed by cancellation checks can publish plaintext. Tag failure,
resource failure or cancellation returns only an error. This is not replay defense.

Botan's pure Ed25519 implementation buffers message bytes in ordinary heap memory.
Therefore signing and verification are deliberately named sign_public/verify_public;
they promise no confidentiality for message copies. Private seed/key material uses
the backend's clearing storage. Signing is deterministic; key generation uses real
entropy. No KDF, key agreement, DER/PEM import or algorithm-string dispatch is exposed.

## Budgets and concurrency

| Limit | Default | Allowed ceiling |
|---|---|---|
| Message and AAD, each independently | 64 KiB | 1 MiB |
| Random result per call | 64 KiB | 1 MiB; nonzero configured limit |
| Live returned secret bytes | 4 MiB | 64 MiB; nonzero |
| Live returned secret allocations | 256 | 4096; nonzero |
| Declared temporary workspace | 3 MiB | 64 MiB; at least 64 KiB |

Zero message limits are valid. A zero-byte result consumes no secret allocation.
Counters are checked before allocation; failed allocation releases its reservation.
Moves transfer reservations; reset/destruction releases them exactly once.
One nonblocking admission mutex serializes backend operations per provider.
Contending calls return busy immediately; no queued payloads or detached threads.
Independent providers share upstream OS RNG/locked-pool internals, whose upstream
synchronization applies. GhostWire's shared accounting has its own short mutex.

AEAD work is admitted as 64 KiB overhead plus twice payload, AAD and tag bytes;
sign/verify use 64 KiB plus twice message bytes. Fixed-size key/hash work fits
the minimum overhead allowance. These conservative logical budgets are not a
measured allocator-exact bound, process RSS cap, CPU quota or locked-memory
guarantee. Callers own returned public ciphertext/signature/digest memory and
must bound retention. Repeatedly creating providers multiplies budgets; the
composition root owns admission across services. No general job framework is added.

## Cancellation and shutdown

Each call observes owned stop state, the application token and its per-call token
before and after bounded backend work. Randomness is requested in chunks of at
most 4096 bytes with checks between chunks. No partial output is returned.
Cancellation cannot forcibly interrupt one OS RNG or cryptographic call.
An error may win a simultaneous cancellation race; no success bypasses the final
check. A stop request arriving after that check may race normal completion.

request_stop is thread-safe and terminal. try_close requests stop and immediately
returns false if an operation still owns the admission lock. CryptoService then
reports outstanding_work and retains its provider for the existing lifecycle
retry/retention rules. Repeated idle shutdown succeeds. A true try_close result
does **not** authorize destruction while any other caller could still enter a
method: the owner must join all consumers first. Destruction cannot race calls.
No hard in-process shutdown deadline or asynchronous cancellation is claimed.

## Errors, diagnostics and memory limits

Crypto Error contains only ErrorCode and Operation enums. describe returns fixed
text; provider exception text, bytes, paths and key identifiers never escape.
Invalid sizes/limits, invalid keys/signatures, authentication, busy, resource,
allocation, entropy, initialization and provider failures remain distinguishable.
core::Result<T,E> adds an error-type parameter with its existing core default,
preserving earlier callers. Core lifecycle maps crypto startup failure to its
fixed internal/unavailable or cancelled error, without exposing backend details.

Diagnostic reports allocation counts and stop state, with
all_secret_memory_locked=false and persistence_available=false. It is structural
metadata, not proof of memory confidentiality. Backend exceptions are caught;
no predictable random fallback or backend downgrade exists.

Secret allocations use Botan allocate_memory/deallocate_memory plus explicit
secure_scrub_memory before release. Botan secure_vector holds sensitive temporary
work. Its locking pool is best effort and may use ordinary heap allocation.
Stack/register/compiler copies, caller buffers, core dumps, swap, hibernation,
VM snapshots, a debugger and a compromised OS remain limits. Freeing a handle
is neither identity revocation nor guaranteed erasure of every physical copy.

Failure controls exist only in a separately compiled test library. Production
targets cannot set faults or replace entropy. Linux additionally tests the actual
production provider against a failing getrandom via a linker-wrapped test binary.
No production executable links that wrapper or test implementation.

## Platform and future gates

The same pinned static Botan subset is compiled in portable and vcpkg modes.
Windows uses BCrypt, Linux getrandom, macOS CommonCrypto. Native/Backend types
stay in implementation files. CMake selects and checks OS-specific generated
source; ordinary builds require neither downloads nor Python nor a Botan DLL.
Native Windows/macOS A03 results must be recorded separately from Linux evidence.

A11-PRE still precedes persistent identity/session/key storage; A02 API discovery
does not fulfill it. AUTH-PRE still precedes privileged listeners/actions.
There is no persistent identity, key file, account, vault, wire protocol, E2EE,
forward-secrecy claim, network, GUI or AI implementation in A03.
