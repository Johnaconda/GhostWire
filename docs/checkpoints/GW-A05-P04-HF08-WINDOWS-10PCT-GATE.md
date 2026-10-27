# GW-A05-P04-HF08 — Windows 10% acceptance (owner)

Extract this **full cumulative HF08 source ZIP** to a fresh directory.
Open the contained `GhostWire` folder in Visual Studio 2026 Community.
Choose the `portable-debug` CMake preset with the **x64 Hostx64/x64** compiler,
not the vcpkg-based Windows preset. The integrated terminal is normally
already at the project root: the folder containing `CMakeLists.txt`.

After Visual Studio loads the CMake project, run the following in its x64
Developer PowerShell terminal from that folder. No `cd`, script bypass,
administrator permissions, or Defender exclusions are needed.

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

If the fault test still times out, run its binary under the **Visual Studio
native debugger** with break-all while hung, and capture the call stack.
Do not disable the timeout or the test. Evidence must include compiler target
architecture, Debug/Release build results, fault-test result, full test totals,
and any Defender event. The Debug CLI binary is
`out/build/portable-debug/bin/ghostwire-cli.exe` (use executable, not the
`(install)` target). Do not infer Windows acceptance from Linux results.

Accepting this provisional **10% Windows foundation gate** does not approve
A05 network-parser security or authorize A06 transport.
