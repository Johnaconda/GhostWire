# A04-P02 Windows 10 / Visual Studio 2026 acceptance

This is the native acceptance procedure for 0.4.0 /
GW-A04-P02-IDENTITY-DEVICE-TRUST. It is not a claim that Windows testing has
already passed. The owner controls acceptance and publication. P01's independent
design/source review is accepted; P02 implementation requires this new run.

## Prepare the source and tools

Verify the downloaded cumulative ZIP with `Get-FileHash -Algorithm SHA256` and
compare with the separately delivered checksum. Extract its **GhostWire** folder
as `C:\Cumpiling-shit\GhostWire`. Keep the previous checkout separately; do not
copy an old `out` directory into the new tree. No private identity migration is
needed or supported: all P02 identities are volatile.

Use Windows 10 x64 with Visual Studio 2026 Community, the Desktop development
with C++ workload, x64 MSVC and a Windows SDK. CMake/Ninja bundled with VS are
supported. Open a fresh **ordinary Windows PowerShell**. The script discovers
VS, imports Hostx64/x64 into its own process, and locates matching CMake/CTest.
No global PATH or execution-policy change is required. Keep original TEMP/TMP.

```powershell
Set-Location 'C:\Cumpiling-shit\GhostWire'
$originalTemp = $env:TEMP
$originalTmp = $env:TMP

powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\windows\dev-windows.ps1 -Action diagnostics -OfflineTests
if ($LASTEXITCODE -ne 0) { throw 'Tool discovery failed' }

powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\windows\dev-windows.ps1 -Action all -Configuration Debug -OfflineTests -Fresh
if ($LASTEXITCODE -ne 0) { throw 'Debug acceptance failed' }

powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\windows\dev-windows.ps1 -Action all -Configuration Release -OfflineTests -Fresh
if ($LASTEXITCODE -ne 0) { throw 'Release acceptance failed' }

if ($env:TEMP -ne $originalTemp -or $env:TMP -ne $originalTmp) {
    throw 'TEMP/TMP changed'
}

foreach ($role in @('desktop', 'server', 'relay', 'cli')) {
    & ".\out\build\portable-release\bin\ghostwire-$role.exe" --version
    if ($LASTEXITCODE -ne 0) { throw 'Version check failed' }
}
.\out\build\portable-release\bin\ghostwire-cli.exe --check-config --config .\examples\config\ghostwire.conf
if ($LASTEXITCODE -ne 0) { throw 'Config check failed' }
```

Expect all four identities to report **0.4.0** and
**GW-A04-P02-IDENTITY-DEVICE-TRUST** with the correct role. Expected Windows CTest
registration: **40 tests**, including `identity-unit`, `identity-faults`,
`cli-identity-session`, existing Unicode/path checks and `windows-tooling`.
Expected registration is not execution evidence; record the actual summary,
compiler/tool versions and failures. Do not substitute an ASCII TEMP workaround.

## Interact with the application

The explicit identity console is usable now. It has no disk-backed profiles,
network enrollment, listener or GUI. Exiting intentionally discards the session's
private keys and trust decisions. Normal executable startup still initializes
and shuts down without creating identities.

For Unicode terminal input/output, set UTF-8 for this PowerShell/console session:

```powershell
$utf8 = New-Object System.Text.UTF8Encoding $false
[Console]::InputEncoding = $utf8
[Console]::OutputEncoding = $utf8
$OutputEncoding = $utf8
.\out\build\portable-release\bin\ghostwire-cli.exe --identity-session
```

Type `help` to inspect the complete command grammar. Use `quit` or EOF for normal
owned cleanup. A forcibly terminated process or Ctrl+C while stdin is blocked
does not establish graceful cleanup evidence. There is no detached input thread.

First exercise two profiles with the same alias and distinct names:

```text
create 00123 John ö profile
create 00123 Second profile
profiles
select 1
show profile
rotate profile 1
show profile
select 2
show profile
diagnostics
quit
```

To exercise explicit trust within one session, create the two profiles above,
then use:

```text
select 1
trust-profile 2
recognize-profile 2 1
block-profile 2
recognize-profile 2 1
unblock-profile 2
recognize-profile 2 1
quit
```

Recognition succeeds after the explicit pin/accept, fails while blocked, and
resumes after explicit unblock. It proves local device recognition only, not a
live remote login or community admission. Changes in another isolated profile
need a new explicit `trust-profile` acceptance; recognition never imports them.

The roots and device public keys must be different despite equal aliases.
Rotating profile 1 changes its device and advances its revision; profile 2 stays
independent. `show` is an explicit disclosure of public identity information to
the terminal, not ordinary diagnostic logging. Do not paste real secrets into
the console; it has no private-key import/export command.

Start another session and run `profiles`: it must be empty. P02 does not restore
identities from disk. The executable integration test also checks malformed and
oversized commands, Unicode labels and paths, cleanup, and absence of private
files. Detailed API tests cover signed records, explicit pins, replay/conflict,
scope, revocation, cancellation, resource limits and staged failures.

## Record acceptance accurately

- Clean Debug and Release configure/build/CTest, actual counts and exit codes.
- Native interactive creation, equal aliases, profile selection, Unicode input,
  device rotation/revocation and clean quit; new session starts empty.
- Original TEMP/TMP retained; all Unicode/tooling regressions pass.
- Four executable identities and unchanged ordinary startup/config behavior.
- If a failure occurs, keep the command and full relevant error output. Do not
  bypass validation or change warnings, UTF-8 rules or tests to obtain a pass.

Both dependency modes remain available. The owner's separate VS2026 vcpkg /
doctest 2.4.12 configuration failure is **still unresolved**; successful portable
testing does not close it. If testing vcpkg, omit `-OfflineTests` or supply the
correct `-VcpkgRoot`, and record that result separately. No vcpkg update is needed
for portable acceptance. Previous hosted Windows/macOS/Linux evidence applies
only to its exact earlier source. New P02 hosted/macOS evidence remains pending
until the owner publishes and the actual jobs are inspected.

Stop after P02 acceptance/review. The next prompt is a proposal, not authority
to start P03 or add networking, persistent private keys or recovery.
