# A02 owner acceptance addendum — 2026-10-13

The owner explicitly accepted GW-A02-PLATFORM-ABSTRACTIONS and authorized A03.
This addendum preserves the earlier candidate report and native checklist as
historical records. Owner-provided results are distinguished from executor tests.

Authoritative continuation: GHOSTWIRE-GW-A02-PLATFORM-ABSTRACTIONS-source(2).zip,
SHA-256 `404fbea230ff612693754d8fa70c56e2a653d85e4a98fea6d80242e2243a5dd7`.
A03 preparation verified that hash, ZIP CRC, safe paths and all 125 source hashes.
Read-only GitHub reconciliation found main still at
`713750550d6fa7bbde7200c494554d1c4f9e008f`, tree
`7913ddbd8a7e8d8cd559c2a6217b8da335ace6d3` (UpdatedReadme). Its supplied HF02
changes were already preserved in A02. No history reset or remote write occurred.

## Actual owner host

- Windows 10 x64; local account win10dev, profile `C:\Users\win10dev`.
- Project root `C:\Cumpiling-shit\GhostWire`.
- Visual Studio 2026 Community; MSVC 19.51.36260, toolset 14.51.36231.
- CMake/Ninja, Windows PowerShell 5.1, portable/offline test dependencies.
- Original TEMP `C:\Users\win10dev\AppData\Local\Temp`; no TEMP/TMP override.

| A02 owner-executed check | Result |
|---|---|
| Clean Debug configure/build | PASS |
| Debug CTest | 35/35 PASS |
| Clean Release configure/build | PASS |
| Release CTest | 35/35 PASS |
| Platform, Unicode and Windows tooling regressions | PASS |
| Native CLI launch | Exit 0 |

Commands used the existing dev-windows.ps1 with `-Action all`, each
`-Configuration Debug` / `Release`, `-OfflineTests -Fresh`. The owner also
accepted independent Linux validation. The former Unicode-profile environment
remains regression scope; it is no longer the primary development login.

The Windows vcpkg/doctest 2.4.12 configure failure remains unresolved. Creating
the new account is not evidence that this separate route works. Native A02 macOS,
full local LeakSanitizer, clang-tidy and A02 hosted CI remain outstanding.
Acceptance of A02 does not certify A03 on any native host or authorize A04.
