# A03 native acceptance checklist

Status: procedure and expected coverage, **not execution evidence**. A02 owner
results are in GW-A02-HOST-ACCEPTANCE.md; they do not accept the new crypto code.
Use the delivered A03 ZIP and SHA-256 sidecar. Record the exact archive hash,
tool versions, configure/build logs, CTest results and unit case/assertion counts.

## Windows owner procedure

Host: Windows10 x64, win10dev, C:\Users\win10dev, VS2026 Community,
MSVC19.51.36260/toolset14.51.36231, Ninja, Windows PowerShell5.1.
Retain original TEMP/TMP, normally C:\Users\win10dev\AppData\Local\Temp.
Do not replace it with an ASCII workaround. The former Unicode profile remains
regression scope, not the required primary account. Fixture paths include ö/spaces.

From a fresh ordinary PowerShell after extraction:

```powershell
Set-Location 'C:\Cumpiling-shit\GhostWire'
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\windows\dev-windows.ps1 -Action diagnostics -OfflineTests
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\windows\dev-windows.ps1 -Action all -Configuration Debug -OfflineTests -Fresh
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\windows\dev-windows.ps1 -Action all -Configuration Release -OfflineTests -Fresh
.\out\build\portable-release\bin\ghostwire-cli.exe --version
.\out\build\portable-release\bin\ghostwire-cli.exe --data-root (Join-Path (Get-Location) 'out\acceptance\data with spaces')
```

| Check | Expected, fill actual outcome |
|---|---|
| Clean Debug and Release configure/build | Four app targets plus tests; no project warnings; pinned Botan 3.13.0 static Windows subset |
| Complete CTest, each configuration | 37/37; no skip or temp override |
| crypto-unit | Published AEAD/signature/hash vectors; tamper, lengths, limits, lifetime, cancellation, concurrency and lifecycle |
| crypto-faults | Initialization/entropy/allocation/cancellation failures, clearing observer and bounded in-flight stop |
| Existing core/platform/CLI/Unicode/tooling | All pass; process/console/ownership and invalid UTF-8 checks remain |
| CLI version and ordinary launch | 0.3.0 / GW-A03-CRYPTOGRAPHIC-PROVIDER; bootstrap exit 0 and clean shutdown |
| Deployment | No extra Botan DLL or Python needed for ordinary build/run; system BCrypt and existing C++ runtime available |
| Local side effects | Ordinary bootstrap makes empty role directories only; no key, identity, message or log file; help/config/path checks retain no-write behavior |

The Linux-specific getrandom failure executable is absent on Windows by design;
private fault tests execute on every OS. A real Windows BCrypt failure scenario
has not been executed locally and must not be inferred from the Linux wrapper.
Native memory locking/cancellation/compiled side-channel behavior still needs
independent security review; passing tests is not E2EE certification.

The Windows standard vcpkg/doctest route is an independent unresolved issue.
If retesting, omit -OfflineTests, select the pinned VcpkgRoot explicitly and keep
both Debug/Release logs including doctest configure errors. A new local account
or portable success does not mark this route fixed. No permanent PATH or
execution-policy change is required by the supplied scripts.

## macOS procedure and other gates

Use supported C++20 libc++/Xcode26 on x64 and ARM64; record exact OS/SDK versions.
Run portable-debug and portable-release from BUILD.md, all **36 expected CTest**
checks, including CommonCrypto RNG, process group/spawn, stop and Keychain API
discovery. Run the standard vcpkg route separately when available. Native A02
macOS branches and new A03 integration remain unexecuted here; old HF02 CI is
historical only. Do not extrapolate a minimum supported macOS deployment target.

Full LeakSanitizer on a supported host, clang-tidy and current hosted workflows
remain open. Linux TSan or sanitizer success covers only the recorded binaries
and options. Attach actual logs and remaining failures to a new host-acceptance
addendum rather than rewriting historical reports. After A03 review and owner
acceptance, explicit authorization is still required before A04.
