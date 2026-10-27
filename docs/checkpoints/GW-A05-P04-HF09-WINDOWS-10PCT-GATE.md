# HF09 Windows 10% acceptance — VS2026 Community (owner)

Extract the **full HF09 ZIP** to a clean folder and open its `GhostWire`
subfolder in Visual Studio. In Solution Explorer, right-click the GhostWire
folder and choose **Open in Terminal**. On this host, that terminal correctly
loads x64, while Tools → Command Line → PowerShell selected x86.
No `cd`, administration, vcpkg preset, PowerShell bypass or Defender exclusion.

```powershell
(Get-Command cl.exe).Source
```

The path **must** include `Hostx64\x64\cl.exe`. If it shows x86, stop
and use the working folder's terminal. Then run from the folder containing
`CMakeLists.txt`:

```powershell
cmake --preset portable-debug --fresh
if ($LASTEXITCODE -ne 0) { throw 'Debug configure failed' }
cmake --build --preset portable-debug --parallel 4
if ($LASTEXITCODE -ne 0) { throw 'Debug build failed' }
ctest --preset portable-debug -R '^protocol-allocation-fault$' --output-on-failure -V
if ($LASTEXITCODE -ne 0) { throw 'Debug fault test failed' }
ctest --preset portable-debug --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'Debug full suite failed' }

cmake --preset portable-release --fresh
if ($LASTEXITCODE -ne 0) { throw 'Release configure failed' }
cmake --build --preset portable-release --parallel 4
if ($LASTEXITCODE -ne 0) { throw 'Release build failed' }
ctest --preset portable-release --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'Release full suite failed' }
```

Expected native Windows registered tests: **50** per configuration, with
`protocol-allocation-fault` running rather than timing out (15s limit).
If a test hangs, capture its native debugger call stack before changing
its timeout. Choose `ghostwire-cli.exe (bin\ghostwire-cli.exe)` for an
interactive launch, **not** the `(install)` target.

Native acceptance evidence must show exact compiler architecture, both builds,
focused fault test, full CTest totals, and any remaining error output.
10% Windows acceptance does not release A05's independent parser review,
real pinned TinyCBOR interop or authorize A06.
