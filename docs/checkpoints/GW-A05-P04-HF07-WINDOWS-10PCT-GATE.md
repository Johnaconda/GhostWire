# GW-A05-P04-HF07 — Windows acceptance procedure (10% provisional milestone)

Use a **fresh extracted HF07 source folder**. Previous Visual Studio logs
showed CLI launch exit=0, but most test executables were missing because
CTest was invoked without completing the entire build. Don't merge old
build directories over fresh source.

Open Visual Studio 2026's x64 developer environment (MSVC Hostx64/x64).
Select the `portable-debug` and `portable-release` presets, **not** the
vcpkg-based `windows-msvc-debug` preset. In the integrated developer
PowerShell, with CMake in PATH, run from the root containing `CMakeLists.txt`:

```powershell
cmake --preset portable-debug --fresh
if ($LASTEXITCODE -ne 0) { throw 'Debug configure failed' }
cmake --build --preset portable-debug --parallel 4
if ($LASTEXITCODE -ne 0) { throw 'Debug build failed' }
ctest --preset portable-debug --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'Debug CTest failed' }

cmake --preset portable-release --fresh
if ($LASTEXITCODE -ne 0) { throw 'Release configure failed' }
cmake --build --preset portable-release --parallel 4
if ($LASTEXITCODE -ne 0) { throw 'Release build failed' }
ctest --preset portable-release --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'Release CTest failed' }
```

This runs no unsigned PowerShell scripts and changes no execution policy.
Record compiler architecture, configure/build results, test totals, and
Defender detection information if any archive is flagged. Test totals may
differ by platform: Linux registers 51; prior incomplete Windows builds
registered 50. A successful CLI process exit is not a substitute for CTest.

Do not approve A06/network exposure: actual pinned upstream TinyCBOR
execution and independent parser security review remain open.
