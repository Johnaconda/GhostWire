# A03-HF01 acceptance addendum — 2026-10-15

The owner supplied successful Windows acceptance and explicitly authorized
continuation without personally validating macOS. Read-only GitHub inspection
also found real, successful hosted macOS execution. This addendum supersedes
the pending acceptance statements in the preserved historical HF01 report;
it does not rewrite their original evidence.

## Exact source

Input: GHOSTWIRE-GW-A03-HF01-MACOS-RECOVERY-source.zip.
SHA-256: `5d203d63341deeb14f2128e86bb8cf5773c19c7084ef6a83caefdc273c1747a8`.
Archive CRC, safe paths and all 166 source hashes verified before extraction.
Owner-published main:
[1f92c5539f7eb3f6f8e3833f27df9384f260251a](https://github.com/Johnaconda/GhostWire/commit/1f92c5539f7eb3f6f8e3833f27df9384f260251a),
tree b116ccee2cfc77029b090dc79c33e1c8f236ccf0, message Ao3-Hf1 / hotfix.
All 166 source files match by Git blob hashes, with zero changed, missing or
extra remote source files. The generated SOURCE_MANIFEST is intentionally not
tracked. No repository write, push, workflow dispatch, tag or merge was performed.

## Owner Windows host

Evidence: Pasted text(8).txt, SHA-256
`ceb9a69ce7348de40c793352b1bab63427b49bb3bb9d44c0db52c0002b8c4415`.
Windows 10 x64 / Visual Studio 2026 Community, MSVC 19.51.36260.0,
HostX64/x64 toolset 14.51.36231, C:\Cumpiling-shit\GhostWire.
Fresh ordinary PowerShell invokes the existing dev-windows script with
-Action all -OfflineTests -Fresh for Debug then Release. Its child-only
ExecutionPolicy Bypass and tool discovery work. No TEMP/TMP override is present
in the supplied commands; no global environment change is required.

| Actual check | Result |
|---|---|
| Fresh portable Debug configure/build | PASS, 43/43 build actions |
| Debug CTest | PASS, 37/37, 5.80 s |
| Fresh portable Release configure/build | PASS, 43/43 build actions |
| Release CTest | PASS, 37/37, 5.37 s |
| Unicode and Windows tooling entries | PASS in both configurations |
| CLI --version and exit guard | PASS, 0.3.1 / GW-A03-HF01-MACOS-RECOVERY |

The initial pthread probing failures are CMake capability probes followed by
Found Threads: TRUE, not failed application compilation. The host log contains
CTest totals, not verbose unit-case/assertion totals; none are inferred.

## Hosted native results at the exact commit

All nine job logs and their successful configure/build/test steps were read.
Machine-readable URLs, job IDs and excerpts are in
[GW-A03-HF01-ACCEPTANCE-EVIDENCE.json](GW-A03-HF01-ACCEPTANCE-EVIDENCE.json).

| Workflow / environment | Actual results |
|---|---|
| [macOS](https://github.com/Johnaconda/GhostWire/actions/runs/38680770477), ARM64 macOS 26.6.2 / Intel 26.6.1, Xcode 26.6, AppleClang 21.0.0.21000101, pinned vcpkg | Debug and Release on both architectures: 37/37 each; platform-unit, crypto-unit and crypto-symlink-temp pass |
| [Windows](https://github.com/Johnaconda/GhostWire/actions/runs/38680770569), Server 2022 / MSVC 19.44.35229.0, pinned vcpkg x64-windows | Debug and Release: 37/37 each |
| [Linux](https://github.com/Johnaconda/GhostWire/actions/runs/38680770539), Ubuntu 24.04.5 / GCC 13.3.0, pinned vcpkg | Debug and Release: 38/38 each |
| Same Linux workflow, portable sanitizer job 116244568468 | Default configured ASan/UBSan: 38/38; workflow/logs contain no configured leak suppression |

The macOS failures at earlier commit 95c9128 are historical failures. This new
source builds and passes native hosted tests on both architectures. The owner
does not need a personal Mac to repeat those same CI checks before A04 design.
No minimum older-macOS support, physical-host test or exhaustive security audit
is inferred. The hosted sanitizer workload passes with its normal settings;
the previous local LeakSanitizer runtime restriction remains independently open.

Linux Release emits a GCC -Wstringop-overread diagnostic through Botan's generated
DER_Encoder/std::vector comparison code. It does not fail the build, but this is
not warning-free evidence. No warning flag or pinned dependency is changed here.
Follow-up triage is recorded in TODO without claiming a confirmed exploit or
dismissing the diagnostic as a proven false positive.

Owner-local Windows vcpkg/doctest configuration remains unresolved even though
the different hosted MSVC environment passes. clang-tidy, independent crypto
review and broader release/platform qualification remain separate pending work.

## Acceptance and next work

HF01 is accepted for continuation on the owner's Windows evidence and instruction;
hosted Linux/Windows/macOS evidence is recorded separately above. A04-P01 produces
the concrete identity design required before identity behavior implementation.
A11-PRE and AUTH-PRE are unchanged. A04 design approval remains pending until the
owner reviews ADR-008; a successful HF01 test does not approve a new identity model.
