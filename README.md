# GhostWire

**Checkpoint candidate: GW-A03-HF01-MACOS-RECOVERY · Version: 0.3.1 · C++20**

**Your identity. Your server. Your network.**

[![Linux CI](https://github.com/Johnaconda/GhostWire/actions/workflows/linux.yml/badge.svg)](https://github.com/Johnaconda/GhostWire/actions/workflows/linux.yml)
[![Windows CI](https://github.com/Johnaconda/GhostWire/actions/workflows/windows.yml/badge.svg)](https://github.com/Johnaconda/GhostWire/actions/workflows/windows.yml)
[![macOS CI](https://github.com/Johnaconda/GhostWire/actions/workflows/macos.yml/badge.svg)](https://github.com/Johnaconda/GhostWire/actions/workflows/macos.yml)

GhostWire is an open-source, privacy-first communication platform in early
foundation development. Independent hosting, decentralized discovery, encrypted
conversations and portable identities are **planned features**, not current
capabilities. See [the roadmap](ROADMAP.md) for sequencing and security gates.

The current source is a clean-room foundation for an independently hostable
communication platform. A01 extends A00 with an owned application context,
dependency-aware service startup and rollback, cooperative cancellation,
shutdown deadlines and bounded resource admission. All four executables use
the same headless core. A02 adds owned native child processes, explicit process-stop
dispatch and truthful secure-storage API discovery. A03 adds a lifecycle-owned,
pinned Botan provider for bounded volatile randomness, authenticated encryption,
public-message signatures and hashing. It is not a messaging application yet.
There is no network listener, account creation, vault, GUI, relay forwarding,
private persistence or remote AI call in this checkpoint.

## Start here

- [A03 HF01 recovery](docs/checkpoints/GW-A03-HF01.md): macOS corrections, actual tests and open native acceptance.
- [A03 host acceptance](docs/checkpoints/GW-A03-HOST-ACCEPTANCE.md): Windows owner results and the exact pushed CI failures.
- [Recovery acceptance and A04 handover](docs/checkpoints/GW-A03-HF01-HANDOVER.md): next host commands and identity design boundaries.
- [BUILD.md](BUILD.md): Windows/MSVC, Linux and macOS commands and exit codes.
- [A03 report](docs/checkpoints/GW-A03.md): implementation, actual validation and limits.
- [A03 native acceptance](docs/checkpoints/GW-A03-NATIVE-ACCEPTANCE.md): Windows/macOS procedure.
- [Crypto contract](docs/architecture/CRYPTO_PROVIDER.md) and [provider decision](docs/decisions/ADR-006-CRYPTO-PROVIDER.md): ownership, bounds, algorithms and limitations.
- [A02 report](docs/checkpoints/GW-A02.md) and [owner acceptance](docs/checkpoints/GW-A02-HOST-ACCEPTANCE.md): preserved baseline and Windows evidence.
- [Platform contracts](docs/architecture/PLATFORM_ABSTRACTIONS.md): process ownership, stop and capability limits.
- [HF02 report](docs/checkpoints/GW-A01-HF02.md): preserved macOS/CI repair record.
- [Repository CI evidence](docs/checkpoints/GW-A02-INPUT-EVIDENCE.md): verified prior commit and successful hosted runs.
- [Published Windows acceptance](docs/checkpoints/GW-A01-HF01-HOST-ACCEPTANCE.md): owner's Debug/Release evidence.
- [HF01 report](docs/checkpoints/GW-A01-HF01.md): preserved historical changes and limits.
- [Windows acceptance](docs/checkpoints/GW-A01-HF01-WINDOWS-ACCEPTANCE.md): original TEMP/TMP checklist.
- [A01 report](docs/checkpoints/GW-A01.md): preserved implementation evidence.
- [A00 report](docs/checkpoints/GW-A00.md): preserved baseline evidence.
- [ARCHITECTURE.md](ARCHITECTURE.md): ownership and dependency rules.
- [ROADMAP.md](ROADMAP.md) and [TODO.md](TODO.md): implementation sequence and work records.
- [AGENTS.md](AGENTS.md): mandatory continuation rules.
- [Lifecycle contracts](docs/architecture/CORE_LIFECYCLE.md): ownership, deadlines and budgets.
- [A02 specification](docs/checkpoints/GW-A02-PROMPT.md): preserved input specification.
- [A03 specification](docs/checkpoints/GW-A03-PROMPT.md): preserved input, subsequently authorized by the owner.
- [A04 proposal](docs/checkpoints/GW-A04-PROMPT.md): requires A03 acceptance and explicit new authorization.
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
Omit `-OfflineTests` for the preserved vcpkg route. A03 passed that route on
GitHub's Windows runner; the owner's earlier local doctest configuration issue
remains unresolved. See BUILD.md before retrying it.

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
A default invocation creates the role's empty directories, initializes the actual
crypto provider, reports its limited capability, and exits after orderly shutdown.
`--check-config` does not write or initialize crypto.
There is deliberately no implicit config search: pass `--config`.

## Privacy and maturity

Normal logs use fixed event identifiers and redact paths, addresses, identities,
secrets and content. `--print-paths` explicitly displays local paths. Debug
logging does not disable redaction. No private keys or messages are persisted.
Directory creation is not a secure-storage implementation. See the
[threat model](docs/security/THREAT_MODEL.md) before extending persistence.

Core, platform and crypto are reusable **static** libraries shared by the four
targets; no stable DLL ABI is promised. Botan 3.13.0 is a pinned BSD-2-Clause
static runtime dependency; its signed source, generated subset, notices and
regeneration script are bundled. doctest remains test-only. Ordinary builds
need no extra Botan DLL or Python. Both portable and vcpkg modes use the same
crypto source. AEAD nonce uniqueness belongs to the caller; memory locking is
best effort. Primitive tests do not establish E2EE or identity-system readiness.

Project code is licensed under MIT. Dependency licenses remain separate.
Source repository: [Johnaconda/GhostWire](https://github.com/Johnaconda/GhostWire).
The owner accepted HF01 Windows 10 x64 portable Debug/Release with 34/34 CTest
passes each using original TEMP/TMP (no ASCII override); see the host record.
The supplied GitHub commit `7137505` includes the HF02 macOS workflow and explicit
byte-conversion fixes; its Linux, Windows and macOS hosted jobs passed. Those
runs predate A02. The owner subsequently accepted A02 on Windows Debug/Release
35/35 each, original TEMP/TMP, on the win10dev account. This A03 candidate has
local Linux evidence; new Windows/macOS execution and hosted CI remain gates.
Windows owner's standard vcpkg/doctest configure failure remains a separate
local-environment issue; passing hosted CI does not prove that route on that host.
No runtime networking, persistent identity or user directory exists at this checkpoint.
