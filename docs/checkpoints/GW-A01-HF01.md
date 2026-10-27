# GW-A01-HF01 — Windows Unicode recovery

Checkpoint: **GW-A01-HF01-WINDOWS-UNICODE-RECOVERY** · Version **0.1.1** · C++20.
Implementation/validation date: **2026-10-12**. The authorized recovery is
implemented. Linux validation is recorded below; **HF01 Windows host acceptance
with original TEMP/TMP is still pending**. Stop here. A02 is not implemented or
authorized by the presence of its continuation proposal.

## Authoritative input and preserved baseline

Input: `GHOSTWIRE-GW-A01-CORE-LIFECYCLE-SERVICES-source(1).zip`.
Measured SHA-256, matching the previous A01 delivery:

```text
0c8ded6652ec0d799cfcfde0789160e3ed5b821ad5f797c1562bae12088aed5d
```

Verified ZIP CRCs, safe member paths and all 100 source hashes in the embedded
manifest before working from a separate extraction. Reviewed the repository
contracts and actual affected source, tests, presets, Windows scripts and
development records. The untouched A01 portable Debug baseline built and passed
29/29 CTest checks, 65/65 unit cases and 589/589 assertions on Linux.

The owner's Windows evidence is recorded separately in the
[host report and acceptance checklist](GW-A01-HF01-WINDOWS-ACCEPTANCE.md).
It reports A01 Debug and Release passing 29/29 checks and 64/64 unit cases with
process-local ASCII TEMP/TMP, versus three failing cases without the workaround.
The one-case Linux/Windows difference is the existing POSIX-only symlink test.
Those Windows results are owner-reported A01 evidence, not HF01 runs here.

All production `src/` and `apps/` bytes, A01 lifecycle tests, dependency pins,
vendored doctest, compiler-warning settings and build presets are retained.
The master contract, A00 records, A01 report/inventory and original A01 prompt
remain byte-for-byte unchanged. No source was reconstructed, reset or deleted.
The [changed-file inventory](GW-A01-HF01-CHANGED-FILES.md) records the exact delta.

## Diagnosis and implementation

Six core fixture conversions used `std::filesystem::path::string()` at boundaries
that require UTF-8. On Windows that narrow conversion can use the system code
page. A native ö can therefore become a lone 0xF6 byte, correctly rejected by
GhostWire's UTF-8 validation with usage exit 2. This defect is consistent with
the owner's original-TEMP failure and ASCII-TEMP recovery. Native reproduction
of the fix remains a host acceptance gate.

The existing production `path_to_utf8()` uses `generic_u8string()`, while
`path_from_utf8()` validates and constructs the native path explicitly. Windows
argv already enters through wide arguments and explicit UTF-8 conversion. No
production runtime encoding defect was found in these inspected paths. The fix
uses that existing interface in fixtures and keeps native `fs::path` values for
filesystem I/O; application validation was not weakened or changed.

| Area | Implemented behavior |
|---|---|
| Existing fixtures | Replace all six `.string()` conversions. Preserve the canonical native temporary root and create a child containing ö and spaces on every host. Keep existence checks on native paths. |
| Unicode unit coverage | Exercise profile-shaped paths, config filename/content, no-write validation, explicit path display, actual bootstrap/shutdown and private-path redaction. Add a Windows-only native UTF-16 path round trip. |
| Negative encoding coverage | Reject raw ANSI-style 0xF6, truncated UTF-8, overlong sequences and surrogate encoding in CLI/config inputs; retain usage/configuration exits 2/3, redaction and no-write behavior. |
| Real executable coverage | Add one Unicode subprocess contract for each of desktop/server/relay/cli, including native argv, config parsing, override precedence, missing-file exit 4 and invalid-config exit 3. No shell or TEMP/TMP workaround. |
| CMake/CTest discovery | Discover CTest beside the selected CMake before PATH, support `-CTestPath`, verify matching versions and invoke its absolute path from ordinary PowerShell. |
| MSVC environment | Capture `cmd /u` output using explicit UTF-16 decoding; request Hostx64/x64; apply environment changes only to the script process and preserve its original TEMP/TMP. |
| Architecture diagnostics | Reject incompatible host/target, compiler/linker paths, mismatched MSVC tool directories and x86/ARM LIB entries; require MSVC, UCRT and Windows SDK libraries. Pass resolved compiler/linker/Ninja paths to CMake. |
| Recovery tooling | `-Fresh` uses CMake's fresh configure for `configure`/`all`, with actionable diagnostics for a mixed developer shell. No permanent PATH or execution-policy changes. |
| Windows tooling regression | Register dependency-free PowerShell checks for tool discovery, architecture rejection and real `cmd.exe` Unicode environment transfer/failure, including spaces and shell metacharacters. |

The runtime target DAG, ownership, lifecycle, cancellation and resource contracts
are unchanged; the platform header only clarifies its existing encoding contract.
These PowerShell helpers are build tooling, not A02's proposed process API.
Both portable/vendored and standard vcpkg modes remain available, with no silent
fallback. Version/checkpoint output advances to 0.1.1/HF01; CLI flags, config
schema and exit codes otherwise remain unchanged.

Primary references reviewed for the recovery:

- [Microsoft filesystem::path reference](https://learn.microsoft.com/en-us/cpp/standard-library/path-class?view=msvc-170)
  and [MSVC filesystem implementation](https://github.com/microsoft/STL/blob/main/stl/inc/filesystem): native/narrow/UTF-8 conversion boundaries.
- [MSVC command-line environment](https://learn.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=msvc-170): compiler, SDK, host/target environment and developer shell setup.
- [cmd /u](https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/cmd)
  and [ProcessStartInfo.StandardOutputEncoding](https://learn.microsoft.com/en-us/dotnet/api/system.diagnostics.processstartinfo.standardoutputencoding?view=net-10.0): explicit Unicode transport for captured environment text.

## Actual validation

Executor: Linux x64, Ubuntu 24.04.3 LTS, GCC 13.3.0, CMake 4.4.4,
Ninja 1.13.2.git.kitware.jobserver-pipe-1, Python 3.12.14,
clang-format 23.1.3. vcpkg baseline remains
`9e593bb18ea69cc5095e012465dcd675a822ed0d`; doctest remains exactly 2.4.12.

| Configuration/check | Actual result |
|---|---|
| Untouched A01 portable-debug | Configure/build pass; 29/29 CTest; 65 cases / 589 assertions |
| HF01 portable-debug | Configure/build all four executables pass; 33/33 CTest; 67 cases / 640 assertions |
| HF01 portable-release | Configure/build all four executables pass; 33/33 CTest; 67 cases / 640 assertions |
| HF01 linux-debug, vcpkg | Configure/dependency/build pass; 33/33 CTest; Windows vcpkg is not inferred from this |
| HF01 portable-sanitize, ASan/UBSan | Configure/build pass; 33/33 CTest with `ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1`; no reported address/undefined-behavior finding |
| Unit executable with `LC_ALL=C` | 67 cases / 640 assertions pass; UTF-8 path boundaries do not depend on the Linux locale |
| Formatting / architecture boundaries | Both project checks pass; warning policy and boundary rules unchanged |
| PowerShell grammar | All three scripts parse without error/missing nodes using Tree-sitter 0.26.0 and PowerShell grammar 0.26.4; not native PowerShell validation |
| Fresh packaged-source portable-release | Clean extraction configured/built all four executables; 33/33 CTest; 67 cases / 640 assertions |
| Source syntax / local documentation links | 3 JSON and 6 YAML files parsed; 38 Markdown files inspected and all 40 local links resolve |
| HF01 Windows 10 x64 / VS2026 | Not executed here; original-TEMP Debug/Release acceptance required |
| Windows vcpkg / macOS / clang-tidy / hosted CI | Unresolved or unexecuted as detailed below |

CTest has 33 Linux registrations: one unit executable and eight process contracts
for each of four roles. Windows adds `windows-tooling`, for an **expected 34**.
The Windows unit suite is **expected to have 67 cases**, including its native
UTF-16 case and excluding the prior POSIX-only case. The PowerShell script has
18 checks. These Windows counts describe implemented coverage, not executed
results. All original A00/A01 tests are preserved.

Reproduce Linux checks from the project root, with CMake/Ninja/compiler on PATH
and a bootstrapped pinned VCPKG_ROOT for the vcpkg preset:

```bash
export VCPKG_DISABLE_METRICS=1
bash tools/dev.sh all portable-debug
out/build/portable-debug/bin/ghostwire-tests --no-colors
bash tools/dev.sh all portable-release
out/build/portable-release/bin/ghostwire-tests --no-colors
bash tools/dev.sh all linux-debug
cmake --preset portable-sanitize
cmake --build --preset portable-sanitize --parallel 2
# Deliberately limited sanitizer evidence; this is not a passing leak check:
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 ctest --preset portable-sanitize
LC_ALL=C out/build/portable-debug/bin/ghostwire-tests --no-colors
python3 tools/format.py --check
python3 tools/check_boundaries.py
```

Full LeakSanitizer was blocked by task inspection in this environment during A01;
it was not rerun for HF01 and remains unverified. No leak-disable setting was
added to presets or CI. A01's ThreadSanitizer result is preserved historically;
TSan was not rerun for this fixture/build-tooling recovery. The local grammar
parser is only a validation tool and is not a project dependency or deliverable.

## Windows acceptance and unresolved gates

Use the [full checklist](GW-A01-HF01-WINDOWS-ACCEPTANCE.md) and updated
[BUILD.md](../../BUILD.md). From a fresh ordinary Windows PowerShell session with
the original non-ASCII profile TEMP/TMP, run the portable acceptance route:

```powershell
.\tools\windows\dev-windows.ps1 -Action all -Configuration Debug -OfflineTests -Fresh
.\tools\windows\dev-windows.ps1 -Action all -Configuration Release -OfflineTests -Fresh
```

Record archive checksum, native tool versions, complete CTest/unit results and
whether the original environment remained intact. Do not switch to an ASCII
temp root or change the system code page to make these tests pass. The script's
native PowerShell/MSVC behavior and new Windows-only cases require actual host
execution; a grammar check and Linux tests cannot prove them.

The owner's standard Windows vcpkg/doctest 2.4.12 configure failure remains
**unresolved**. No raw diagnostic log establishing its cause was supplied with
the report. Preserve the vcpkg route and capture its exact configure/buildtree
logs separately; do not attribute it to the recovered x86/x64 linker mismatch
or claim `-Fresh` fixes it. Linux vcpkg success and Windows vendored success do
not close this gate. macOS x64/ARM64, full LSan, clang-tidy and hosted CI remain
open. No remote repository or hosted run was created.

## Security and limits

- Strict UTF-8, no-write checks, existing exit codes and privacy-aware diagnostics
  remain enforced. Invalid path text and test markers must not appear in ordinary
  application logs; `--print-paths` retains its explicit disclosure contract.
- Build scripts do not print the captured environment or persist machine/user
  changes. Diagnostics intentionally show selected development tool paths. Test
  files contain synthetic configuration only and clean up their owned roots.
- x64 diagnostics catch the observed environment mistakes; they are not a binary
  machine-type audit of every library, nor a replacement for a clean native link.
- No listener, GUI, discovery, messaging, crypto, private persistence, model code,
  inference/training, mandatory cloud dependency or telemetry was introduced.
  A11-PRE and AUTH-PRE are unchanged. Temp fixtures contain no keys/chat content;
  no backup, cache or private-storage policy changed.
- A01's accepted cooperative shutdown and declared-budget limitations remain:
  no hard preemption, RSS enforcement or GPU allocation guarantee was added.
  Existing lifetime and service ownership contracts are unchanged.

## Source verification and continuation

The cumulative package contains 106 source files plus the regenerated
SOURCE_MANIFEST.json: 6 added, 19 modified, 0 deleted and 81 unchanged compared
with the verified A01 input. The changed-file inventory matches the exact source
hash comparison. All archive CRCs and manifest hashes were verified.

A candidate source ZIP was extracted into a new directory with no build cache.
`bash tools/dev.sh all portable-release` configured and built all four executables
and passed 33/33 CTest checks, including 67/67 unit cases and 640/640 assertions.
Only documentation changed afterward. All non-Markdown source, build, test,
script and dependency bytes in the final ZIP were compared with the tested
extraction. The final archive was regenerated and verified again against its
manifest and the authoritative input file set. JSON/YAML and local links pass.

The delivery is
`GHOSTWIRE-GW-A01-HF01-WINDOWS-UNICODE-RECOVERY-source.zip`. The external SHA-256
is supplied with the delivery and emitted by the packager as a `.zip.sha256`
sidecar; it is not recursively embedded inside the ZIP. The archive contains
the entire cumulative source, including historical records. Build caches,
binaries, validation tool installs, credentials and temporary files are excluded.

TODO, roadmap, changelog, completed records and validation policy separate
implementation, measured results and pending owner acceptance. The
[A02 continuation prompt](GW-A02-PROMPT.md) names HF01 as its input and carries
forward all open gates; it requires original-TEMP Windows acceptance and a new
explicit authorization. Future proposals remain in FUTURE_FEATURES.md. Stop
after HF01.
