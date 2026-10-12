# GhostWire

**Checkpoint: GW-A01-HF01-WINDOWS-UNICODE-RECOVERY · Version: 0.1.1 · C++20**

A clean-room foundation for an independently hostable communication platform.
A01 extends A00 with an owned application context, dependency-aware service
startup and rollback, cooperative cancellation, shutdown deadlines and bounded
resource admission. All four executables use the same headless core. It is
not a messaging application yet. There is no network listener, account creation,
cryptography, vault, GUI, relay forwarding, or remote AI call in this checkpoint.

## Start here

- [BUILD.md](BUILD.md): Windows/MSVC, Linux and macOS commands and exit codes.
- [HF01 report](docs/checkpoints/GW-A01-HF01.md): current changes, validation and limits.
- [Windows acceptance](docs/checkpoints/GW-A01-HF01-WINDOWS-ACCEPTANCE.md): original TEMP/TMP checklist.
- [A01 report](docs/checkpoints/GW-A01.md): preserved implementation evidence.
- [A00 report](docs/checkpoints/GW-A00.md): preserved baseline evidence.
- [ARCHITECTURE.md](ARCHITECTURE.md): ownership and dependency rules.
- [ROADMAP.md](ROADMAP.md) and [TODO.md](TODO.md): implementation sequence and work records.
- [AGENTS.md](AGENTS.md): mandatory continuation rules.
- [Lifecycle contracts](docs/architecture/CORE_LIFECYCLE.md): ownership, deadlines and budgets.
- [A02 proposal](docs/checkpoints/GW-A02-PROMPT.md): requires acceptance and new authorization.
- [Future features](docs/architecture/FUTURE_FEATURES.md): proposals, not implemented capabilities.

## Build quickly on Windows

From the extracted `GhostWire` directory, use a fresh ordinary PowerShell with
the original TEMP/TMP. Start with the explicitly supported vendored route:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\windows\dev-windows.ps1 -Action all -Configuration Debug -OfflineTests -Fresh
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\windows\dev-windows.ps1 -Action all -Configuration Release -OfflineTests -Fresh
.\out\build\portable-release\bin\ghostwire-cli.exe --version
```

The execution-policy override applies only to the child process. Tool discovery
supports Visual Studio without hardcoded edition, year or installation paths.
Pass `-VcpkgRoot`, `-VisualStudioRoot`, `-CMakePath`, `-CTestPath` or `-NinjaPath`
when needed. The script pairs CTest with CMake and diagnoses mixed MSVC tools
and x86/x64 libraries. `-Fresh` resets only the selected CMake configuration
cache. No permanent PATH/execution-policy change or temp-directory workaround.
Omit `-OfflineTests` for the preserved vcpkg route; its reported Windows doctest
configuration failure remains unresolved. See BUILD.md before retrying it.

On Linux/macOS with CMake, Ninja and a C++20 compiler:

```bash
bash tools/dev.sh all portable-debug
bash tools/dev.sh all portable-release
./out/build/portable-release/bin/ghostwire-cli --version
```

## Executables

| Target | Eventual role | Current behavior |
|---|---|---|
| ghostwire-desktop | Desktop client composition | Console bootstrap only |
| ghostwire-server | Headless community host | Console bootstrap only |
| ghostwire-relay | Independent forwarding node | Console bootstrap only |
| ghostwire-cli | Local diagnostics and administration | Console bootstrap and config checking |

All support `--help`, `--version`, `--check-config`, `--config PATH`,
`--data-root ABS_PATH`, and `--print-paths`. No command starts a network service.
A default invocation creates the role's empty directories, reports its limited
capability, and exits after orderly shutdown. `--check-config` does not write.
There is deliberately no implicit config search: pass `--config`.

## Privacy and maturity

Normal logs use fixed event identifiers and redact paths, addresses, identities,
secrets and content. `--print-paths` explicitly displays local paths. Debug
logging does not disable redaction. No private keys or messages are persisted.
Directory creation is not a secure-storage implementation. See the
[threat model](docs/security/THREAT_MODEL.md) before extending persistence.

The core is one reusable **static** library shared by the four targets; A01 does
not promise a stable DLL ABI. The shipped source includes an upstream test-only
header with its MIT notice and checksum. The production executables have no
third-party application dependency.

Project code is licensed under MIT. Dependency licenses remain separate.
No remote repository has been created or pushed. The owner validated A01 on
Windows with the ASCII temp workaround. HF01 without that workaround and macOS
remain unverified; configured CI is not evidence that CI ran.
