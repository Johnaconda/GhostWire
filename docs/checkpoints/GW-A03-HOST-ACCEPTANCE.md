# A03 owner acceptance and published CI — 2026-10-14

This addendum preserves the historical A03 report and records later evidence.
It accepts the demonstrated Windows scope, not every platform or a crypto audit.

## Owner Windows host

The owner supplied `Pasted text(7).txt`, SHA-256
`b76143f82fc93eb42b49019a0fbe63f32d2764123362733e45a3090ac351b101`,
and reported publishing A03 to main. The log records Windows 10 x64, VS 2026
Community, MSVC 19.51.36260.0, toolset 14.51.36231, HostX64/x64, Ninja and portable
vendored dependencies. Root: `C:\Cumpiling-shit\GhostWire`; account: win10dev.
TEMP remains `C:\Users\win10dev\AppData\Local\Temp`. No ASCII TEMP override.

| Owner command/check | Actual outcome |
|---|---|
| dev-windows.ps1 -Action all -Configuration Debug -OfflineTests -Fresh | Clean configure/build; 37/37 CTest PASS, 5.22 seconds |
| Same command with Release | Clean configure/build; 37/37 CTest PASS, 4.98 seconds |
| Four executable --version commands | Each reports GhostWire 0.3.0, A03, correct role, Windows/x86_64 |
| CLI --help | Printed successfully; legacy console rendered the UTF-8 em dash incorrectly |
| Crypto, Unicode and Windows tooling CTest entries | PASS in both configurations |

The owner log does not report verbose unit case/assertion totals. None are
inferred from Linux. The help-banner rendering issue is presentation-only
evidence; it does not override the successful Unicode path tests.

## Read-only repository reconciliation

Main commit [95c9128](https://github.com/Johnaconda/GhostWire/commit/95c9128ea5438663b14038c66f4986dfb4b90279),
tree `24e39264d507826c3ae7da62430b8ae66a4cce73`, message `A03`, dated
2026-10-14T09:36:22Z, exactly matches all 158 source files in the delivered A03
archive. Each Git blob SHA was independently calculated from the archive bytes.
The archive SHA-256 is
`b3d3d39e1a369185a1309e41213568f88ef3b212c80898bb577334b0616f3c64`.
Its CRC, safe paths and all manifest hashes were verified before extraction.
Only the generated SOURCE_MANIFEST.json is absent from Git, as intended by the
existing ignore rule. No owner change was discarded, and no remote write occurred.

## Hosted evidence at that exact commit

Run/job identifiers and counts are retained in
[GW-A03-HOSTED-ACCEPTANCE.json](GW-A03-HOSTED-ACCEPTANCE.json).
All nine job logs were inspected, including every macOS matrix entry.

| Workflow | Actual outcomes |
|---|---|
| [Linux](https://github.com/Johnaconda/GhostWire/actions/runs/38543593637) | GCC 13.3.0: pinned-vcpkg Debug and Release 37/37 each; default portable ASan/UBSan job 37/37 |
| [Windows](https://github.com/Johnaconda/GhostWire/actions/runs/38543593616) | Windows Server 2022/MSVC 19.44.35229.0: pinned-vcpkg doctest 2.4.12 Debug and Release 37/37 each |
| [macOS](https://github.com/Johnaconda/GhostWire/actions/runs/38543593596) | macOS 26 Intel and ARM64/AppleClang 21.0.0.21000101: all four configure/build jobs succeeded, then CTest failed 34/36 |

Every macOS job failed the same two runners. Platform reported 7/18 cases and
123/134 assertions passing; 11 failures were owned-process wait/cleanup checks.
Crypto reported 19/20 cases and 1429/1430 assertions passing; the lifecycle
fixture failed Application::start. The primitive vectors and crypto fault runner
passed. A03-HF01 addresses these failures; it has not yet run on hosted macOS.

The hosted sanitizer preset ran without leak suppression in the workflow/logs.
That is real default-preset workload evidence; it neither changes the recorded
local LSan runtime restriction nor constitutes an exhaustive leak audit.
The owner's separate Windows vcpkg failure is still unresolved: hosted MSVC 19.44
success cannot prove their VS 2026/MSVC 19.51 setup is repaired. clang-tidy and an
independent cryptographic audit remain unexecuted. No A04 design is approved by
these build results; A11-PRE and AUTH-PRE remain mandatory.
