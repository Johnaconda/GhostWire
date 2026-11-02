# GhostWire

**Agents: read [START-HERE.md](START-HERE.md) and [AGENTS.md](AGENTS.md) once, then
run `python3 tools/development/task_context.py --task TASK_ID`.** The short packet
routes you to relevant code/contracts and checks; do not read all reports at init.
Current source: GW-DEV-CONTEXT-BOOTSTRAP r1, author tag **GPT-6.1-Sol-Max**.
This tooling-only checkpoint preserves runtime 0.4.1 and the P02 r2 application.


> **CURRENT (2026-11-01):** P02 revision 2 fixes prompt navigation: back/cancel returns to the main menu, help explains the field, and quit/exit closes from any prompt. Numeric alias and display name are clearly separated. Owner revision 1 Windows Debug/Release CLI builds and identity workflow passed; revision 2 passes Linux Debug/Release 9/9 scoped checks each, with native confirmation pending. Patch author tag: **GPT-6.1-Sol-Max** (owner-provided). Runtime 0.4.1; accepted floor about 10%, with 20% not reached. [Current map](current-development-map.md).


**GW-APP-P02-MEMBERSHIP-LIFECYCLE · Runtime: 0.4.1 · C++20**

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

## A05-P01 proposal (runtime still 0.4.1)

The owner authorized the next **design-only** protocol-schema checkpoint.
The proposed versioned CBOR schema, decision record, golden vectors and
security test plan are in
[GW-A05-P01](docs/checkpoints/GW-A05-P01.md). They do not add a wire codec,
networking, login, secure messaging or private-key persistence; accepted A04
identity functionality and the current binary identity remain unchanged.

## A05-P02 strict in-memory codec candidate (runtime still 0.4.1)

Following owner Windows Debug and Release A05-P01 40/40 acceptance, this
checkpoint implements the isolated `GhostWire::protocol` strict deterministic
CBOR v1.0 library: only public kinds 1/2/10, borrowed input with bounded decode,
fixed diagnostics, optional transient routing hints, byte-identical A04
DeviceSet carrier and explicit caller admission. See
[implementation report](docs/checkpoints/GW-A05-P02.md),
[ADR-010 controlled exception](docs/decisions/ADR-010-RESTRICTED-WIRE-CODEC-P02.md)
and [Windows P02 acceptance](docs/checkpoints/GW-A05-P02-WINDOWS-ACCEPTANCE.md).
This library is not remote authentication, E2EE, a connection/listener or a
securely audited network interface. There is no new CLI protocol command.

## Latest A05 source checkpoint

[A05-P04-HF03](docs/checkpoints/GW-A05-P04-HF03.md) strengthens the optional
TinyCBOR upstream comparison: five frozen noncanonical CBOR examples AND five canonical CBOR examples rejected only by GhostWire schema now MUST
be rejected by the upstream parser. An offline fault-injection test verifies
that the comparison fails on broken probe outputs. Source import publication
also refuses dangling-symlink destinations. This is **not** actual TinyCBOR
execution, a production dependency, network authorization or external audit.

[A05-P04-HF02](docs/checkpoints/GW-A05-P04-HF02.md) adds bounded
interoperability-probe input, a manually opted-in pinned upstream-source fetch
and expanded frozen vectors. Native TinyCBOR execution and independent parser
audit remain unrun; **no network parsing or remote authorization is approved**.
The default runtime and offline build remain unchanged.

## Preserved checkpoint references (read only when relevant)

- [Membership lifecycle delivery](docs/checkpoints/GW-APP-P02-MEMBERSHIP-LIFECYCLE.md): complete local workflow, confirmed cleanup, actual checks and Windows reproduction.
- [Local console delivery](docs/checkpoints/GW-APP-P01-LOCAL-CONSOLE.md): launch behavior, guided menu, actual checks and Windows reproduction.
- [P03 hardening report](docs/checkpoints/GW-A04-P03.md) and [Windows acceptance](docs/checkpoints/GW-A04-P03-WINDOWS-ACCEPTANCE.md): scoped revocation/deletion repair and actual validation.
- [P02 Windows evidence](docs/checkpoints/GW-A04-P02-HOST-ACCEPTANCE.md): supplied Debug/Release 40/40 each; P03 needs a new native run.

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

## Build quickly on Windows

Open the extracted `GhostWire` project folder in Visual Studio, then use
Explorer → **Open in Terminal**. The terminal is already at the source root.
Keep original TEMP/TMP and the configured PowerShell policy. The compiler path
must contain `Hostx64\x64`. Use the supported vendored route:

```powershell
(Get-Command cl.exe).Source
.\tools\windows\dev-windows.ps1 -Action all -Configuration Debug -OfflineTests -Fresh
.\tools\windows\dev-windows.ps1 -Action all -Configuration Release -OfflineTests -Fresh
.\out\build\portable-release\bin\ghostwire-cli.exe --version
.\out\build\portable-release\bin\ghostwire-cli.exe
```

Tool discovery supports Visual Studio without hardcoded edition, year or installation paths.
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
./out/build/portable-release/bin/ghostwire-cli
```

## Executables

| Target | Eventual role | Current behavior |
|---|---|---|
| ghostwire-desktop | Desktop client composition | Console bootstrap only |
| ghostwire-server | Headless community host | Console bootstrap only |
| ghostwire-relay | Independent forwarding node | Console bootstrap only |
| ghostwire-cli | Local diagnostics and administration | Local identity menu on argument-free terminal launch; explicit session and one-shot diagnostics remain available |

All support `--help`, `--version`, `--bootstrap`, `--check-config`, `--config PATH`,
`--data-root ABS_PATH`, and `--print-paths`. No command starts a network service.
A default desktop/server/relay invocation creates the role's empty directories,
initializes the actual crypto provider and exits after orderly shutdown. The CLI
with no arguments opens the local console when both input and output are terminals.
With either stream redirected, or with explicit configuration options alone, it
keeps one-shot startup behavior. `--bootstrap` always selects one-shot startup.
`--check-config` does not write or initialize crypto.
There is deliberately no implicit config search: pass `--config`.

`ghostwire-cli --identity-session [--data-root ABS_PATH]` opens an owner-controlled
console, including when input/output are redirected or the IDE exposes pipes.
The menu accepts `1` to create a profile, `2` to list, `3` to select, `4` for
selected-profile status, `5` for prepared memberships, `6` for all commands and
`0` to quit. `create` and `select` also open the guided prompts; `cancel` leaves
a pending prompt. Existing complete commands such as `create 123 Johna` and
`select 1` remain available. Type `menu` to redisplay navigation and `quit` to leave.
At every guided prompt, `back`, `cancel`, `menu` or `..` discards pending input
and shows the main menu; `quit` or `exit` closes the console cleanly. `help` explains
the current field and keeps its pending context. Navigation words are reserved in
guided fields; use a complete command such as `create 123 back` for a literal word
display name. Numeric alias `0` remains valid data. Guided creation asks first for
a numeric alias such as `123`, then a visible display name such as `Alex`.
Menu `7` guides membership preparation using a full independently verified community
public key. Menu `8` opens a prepared membership's actions: status, rename, add/rotate/
revoke device, explicit public-record/device display, and confirmed local deletion.
Use `0` inside that panel to return to commands. Menu `9` renames the selected profile;
`10` requests its deletion. Type exactly `delete` to confirm deletion or `cancel` to
keep the identity. Deleting a profile clears its selection and owned memberships,
while other profiles and their trust decisions remain independent.

`rename-membership NUMBER [ALIAS DISPLAY_NAME]` supports guided or complete commands;
`delete profile` and `delete membership NUMBER` open the same confirmation flow.
`trusts` lists the selected profile's console-managed profile trust pins, including
pins whose local subject was deleted. `forget-trust NUMBER` requires typed `forget`
confirmation and discards that pin's revision history. It changes no private keys.
Community pins are separate: a submitted preparation failure or membership deletion
can retain its explicit community pin, as reported by the console.

Creation, pinning,
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
see its acceptance addendum. Those results do not validate P02's new code.
P02's measured results and outstanding native checks are in its report.
Windows owner's standard vcpkg/doctest configure failure remains a separate
local-environment issue; passing hosted CI does not prove that route on that host.
No runtime networking, persistent identity or public user directory exists at this checkpoint.


A05-P04-HF04 partial security checkpoint: the optional interoperability comparator now strictly verifies the GhostWire probe response format and echoed wire bytes (22 offline fault checks). Actual pinned TinyCBOR execution and independent security review remain mandatory before networking.
