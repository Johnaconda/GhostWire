# Build-tool helpers only. Dot-sourcing defines functions without changing state.
function Find-GhostWireTool {
    param([string]$Explicit, [string]$Name, [string[]]$Candidates = @(), [switch]$CandidatesFirst)
    if ($Explicit) {
        if (-not (Test-Path -LiteralPath $Explicit -PathType Leaf)) { throw "Tool does not exist: $Explicit" }
        return (Resolve-Path -LiteralPath $Explicit).Path
    }
    if (-not $CandidatesFirst) {
        $found = Get-Command $Name -CommandType Application -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($found) { return $found.Source }
    }
    foreach ($candidate in $Candidates) {
        if ($candidate -and (Test-Path -LiteralPath $candidate -PathType Leaf)) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }
    if ($CandidatesFirst) {
        $found = Get-Command $Name -CommandType Application -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($found) { return $found.Source }
    }
    throw "Cannot find $Name. Install it or pass its explicit tool path; a permanent PATH change is not required."
}

function Get-GhostWireDeveloperEnvironment([string]$DevCmd) {
    if (-not (Test-Path -LiteralPath $DevCmd -PathType Leaf)) { throw 'Selected VsDevCmd.bat is missing.' }
    # cmd /u makes its SET output UTF-16LE. Decode explicitly rather than using
    # the active OEM/ANSI console code page to transport user/profile paths.
    $start = New-Object System.Diagnostics.ProcessStartInfo
    $start.FileName = $env:ComSpec
    $start.UseShellExecute = $false
    $start.CreateNoWindow = $true
    $start.RedirectStandardOutput = $true
    $start.StandardOutputEncoding = [Text.Encoding]::Unicode
    $start.EnvironmentVariables['GHOSTWIRE_DEVCMD_PATH'] = $DevCmd
    $start.Arguments = '/d /u /v:off /s /c ""%GHOSTWIRE_DEVCMD_PATH%" -no_logo -arch=x64 -host_arch=x64 >nul && set"'
    $process = New-Object System.Diagnostics.Process
    $process.StartInfo = $start
    try {
        if (-not $process.Start()) { throw 'Could not start MSVC environment discovery.' }
        $captured = $process.StandardOutput.ReadToEnd()
        $process.WaitForExit()
        if ($process.ExitCode -ne 0) { throw 'VsDevCmd failed to initialize Hostx64/x64. Use a fresh PowerShell session.' }
    } finally { $process.Dispose() }
    $values = @{}
    foreach ($line in ($captured -split "`r?`n")) {
        if ($line -match '^([^=]+)=(.*)$' -and $matches[1] -ne 'GHOSTWIRE_DEVCMD_PATH') {
            $values[$matches[1]] = $matches[2]
        }
    }
    return $values
}

function Get-GhostWireArchitectureIssues {
    param([string]$HostArchitecture, [string]$TargetArchitecture,
          [string]$Compiler, [string]$Linker, [string]$Libraries)
    if ($HostArchitecture -ne 'x64' -or $TargetArchitecture -ne 'x64') {
        'Developer environment must select host x64 and target x64.'
    }
    if ($Compiler -notmatch '[\\/]bin[\\/]Hostx64[\\/]x64[\\/]cl\.exe$') {
        'Selected compiler is not the Hostx64/x64 MSVC compiler.'
    }
    if ($Linker -notmatch '[\\/]bin[\\/]Hostx64[\\/]x64[\\/]link\.exe$') {
        'Selected linker is not the Hostx64/x64 MSVC linker (check PATH shadowing).'
    }
    if (($Compiler -replace '[\\/][^\\/]+$', '') -ne ($Linker -replace '[\\/][^\\/]+$', '')) {
        'Compiler and linker belong to different MSVC tool directories.'
    }
    if ([string]::IsNullOrWhiteSpace($Libraries)) { 'MSVC LIB is empty; SDK/runtime libraries are unavailable.' }
    foreach ($entry in ($Libraries -split ';')) {
        if ($entry.Trim().Trim('"') -match '[\\/](x86|arm|arm64)[\\/]?$') {
            'LIB contains an incompatible x86/ARM library directory. Do not mix SDK/runtime architectures.'
            break
        }
    }
}
