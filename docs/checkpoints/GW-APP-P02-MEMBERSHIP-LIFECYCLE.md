# GW-APP-P02-MEMBERSHIP-LIFECYCLE — 2026-11-01

## Current revision 2 — guided prompt navigation

**Patch author tag: GPT-6.1-Sol-Max**, owner-provided. Runtime remains
0.4.1 / GW-A04-P03-IDENTITY-HARDENING. Revision 2 is a correction inside P02;
it does not advance the service/security checkpoint sequence.

The owner's revision 1 native manual session rejected `Johnaconda` at the numeric
alias field, then also rejected `back`, `quit`, `..` and `cd` as alias data. The
validation rule was correct, but navigation was dispatched too late. Revision 2
checks standalone navigation before guided data parsing:

- `back`, `cancel`, `menu` and `..` discard pending fields/handles and show the main
  menu, preserving the selected profile and already committed service state.
- `quit` and `exit` close from every prompt through normal owned-service teardown.
- `help` explains the current field and preserves its pending context.
- Prompts distinguish a numeric alias such as `123` from a visible name such as
  `Alex`. Numeric zero is still valid alias data. Guided fields reserve navigation
  words; complete commands such as `create 123 back` retain literal word names.
- Exact deletion/forget confirmation, canonical validation, service APIs, terminal
  modes, key ownership, wire bytes, dependencies and security gates are unchanged.

Input revision 1 archive SHA-256:
`16ffe8c4d5e326eac44a0925c273e25d2ed0763d64bb022a82541d9a294de469`.
All 315 source hashes matched before extracting an isolated working copy. One
coordinator implemented/reviewed the correction; independent agents: none.

Actual new evidence: fresh Linux GCC 13.3.0 Debug/Release CLI builds **31/31** and
scoped existing CLI CTests **9/9 each PASS**. The existing identity workflow adds
help/back/quit coverage across 19 pending contexts, whitespace and synonym recovery,
the owner reproduction, zero aliases, literal navigation-word names and no private
file persistence. No new CTest target or parser harness is added. Touched C++ format,
dependency boundaries and existing registry validation/tests pass. Tool versions
remain those recorded in revision 1 below. No new sanitizer/fuzz campaign or native
revision 2 Windows/macOS run is claimed; native revision 1 evidence is separate.

Changed behavior is confined to `apps/cli/identity_session.cpp/.hpp` and the existing
`tests/integration/identity_session.cmake`. Records are updated in place. The
packager now records the author tag and creates
`GHOSTWIRE-GW-APP-P02-MEMBERSHIP-LIFECYCLE-r2-source-GPT-6.1-Sol-Max.zip`, with matched
SHA-256 filename/content. This is an attribution label, not model discovery or a
cryptographic signature. Parent archive identity and source revision are embedded.

For a clean native reproduction, retain the established x64 Visual Studio terminal,
original TEMP/TMP and configured PowerShell policy. Use a fresh source/build tree,
or `cmake --fresh --preset portable-debug` with the explicit bundled route:
`-DCMAKE_TOOLCHAIN_FILE:FILEPATH=` and `-DGW_TEST_DEPENDENCY=vendored`, plus
`-DPython3_EXECUTABLE=C:/Python/Python313/python.exe`. Then build only
`ghostwire-cli` and run `ctest --test-dir out/build/portable-debug -R '^cli-identity-session$' --output-on-failure`.
Fresh configuration avoids stale outputs when source archives preserve deterministic
timestamps. The same scoped route is available for `portable-release`.
No immediate full acceptance repeat is requested. A direct live creation sequence
is `1`, numeric alias `123`, then visible display name `Johnaconda`; `4` shows status.

Implementation confidence **0.94**, a qualitative estimate, separate from progress
or native acceptance. Official completion remains null; accepted effort floor
remains 11/110 (about 10%). Current source is cumulative revision 2; the original
implementation and evidence below are retained as historical records.

## Owner revision 1 Windows evidence — received 2026-11-01

Owner-reported filename: `GHOSTWIRE-GW-APP-P02-MEMBERSHIP-LIFECYCLE-source-6.1Sol.zip`.
The user stated it was the delivered P02 archive renamed for attribution; no host
archive checksum was posted. Evidence is reported for that source, not independent
host byte verification.

| Item | Actual supplied result |
|---|---|
| Environment | Windows host; VS2026 Developer PowerShell v18.10.3 |
| MSVC | 19.51.36260.0; compiler path ends `MSVC/14.51.36231/bin/Hostx64/x64/cl.exe` |
| Python | C:/Python/Python313/python.exe, version 3.13.13 |
| Portable Release CLI | Configure and build 31/31 PASS; identity workflow 1/1 PASS, 0.49 seconds |
| Portable Debug CLI | Fresh bundled configure and build 31/31 PASS; workflow 1/1 PASS, 0.54 seconds, repeated successfully |
| Live console | Automatic menu appears; invalid alphabetic alias is correctly rejected; navigation trap reproduced |
| Unverified | Successful manual create/rename/quit, full native suite, revision 2, macOS and A05 encoder/upstream native evidence |

The user's vcpkg configure failure named `windows-msvc-debug`; successful native
checks use `portable-*` with bundled doctest. This does not diagnose or close the
separate vcpkg failure. No account, TEMP/TMP or policy change is required here.

After returning to the PowerShell prompt, a PSReadLine `HistoryRecall`/`ReallyRender`
exception attempted `System.Console.SetCursorPosition` with negative `top`. That
stack belongs to the shell line editor; this patch does not claim to repair it or
identify its underlying trigger. Reopening the terminal is a local recovery option.
If needed, `Remove-Module PSReadLine` unloads the line editor for the current shell
session; it does not uninstall it. This command is an optional manual workaround,
never called by GhostWire or its scripts. See [Microsoft Remove-Module documentation](https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.core/remove-module?view=powershell-5.1).

## Original revision 1 delivery — historical

Source revision **1**. Runtime remains **0.4.1 / GW-A04-P03-IDENTITY-HARDENING**.
Implemented and Linux-validated; native Windows/macOS and owner acceptance of this
delivery remain pending. User requested continuation from P01 without a new native
test report; that authorizes development, not an invented Windows pass.

## Result

The local console now supports a complete prepared-membership workflow through
existing A04 services: guided preparation, inspection, label changes, device
addition/rotation/revocation, explicit public-record display and confirmed removal.
Profiles can be renamed through prompts and removed with explicit confirmation.
Console-managed profile trust pins can be listed and explicitly forgotten, including
public pins retained by other profiles after a local subject has been deleted.

| Main menu | Behavior |
|---|---|
| 1–6, 0 | P01 create/list/select/status/memberships/help/quit preserved |
| 7 | Full independently verified community key, alias and name; prepare credentials, not admission |
| 8 | Choose a prepared membership; status, rename, add/rotate/revoke device, explicit public record, confirmed delete, back |
| 9 | Rename selected profile; keys and identity unchanged |
| 10 | Request selected profile deletion; type exactly `delete` or `cancel` |

Complete commands remain available. New commands are `manage-membership [NUMBER]`,
`rename-membership NUMBER [ALIAS DISPLAY_NAME]`, `delete profile`,
`delete membership NUMBER`, `trusts` and `forget-trust TRUST_NUMBER`.
`rename` and `membership` without arguments open guided prompts. Inside the
membership panel, `0` returns to commands; `7` explicitly displays its public record
and numbered devices. `cancel` leaves a pending operation without applying it.

## Source and scope

Authoritative parent: `GHOSTWIRE-GW-APP-P01-LOCAL-CONSOLE-source.zip`, revision 1.
SHA-256: `faf190b398f30f7ab9e0315687b3fde7f25526982bcf10b1e7e0dd5bd9d2f081`.
All 314 embedded source hashes and archive checksum matched before extraction into
an isolated P02 working copy. The entire cumulative source is retained.

One coordinator implemented and reviewed the change; independent coding agents:
none. No remote operation ran. Only `apps/cli/identity_session.cpp/.hpp` and the
existing `tests/integration/identity_session.cmake` change runtime/test behavior.
Core/platform/composition/identity/crypto/codec/permissions sources, public contracts,
terminal routing, dependency pins, CMake/presets and CI are byte-identical to P01.
The remaining changes are current documentation/registry and packaging metadata.

This remains a local volatile A04 application capability. There is no new service,
executor, private-key store, wire schema, socket, remote authorization, login,
community admission, GUI or messaging implementation. A05/native/external-review,
AUTH-PRE and A11-PRE prerequisites remain open. No ADR boundary is changed.

## Ownership, confirmation and error behavior

The main-thread Session stages a bounded public key/alias and captures exact
profile/member/trust handles. An alias never authorizes selection. Device numbers
are resolved against a fresh snapshot of the captured membership. No borrowed
input view is retained between input lines. Service limits and canonical validation
remain the authority; the reader still bounds retained input to 4096 bytes.

No community pin or membership is created until all preparation fields arrive.
Once those fields are submitted, the existing pin-then-prepare service sequence
applies: a preparation failure can retain its explicit community pin. This is
reported, not silently rolled back. Community pins are separate from the
console-managed profile pins listed by `trusts`. Deleting a membership retains its
community pin and changes no other profile or membership scope.

Deletion shows the target labels and loss scope, then accepts exactly `delete`.
Wrong, oversized or partial confirmation does not authorize deletion. EOF before
completion/confirmation applies no pending mutation. A complete final input line
without a newline retains the existing reader semantics. No new Ctrl+C behavior
is claimed. Cancelling a membership action returns to commands and keeps the
selected profile. Profile deletion clears its selection and owned UI trust handles;
another profile's independent public trust pin remains intact and listable.

`forget-trust` captures the selected owner's trust handle, warns that its revision
history will be lost, and requires exactly `forget`. It changes no private keys.
Numbers belonging to another owner are rejected. A deleted/recreated alias gets
fresh identity/handles and inherits no UI trust mapping. Private state is volatile;
these operations write no profile, history, cache, backup or search-index file.

Public keys/records appear only after explicit `show` or panel public-record
requests. Rejected input and canonical service errors use fixed diagnostics.
User labels are printed only in explicitly requested console views/actions.
Input/output retain P01's UTF-8 contract and terminal adapter behavior.

## Actual validation

Linux x86_64, Ubuntu 24.04, GCC 13.3.0, CMake 4.4.4,
Ninja 1.13.2.git.kitware.jobserver-pipe-1, Python 3.12.14,
clang-format 23.1.3. Private tools from the prior pass were reused; no project
dependency or global tool installation changed.

| Validation | Debug | Release |
|---|---|---|
| CLI + existing identity/fault test targets | PASS | PASS |
| Scoped existing CTests | 11/11 PASS | 11/11 PASS |
| Identity unit runner | 41 cases / 1799 assertions PASS | 41 cases / 1799 assertions PASS |
| Identity fault runner | 11 cases / 127 assertions PASS | 11 cases / 127 assertions PASS |
| Additional host PTY workflows | 2/2 PASS | 2/2 PASS |

The existing integration suite retains P01 and original identity workflows and
adds guided preparation/member/device/rename/removal, profile deletion with another
owner's independent trust, explicit pin forgetting/reuse, malformed/bounded input,
cancelled actions and eight pending lifecycle EOF paths. Real disclosed IDs/keys
are compared: label changes preserve authority, other profiles' same-alias/same-
community credentials differ, recreated membership/profile authorities are fresh,
and membership cleanup does not change profile authority. Canonical failure after
pinning reports retained pin state. The exercised roots contain no persisted file.

Host PTY workflows exercise the new menu-to-membership/device/record/removal path
and actual terminal EOF at unconfirmed membership deletion. Both pass in each
configuration, with no persisted file. These are temporary host exercises, not
additional CTest entries; their temporary probe is excluded from the archive.

Formatting of touched C++ and the existing dependency-boundary check pass. Existing
module registry validation/tests pass. No CTest target, fixture or parser harness
is added. The unchanged vendored Botan Release source emits the already recorded
GCC `-Wstringop-overread` warning; project warnings remain errors.

No full A05 regression/fuzz campaign, new sanitizer campaign, hosted job, PowerShell
execution or native Windows/macOS run is claimed for this UI-only change.

## Reproduction and Windows workflow

From the source root, with CMake/Ninja on PATH:

```bash
cmake --preset portable-debug
cmake --build out/build/portable-debug --target ghostwire-cli ghostwire-identity-tests ghostwire-identity-fault-tests --parallel 4
ctest --test-dir out/build/portable-debug -R '^(identity-unit|identity-faults|cli-identity-session|cli-(help|version|check-config|unknown|invalid-config|missing-config|bootstrap|unicode-paths))$' --output-on-failure
```

The same commands ran for `portable-release`, using absolute private-tool paths
and CMAKE_MAKE_PROGRAM for the available host. Fresh scoped builds passed, followed
by the final incremental CLI build/check after UI text/record-panel completion.

For native confirmation at the next agreed gate, open the project in Visual Studio
Explorer → **Open in Terminal**, already at `C:\Cumpiling-shit\GhostWire`.
Confirm `(Get-Command cl.exe).Source` contains `Hostx64\x64`, preserve TEMP/TMP and
the configured PowerShell policy. Configure `portable-debug` and `portable-release`
with `-DPython3_EXECUTABLE=C:/Python/Python313/python.exe`, then use the targets and
CTest selection above for each build directory. Launch
`.\out\build\portable-release\bin\ghostwire-cli.exe`; an IDE with pipes can use
`--identity-session` explicitly. The broader established `dev-windows.ps1 -OfflineTests`
route remains available for normal milestone acceptance.

Menu smoke: create a profile; choose 7 and a verified full community public key;
choose 8, select the membership, rename it and manage its devices; use panel 7 to
inspect the numbered public devices. Cancel a deletion, then confirm a separate
removal. Check profile rename and confirmed profile removal. A restart starts empty.
This report supplies reproduction, not an immediate Windows acceptance request.

## Delivery, progress and next task

Changed runtime/test files are the three listed above. Updated records are
`AGENTS.md`, `ARCHITECTURE.md`, `README.md`, `BUILD.md`, `ROADMAP.md`, `TODO.md`,
`current-development-map.md`, `docs/CHANGELOG.md`, `docs/COMPLETED.md`, this report,
the existing module/worker JSON records and `tools/packaging/package_source.py`.
The existing packager produces `GHOSTWIRE-GW-APP-P02-MEMBERSHIP-LIFECYCLE-source.zip`,
embedded source inventory and SHA-256 sidecar, excluding tools/builds/runtime state.
Cumulative inventory, source comparison, CRC/path and checksum checks precede delivery.

Implementation confidence **0.92** is a qualitative assessment, not security
certification or completion percentage. Official progress remains null; accepted
effort floor remains **11/110, about 10%; 20% is not reached**.
The next proposed local task is bounded offline public-record exchange through
existing A04 APIs with explicit full authority/owner selection. It is unimplemented
and unassigned. A05 native evidence and independent parser review/owner disposition
still precede deferred AUTH-PRE/A06. No publication is authorized by packaging.
