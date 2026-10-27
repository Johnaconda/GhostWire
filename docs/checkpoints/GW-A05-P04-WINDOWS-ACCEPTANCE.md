# A05-P04 candidate — Windows 10 / VS2026 acceptance

Use the freshly extracted cumulative source directory, not the published older
GitHub main checkout. The source root may be moved or renamed.

```powershell
$root = 'C:\GhostWire\GhostWire'  # Set to the actual extracted GhostWire folder
Set-Location $root
.\tools\windows\dev-windows.ps1 -Action all -Configuration Debug -OfflineTests -Fresh
if ($LASTEXITCODE -ne 0) { throw 'Debug failed' }
.\tools\windows\dev-windows.ps1 -Action all -Configuration Release -OfflineTests -Fresh
if ($LASTEXITCODE -ne 0) { throw 'Release failed' }
py -3 .\tools\check_boundaries.py
if ($LASTEXITCODE -ne 0) { throw 'Architecture failed' }
py -3 .\tools\protocol\test_tinycbor_import.py
if ($LASTEXITCODE -ne 0) { throw 'Import security check failed' }
py -3 .\tools\protocol\differential_check.py .\out\build\portable-debug\bin\ghostwire-protocol-probe.exe
if ($LASTEXITCODE -ne 0) { throw 'Debug differential failed' }
py -3 .\tools\protocol\differential_check.py .\out\build\portable-release\bin\ghostwire-protocol-probe.exe
if ($LASTEXITCODE -ne 0) { throw 'Release differential failed' }
```

Expected: 42 prior Windows test registrations plus the new allocation-fault
harness, **43 tests if CMake finds Python3** and registers the import-guard
test; otherwise 42. Report actual counts rather than assuming them.

### Explicit maintained-library gate (source must first be supplied)

The default archive does **not** include upstream source; this section cannot
pass without a separately obtained exact v7.0 source archive. On a connected
operator machine, download TinyCBOR's official v7.0 source ZIP by an explicit
owner action (not as a normal build side effect). Point `$zip` to that file.

```powershell
$zip = 'C:\path\to\tinycbor-v7.0.zip'
py -3 .\tools\protocol\import_tinycbor.py --source "$zip"
if ($LASTEXITCODE -ne 0) { throw 'Upstream pin mismatch' }
py -3 .\tools\protocol\import_tinycbor.py --source "$zip" --install
if ($LASTEXITCODE -ne 0) { throw 'Upstream import failed' }
cmake -S . -B out\build\tinycbor-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DGW_A05_ENABLE_TINYCBOR_INTEROP=ON
if ($LASTEXITCODE -ne 0) { throw 'Upstream configure failed' }
cmake --build out\build\tinycbor-debug --target ghostwire-protocol-probe ghostwire-tinycbor-probe
if ($LASTEXITCODE -ne 0) { throw 'Upstream build failed' }
ctest --test-dir out\build\tinycbor-debug -R '^protocol-tinycbor-interop$' --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'Upstream interop failed' }
```

The additional test target must be built and evaluated on MSVC, Linux GCC/
Clang and macOS Clang. An imported library is **test-only**, not a production
parser or permission to open listeners. Preserve the original upstream MIT
license in the imported files and distribution if later accepted.
