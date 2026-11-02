# GW-A05-P04-UPSTREAM-INTEROP — updated 2026-10-31

**Current source revision: 6 — bounded fuzz properties and corpus replay.**
The existing optional fuzz entrypoint now checks canonical round trips and
admission release. Two bounded Clang ASan/UBSan campaigns and independent corpus
replay pass. Production source matches revision 4 and CI matches revision 5.
Leak-enabled and hosted native execution remain pending; owner revision 2 MSVC
results remain evidence for that earlier source only.

Real upstream interoperability is now evidenced on Linux GCC and Windows MSVC. This is an
A05 security dependency checkpoint, not the 20% milestone or network approval.
Runtime remains 0.4.1. Native macOS evidence and independent parser review remain open.

## Revision 6 — bounded fuzz properties, 2026-10-31

Input revision 5 ZIP SHA-256:
`9203ef476093906cc21e5fb43eaa68b1e52078f60ad1fb7b2f0605d0d8dcc540`.
All 311 source hashes matched before work and before integration. One coordinator
worked in an isolated detached worktree; the two concurrent fuzz processes were
test processes, not independent coding agents or an external review.

The existing fuzz entrypoint previously called decode and discarded its result.
`GW-A05-FUZZ-INVARIANTS` adds three properties: every accepted frame re-encodes to
the exact original bytes; decoding that encoding preserves the decoded value;
and admission is released after every operation, including rejected input.
Failures abort even under NDEBUG. Only `tests/fuzz/protocol_decode_fuzz.cpp` changes
executable test logic; no production source, normal test target, CMake, preset,
CI, dependency pin or wire-format change is made. No new harness is delivered.

GCC 13.3.0 strict-warning compilation passes. The target and production codec
compile together with Clang 18.1.3, C++20, O1, NDEBUG, warnings-as-errors,
libFuzzer, ASan and UBSan, with sanitizer recovery disabled. The private compiler
was extracted from seven Ubuntu 24.04 packages after verifying the Ubuntu-signed
index and package SHA-256 hashes; no system package or project dependency was
changed, and the compiler files are excluded from the source ZIP.

The unchanged 4,023-case corpus plus five framing boundaries produce 3,412
distinct seed files, including an empty input. Both campaigns use
`-max_total_time=120`, `-max_len=65541`, `-timeout=5`, `-rss_limit_mb=512`,
`-use_value_profile=1`, `-reload=0` and separate output/artifact directories.
The largest permitted mutation is one byte above the maximum framed length.

| Campaign seed | Executions | Reported elapsed | Peak process RSS | Result |
|---|---:|---:|---:|---|
| 2690580486 | 18,969,914 | 121 seconds | 392 MiB | Exit 0; no failure artifact |
| 2690580487 | 18,544,083 | 121 seconds | 301 MiB | Exit 0; no failure artifact |

Total **37,513,997 executions**, not distinct test cases or a coverage percentage.
RSS includes the fuzzer/sanitizer runtime. Temporary negative controls outside
the source tree deliberately corrupt the encoder prefix and omit lease release.
Both trigger the expected fuzz failure (exit 77) under NDEBUG. Neither control is
part of the delivered source.

`-merge=1` minimizes the combined inputs to 1,937 files. Replaying these with the
18 frozen controls gives 1,955 cases: 1,954 fit the existing probes' input bounds;
one 65,541-byte input is separately confirmed rejected by both probe transports
and the reference. Within the codec comparison, GhostWire/reference accept 171
and reject 1,783; TinyCBOR accepts 216 and rejects 1,738. All 45 upstream-only
acceptances are schema-stricter GhostWire rejects. Mandatory canonical/generic
controls remain present. Minimized corpus SHA-256 inventory fingerprint:
`981f173967784b6c34edd11c577cce5057c813f59da9e1fe1a1417b9d7486fc8`
(sorted per-input SHA-256 lines with a final newline).

The probes are freshly built with Clang Debug ASan/UBSan, including upstream C
instrumentation. Both existing TinyCBOR CTests pass **2/2**, with the unchanged
4,023-case results (GhostWire 918/3,105; upstream 991/3,032; 73 expected stricter
rejects) and oversized stdin rejection/resynchronization.

After integration, boundary and registry validation pass, along with 13 registry,
seven effort-forecast and three Windows-documentation tests. The prior revision 5
full Debug 54/54 result is retained as historical evidence, not rerun for this
standalone fuzz-target change.

Default seeded replay completes 3,412 executions but then exits 1 because
LeakSanitizer cannot inspect `/proc/.../task` in this host. It is not counted as a
passing default-sanitizer run. The bounded campaigns, minimization and replay
explicitly use `ASAN_OPTIONS=detect_leaks=0`, `UBSAN_OPTIONS=halt_on_error=1`, with
libFuzzer `-detect_leaks=0`. No leak suppression is added to source or CI.
Reproduction commands are in [BUILD.md](../../BUILD.md); they retain default leak
checking and explain the recorded local limitation. The LLVM
[libFuzzer documentation](https://llvm.org/docs/LibFuzzer.html) describes the
instrumentation, seeded mutation and minimization flags used here.

Engineering confidence for the test-target change is 0.95, based on strict builds,
negative controls and actual property/corpus execution. This is not security
certification. No new production defect was found in these bounded runs. Native
Windows/macOS, leak-enabled and cross-platform fuzz validation, independent parser
review and the owner's A05 disposition remain open. No remote write or immediate
Windows rerun is requested. The provisional accepted floor remains 11/110 (10%);
20% is not reached. Existing records and the cumulative 311-file archive are
updated without shipping generated corpora, binaries or additional reports.

## Revision 5 — native CI coverage, 2026-10-31

Input revision 4 ZIP SHA-256:
`3a891086a363b5a4505bad3d6840126c6e3d4f973214da47f2fe5e2ff10913f0`.
All 311 source hashes matched before integration. One coordinator used an
isolated detached worktree; no separate agent or external reviewer took part.

The existing platform workflows never selected `GW_A05_ENABLE_TINYCBOR_INTEROP`,
whose default is OFF. Consequently, they could pass without compiling the actual
upstream parser or running its comparison and bounds tests. Revision 5 sets the
option explicitly in Linux Debug/Release, Linux sanitizer, macOS Intel/ARM
Debug/Release and Windows Debug/Release jobs. All existing tests remain selected;
verbose CTest output exposes the successful corpus counts as well as failures.
Runner matrices, job timeouts, action and dependency pins, triggers and read-only
workflow permissions are unchanged. No new job, action or test harness is added.

Linux/macOS use the same preset configure, all-target build and full test sequence
as the earlier helper, with the explicit option and verbose output. The Windows
helper adds `-TinyCborInterop` for configure/all, routes the option through its
checked configure call and adds verbose output for `all`. Its existing x64 MSVC,
matching CMake/CTest, failure handling and vcpkg discovery remain intact. Normal
fresh local builds still default to interop OFF; the option persists in the
selected CMake cache until reconfigured. Upstream source remains bundled,
pin-verified, unmodified and test-only. No configure-time download is introduced.

| Revision 5 validation | Actual result |
|---|---|
| Workflow structure and change scope | Three YAML files parse; same triggers, permissions, job definitions, matrices, runner selections and prerequisite steps; all four job definitions opt into interop |
| Shell syntax | All existing Bash run blocks pass `bash -n`; PowerShell reviewed but not executed, with no PowerShell runtime available locally |
| Fresh portable GCC Debug, interop ON | Configure PASS; all-target build **79/79 steps**; full verbose CTest **54/54 PASS** |
| Actual upstream comparison | **4,023 cases PASS**; GhostWire/reference 918 accepted / 3,105 rejected; TinyCBOR 991 / 3,032; 73 expected schema-only rejects |
| Existing oversized stdin tests | Rejection and resynchronization PASS |
| Final integrated metadata and boundaries | Boundary and registry validators PASS; 13 registry, seven effort-forecast and three Windows-documentation tests PASS |
| Hosted vcpkg jobs, Windows/macOS and default hosted sanitizer execution | **NOT RUN**; the local execution uses the bundled portable test dependency |

The fresh execution used Linux x86_64, GCC 13.3.0, CMake 4.4.4,
Ninja 1.13.2.git.kitware.jobserver-pipe-1 and Python 3.12.14:

```sh
cmake --preset portable-debug -DGW_A05_ENABLE_TINYCBOR_INTEROP=ON
cmake --build --preset portable-debug --parallel 4
ctest --preset portable-debug --output-on-failure --verbose
```

The four implementation/configuration edits are `.github/workflows/linux.yml`,
`macos.yml`, `windows.yml` and `tools/windows/dev-windows.ps1`. Existing build/status
guides, this report, module/worker records and source packaging metadata are
synchronized. No production source, test case, CMake setting definition, preset,
dependency or wire format changes relative to revision 4. The cumulative archive
retains 311 source files plus its manifest; no new report or harness is delivered.

Engineering confidence for the narrow CI wiring is 0.93, based on inspected
command flow, preserved configuration scope and the fresh Linux build/test run.
This is qualitative implementation confidence, not proof that hosted jobs pass.
No new Clang/libFuzzer run was possible with the available toolchain; sustained
fuzzing and unrestricted leak validation remain open. No sanitizer suppression
is added to CI, and revision 4's scoped sanitizer evidence stays source-specific.

The next native evidence must come from the owner's later published source or
the next agreed acceptance checkpoint. No push, PR, dispatch or other remote
write occurred. No immediate Windows rerun is requested. Independent parser
review and the owner's A05 decision remain required; AUTH-PRE composition and
A06 stay gated. The provisional accepted floor stays 11/110 (10%); 20% is not
reached, and the official project-completion field remains unset.

## Revision 4 — focused internal review, 2026-10-30

Input revision 3 ZIP SHA-256:
`ce2f4697225bc6d619692d9f570a7c4a49d72a12d8763b79ed35bb86ad97e79e`.
All 311 source hashes matched before work. The coordinator reviewed the public
codec header, complete codec implementation, existing unit/fault tests, schema,
test plan and comparison boundary. A detached worktree received the verified
cumulative source; three reviewed source/test files were integrated only after
isolated validation. No separate agent or independent external auditor took part.

| Reviewed area | Finding / disposition |
|---|---|
| Prefix and cursor arithmetic | Size is checked before prefix indexing; declared size is bounded and must exactly match remaining input. Token widths and byte spans are checked before indexing/copying. No additional defect confirmed. |
| Canonical forms and schema | Preferred integer/length widths, strictly increasing one-byte map keys, closed kinds/fields and forbidden types remain enforced. Grammar depth is fixed, with bounded version/capability loops and no recursive skipping. No additional defect confirmed. |
| Resource and failure ownership | Caller limits tighten hard ceilings; leases release on early returns and allocation exceptions. Copies are bounded; stop/deadline is checked before publication. Existing fault/lease tests pass. |
| Encoder caller state | **Fixed GW-A05-ENCODE-STATE:** a public variant can be valueless after failed construction; the old encoder then dereferenced a null `PublicDeviceSet` pointer during size preflight. |
| Trust boundary | Public DeviceSet bytes remain opaque to the codec; A04 verification, caller-selected authority and later possession/authorization remain separate. Routing and version fields grant no authority. |

The finding concerns a caller-created in-memory envelope after an exception;
no network-triggered exploit or new network path is established. The C++20
[variant modifier contract](https://timsong-cpp.github.io/cppwp/n4861/variant.mod)
permits a valueless result after failed `emplace`. The original source, compiled
with GCC ASan/UBSan, reached `valueless=1` and UBSan stopped at
`wire_codec.cpp:306` with null-member access (exit 1). After the guard, the same
state returns `malformed_frame`, then a valid encode succeeds using the same
admission slot. The input state is not repaired or mutated by the codec.

The guard follows the existing busy/cancel/deadline checks and precedes every
variant-alternative dereference and output allocation. The public header records
the error contract. One case extends `protocol-unit` using a throwing conversion
at caller-side construction, without replacing global `operator new`. It tests
both standard-library outcomes: rejection if valueless, or preserved valid bytes
if the prior value survives, followed by successful reuse of the same slot.
Canonical requirements were checked against
[RFC 8949 section 4.2.1](https://www.rfc-editor.org/rfc/rfc8949.html#section-4.2.1);
no wire format, decoder rule or dependency change was needed.

| Revision 4 validation | Actual result |
|---|---|
| Isolated GCC strict-warning protocol suite | 14/14 cases, 2,494 assertions PASS |
| Before/after valueless-body reproduction | Original UBSan exit 1; corrected ASan/UBSan rejection and recovery PASS |
| CMake GCC Release, all targets, interop ON | Incremental build succeeds; full CTest **54/54 PASS** |
| CMake GCC Debug, five selected protocol checks | **5/5 PASS**: unit, both existing allocation fixtures, interop and stdin bounds; unit suite 14/14 cases, 2,494 assertions |
| GCC ASan/UBSan, existing allocation fixture, actual upstream interop and stdin bounds | **3/3 PASS**, `ASAN_OPTIONS=detect_leaks=0`, `UBSAN_OPTIONS=halt_on_error=1` |
| Existing seeded interop corpus | 4,023 cases; GhostWire 918 accepted / 3,105 rejected; upstream 991 / 3,032; same 73 expected schema-only differences |
| Final metadata checks | Registry validation plus 13 registry, seven effort-forecast and three Windows-documentation tests PASS |
| New native Windows/macOS, independent external review | NOT RUN; prior Windows revision 2 evidence remains below |

Environment remains Linux x86_64, GCC 13.3.0, CMake 4.4.4 and Python 3.12.14.
Ninja reports `1.13.2.git.kitware.jobserver-pipe-1`.
Initial CMake invocations failed before configure because the persisted tool
binary was truncated; restoring the same CMake package and Ninja 1.13.2 resolved
the tool failure. No repository build-setting change was made for it. This pass
does not claim unrestricted leaks or sustained coverage-guided fuzzing; the
existing sanitizer limitation remains, and no Clang/libFuzzer run was performed.

Integrated code changes are limited to `src/protocol/wire_codec.cpp`, its public
header and `tests/unit/protocol_tests.cpp`. Existing authority/status documents,
ADR-010, the protocol test plan, module/worker records and this consolidated report
are synchronized; the packager updates revision/date metadata. There is no new
delivered test harness, CMake target, upstream patch, network capability or private
persistence. The cumulative archive still contains 311 source files plus its
generated manifest; pins, source hashes and SHA-256 sidecar are reverified.

Engineering confidence for the narrow guard is 0.96, based on the reproduced
failure, recovery regression and unchanged interoperability results; this is
qualitative implementation confidence, not project completion or security proof.
The current offline retain recommendation stands. Independent external review,
macOS evidence and the owner's parser decision remain open. Native confirmation
of the guard is deferred to the next agreed checkpoint; no immediate Windows
rerun is requested. A05 is still partial and the 20% milestone is not reached.

Reproduction uses the existing CMake build directories. Release was configured
with `-DCMAKE_BUILD_TYPE=Release -DGW_A05_ENABLE_TINYCBOR_INTEROP=ON`, built with
`cmake --build out/build/default-release --parallel 2`, then checked with
`ctest --test-dir out/build/default-release --output-on-failure`. Debug selected
`^protocol-(unit|allocation-fault|global-allocation-linux|tinycbor-(interop|stdin-bounds))$`;
the sanitizer build selected `^protocol-(allocation-fault|tinycbor-(interop|stdin-bounds))$`
with the sanitizer environment above. The existing 15,000-input unit mutation
corpus is included in `protocol-unit`.

For the **next agreed Windows checkpoint**, configure as in the retained loop
below, but build `ghostwire-protocol-tests` and `ghostwire-protocol-allocation-fault`
alongside both probes, then run:

```powershell
cmake --build $build --target ghostwire-protocol-tests ghostwire-protocol-allocation-fault ghostwire-protocol-probe ghostwire-tinycbor-probe --parallel 2
if ($LASTEXITCODE) { throw "$cfg build failed" }
ctest --test-dir $build -R '^protocol-(unit|allocation-fault|tinycbor-(interop|stdin-bounds))$' --output-on-failure -V
if ($LASTEXITCODE) { throw "$cfg tests failed" }
```

Expected at that future check: four selected tests per configuration. These
extended native commands have not yet been run by the owner; the narrower loop
in the historical Windows section covers revision 2 only.

## Revision 1 source continuity

- Authoritative input: HF19 clean cumulative ZIP, SHA-256
  `7c94517caed47ef4bbd7df634443a67d56d07429484fec2bf1f67ac213997359`;
  all 298 manifest entries, member paths and the exact file set verified.
- GitHub main and a fresh read-only clone both resolve to
  `89af5eebd1ac796d80b570651b21feef5d6122ae`. Against its 296 tracked blobs,
  HF19 has nine modified files and two added policy source files; nothing missing.
  Its generated manifest is additional. The nine modifications are CMakeLists,
  README, ROADMAP, current-development-map, HF09 report, module registry, worker
  queue, source packager and the macOS importer fixture. No older main overwrite.
- This pass preserves HF10/HF11/HF19 and all historical acceptance records.
  No GitHub mutation or independently spawned coding worker occurred.

## Revision 1 changes

- Include the 12 exact TinyCBOR v7.0 files from immutable commit
  `6442e749ca811e24afad1551338a45c75e09808f`, acquired through the existing explicit
  fetch utility and verified by the existing importer. All length/SHA-256 pins
  pass; upstream C sources and MIT license are unchanged.
- Fix the existing packager: it previously rejected imported `cborerrorstrings.c`.
  Admit the exact verified pinned paths, without allowing arbitrary C/binary files.
- Set C++20 explicitly on the optional probe and propagate sanitizer instrumentation
  to the upstream C library. Treat unmodified upstream include paths as SYSTEM,
  consistent with vendor separation; project /WX and -Werror remain enabled.
  GCC sanitizer compilation otherwise rejects an unused inline upstream boolean
  encoder's integer conversion. No vendor header modification or global warning
  suppression was used.
- Update current status, registries, notices and ADR-010's bounded retain/replace
  recommendation. No new wrapper, fixture or comparison algorithm was introduced.
  Production `src/`, `include/`, `apps/`, wire bytes and runtime linkage are unchanged
  from HF19. TinyCBOR remains opt-in and test-only.

## Revision 1 observed validation

Environment: Linux x86_64, GCC 13.3.0, CMake 4.4.4,
Ninja 1.13.2.git.kitware.jobserver-pipe-1,
Python 3.12.14. This is native Linux execution, not Windows/macOS acceptance.

| Check | Actual result |
|---|---|
| Pinned upstream import | PASS, 12/12 lengths and SHA-256 hashes |
| Opt-in GCC Debug | Both native probes build; interop and stdin bounds 2/2 PASS |
| Opt-in GCC Release | Both native probes build; interop and stdin bounds 2/2 PASS |
| ASan/UBSan, upstream C and wrapper instrumented | 2/2 PASS with `ASAN_OPTIONS=detect_leaks=0`, `UBSAN_OPTIONS=halt_on_error=1` |
| Default sanitizer attempt | FAILED at LeakSanitizer teardown; host denies `/proc/.../task` inspection; leak checking remains unverified |
| Default GCC Release, interop OFF | All targets build, including the HF19 policy library; existing CTest 52/52 PASS |
| Boundaries and registry | PASS; 13 registry, seven effort-forecast and three Windows-doc checks pass after status updates |
| Cumulative packaging / extracted-source check | PASS, 311/311 source hashes and 12/12 upstream pins; fresh extracted Release probe build and both interop checks pass |
| Native Windows/macOS; external security audit at initial delivery | NOT RUN; subsequent Windows attempt is recorded below |

The existing seeded corpus has 4,023 cases: 18 frozen design vectors,
500 generated positives, 3,500 mutations and five additional structural negatives.
GhostWire and its Python schema reference agree: 918 accepted, 3,105 rejected.
Actual TinyCBOR accepts 991 and rejects 3,032. All 918 GhostWire successes pass
TinyCBOR; the five mandatory noncanonical controls fail upstream, and the five
mandatory generic-CBOR controls succeed. The 73 upstream-only accepts are rejected
by the schema reference as 30 unsupported versions, four kinds, one capability
and 38 frame-shape/type violations. These counts are corpus evidence, not an
exhaustive parser audit. Oversized 4 MiB stdin rejection/resynchronization passes.

No leak suppression was added to source or presets. The existing known Botan
GCC Release `-Wstringop-overread` warning remains an open upstream triage item.
Final report bookkeeping is the only difference from the tested extraction;
all build, source, dependency and test bytes were compared unchanged before
regenerating the final archive and SHA-256 sidecar.

## Revision 2 — owner Windows C4245 correction

Input revision 1 ZIP SHA-256:
`93395a39cb587e62d60e0ab4d2f6ae0dd511a27a632b93865ef8fb3090a342bf`;
311/311 source files match the delivered manifest before this edit.

Owner evidence: VS2026 Developer PowerShell 18.10.3, MSVC 19.51.36260.0,
Hostx64/x64 compiler and Python 3.13.13. Debug configuration succeeds. The actual
upstream C parser compiles with a nonfatal C5287 enum warning. Compilation of
`tests/interop/tinycbor_probe.cpp:35` then fails C4245, promoted by /WX to C2220.
The loop stops there: neither Windows CTest nor Release was reached. The failed
pthread feature probes are ordinary CMake detection; `Found Threads: TRUE` follows.

The upstream flag `CborValidateCompleteData` is defined as `(int)0x80000000`.
OR-ing the enum constants produced a negative signed integer and implicitly
assigned it to `uint32_t`. The correction casts each selected flag to
`std::uint32_t` before OR-ing. The mask remains exactly `0x80401301`, including
CompleteData's high bit; no validation flag is removed or warning disabled.
The upstream files and all 12 hashes are unchanged. Only our test probe changes
executable behavior at the source level; production source and CMake are unchanged.

Validation for revision 2:

- The original expression reproduces a compiler error with GCC's explicit
  `-Wsign-conversion -Werror`; the corrected complete probe translation unit passes.
- Existing GCC Debug and Release interop and stdin-bounds tests pass 2/2 each;
  the 4,023-case corpus retains exactly 918 GhostWire accepts, 991 upstream accepts
  and 73 schema-only differences.
- Existing ASan/UBSan interop and bounds checks pass 2/2 with the same local
  leak-detection limitation as revision 1. No new harness or fixture.
- Registry validation, 13 registry tests and three Windows-documentation tests
  pass; all 12 upstream hashes reverify. The cumulative package retains 311 source
  files and is checked against its regenerated SHA-256 manifest and sidecar.
  A default full-suite rerun is unnecessary for this test-only conversion;
  revision 1's 52/52 result remains historical evidence.
- At revision 2 delivery, Windows correction execution was pending; the subsequent
  owner result is recorded below. Upstream C5287 remains visible and nonfatal.

Revision 2 replaced revision 1 under the same checkpoint filename, with
`source_revision: 2` in its manifest. Its targeted Windows rerun is now complete.
Per-change engineering confidence at delivery was 0.94 based on a reproduced
signed-conversion failure and unchanged corpus results; this was not a claim of
native acceptance or security certification.

## Revision 3 — owner Windows evidence, 2026-10-29

Input revision 2 ZIP SHA-256:
`4ff6ce9137f86d5c0feaa1be9c1ca26bbd2525e43dd6665bb197e8bf3e3348d8`.
All 311 local source files match its manifest before this evidence update.

The owner supplied the successful output of the same targeted PowerShell loop
after the revision 2 correction. Environment: VS2026 Developer PowerShell
18.10.3, MSVC 19.51.36260.0, Hostx64/x64, Python 3.13.13, Ninja generator;
source root `C:\Cumpiling-shit\GhostWire`. This is user-run native Windows
evidence, distinct from coordinator-run Linux checks.

| Owner Windows check | Debug | Release |
|---|---|---|
| Configure and both requested probe targets | PASS, incremental 7/7 build steps | PASS, 11/11 build steps |
| `protocol-tinycbor-interop` | PASS, 1.74 s | PASS, 0.61 s |
| `protocol-tinycbor-stdin-bounds` | PASS, 1.04 s | PASS, 0.42 s |
| Selected CTest result | 2/2 PASS, 2.80 s total | 2/2 PASS, 1.05 s total |

Both configurations report seed `2690580483` and the same 4,023 cases:
GhostWire/reference 918 accepted and 3,105 rejected; TinyCBOR 991 accepted and
3,032 rejected; 73 expected GhostWire-only stricter rejects. Oversized-stdin
rejection and resynchronization pass in both builds. The earlier C4245/C2220
compile failure is resolved. The upstream `cborparser.c:571` C5287 enum-type
warning appears in each build and remains nonfatal; no upstream byte or warning
policy was changed. Failed pthread probes are followed by `Found Threads: TRUE`.

This closes the Windows portion of `GW-A05-UPSTREAM-NATIVE`; its macOS portion
remains blocked. The run covers the two selected tests only, not a new full
Windows suite or an independent security audit. A05 owner parser disposition,
AUTH-PRE and A06 authorization remain separate.

Revision 3 changes existing evidence documents/registries and the packager's
source-revision value only. Existing registry validation, 13 registry tests and
three Windows-documentation tests pass. The regenerated cumulative archive is
verified against its 311-entry manifest and SHA-256 sidecar; application sources,
CMake/build settings, upstream dependencies and tests match revision 2 byte for
byte. No additional compile or Windows acceptance run is requested for this
evidence-only update.

Changed files: `AGENTS.md`, `ARCHITECTURE.md`, `README.md`, `ROADMAP.md`,
`TODO.md`, `current-development-map.md`, `docs/CHANGELOG.md`,
`docs/COMPLETED.md`, this report, ADR-010, `docs/development/STRUCTURE_RULES.md`,
the existing module/worker queue/integration JSON records, and
`tools/packaging/package_source.py` (revision value only).

## Windows reproduction command — already passed

Retained for reproduction at the source root in the VS2026 **x64** developer
terminal. The dependency is bundled; no fetch/import step is needed. The owner's
successful loop used the existing dependency default:

```powershell
foreach ($cfg in 'Debug', 'Release') {
    $build = "out/build/tinycbor-$cfg"
    cmake -S . -B $build -G Ninja "-DCMAKE_BUILD_TYPE=$cfg" -DGW_A05_ENABLE_TINYCBOR_INTEROP=ON
    if ($LASTEXITCODE) { throw "$cfg configure failed" }
    cmake --build $build --target ghostwire-protocol-probe ghostwire-tinycbor-probe --parallel 2
    if ($LASTEXITCODE) { throw "$cfg build failed" }
    ctest --test-dir $build -R '^protocol-tinycbor-(interop|stdin-bounds)$' --output-on-failure -V
    if ($LASTEXITCODE) { throw "$cfg tests failed" }
}
```

Observed: two checks pass in each configuration with the 4,023-case counts above.
No repeat run or full Windows acceptance replay is requested by revision 3.

## Decision and next capability

Recommend retaining the restricted codec for its current offline scope; no observed
corpus mismatch justifies a replacement now. [ADR-010](../decisions/ADR-010-RESTRICTED-WIRE-CODEC-P02.md)
records the reasoning. Independent parser review, remaining native macOS evidence and
owner approval remain separate; this checkpoint does not close all of A05.

What works interactively is still the volatile CLI `--identity-session`. Desktop,
server and relay remain bootstrap programs. HF19's policy table compiles but is
not runtime authorization. After accepted A05, the next useful capability is a
composition-owned local action flow that checks scoped membership, current trusted
device revision, device validity, fresh possession context and policy/revocation
at commit. A06 listeners and private-key persistence remain gated.

Engineering confidence in this narrow source/build integration: 0.91, a qualitative
assessment of pin verification, actual native comparison and scoped validation;
not a security probability or overall completion percentage. Official project
completion stays null; the provisional accepted floor stays 11/110 points (10%).
