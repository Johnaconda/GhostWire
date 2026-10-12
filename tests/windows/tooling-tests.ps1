# No Pester dependency. Exercises build tooling, not simulated product services.
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot '../../tools/windows/tooling.ps1')
$checks = 0
function Assert-Tooling([bool]$Condition, [string]$Message) {
    if (-not $Condition) { throw $Message }
    $script:checks++
}
$valid = @{
    HostArchitecture = 'x64'; TargetArchitecture = 'x64'
    Compiler = 'C:\VS\VC\Tools\MSVC\14.51\bin\Hostx64\x64\cl.exe'
    Linker = 'C:\VS\VC\Tools\MSVC\14.51\bin\Hostx64\x64\link.exe'
    Libraries = 'C:\VS\VC\Tools\MSVC\14.51\lib\x64;C:\SDK\Lib\10\um\x64;C:\SDK\Lib\10\ucrt\x64'
}
Assert-Tooling (@(Get-GhostWireArchitectureIssues @valid).Count -eq 0) 'Valid x64 environment rejected.'
foreach ($arch in @('x86', 'arm64')) {
    $bad = $valid.Clone(); $bad.TargetArchitecture = $arch
    Assert-Tooling (@(Get-GhostWireArchitectureIssues @bad).Count -gt 0) 'Non-x64 target accepted.'
    $bad = $valid.Clone(); $bad.Libraries += (';C:\SDK\Lib\10\um\' + $arch)
    Assert-Tooling (@(Get-GhostWireArchitectureIssues @bad).Count -gt 0) 'Mixed SDK libraries accepted.'
}
$bad = $valid.Clone(); $bad.HostArchitecture = 'x86'
Assert-Tooling (@(Get-GhostWireArchitectureIssues @bad).Count -gt 0) 'Non-x64 host accepted.'
$bad = $valid.Clone(); $bad.Linker = 'C:\Git\usr\bin\link.exe'
Assert-Tooling (@(Get-GhostWireArchitectureIssues @bad).Count -gt 0) 'Foreign linker accepted.'
$bad = $valid.Clone(); $bad.Linker = $valid.Linker.Replace('14.51', '14.50')
Assert-Tooling (@(Get-GhostWireArchitectureIssues @bad).Count -gt 0) 'Mixed MSVC toolsets accepted.'
$bad = $valid.Clone(); $bad.Libraries = ''
Assert-Tooling (@(Get-GhostWireArchitectureIssues @bad).Count -gt 0) 'Empty LIB accepted.'

# ASCII source for Windows PowerShell 5.1; explicitly construct Unicode at runtime.
$root = Join-Path ([IO.Path]::GetTempPath()) ('ghostwire-tooling-' + [guid]::NewGuid().ToString('N'))
$unicode = 'J' + [char]0x00f6 + 'rg Owner'
$toolDir = Join-Path $root ($unicode + ' & tools')
$oldPath = $env:PATH
$oldValue = [Environment]::GetEnvironmentVariable('GHOSTWIRE_TOOLING_TEST_VALUE', 'Process')
$originalTemp = $env:TEMP
$originalTmp = $env:TMP
try {
    [void][IO.Directory]::CreateDirectory($toolDir)
    $paired = Join-Path $toolDir 'ctest.exe'
    [IO.File]::WriteAllText($paired, '') # Discovery fixture; never executed.
    $resolved = Find-GhostWireTool -Name 'ctest.exe' -Candidates @($paired) -CandidatesFirst
    Assert-Tooling ($resolved -eq $paired) 'Paired CTest was not selected.'
    Assert-Tooling ((Find-GhostWireTool -Explicit $paired -Name 'ctest.exe') -eq $paired) 'Explicit tool path failed.'
    $rejected = $false
    try { [void](Find-GhostWireTool -Explicit (Join-Path $root 'absent.exe') -Name 'ctest.exe') }
    catch { $rejected = $true }
    Assert-Tooling $rejected 'Missing explicit tool was silently replaced.'

    # Prove sibling preference despite a different CTest appearing first on PATH.
    $shadowDir = Join-Path $root 'shadow'
    [void][IO.Directory]::CreateDirectory($shadowDir)
    [IO.File]::WriteAllText((Join-Path $shadowDir 'ctest.exe'), '')
    $env:PATH = $shadowDir + ';' + $oldPath
    Assert-Tooling ((Find-GhostWireTool -Name 'ctest.exe' -Candidates @($paired) -CandidatesFirst) -eq $paired) 'PATH shadowed paired CTest.'

    # Actual cmd.exe round trip through the same environment transport as VsDevCmd.
    $batch = Join-Path $toolDir 'setup.bat'
    [IO.File]::WriteAllText($batch, "@echo off`r`nexit /b 0`r`n", [Text.Encoding]::ASCII)
    $env:GHOSTWIRE_TOOLING_TEST_VALUE = $unicode + ' = spaces & literal %TEMP% !'
    $captured = Get-GhostWireDeveloperEnvironment $batch
    Assert-Tooling ($captured['GHOSTWIRE_TOOLING_TEST_VALUE'] -ceq $env:GHOSTWIRE_TOOLING_TEST_VALUE) 'Unicode environment round trip changed bytes.'
    Assert-Tooling ($env:TEMP -ceq $originalTemp) 'Tool discovery changed TEMP.'
    Assert-Tooling ($env:TMP -ceq $originalTmp) 'Tool discovery changed TMP.'
    Assert-Tooling (-not $captured.ContainsKey('GHOSTWIRE_DEVCMD_PATH')) 'Private transport variable escaped.'
    [IO.File]::WriteAllText($batch, "@echo off`r`nexit /b 9`r`n", [Text.Encoding]::ASCII)
    $rejected = $false
    try { [void](Get-GhostWireDeveloperEnvironment $batch) } catch { $rejected = $true }
    Assert-Tooling $rejected 'Failed developer environment was accepted.'
} finally {
    $env:PATH = $oldPath
    [Environment]::SetEnvironmentVariable('GHOSTWIRE_TOOLING_TEST_VALUE', $oldValue, 'Process')
    if ([IO.Directory]::Exists($root)) { [IO.Directory]::Delete($root, $true) }
}
Write-Host "Windows tooling checks: $checks PASS"
