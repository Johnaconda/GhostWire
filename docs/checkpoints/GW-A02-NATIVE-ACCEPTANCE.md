# A02 native acceptance checklist

Status: procedure only; no A02 Windows/macOS execution is claimed here.
HF01 owner acceptance and HF02 hosted CI evidence are preserved separately.

## Windows 10 x64 / VS2026 Community

1. Verify the delivered A02 ZIP against its accompanying SHA-256. Extract the
   entire cumulative project. Preserve any existing .git and local user data.
   Record the actual compiler, SDK, Ninja, CMake/CTest and source checksum.
2. Use the owner's original TEMP/TMP without an ASCII override. The previously
   accepted root is `C:\Cumpiling-shit\GhostWire`. Also exercise a source path with
   spaces if used in development. Dedicated tests copy a real executable into an
   `ö`/spaces directory below native `temp_directory_path()`.
3. From ordinary PowerShell, run these existing process-scoped tooling commands;
   no permanent PATH or execution-policy change is required:

```powershell
Set-Location 'C:\Cumpiling-shit\GhostWire'
.\tools\windows\dev-windows.ps1 -Action all -Configuration Debug -OfflineTests -Fresh
.\tools\windows\dev-windows.ps1 -Action all -Configuration Release -OfflineTests -Fresh
.\out\build\portable-debug\bin\ghostwire-cli.exe --version
```

Expected A02 registrations: **35 per Windows configuration**, including
`core-unit`, new `platform-unit`, 32 role subprocess contracts and
`windows-tooling`. Counts are expectations until logs show passes. No skip or
ASCII-temp adjustment is an acceptable substitute. Check version 0.2.0/A02.

| Area | Acceptance evidence required |
|---|---|
| Existing HF01 | All Unicode/profile/config/invalid-UTF-8 and PowerShell tests pass with original TEMP/TMP |
| Launch | Real child preserves Unicode exe/cwd/argv/env, spaces, quotes, empty values, trailing backslashes and literal shell metacharacters; no shell side effects |
| Ownership | Job assigned before resume; timeout/cancel retains child; force/RAII clean up; repeat and concurrent waits succeed |
| Handles/output | Foreign inheritable file handle excluded; both output streams bounded and drained; ordinary descendant writers stopped |
| Native stop | CTRL_BREAK reaches child bridge, cancellation callback runs on child owner thread; duplicate/teardown/reinstall checks pass |
| Real service | A01 application service owns and stops an actual bridge child within its shutdown budget |
| Capability | DPAPI API presence is reported; disabled probe unavailable; persistence_ready remains false |
| CLI | Four roles preserve flags/exits/paths/privacy and clean one-shot exit; no GUI/listener/profile created |

The test runner owns a console only if CTest has none, so CTRL_BREAK tests have
the shared console required by Windows. This is test setup, not a production
promise for console-less processes. Interactive Ctrl+C bootstrap checks may be
too short to observe reliably; the real child handshake tests establish delivery.
Close/logoff/shutdown cancellation is explicitly not an A02 feature.

The owner's vcpkg/doctest route remains a separate unresolved environment gate.
If testing it, remove `-OfflineTests`, retain VCPKG_ROOT and collect the original
configure/buildtree logs. Preserve both routes; do not infer local resolution
from prior hosted Windows vcpkg success. Do not change SDK architecture or warnings.

## macOS x64 and ARM64

Use the preserved HF02 Xcode 26/macOS 26 CI matrix and exact pinned vcpkg.
Run `bash tools/dev.sh all macos-debug` and `macos-release` on each architecture;
portable Debug/Release are available separately. Expected **34 CTest checks**.
Record AppleClang/libc++/SDK identities and actual deployed macOS version.
C++20 stop-token/jthread support and spawn chdir/descriptor isolation are required;
do not weaken compiler checks for an older SDK. Verify process groups/reaping,
prior signal handler restoration, native Unicode/temp paths and API-only Keychain
discovery. No unlocked-keychain or persistence test is claimed by this probe.

## Evidence and stop point

Supply complete configure/build/CTest summaries and failures, without private
environment values. Rerun default sanitizers on a supported host; clang-tidy and
native A02 hosted CI are separate checks. Independent review and Windows host
acceptance are required before the owner can authorize the proposed A03 prompt.
