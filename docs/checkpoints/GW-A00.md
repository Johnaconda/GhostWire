# GW-A00-FOUNDATION-BOOTSTRAP — implementation and validation report

Date: 2026-10-11. Source version: 0.0.1. Clean-room cumulative foundation.
Status: implemented and validated on the available Linux host; Windows/macOS
and user acceptance pending. A01 has not been started. No remote was created.

## Delivered behavior

Four distinct executable composition roots share reusable C++20 core/platform
libraries. Each has help/version/config checking, explicit config/root options,
role-scoped paths, meaningful exit codes and honest bootstrap-only output.

Core includes Error/Result, numeric version, bounded strict nonsecret config,
typed redacted JSON logging, paths, explicit lifecycle and std cancellation.
Platform implements OS directory discovery, UTF-8 argument/path conversion,
bounded config reads, collision/symlink checks and coarse local capabilities.
No GUI, listeners, live networking, relay forwarding, identities, cryptography,
vault, messaging, media, plugins or AI provider calls exist in A00.

Build/tooling includes single-config Ninja presets, Debug/Release separation,
pinned vcpkg, verified vendored test dependency, doctest/CTest, platform CI,
Windows tool discovery/overrides, POSIX commands and deterministic packaging.
The shared core is a common **static** library, not a stable exported DLL ABI.

## Architecture decisions and security review

- Preserve A00–A25 IDs; gate persistent identity/messages on A11-PRE and require
  base authorization before network/community/AI exposure.
- Keep C++20 now; evaluate a narrow Rust FFI only if a reviewed library warrants
  it. No custom crypto/protocol to avoid a toolchain or license decision.
- MIT selected for project code; separate upstream test notice retained.
- Protocols, libraries and native UI are documented candidates, not functionality.
- Logging redacts by typed field/category, not speculative secret-recognition regex.
- Empty bootstrap directories are not advertised as a vault or secure filesystem.
- Minimum privacy modes parse now; fail-closed route enforcement belongs to A06.
- Updated the development pin to vcpkg 2026.07.29 after checking VS2026/v145
  support history. Explicit/incoming VCPKG_ROOT survives VsDevCmd initialization.

The dependency graph, eight-field future header contracts, boundary map, threat
model and security invariant/test mapping are included under docs/architecture
and docs/security. Primary sources and decisions are recorded there.

## Actual environment

Ubuntu 24.04.3 LTS x86_64; GCC 13.3.0; CMake 4.4.4;
Ninja 1.13.2.git.kitware.jobserver-pipe-1; clang-format 23.1.3;
Python 3.12. vcpkg tool 2026-07-27-98d7cb0cf1f4686a3e43aa5672b6230c1d56bce8,
checkout/baseline 9e593bb18ea69cc5095e012465dcd675a822ed0d (2026.07.29).
Doctest 2.4.12; build helpers vcpkg-cmake 2024-04-23 and
vcpkg-cmake-config 2026-07-21. Tools were available or installed locally for
validation; they are not bundled in the source archive.

## Executed validation

| Check | Result | Scope |
|---|---|---|
| vcpkg manifest install at final pin | PASS | Exact doctest and baseline build helpers installed |
| linux-debug configure/build | PASS | All four apps, both libraries, test executable; warnings as errors |
| linux-debug CTest | PASS: 29/29 | 1 unit runner + 28 real subprocess tests |
| linux-release configure/build | PASS | All four apps and test executable; warnings as errors |
| linux-release CTest | PASS: 29/29 | Same behavioral matrix |
| Unit runner within each suite | PASS: 26 cases, 195 assertions | No skipped unit cases on Linux |
| portable-debug (vendored doctest) | PASS: build + 29/29 CTest | Exact bundled dependency route |
| portable-sanitize compile/link | PASS | GCC AddressSanitizer + UndefinedBehaviorSanitizer |
| ASan/UBSan execution with detect_leaks=0 | PASS: 29/29 CTest; 26/195 unit assertions | UBSan halt_on_error=1; leak detection separately blocked |
| Default sanitizer execution including LeakSanitizer | BLOCKED by host | /proc process/task inspection unavailable; details below |
| clang-format check | PASS | Project C++ files; upstream header excluded |
| Module-boundary check | PASS | Platform include allowlist; no UI import into core/platform |
| CMake install/staged CLI version | PASS | Linux Release stage and actual executable invocation |
| Source archive CRC + SHA-256 inventory | PASS | Package script verifies every included source file |
| Clean extracted source build | PASS | Fresh portable-release configure/build; 29/29 CTest and 26 cases / 195 assertions |
| Windows/MSVC and PowerShell execution | UNVERIFIED | No Windows host available |
| macOS/AppleClang execution | UNVERIFIED | No macOS host available |
| GitHub Actions | CONFIGURED, NOT RUN | No authorized remote repository created/pushed |
| clang-tidy | NOT RUN | Tool unavailable in this environment |
| Crypto/protocol fuzzing and independent audit | NOT APPLICABLE to A00 runtime; FUTURE GATES | No such subsystem implemented |

Representative commands actually run (tool locations added to the local PATH):

```bash
cmake --fresh --preset linux-debug
cmake --build --preset linux-debug --clean-first --parallel 2
ctest --preset linux-debug --output-on-failure
cmake --preset linux-release
cmake --build --preset linux-release --parallel 4
ctest --preset linux-release --output-on-failure
cmake --preset portable-debug
cmake --build --preset portable-debug --parallel 4
ctest --preset portable-debug --output-on-failure
bash tools/dev.sh all portable-sanitize
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 ctest --preset portable-sanitize --output-on-failure
python3 tools/format.py --check
python3 tools/check_boundaries.py
cmake --install out/build/linux-release --prefix out/stage
out/stage/bin/ghostwire-cli --version
python3 tools/packaging/package_source.py
```

### LeakSanitizer limitation

The original sanitizer unit process reported 26/26 cases and 195/195 assertions
successful, then LeakSanitizer failed while opening `/proc/<pid>/task` with its
fatal runtime diagnostic about tracing/task inspection. Child executable tests
also exited during that sanitizer teardown. The suite is therefore not recorded
as passing with leak detection. Rerunning ASan/UBSan with only leak detection
disabled passed. No source or CI default suppresses leaks; an unrestricted host
must run the original sanitizer preset to close GW-A00-LEAK.

## Coverage and limitations

Unit cases include malformed/overflowing version and configuration, duplicate
keys and unsupported schema, 64 KiB read limits, embedded controls, valid/invalid
UTF-8, path role isolation and no-write checks, file/symlink collision behavior,
lifecycle failure/RAII/repeated shutdown/cancellation, redaction despite wrong
public classification, log injection/clipping/concurrency, output failure,
config precedence and exact CLI error codes. Integration runs seven real child
process scenarios per target: help, version, check-config, unknown option,
invalid config, missing config and successful initialize/shutdown.

A00 does not run forever or install a service; default startup ends normally.
OS signal/service shutdown bridging and dependency-managed services are A01/A02
work. Existing filesystem permissions are not hardened; empty directories can
remain after partial failure; symlink checks are not a defense against a local
racing attacker. Log public metadata still needs disciplined caller review.
No unit test implies E2EE, anonymity, secure erasure or post-compromise recovery.

Windows source received static review for native header order, drive-root path
handling, Unicode argv, VS tool discovery and execution-policy behavior. This is
not native compilation. Follow BUILD.md on the user's Windows 10 x64 / VS2026
host, then supply actual results. macOS paths and AppleClang also need host tests.
The configured Windows CI uses windows-2022, which is separate from that host.

## Packaging and continuation

The source package excludes .git, build outputs, local tooling, caches, raw test
logs, credentials and runtime state. The embedded SOURCE_MANIFEST.json hashes
all shipped source files; the delivered ZIP has an external SHA-256 sidecar and
response checksum. Rebuilding byte-identical binaries is not claimed: compiler,
SDK and runner versions still matter. The ZIP is deterministic for identical
source bytes with the same packager/runtime compression implementation.

Changed-file inventory: GW-A00-CHANGED-FILES.md. Detailed TODO and completed
implementation records are separate from user acceptance. Next proposal:
GW-A01-CORE-LIFECYCLE-SERVICES, in GW-A01-PROMPT.md. Stop at A00.

### Build-environment observation

One incremental rebuild left two generated Linux executables without their
execute bit, causing permission-denied process tests. A clean-first rebuild
restored normal linker output permissions and passed 29/29; no chmod workaround
or source weakening was used. The exact transient filesystem cause is unknown.
The final clean extracted build is recorded separately below.

### Clean package evidence

Extracted the generated ZIP into a new directory with no CMake cache or installed
vcpkg tree, then ran `bash tools/dev.sh all portable-release` and the unit runner.
Configure, compile/link and all 29 CTest entries passed; doctest reported 26 cases
and 195 assertions with none skipped. Executable permissions were correct in
that clean build. Final report bookkeeping was added afterward; all non-Markdown
source/build/dependency inputs were byte-compared against the tested extraction.
The final ZIP was regenerated and CRC/inventory verification repeated. JSON/YAML
syntax, local documentation links, example config and native Linux default-path
resolution were also checked successfully.
