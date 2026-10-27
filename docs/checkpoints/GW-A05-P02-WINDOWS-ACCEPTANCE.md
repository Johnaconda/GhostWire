# A05-P02 — Windows 10 / Visual Studio 2026 host acceptance

The owner's supplied A05-P01 output confirms Debug 40/40 and Release 40/40 on
MSVC 19.51.36260. This P02 candidate adds one new CTest registration named
`protocol-unit`, so Windows is expected to report **41/41 per configuration**,
not proven until run. No new CLI commands, sockets or GUI appear.

**Do not overlay an older tree or combine source copies.** Extract the delivered
single-root source ZIP into a clean directory and use that actual root. The
latest submitted Windows log ran from `C:\GhostWire\GhostWire`; change `$root`
only if your selected location differs.

```powershell
$root = 'C:\GhostWire\GhostWire'
$zip = '<absolute-path-to-GHOSTWIRE-GW-A05-P02-STRICT-VERSIONED-WIRE-CODEC-source.zip>'
Get-FileHash $zip -Algorithm SHA256 | Format-List
Set-Location $root

# Run these as separate complete commands, not pasted in reverse order.
.\tools\windows\dev-windows.ps1 -Action all -Configuration Debug -OfflineTests -Fresh
if ($LASTEXITCODE -ne 0) { throw 'Debug failed' }

.\tools\windows\dev-windows.ps1 -Action all -Configuration Release -OfflineTests -Fresh
if ($LASTEXITCODE -ne 0) { throw 'Release failed' }

py -3 .\tools\check_boundaries.py
if ($LASTEXITCODE -ne 0) { throw 'Boundary check failed' }
py -3 .\tools\protocol\check_design_vectors.py
if ($LASTEXITCODE -ne 0) { throw 'Design fixture check failed' }
py -3 .\tools\protocol\reference_check.py
if ($LASTEXITCODE -ne 0) { throw 'Independent reference check failed' }

# Explicit CTest focus if diagnosing only the new library:
ctest --test-dir .\out\build\portable-debug -R protocol-unit --output-on-failure
ctest --test-dir .\out\build\portable-release -R protocol-unit --output-on-failure

foreach ($role in @('desktop','server','relay','cli')) {
    & ".\out\build\portable-release\bin\ghostwire-$role.exe" --version
    if ($LASTEXITCODE -ne 0) { throw "Role version failed: $role" }
}
```

Expected executable runtime version remains `0.4.1` / A04-P03. Confirm
`ghostwire-cli.exe --identity-session` still supports an ephemeral local
profile and exits normally. No external networking/protocol functionality is
activated by this pass. Report full Debug/Release output and any MSVC errors
before owner acceptance or pushing a new checkpoint.
