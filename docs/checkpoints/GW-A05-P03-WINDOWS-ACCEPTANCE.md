# Windows 10 / Visual Studio 2026 acceptance — A05-P03

This candidate **has not been tested on the owner's Windows 10 host**. Use a **freshly extracted** single-root source ZIP, do not overlay earlier checkpoints or the published A04 `main` tree. The last confirmed owner root was `C:\GhostWire\GhostWire`; change the following line to the exact extracted directory if needed.

Run each full command in this order, in Visual Studio Developer PowerShell or a regular PowerShell that discovers MSVC through the existing helper:

```powershell
$root = 'C:\GhostWire\GhostWire'
Set-Location $root

.\tools\windows\dev-windows.ps1 -Action all -Configuration Debug -OfflineTests -Fresh
if ($LASTEXITCODE -ne 0) { throw 'Debug build or CTest failed' }

.\tools\windows\dev-windows.ps1 -Action all -Configuration Release -OfflineTests -Fresh
if ($LASTEXITCODE -ne 0) { throw 'Release build or CTest failed' }

py -3 .\tools\check_boundaries.py
if ($LASTEXITCODE -ne 0) { throw 'Architecture boundary check failed' }
py -3 .\tools\protocol\check_design_vectors.py
if ($LASTEXITCODE -ne 0) { throw 'Design vectors failed' }
py -3 .\tools\protocol\reference_check.py
if ($LASTEXITCODE -ne 0) { throw 'Independent reference check failed' }

# Separately run each test-only C++ probe through the independent checker.
py -3 .\tools\protocol\differential_check.py .\out\build\portable-debug\bin\ghostwire-protocol-probe.exe
if ($LASTEXITCODE -ne 0) { throw 'Debug differential mismatch' }
py -3 .\tools\protocol\differential_check.py .\out\build\portable-release\bin\ghostwire-protocol-probe.exe
if ($LASTEXITCODE -ne 0) { throw 'Release differential mismatch' }

# Normal four role CLI identity remains available and unchanged.
foreach ($role in @('desktop', 'server', 'relay', 'cli')) {
  & ".\out\build\portable-release\bin\ghostwire-$role.exe" --version
  if ($LASTEXITCODE -ne 0) { throw "Role $role failed" }
}
```

Expected on the bundled portable build: **41 CTest registrations per configuration** (`protocol-unit` contains 10 doctest cases), if baseline CMake registrations remain unchanged. Each differential run expects **4,005 deterministic cases** with identical acceptance/rejection and canonical round-tripping. No GUI, network session, remote account, private-key persistence or actual protocol handshake should appear. Record the complete Windows logs and architecture/test output before declaring owner acceptance. The P02 Windows validation gate is not implicitly closed merely by the P03 Linux run.
