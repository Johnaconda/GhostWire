# A01 Windows report and HF01 acceptance checklist

Date: 2026-10-12. This record separates the owner's A01 report from HF01 tests
that must still run on the Windows host. **HF01 is not yet accepted.**

## Received A01 host evidence

Authoritative input: `GHOSTWIRE-GW-A01-CORE-LIFECYCLE-SERVICES-source(1).zip`,
SHA-256 `0c8ded6652ec0d799cfcfde0789160e3ed5b821ad5f797c1562bae12088aed5d`.
This matches the previous cumulative delivery exactly, including all 100
source manifest entries. Evidence below is owner-reported, not run by the Linux
executor or inferred from a screenshot. No raw vcpkg config log was supplied
with this report, so its root cause is not established.

| Host detail | Owner-reported value |
|---|---|
| OS / IDE | Windows 10 x64 / Visual Studio 2026 Community |
| MSVC / toolset | 19.51.36260 / 14.51.36231 |
| Tools / architecture | Visual Studio bundled CMake, Ninja, Host x64 / Target x64 |
| Compiler / linker suffix | VC\Tools\MSVC\14.51.36231\bin\Hostx64\x64\cl.exe and link.exe |
| Dependency route | Portable / vendored doctest 2.4.12 |
| Project | C:\Cumpiling shit\GhostWire |
| Debug and Release | Configure, all four targets, CTest 29/29 and unit cases 64/64 passed |
| CLI | Launched successfully |
| Workaround | Process-local TEMP and TMP set to C:\GWTemp; no global change |
| Original temp environment | 61/64 cases and 580/587 assertions passed; three fixture cases fail with exit 2; profile contains ö |
| Earlier linker failure | x64 target mixed with x86 SDK/runtime; LNK4272 and about 152 unresolved externals; recovered with clean Hostx64/x64 configuration |
| Standard Windows vcpkg route | doctest 2.4.12 configure failure remains unresolved |

The 64 Windows versus 65 Linux cases in A01 are intentional: one symlink test
and two assertions are compiled only on POSIX. The remaining three failing
cases match native path.string() bytes being passed to strict UTF-8 CLI parsing.
The production CLI already uses native wide argv conversion and explicit UTF-8
path decoding. The fixture correction does not broaden accepted input encodings.

## HF01 owner procedure

1. Extract the full HF01 ZIP into a clean source directory; do not overlay old
   build caches. Verify its delivered SHA-256 with `Get-FileHash -Algorithm SHA256`.
2. Close the workaround shell. Open ordinary Windows PowerShell from Start so it
   inherits the profile's normal TEMP/TMP, not C:\GWTemp. Do not replace the temp
   directory, switch the system code page or change permanent PATH/policy.
3. Use the existing profile containing ö and the project path containing spaces.
   Run the following from the extracted GhostWire root:

```powershell
cd 'C:\Cumpiling shit\GhostWire'
$originalTemp = $env:TEMP
$originalTmp = $env:TMP

.\tools\windows\dev-windows.ps1 -Action diagnostics -OfflineTests
if ($LASTEXITCODE -ne 0) { throw 'Tool discovery failed' }

.\tools\windows\dev-windows.ps1 -Action all -Configuration Debug -OfflineTests -Fresh
if ($LASTEXITCODE -ne 0) { throw 'Debug acceptance failed' }
.\out\build\portable-debug\bin\ghostwire-tests.exe --no-colors
if ($LASTEXITCODE -ne 0) { throw 'Debug unit suite failed' }

.\tools\windows\dev-windows.ps1 -Action all -Configuration Release -OfflineTests -Fresh
if ($LASTEXITCODE -ne 0) { throw 'Release acceptance failed' }
.\out\build\portable-release\bin\ghostwire-tests.exe --no-colors
if ($LASTEXITCODE -ne 0) { throw 'Release unit suite failed' }

.\tools\windows\dev-windows.ps1 -Action test -Configuration Release -OfflineTests
if ($LASTEXITCODE -ne 0) { throw 'Ordinary-shell CTest discovery failed' }
.\out\build\portable-release\bin\ghostwire-cli.exe --version
if ($LASTEXITCODE -ne 0) { throw 'CLI launch failed' }
if ($env:TEMP -cne $originalTemp -or $env:TMP -cne $originalTmp) { throw 'Temporary environment was changed' }
```

Expected **but not yet executed here**: 34/34 CTest checks per Windows build and
67/67 unit cases. Four `*-unicode-paths` cases test actual executable argv,
config files named with ö/spaces, explicit path display, no-write checking,
bootstrap override, missing config exit 4 and invalid UTF-8 config exit 3.
`windows-tooling` runs 18 checks, including actual cmd.exe Unicode environment
transfer, failed setup, mismatched LIB/toolset rejection and paired CTest discovery.
No registered test sets an ASCII temp workaround. The original POSIX-only case
remains excluded; the new native UTF-16 case runs only on Windows.

Confirm CLI identity is `GW-A01-HF01-WINDOWS-UNICODE-RECOVERY`, version 0.1.1.
Also perform BUILD.md's explicit path/invalid-config/unknown-flag checks. If a
failure remains, preserve its exact case/exit/output and stop acceptance there;
do not delete the test, relax validation or mask it by changing TEMP/TMP.

## Separate vcpkg investigation

Repeat with `-OfflineTests` omitted and record the selected vcpkg root, tool
version, x64 triplets and exact doctest configure/buildtree logs. `-Fresh` resets
the project cache; it is not a demonstrated fix for the dependency's configure
failure. Keep Debug and Release outcomes separate. Portable success does not
prove Windows vcpkg compatibility, and Linux vcpkg success does not close it.

The native MSVC/PowerShell checks, original TEMP acceptance, Windows vcpkg,
macOS x64/ARM64, full LeakSanitizer, clang-tidy and hosted CI remain outstanding.
The accepted A01 cooperative deadlines and declared resource budgets are
unchanged. A11-PRE and AUTH-PRE remain intact. Stop after HF01 until owner acceptance
and a separate A02 authorization.
