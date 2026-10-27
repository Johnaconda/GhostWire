# Windows PowerShell documentation security review — HF05

## Context and change

Microsoft Defender displayed `Trojan:Win32/MalUri.A!cl` against Markdown files
embedded in the earlier HF01 source archive. That is a severe detection, not
proof of malware and **not proof of a false positive**. The prior build guides
repeatedly recommended launching development scripts with an unconditional
`-ExecutionPolicy Bypass` argument. Such examples can trigger heuristics and
encourage avoidable policy overrides. No AV engine was available here.

The active BUILD guide and historical checkpoint *command examples* were changed
to invoke the local `tools/windows/dev-windows.ps1` directly under the user's
existing PowerShell policy. Existing signed scripts and policy settings remain
respected. Script execution under Windows policy is not guaranteed; if a
managed host blocks unsigned scripts, consult local/organization policy rather
than automatically bypassing it. No Defender exclusions, security-feature
disabling, remote code download or script execution have been added.

Historical checkpoint validation *results and evidence remain unchanged*, except
for the textual invocation examples, which are now current recommended usage.
Production C++ and script contents are unchanged in HF05.

## Audit boundary

- Review: the flagged Markdown paths and actual `tools/windows/*.ps1` sources.
- No claim the original or replacement archive is malware-free.
- Do not equate archive CRC / SHA-256 with AV scanning or code safety.
- Default builds retain offline Botan and fail-closed cryptographic capability
  boundaries. Protocol networking remains disabled.
- A future Windows Defender rescan is required to know whether the new archive
  avoids the specific heuristic detection; the owner may share Protection History.

## Run without policy override

From an existing Windows PowerShell session at the unpacked project root:

```powershell
.\tools\windows\dev-windows.ps1 -Action all -Configuration Debug -OfflineTests -Fresh
.\tools\windows\dev-windows.ps1 -Action all -Configuration Release -OfflineTests -Fresh
```

If execution is restricted by your current policy, do not blindly override a
security restriction; choose native CMake/Ninja or contact your administrator.
