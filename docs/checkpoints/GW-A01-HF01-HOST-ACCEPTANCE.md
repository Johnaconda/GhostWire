# GW-A01-HF01 — Owner Windows acceptance evidence

Date: 2026-10-12. Source input: accepted
`GHOSTWIRE-GW-A01-HF01-WINDOWS-UNICODE-RECOVERY-source(1).zip`.
SHA-256: `751583206efef4170b9d28addf6c6d0f203b466c2fcb9f3aff33ac36c5da2c41`.

The owner executed tests on Windows 10 x64 with Visual Studio 2026 Community,
MSVC 19.51.36260, HostX64/x64 cl.exe (14.51.36231) and Ninja.
Project location: `C:\Cumpiling-shit\GhostWire`.
Reported original ordinary-shell environment: `TEMP` and `TMP` =
`C:\Users\WINDOW~1\AppData\Local\Temp` (Windows short path), without
manually setting either variable to an ASCII-only substitute. Note that this
run does not by itself retest the literal non-ASCII profile path spelling; the
dedicated tests separately exercise paths containing `ö` and spaces.

Commands (from project root):

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\windows\dev-windows.ps1 -Action all -Configuration Debug -OfflineTests -Fresh
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\windows\dev-windows.ps1 -Action all -Configuration Release -OfflineTests -Fresh
```

| Configuration | CMake/MSVC | CTest | Time |
|---|---|---|---|
| portable-debug | Configured and built | 34/34 PASS | 3.37 sec |
| portable-release | Configured and built | 34/34 PASS | 3.23 sec |

All four `*-unicode-paths` checks and `windows-tooling` passed in each mode.
The CLI was additionally launched from Visual Studio Debug and exited 0.
These are **owner-provided native logs**, not a native execution in the
packaging environment. Earlier ASCII-TEMP workaround remains historical only.

Following repository publication, hosted Windows and Linux Actions jobs passed;
initial hosted macOS-14 jobs failed during compilation. Those hosted outcomes
are independent evidence and must not be attributed to owner Windows 10 runs.

HF01 is accepted for Windows portable Debug and Release. Outstanding: native
owner Windows vcpkg/doctest path, macOS compile/test acceptance, full LSan and
other unrun audits. A02 authorization was granted after this acceptance; this
record does not claim A02 implementation.
