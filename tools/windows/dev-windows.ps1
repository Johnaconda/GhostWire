[CmdletBinding()]
param(
    [ValidateSet('configure','build','test','run','clean','diagnostics','all')]
    [string]$Action = 'all',
    [ValidateSet('Debug','Release')][string]$Configuration = 'Debug',
    [ValidateSet('desktop','server','relay','cli')][string]$Target = 'cli',
    [string]$VisualStudioRoot,
    [string]$VcpkgRoot,
    [string]$CMakePath,
    [string]$CTestPath,
    [string]$NinjaPath,
    [switch]$OfflineTests,
    [switch]$Fresh,
    [string[]]$AppArgs = @()
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$ProjectRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../..')).Path
. (Join-Path $PSScriptRoot 'tooling.ps1')
$ForceVisualStudio = [bool]$VisualStudioRoot
# Preserve the requested root before VsDevCmd can alter VCPKG_ROOT.
$RequestedVcpkgRoot = if ($VcpkgRoot) { $VcpkgRoot } else { $env:VCPKG_ROOT }
$env:VCPKG_DISABLE_METRICS = '1'

function Invoke-Checked([string]$File, [string[]]$Arguments) {
    & $File @Arguments
    if ($LASTEXITCODE -ne 0) { throw "Command failed with exit $LASTEXITCODE : $File" }
}
function Initialize-VisualStudio {
    if (-not $script:VisualStudioRoot -and $env:VSINSTALLDIR -and
        (Test-Path -LiteralPath (Join-Path $env:VSINSTALLDIR 'Common7/Tools/VsDevCmd.bat'))) {
        $script:VisualStudioRoot = $env:VSINSTALLDIR
    }
    if (-not $script:VisualStudioRoot) {
        $installerBase = [Environment]::GetFolderPath('ProgramFilesX86')
        $vswhere = Join-Path $installerBase 'Microsoft Visual Studio/Installer/vswhere.exe'
        if (Test-Path -LiteralPath $vswhere) {
            $candidate = & $vswhere -latest -products '*' -prerelease -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
            if ($LASTEXITCODE -ne 0) { throw 'Visual Studio discovery failed.' }
            if ($candidate) { $script:VisualStudioRoot = [string]($candidate | Select-Object -First 1) }
        }
    }
    $compiler = Get-Command cl.exe -CommandType Application -ErrorAction SilentlyContinue
    if (-not $compiler -or $env:VSCMD_ARG_TGT_ARCH -ne 'x64' -or
        $env:VSCMD_ARG_HOST_ARCH -ne 'x64' -or $script:ForceVisualStudio) {
        if (-not $script:VisualStudioRoot) {
            throw 'An x64 MSVC environment is required. Install Desktop development with C++, use an x64 Developer PowerShell, or pass -VisualStudioRoot.'
        }
        $devcmd = Join-Path $script:VisualStudioRoot 'Common7/Tools/VsDevCmd.bat'
        if (-not (Test-Path -LiteralPath $devcmd)) { throw 'VsDevCmd.bat is missing in the selected Visual Studio installation.' }
        # Unicode-safe import into this process only; retain original TEMP/TMP.
        # Never print the captured environment or modify user/machine settings.
        $values = Get-GhostWireDeveloperEnvironment $devcmd
        foreach ($name in $values.Keys) {
            if ($name -notin @('TEMP', 'TMP')) {
                [Environment]::SetEnvironmentVariable($name, $values[$name], 'Process')
            }
        }
    }
    if (-not (Get-Command cl.exe -CommandType Application -ErrorAction SilentlyContinue) -or $env:VSCMD_ARG_TGT_ARCH -ne 'x64') {
        throw 'MSVC x64 compiler was not found after initialization. Check the installed C++ workload and Windows SDK.'
    }
    $cl = Get-Command cl.exe -CommandType Application -ErrorAction Stop | Select-Object -First 1
    $link = Get-Command link.exe -CommandType Application -ErrorAction SilentlyContinue | Select-Object -First 1
    $linkPath = if ($link) { $link.Source } else { '' }
    $issues = @(Get-GhostWireArchitectureIssues -HostArchitecture $env:VSCMD_ARG_HOST_ARCH `
        -TargetArchitecture $env:VSCMD_ARG_TGT_ARCH -Compiler $cl.Source -Linker $linkPath -Libraries $env:LIB)
    if ($issues.Count -gt 0) {
        throw (($issues -join ' ') + ' Close the mixed developer shell and use a fresh ordinary PowerShell. Reconfigure with -Fresh; build-system clean does not reset a CMake cache.')
    }
    # A superficially x64 PATH is insufficient without the matching SDK/runtime.
    foreach ($library in @('msvcrt.lib', 'kernel32.lib', 'ucrt.lib')) {
        $present = $false
        foreach ($directory in ($env:LIB -split ';')) {
            if ($directory -and (Test-Path -LiteralPath (Join-Path ($directory.Trim().Trim('"')) $library) -PathType Leaf)) {
                $present = $true
                break
            }
        }
        if (-not $present) { throw "Missing $library in LIB. Install the x64 MSVC/Windows SDK components and open a fresh shell." }
    }
}

try {
    if ($Fresh -and $Action -notin @('configure','all')) { throw '-Fresh applies only to configure/all.' }
    Initialize-VisualStudio
    $cmakeCandidates = @()
    $ninjaCandidates = @()
    if ($VisualStudioRoot) {
        $cmakeCandidates += Join-Path $VisualStudioRoot 'Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe'
        $ninjaCandidates += Join-Path $VisualStudioRoot 'Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe'
    }
    $cmake = Find-GhostWireTool $CMakePath 'cmake.exe' $cmakeCandidates
    $ninja = Find-GhostWireTool $NinjaPath 'ninja.exe' $ninjaCandidates
    $ctest = Find-GhostWireTool -Explicit $CTestPath -Name 'ctest.exe' `
        -Candidates @((Join-Path (Split-Path $cmake) 'ctest.exe')) -CandidatesFirst
    $cmakeVersion = & $cmake --version
    if ($LASTEXITCODE -ne 0 -or "$cmakeVersion" -notmatch 'version (\d+\.\d+\.\d+)') { throw 'Cannot determine CMake version.' }
    $cmakeNumber = $matches[1]
    $ctestVersion = & $ctest --version
    if ($LASTEXITCODE -ne 0 -or "$ctestVersion" -notmatch 'version (\d+\.\d+\.\d+)') { throw 'Cannot determine CTest version.' }
    if ($cmakeNumber -ne $matches[1]) { throw 'CMake and CTest versions differ. Select matching -CMakePath/-CTestPath tools.' }
    $compilerPath = (Get-Command cl.exe -CommandType Application | Select-Object -First 1).Source
    $linkerPath = (Get-Command link.exe -CommandType Application | Select-Object -First 1).Source
    if (-not $OfflineTests) {
        if (-not $VcpkgRoot) { $VcpkgRoot = $RequestedVcpkgRoot }
        if (-not $VcpkgRoot -and $VisualStudioRoot) {
            $candidate = Join-Path $VisualStudioRoot 'VC/vcpkg'
            if (Test-Path -LiteralPath (Join-Path $candidate 'scripts/buildsystems/vcpkg.cmake')) { $VcpkgRoot = $candidate }
        }
        if (-not $VcpkgRoot -or -not (Test-Path -LiteralPath (Join-Path $VcpkgRoot 'scripts/buildsystems/vcpkg.cmake'))) {
            throw 'Set VCPKG_ROOT or pass -VcpkgRoot pointing at a bootstrapped vcpkg checkout. For the bundled tests, use -OfflineTests.'
        }
        $env:VCPKG_ROOT = (Resolve-Path -LiteralPath $VcpkgRoot).Path
        if (-not (Test-Path -LiteralPath (Join-Path $env:VCPKG_ROOT 'vcpkg.exe'))) {
            throw 'vcpkg.exe is missing. Run bootstrap-vcpkg.bat -disableMetrics in that checkout first.'
        }
        $vcpkgVersion = & (Join-Path $env:VCPKG_ROOT 'vcpkg.exe') version
        if ($LASTEXITCODE -ne 0) { throw 'Selected vcpkg tool could not report its version.' }
        if ($env:VisualStudioVersion -like '18.*' -and "$vcpkgVersion" -match '(\d{4}-\d{2}-\d{2})') {
            if ([datetime]::ParseExact($matches[1], 'yyyy-MM-dd', [Globalization.CultureInfo]::InvariantCulture) -lt [datetime]'2025-10-16') {
                throw 'This vcpkg tool predates Visual Studio 2026 support. Use the pinned checkout in BUILD.md or -OfflineTests.'
            }
        }
    }
    $prefix = if ($OfflineTests) { 'portable' } else { 'windows-msvc' }
    $preset = $prefix + '-' + $Configuration.ToLowerInvariant()
    $buildDir = Join-Path $ProjectRoot ('out/build/' + $preset)
    # The default local build tooling does not send vcpkg telemetry.
    $env:VCPKG_DISABLE_METRICS = '1'
    Push-Location $ProjectRoot
    try {
        if ($Action -eq 'diagnostics') {
            Write-Host "Project: $ProjectRoot"
            Write-Host "Visual Studio: $VisualStudioRoot"
            Write-Host "Compiler: $((Get-Command cl.exe).Source)"
            Write-Host "Linker: $linkerPath"
            Write-Host "MSVC host/target: $env:VSCMD_ARG_HOST_ARCH/$env:VSCMD_ARG_TGT_ARCH"
            Write-Host "CMake: $cmake"
            Write-Host "Ninja: $ninja"
            Write-Host "CTest: $ctest"
            Write-Host "Preset: $preset"
            if (-not $OfflineTests) { Write-Host "vcpkg: $env:VCPKG_ROOT" }
            Invoke-Checked $cmake @('--version')
            Invoke-Checked $ninja @('--version')
            Invoke-Checked $ctest @('--version')
            Write-Host 'TEMP/TMP: inherited unchanged; no ASCII workaround is applied.'
        }
        if ($Action -in @('configure','all')) {
            $configureArgs = @('--preset', $preset, "-DCMAKE_MAKE_PROGRAM=$ninja",
                "-DCMAKE_CXX_COMPILER=$compilerPath", "-DCMAKE_LINKER=$linkerPath")
            if ($Fresh) { $configureArgs += '--fresh' }
            Invoke-Checked $cmake $configureArgs
        }
        if ($Action -in @('build','all')) { Invoke-Checked $cmake @('--build', '--preset', $preset, '--parallel', '4') }
        if ($Action -in @('test','all')) { Invoke-Checked $ctest @('--preset', $preset, '--output-on-failure') }
        if ($Action -eq 'run') {
            $exe = Join-Path $buildDir ("bin/ghostwire-$Target.exe")
            if (-not (Test-Path -LiteralPath $exe)) { throw 'Executable missing. Run -Action all for this configuration first.' }
            & $exe @AppArgs
            exit $LASTEXITCODE
        }
        if ($Action -eq 'clean') {
            # Build-system clean only; never recursively delete user-selected paths.
            Invoke-Checked $cmake @('--build', '--preset', $preset, '--target', 'clean')
        }
    } finally { Pop-Location }
} catch {
    Write-Error -Message $_.Exception.Message -ErrorAction Continue
    exit 1
}
