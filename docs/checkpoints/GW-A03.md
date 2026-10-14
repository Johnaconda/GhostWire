# GW-A03-CRYPTOGRAPHIC-PROVIDER — implementation and validation

Date: 2026-10-13. Version: 0.3.0. Status: implemented and locally Linux-validated;
independent review and owner native A03 acceptance pending. A04 is unstarted.

## Authority and preserved baseline

The owner accepted A02 and explicitly authorized A03 only. Input:
GHOSTWIRE-GW-A02-PLATFORM-ABSTRACTIONS-source(2).zip, SHA-256
`404fbea230ff612693754d8fa70c56e2a653d85e4a98fea6d80242e2243a5dd7`.
Preparation checked exact SHA, ZIP CRC, safe paths and all 125 manifest hashes
before modifying the extracted tree. All input files are retained; historical
checkpoint reports/prompts remain unchanged. The changed-file inventory records
the exact delta against that manifest, not a reconstructed baseline.

Read-only GitHub HEAD remained 713750550d6fa7bbde7200c494554d1c4f9e008f;
the owner HF02 repairs/README were already incorporated in A02. No repository
creation, reset, push or hosted workflow was performed. A02 owner Windows
Debug/Release 35/35 each, original TEMP/TMP and win10dev environment are recorded
separately in [GW-A02-HOST-ACCEPTANCE.md](GW-A02-HOST-ACCEPTANCE.md).

An untouched A02 Debug build was also revalidated: 34/34 CTest after relinking
one generated server binary that the executor had left non-executable. No A02
source change, chmod workaround or relaxed assertion was used for that baseline.

## Implemented functionality and decisions

- Botan 3.13.0 selected after current upstream maintenance/advisory/license,
  historical review, platform and alternative-library assessment. ADR-006 records
  libsodium/OpenSSL trade-offs and audit limits. BSD-2-Clause notices are included.
- One static runtime provider in portable and vcpkg modes. Bundled official signed
  source and three minimized upstream-generated generic CPU amalgamations enable
  offline builds/regeneration. CMake checks the original archive and selected
  generated files. The existing vcpkg baseline/doctest 2.4.12 pin is unchanged.
- Owned bounded OS random generation, RFC 8439 ChaCha20-Poly1305, RFC 8032 pure
  Ed25519 for public messages and SHA-256. No algorithm-string choice, custom
  primitive, KDF policy, identity format or communication protocol.
- Move-only distinct AEAD/signing keys and clearing SecretBuffer, explicit
  borrowing, shared allocation accounting, safe reset/move/provider-outliving
  ownership. No key serializer, export convenience or secret-bearing diagnostics.
- Typed enum errors, input/workspace/live-secret limits, immediate busy admission,
  application/per-call/owned cancellation and nonblocking terminal close.
- Real lifecycle-owned CryptoService in all four one-shot roles. Actual entropy
  failure aborts startup with fixed error/exit 70 and rollback. Help/version/config
  and path-only operations still return before crypto initialization or writes.
- Existing Result supports a defaulted error type for the crypto facade; existing
  core callers keep their original contract. Module edges remain acyclic:
  core -> crypto -> Botan/header-only values, core -> platform/header-only values.
- Private fault controls compile into a separate test library only. Linux also
  tests real production System_RNG failure through test-binary getrandom wrapping.

CLI flags, schema 1, exit codes, Unicode behavior, native process/stop ownership,
service rollback, resource admission and privacy logging are preserved. The
version/checkpoint and truthful bootstrap capability text are intentionally updated.
All earlier test cases remain; capability/version assertions follow that change.

See [CRYPTO_PROVIDER.md](../architecture/CRYPTO_PROVIDER.md) for complete ownership,
lifetime, threading, input/error, security, resource and platform contracts, and
[GW-A03-CHANGED-FILES.md](GW-A03-CHANGED-FILES.md) for the complete inventory.

## Actual executor and measured matrix

Ubuntu 24.04.3 x64; GCC/G++ 13.3.0; CMake 4.4.4;
Ninja 1.13.2.git.kitware.jobserver-pipe-1; Python 3.12.14;
clang-format 23.1.3. Warnings-as-errors remains enabled on project code.
Botan keeps its upstream warning set; ASan/UBSan/TSan instrument the backend too.
No native Windows, macOS, clang-tidy or hosted A03 execution is claimed.

| Actual check | Result |
|---|---|
| Accepted input SHA/CRC/paths/125 hashes | PASS |
| Untouched A02 Linux portable Debug | 34/34 CTest PASS after executor binary relink |
| A03 clean portable Debug configure/build | PASS after clean retry of empty generated object |
| A03 Debug CTest | 37/37 PASS, zero test skips |
| A03 clean portable Release configure/build/CTest | PASS, 37/37, zero test skips |
| A03 vcpkg Linux Debug configure/build/CTest | PASS, 37/37; same pinned Botan, exact doctest 2.4.12 |
| ASan/UBSan default crypto-unit attempt | Assertions PASS, LeakSanitizer runtime fatal at exit; check FAILED |
| Separate ASan/UBSan with detect_leaks=0 | 37/37 PASS after executor binary relink; no leak-validation claim |
| TSan, all five unit runners and instrumented native child | 5/5 PASS, halt_on_error=1; role CLI integration not included in this run |
| clang-format / include-boundary checks | PASS; no upstream source formatting |
| Dependency archive signature and hash | PASS; same-origin key trust limitation below |
| Offline regeneration of all three platform outputs | Byte-for-byte PASS after fixed unused prefix |
| Corrupted archive / generated source / changed version configure | All three rejected, PASS |
| Production symbol isolation / runtime linkage | No TestAccess/TestHooks in crypto archive or four apps; no shared Botan dependency |
| Upstream minimized-module suite linked to GhostWire's built Botan archive | 485,213 upstream checks PASS, 98.85 seconds; no enabled-suite failure |
| JSON, workflow YAML, Python syntax and local Markdown links | PASS |
| Fresh source-package extraction, clean Release configure/build/CTest | 37/37 PASS; all compiled inputs byte-identical to final delivery |

Per successful full Linux suite:

| Runner | Cases | Assertions | Skips |
|---|---:|---:|---:|
| core-unit | 67 | 640 | 0 |
| platform-unit | 18 | 191 | 0 |
| crypto-unit | 20 | 1449 | 0 |
| crypto-faults | 7 | 67 | 0 |
| crypto-system-entropy-failure | 2 | 22 | 0 |
| Total | 114 | 2369 | 0 |

The other 32 CTest entries are eight actual CLI contracts per role, including
Unicode. Windows expected count is 37 (its tooling check replaces Linux's OS
entropy wrapper); macOS expected count is 36. Expected counts are not outcomes.
Selected actual command output is retained in GW-A03-VALIDATION-EXCERPTS.txt.

## Tests and reproducible commands

Crypto vectors use published RFC/upstream values, not production-generated
expectations: RFC 8439 plus three upstream ChaCha20-Poly1305 block regressions,
three RFC 8032 Ed25519 examples, SHA-256 empty/abc/multiblock known answers and
upstream identity/noncanonical signature rejection. Tests modify each byte of
the small ciphertext/tag and signature vectors and cover wrong AAD/nonce/key,
truncation/extension, zero/boundary inputs and strict key/nonce/signature sizes.
Fixture provenance is in tests/fixtures/crypto/README.md.

The pinned release's upstream test objects were also built for the same generic,
no-assembly module set and explicitly linked to GhostWire's actual Debug
libghostwire_botan.a. The executable reported distribution GhostWire-A03 and
passed 485,213 upstream checks. This covers the selected dependency closure,
not excluded TLS/network/other modules; no long, memory-intensive or online test
mode was requested. It is separate from GhostWire's 114 case / 2369 assertion
counts and is not an external security audit.

Negative tests cover live-byte/handle/workspace limits, decrypt-output admission,
move/reset/outliving-provider, cleanup after partial entropy/allocation failure,
post-operation cancellation, deterministic in-flight busy/close, concurrent calls,
initialization failure, fixed error descriptions and real lifecycle bootstrap.
The wipe observer inspects bytes after secure scrub and before free, only in the
isolated test build. No production entropy source can be replaced through an API.

Commands run from the project root, with the recorded compiler/CMake/Ninja on PATH:

```bash
cmake --preset portable-debug --fresh
cmake --build --preset portable-debug --clean-first -j1
ctest --preset portable-debug --verbose
cmake --preset portable-release --fresh
cmake --build --preset portable-release -j1
ctest --preset portable-release --verbose
# Pinned VCPKG_ROOT and VCPKG_DISABLE_METRICS=1:
cmake --preset linux-debug --fresh
cmake --build --preset linux-debug -j1
ctest --preset linux-debug --verbose
cmake --preset portable-sanitize --fresh
cmake --build --preset portable-sanitize -j1
ctest --preset portable-sanitize -R '^crypto-unit$' --verbose
# Separate limited run, not a preset/CI policy change:
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 ctest --preset portable-sanitize --verbose
cmake -S . -B out/build/portable-tsan -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DGW_TEST_DEPENDENCY=vendored \
  '-DCMAKE_CXX_FLAGS=-fsanitize=thread -fno-omit-frame-pointer' \
  -DCMAKE_EXE_LINKER_FLAGS=-fsanitize=thread
cmake --build out/build/portable-tsan --target ghostwire-tests ghostwire-platform-tests \
  ghostwire-platform-fixture ghostwire-crypto-tests ghostwire-crypto-fault-tests \
  ghostwire-crypto-entropy-tests -j1
TSAN_OPTIONS=halt_on_error=1 ctest --test-dir out/build/portable-tsan -L unit --verbose
python3 tools/format.py --check
python3 tools/check_boundaries.py
python3 tools/crypto/generate_botan.py --check
python3 tools/packaging/package_source.py
```

Test child environments forward explicitly supplied sanitizer variables; no
production policy or default leak suppression changed. Negative pin tests used
temporary source copies. Symbol checks used nm -C on the production archive and
four executables; the separate fault library contained the expected test symbols.
ldd verified no shared Botan dependency. These are Linux checks, not DLL inspection.

Upstream validation used the exact bundled source distribution extracted outside
the project, configure.py with `--os=linux --cc=gcc --cpu=generic --disable-asm
--minimized-build --enable-modules=system_rng,chacha20poly1305,ed25519,sha2_32,locking_allocator
--disable-shared-library --build-targets=static,tests --amalgamation --link-method=copy
--without-documentation --build-tool=ninja`, and serial Ninja. The compiled
`build/obj/test/*.o` were linked using g++ -fstack-protector -pthread with
GhostWire's libghostwire_botan.a instead of the separately generated upstream
archive. The runner used `--data-dir=<release>/src/tests/data --test-threads=1`.
An initial wrong Ninja working directory and a removed ephemeral response-file
reference were corrected in this external harness before that successful run;
no upstream or GhostWire source change was involved.

## Failed attempts and environment limits

The first Debug test run passed 33/37: four bootstrap assertions still expected
the old text claiming no encryption. They were updated to the actual A03 capability
message; no prior CLI behavior/assertion was weakened. The final full suite passed.

The executor intermittently produced invalid build artifacts despite compiler/
linker success: a baseline server and an ASan entropy-test binary at mode 0644,
and one empty Debug platform-test object causing an undefined main link failure.
The untouched baseline first passed 26/34; ASan first passed 36/37, with the other
test unable to launch. Relinking only the affected generated binaries, or a clean
serial Debug rebuild, restored valid output and full reruns passed. No chmod,
source workaround, warning suppression or native-host claim was used.

The first provider regeneration check detected a temporary prefix embedded by
upstream configure. Generation now uses fixed /opt/ghostwire-botan without
installing there; no upstream source patch was made. Repeated generation compares
all three outputs exactly. Release source was subsequently revalidated from the
packaged tree; only generated prefix metadata changed from its first clean run.

Default LeakSanitizer reported a fatal runtime error after all 20 crypto cases /
1449 assertions passed, with its ptrace limitation hint. Earlier A02 also reported
/proc inspection failures. This does not establish absence of leaks. The separate
detect_leaks=0 ASan/UBSan pass is recorded only for address/undefined behavior.
clang-tidy is not available. Compiler warnings and formatting are not substitutes.

## Security considerations and remaining gates

Botan source SHA-256 is
`12f5a8358890bbee82edfe9d2e7769b0a610b6dd0e0698aea13d20a675d84620`.
Its detached signature verified against fingerprint
`621DAF6411E1851C4CF9A2E16211EBF1EFBADFBC`, obtained from the same official HTTPS
site. This is reproducible provenance, not independent web-of-trust certification.
The historical upstream external audit does not certify Botan 3.13 or this facade.

The caller must guarantee unique AEAD nonces per key. Ed25519 signs public bytes
only because upstream message buffering is not clearing memory. Secret allocation
locking is best effort; caller/stack/register copies, dumps, swap/hibernation,
VM snapshots and compromised endpoints remain limits. Workspace/admission limits
are logical budgets, not total RSS/CPU quotas. OS RNG/primitive calls are not
forcibly interruptible. Owners must join consumers before provider destruction.

Windows10/VS2026 A03 Debug/Release and macOS x64/ARM64 remain unexecuted here.
Windows vcpkg/doctest remains unresolved independently of the accepted portable
route and new account. Native A02 macOS, full leaks, clang-tidy, native concurrency/
entropy-failure behavior, independent crypto review and hosted A03 CI remain open.
Prior HF02 CI is not A03 evidence. See the exact owner commands and checklist in
[GW-A03-NATIVE-ACCEPTANCE.md](GW-A03-NATIVE-ACCEPTANCE.md) and BUILD.md.

A11-PRE still blocks private identity/session/key persistence. AUTH-PRE still
blocks relevant listeners and privileged remote actions. No identities, account
profiles, vault, protocol/E2EE, network, GUI or AI feature is implemented. API-only
DPAPI/Keychain discovery is not a protected store. Public vector seeds and release
public keys are test/provenance data, not operational secrets.

## Delivery and stop condition

The cumulative archive contains A00–A03 source, pinned provider source, tests,
notices, updated records and a generated SOURCE_MANIFEST.json. The archive and
sidecar checksum are external to that manifest to avoid a self-hash cycle.
Build outputs, tool caches, temporary fixtures, credentials and .git are excluded.
The signed upstream tar.xz/asc are explicitly allowlisted source dependencies.
The staged package was freshly extracted, its CRC/all source hashes checked and
a clean Release build passed 37/37. The final repack changes only documentation,
inventory and generated manifest; every compilation/build/test input was compared
byte-for-byte against that extracted tree. Final CRC, safe paths, exact manifest
membership and all source hashes are rechecked before delivery. Historical input
reports/prompts remain unchanged. The external sidecar identifies the final ZIP.

TODO/ROADMAP distinguish A02 owner acceptance, A03 implemented/local validation
and pending A03 native acceptance. [GW-A04-PROMPT.md](GW-A04-PROMPT.md) is a
proposal requiring the owner's current identity decisions and explicit further
authorization; persistent work still needs A11-PRE. Stop after A03.
