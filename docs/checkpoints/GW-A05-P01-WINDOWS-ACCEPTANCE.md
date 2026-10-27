# GW-A05-P01 — Windows 10 / Visual Studio 2026 acceptance

A05-P01 contains **only design/documentation/fixtures** and package metadata.
Runtime remains **0.4.1 / GW-A04-P03-IDENTITY-HARDENING**. A05 wire codec tests
do **not** yet exist. Owner decides acceptance; do not push automatically.

Extract the delivered ZIP's single `GhostWire/` root into a new clean tree
at `C:\Cumpiling-shit\GhostWire`. Do not overlay onto a newer checkout without
diff review. Verify ZIP SHA-256 against the sidecar; compare the printed value
from `Get-FileHash` to the one delivered with this exact package.

In an ordinary Windows PowerShell (fresh process, original TEMP/TMP):

```powershell
Set-Location 'C:\Cumpiling-shit\GhostWire'
$package = '<full-path-to-GHOSTWIRE-GW-A05-P01-PROTOCOL-SCHEMA-VERSIONING-DESIGN-source.zip>'
Get-FileHash -Path $package -Algorithm SHA256 | Format-List

$oldTemp = $env:TEMP
$oldTmp = $env:TMP
.\tools\windows\dev-windows.ps1 -Action diagnostics -OfflineTests
if ($LASTEXITCODE -ne 0) { throw 'Tool discovery failed' }

.\tools\windows\dev-windows.ps1 -Action all -Configuration Debug -OfflineTests -Fresh
if ($LASTEXITCODE -ne 0) { throw 'Debug build/test failed' }

.\tools\windows\dev-windows.ps1 -Action all -Configuration Release -OfflineTests -Fresh
if ($LASTEXITCODE -ne 0) { throw 'Release build/test failed' }

if ($env:TEMP -ne $oldTemp -or $env:TMP -ne $oldTmp) { throw 'TEMP/TMP mutated' }

foreach ($role in @('desktop','server','relay','cli')) {
    & ".\out\build\portable-release\bin\ghostwire-$role.exe" --version
    if ($LASTEXITCODE -ne 0) { throw "Version failed: $role" }
}

py -3 .\tools\check_boundaries.py
if ($LASTEXITCODE -ne 0) { throw 'Boundary check failed' }
```

The previous accepted Windows baseline had **40/40 CTest** for both Debug
and Release. Because A05-P01 does not alter CMake/test definitions or executable
code, the same count is expected, **not independently proved on the owner
machine yet**. Version and checkpoint printed by all four binaries should
remain `0.4.1` and `GW-A04-P03-IDENTITY-HARDENING`. Existing opt-in
`ghostwire-cli.exe --identity-session` must still function without saving keys;
`help`, a local profile creation and `quit` suffice for a manual smoke check.

Review these design-only deliverables separately:

- `docs/architecture/PROTOCOL_SCHEMA.md` and `schemas/ghostwire-wire-v1.cddl`;
- `docs/decisions/ADR-009-CANONICAL-PROTOCOL-SERIALIZATION.md`;
- `docs/security/PROTOCOL_TEST_PLAN.md` and the design vector JSON;
- `docs/checkpoints/GW-A05-P02-PROMPT.md` and this checkpoint report.

Report compiler/toolchain, actual CTest totals, any failures and whether the
owner accepts the **design**. No successful build would validate a protocol
codec, live network authentication or encrypted communication, which are absent.
