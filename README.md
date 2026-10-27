# GhostWire

**Accepted implementation baseline: `GW-A05-P04-HF09` · Docs refreshed 2026-10-27 · Runtime: 0.4.1 · C++20**

> **Windows 10% build/test acceptance — PASSED 2026-10-27.** Owner-operated Windows 10 / VS2026 MSVC x64 portable Debug: **66/66 build, 50/50 CTest**, and Release: **66/66 build, 50/50 CTest**. This accepts the foundation build/test milestone, **not** the outstanding A05 protocol security gate. The upstream TinyCBOR execution and independent parser/security review remain pending. No A06 networking or private persistence is authorized.
>
> The HF09 archive is the accepted source baseline; its GitHub push/tag has **not been verified**. The human-readable coordination board is [current-development-map.md](current-development-map.md), alongside the [full implementation roadmap](ROADMAP.md) and machine-readable [module registry](docs/development/MODULE_REGISTRY.json).

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
public-message signatures and hashing. A04-P02 adds explicitly owned, volatile
profiles, independent community credentials, signed device authorization and
local trust decisions. An interactive CLI session lets you use those services.
It is not a messaging application yet. There is no network listener, remote
account enrollment, vault, GUI, relay forwarding, private persistence or AI call.

## What is implemented in HF09

- **A00–A04 foundation:** Portable project/build, bounded core lifecycle, OS/platform APIs, pinned Botan 3.13.0 volatile cryptography, isolated local profiles and explicit community/device trust in an interactive CLI session. Identity keys are **volatile**, not securely persisted. Matching human-facing aliases never merge users or membership authority.
- **A05 in-memory CBOR codec (partial):** Strict bounded, versioned public-only frames (kinds 1/2/10), protocol negotiation checks, deterministic encoding/decoding, resource ceilings, negative/fault/differential tests and offline upstream comparison tooling. HF08/HF09 repaired the Windows allocation-fault fixture; the native `protocol-allocation-fault` test now passes in both configurations.
- **Not implemented:** Live listeners, remote authentication, end-to-end messaging, persistent private-key vault, functional desktop GUI, relay forwarding, media calls, discovery or AI agent runtime.

## Verified HF09 build/test checkpoint (2026-10-27)

| Host/configuration | Build | CTest | Scope |
|---|---|---|---|
| Windows 10 x64 / MSVC Debug | **66/66 PASS** | **50/50 PASS** | User-operated portable build; Python checks included |
| Windows 10 x64 / MSVC Release | **66/66 PASS** | **50/50 PASS** | User-operated portable build; Python checks included |
| Linux GCC Debug | PASS (incremental) | **52/52 PASS** | HF09 recorded incremental full-suite regression |
| Linux GCC Release | PASS (incremental) | **52/52 PASS** | HF09 recorded incremental full-suite regression |

The Linux HF09 full-suite runs reused a validated earlier build cache; do **not** interpret them as fresh-clean HF09 build evidence. The Windows acceptance results were supplied by the owner after the HF09 source archive was generated, so some frozen registry/checkpoint JSON fields still say `pending` until a follow-up documentation/evidence synchronization commit.

See [HF09 checkpoint](docs/checkpoints/GW-A05-P04-HF09.md), [Windows acceptance procedure](docs/checkpoints/GW-A05-P04-HF09-WINDOWS-10PCT-GATE.md) and [development map](current-development-map.md). No claim is made here that the new GitHub baseline was pushed or hosted CI ran against HF09.

## Next priorities — 10% to 20%

1. **A05 upstream interoperability:** Execute a verified, pinned *actual* TinyCBOR build and compare genuine upstream results against frozen and generated positive/negative cases; distinguish CBOR parser acceptance from GhostWire-specific schema rejection.
2. **A05 independent parser security decision:** Review memory/resource/length arithmetic, malformed frames, exception handling, canonical encoding, and write an explicit retain/replace decision. **A06 remains blocked.**
3. **Windows development tooling:** Fail early on x86 MSVC and on missing Python security-check registration; provide discoverable x64 Visual Studio/CMake/CTest without hardcoding a user profile.
4. **Safety prerequisites (design only until approved):** Prepare AUTH-PRE authorization and A11-PRE protected-key-store proposals. No listeners or persistent keys just because documentation lists a task.
5. **Development coordination:** Track real versus proposed worker assignments in [current-development-map.md](current-development-map.md), [WORKER_QUEUE.json](docs/development/WORKER_QUEUE.json) and [MODULE_REGISTRY.json](docs/development/MODULE_REGISTRY.json). Proposed agent aliases are **not** claims of currently running agents.

The first Windows 10% *build/test* milestone is accepted, but project completion is still a provisional effort estimate rather than a measured feature-complete percentage. Owner-operated Windows acceptance is normally reserved for meaningful 10% milestones; targeted earlier security checks may be necessary. Historical A05-P01/P02/P04 documentation remains linked below.

## Start here — reference and historical checkpoints

- [Current development map](current-development-map.md): stage/function/TODO/worker ownership overview.
- [HF09 source checkpoint](docs/checkpoints/GW-A05-P04-HF09.md) and [HF09 Windows gate procedure](docs/checkpoints/GW-A05-P04-HF09-WINDOWS-10PCT-GATE.md): accepted foundation baseline, follow-up evidence synchronization outstanding.
- [Machine-readable module registry](docs/development/MODULE_REGISTRY.json) and [worker queue](docs/development/WORKER_QUEUE.json): existing structured tracking (owner acceptance record needs syncing).

- [P03 hardening report](docs/checkpoints/GW-A04-P03.md) and [Windows acceptance](docs/checkpoints/GW-A04-P03-WINDOWS-ACCEPTANCE.md): scoped revocation/deletion repair and actual validation.
- [P02 Windows evidence](docs/checkpoints/GW-A04-P02-HOST-ACCEPTANCE.md): historical Debug/Release 40/40 evidence; later A04 and HF09 results are recorded separately.

- [A04-P02 report](docs/checkpoints/GW-A04-P02.md): implemented functionality, actual validation, security limits and cumulative source delivery.
- [P02 Windows acceptance](docs/checkpoints/GW-A04-P02-WINDOWS-ACCEPTANCE.md): exact Windows 10 / VS2026 Debug/Release and interactive session instructions.
- [Accepted identity design](docs/decisions/ADR-008-IDENTITY-AND-DEVICE-TRUST.md) and [module contract](docs/architecture/IDENTITY_DESIGN.md): isolated profiles, scoped keys, alias collisions and recovery limits.
- [A04-P01 report](docs/checkpoints/GW-A04-P01.md): preserved design-only delivery; the owner subsequently accepted its design.
- [HF01 acceptance](docs/checkpoints/GW-A03-HF01-ACCEPTANCE.md): owner Windows 37/37 each; hosted Windows/macOS 37/37 and Linux 38/38; all nine jobs passed at exact published source.
- [P02 input specification](docs/checkpoints/GW-A04-P02-PROMPT.md): preserved proposal, subsequently authorized by the owner; current scope and stop point are recorded in the P02 report.
- [A03 HF01 recovery](docs/checkpoints/GW-A03-HF01.md): preserved delivery report, corrections and then-pending native checks; see the acceptance addendum above.
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
- [Original A04 prompt](docs/checkpoints/GW-A04-PROMPT.md): preserved specification; design accepted, bounded P02 implementation authorized.
- [Future features](docs/architecture/FUTURE_FEATURES.md): proposals, not implemented capabilities.

## Build quickly on Windows (verified HF09 portable route)

Use Visual Studio 2026's **x64** developer environment, or initialize it explicitly from a PowerShell window. In the GhostWire source root, verify `(Get-Command cl.exe).Source` contains `Hostx64\x64\cl.exe`; `Hostx86\x86` **must not** be used. Prefer `portable-debug` and `portable-release`, not the optional `windows-msvc-*` vcpkg presets (a separate vcpkg/doctest issue has existed on the host).

Python 3.9+ is needed to register the **full 50-test Windows security/development suite**; the production application binaries do not require Python to run. A missing Python interpreter may silently leave only **42 registered tests** in this HF09 source, so confirm the count. The working Windows test account uses Python 3.13.13; pass its actual executable path (not the Microsoft Store execution alias) explicitly:

```powershell
$ErrorActionPreference = 'Stop'
$python = (Get-Command python.exe).Source
if ($python -match 'WindowsApps' -or -not (Test-Path $python)) { throw 'Set $python to a real Python 3.9+ executable' }
if ((Get-Command cl.exe).Source -notlike '*Hostx64\x64\cl.exe') { throw 'Open an x64 MSVC developer terminal' }

foreach ($cfg in @('portable-debug', 'portable-release')) {
    cmake --preset $cfg --fresh "-DPython3_EXECUTABLE=$python"
    if ($LASTEXITCODE -ne 0) { throw "$cfg configure failed" }
    cmake --build --preset $cfg --parallel 4
    if ($LASTEXITCODE -ne 0) { throw "$cfg build failed" }
    ctest --preset $cfg --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw "$cfg CTest failed" }
}
```

When opening a *normal* PowerShell, first initialize the Visual Studio x64 developer shell (using your installed VS path); otherwise `cl`, `cmake` and `ctest` may be missing. See [BUILD.md](BUILD.md) for the supported scripted and alternative vcpkg routes. This documentation-only update does not change CMake detection behavior.

On Linux/macOS with CMake, Ninja and a C++20 compiler:

```bash
bash tools/dev.sh all portable-debug
bash tools/dev.sh all portable-release
./out/build/portable-release/bin/ghostwire-cli --version
./out/build/portable-release/bin/ghostwire-cli --identity-session
```

## Executables

| Target | Eventual role | Current behavior |
|---|---|---|
| ghostwire-desktop | Desktop client composition | Console bootstrap only |
| ghostwire-server | Headless community host | Console bootstrap only |
| ghostwire-relay | Independent forwarding node | Console bootstrap only |
| ghostwire-cli | Local diagnostics and administration | Existing bootstrap/config checking plus explicit interactive volatile identity session |

All support `--help`, `--version`, `--check-config`, `--config PATH`,
`--data-root ABS_PATH`, and `--print-paths`. No command starts a network service.
A default invocation creates the role's empty directories, initializes the actual
crypto provider, reports its limited capability, and exits after orderly shutdown.
`--check-config` does not write or initialize crypto.
There is deliberately no implicit config search: pass `--config`.

`ghostwire-cli --identity-session [--data-root ABS_PATH]` opens an owner-controlled
console. Enter `help` for its commands and `quit` to leave. Creation, pinning,
record acceptance and device changes are explicit operations on selected local
handles; matching aliases never select or merge identities. Public identity data
is displayed only through explicit commands. Preparing community credentials
does not join a server. Keys, profiles and trust decisions exist only until this
session closes; there is no save, private-key export or recovery operation.

## Privacy and maturity

Normal logs use fixed event identifiers and redact paths, addresses, identities,
secrets and content. `--print-paths` explicitly displays local paths. Debug
logging does not disable redaction. No private keys or messages are persisted.
Directory creation is not a secure-storage implementation. See the
[threat model](docs/security/THREAT_MODEL.md) before extending persistence.

Core, platform, crypto and identity are reusable **static** libraries. A separate
composition library connects identity to the lifecycle without a dependency
cycle; ordinary executables create no profile. No stable DLL ABI is promised.
Botan 3.13.0 is a pinned BSD-2-Clause
static runtime dependency; its signed source, generated subset, notices and
regeneration script are bundled. doctest remains test-only. Ordinary builds
need no extra Botan DLL or Python. Both portable and vcpkg modes use the same
crypto source. AEAD nonce uniqueness belongs to the caller; memory locking is
best effort. Signature verification and local device recognition do not prove
live remote possession, community admission, action permission or E2EE readiness.

Project code is licensed under MIT. Dependency licenses remain separate.
Source repository: [Johnaconda/GhostWire](https://github.com/Johnaconda/GhostWire).
Historical acceptance: the owner accepted A01-HF01 Windows 10 x64 portable Debug/Release with 34/34 CTest
passes each using original TEMP/TMP (no ASCII override); see the host record.
The supplied GitHub commit `7137505` includes the HF02 macOS workflow and explicit
byte-conversion fixes; its Linux, Windows and macOS hosted jobs passed. Those
runs predate A02. The owner subsequently accepted A02 on Windows Debug/Release
35/35 each, original TEMP/TMP, on the win10dev account. A03-HF01 subsequently
passed owner Windows 37/37 each and all nine hosted Windows/Linux/macOS jobs;
see its acceptance addendum. These are historical checkpoints. HF09 has separate Windows Debug and Release
50/50 native acceptance, recorded above; do not infer hosted CI from those local results.
Windows owner's standard vcpkg/doctest configure failure remains a separate
local-environment issue; passing hosted CI does not prove that route on that host.
No runtime networking, persistent identity or public user directory exists at HF09.

Historical A05-P04-HF04 partial security checkpoint: the optional interoperability comparator now strictly verifies the GhostWire probe response format and echoed wire bytes (22 offline fault checks). Actual pinned TinyCBOR execution and independent security review remain mandatory before networking.
