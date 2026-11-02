# GW-APP-P01-LOCAL-CONSOLE — 2026-11-01

Source revision **1**. Runtime remains **0.4.1 / GW-A04-P03-IDENTITY-HARDENING**;
the source delivery identifier and compiled runtime identity are separate.
Implemented and validated on Linux; new native and owner acceptance pending.

## Result and scope

Previously an argument-free CLI launch initialized crypto and exited. The real
identity experience required discovering `--identity-session` and command syntax.
Now an argument-free launch opens a local identity menu when both input and output
are terminals. It creates no profile automatically. Menu `1` prompts for alias/name
and creates/selects a real profile. `2` lists profiles with a selected marker; `3`
lists profiles and asks for a number; `4` shows selected labels, devices, revision
and prepared-membership count; `5` lists memberships; `6` shows commands; `0` exits.

`create` and `select` also open guided prompts; `cancel` leaves pending input while
preserving the selection. Complete existing commands remain available. EOF during
creation discards pending input: mutation happens only after both fields arrive.
Invalid selections are recoverable. Bare startup with either stream redirected
retains one-shot bootstrap. Explicit `--identity-session [--data-root ABS_PATH]`
still accepts files/pipes. `--bootstrap` explicitly selects one-shot startup in all
four roles and uses the existing mutually exclusive command parser. Configuration
and diagnostic commands, or other roles, do not auto-enter the console.

The supplied handover authorized local usability around existing services. This
does not advance A05/A06 or implement AUTH-PRE/protected storage. There is no new
cryptography, dependency, service, wire format, socket, persistence, GUI, login or
messaging. Existing ADRs apply. No private-state file, history, backup, search index
or cache is added. Empty directories retain their existing bootstrap behavior.

## Authoritative input and ownership

Input: `GHOSTWIRE-GW-A05-P04-UPSTREAM-INTEROP-source.zip`, revision 6.
SHA-256: `24dde54c4862edd79083dd13393e8cf3df0989dff82c4e11d6d4d72457c2eb95`.
All 311 embedded source hashes matched, with no missing/extra source. The isolated
working copy was extracted from those bytes; all cumulative sources are retained.

One coordinator implemented and reviewed the change; no independent coding agents
or external audit were used. Tests are not agent reviews. The owner remains native
Windows acceptance and publication authority. No GitHub operation ran.

## Runtime contracts

- The CLI terminal adapter borrows standard handles, changes no terminal mode or
  encoding, and confines native types to its `.cpp`. POSIX uses `isatty` on both
  streams; Windows uses `GetConsoleMode`. Unavailable/unsupported/redirected handles
  do not trigger automatic entry.
- Main-thread Session owns navigation state, at most 20 pending alias bytes and an
  explicit profile handle. The existing identity/provider graph and reverse shutdown
  order remain intact. There is no additional executor, key owner or service.
- The existing reader drains oversized lines while retaining at most 4096 bytes.
  Guided names are bounded before creation and canonically checked by IdentityService.
  Invalid input is not echoed. Cancelled/incomplete creation publishes nothing.
- List/status show labels after explicit requests. Keys/signed records still require
  explicit `show`; lifecycle logs retain fixed diagnostics. Matching aliases remain
  distinct handles with independent keys. Navigation grants no trust, admission,
  live possession or action permission. Session state disappears at exit.

## Actual validation

Linux x86_64 / Ubuntu 24.04; GCC 13.3.0; CMake 4.4.4; Ninja
1.13.2.git.kitware.jobserver-pipe-1; Python 3.12.14; clang-format 23.1.3.
Host tools were installed privately under excluded `out/host-tools`; project
dependencies, presets, warning policy, CI and native tooling are preserved.

| Check | Debug | Release |
|---|---|---|
| Four roles + `ghostwire-tests` | 40/40 build steps PASS | 40/40 build steps PASS |
| Existing scoped CTests | 34/34 PASS | 34/34 PASS |
| Core unit runner | 67 cases / 643 assertions PASS | 67 cases / 643 assertions PASS |
| Host POSIX terminal/redirection exercise | 14/14 scenarios PASS | 14/14 scenarios PASS |

Scoped CTests cover core CLI/config/lifecycle behavior, the full existing identity
workflow, guided menu/selection/status, duplicate aliases, cancellation, bad UTF-8,
rejected/oversized fields, unavailable selections, EOF during creation, no persistent
files, incompatible commands, and all four roles' help/version/config failures,
default/explicit bootstrap and Unicode paths. Existing suites are extended; no
new CTest entry, compiled fixture or parser harness is delivered.

The temporary host PTY exercise covered: automatic menu-to-profile/status/quit;
automatic console EOF; input-only, output-only and both-stream redirection; explicit
bootstrap in a terminal; terminal help/version/check-config; conflicting commands;
unknown-option redaction; and terminal desktop/server/relay startup. It found no
persistent files. These are 14 additional host scenarios, not 14 CTests. Its temporary
probe remains excluded from the package.

Touched C++ formatting and dependency-boundary checks pass. Existing module registry
validation and its tests pass. The unchanged vendored Botan Release source emits
GCC 13's `-Wstringop-overread`; project warnings remain errors. No policy is weakened.

Commands from the source root, with CMake/Ninja on PATH:

```bash
cmake --preset portable-debug
cmake --build out/build/portable-debug --target ghostwire-cli ghostwire-desktop ghostwire-server ghostwire-relay ghostwire-tests --parallel 4
ctest --test-dir out/build/portable-debug -R '^(core-unit|cli-identity-session|(desktop|server|relay|cli)-(help|version|check-config|unknown|invalid-config|missing-config|bootstrap|unicode-paths))$' --output-on-failure
```

The same commands ran for `portable-release`; the available host used absolute
private-tool paths and an explicit CMAKE_MAKE_PROGRAM override. No repeated A05 full
suite/fuzzing, new sanitizer campaign, hosted job, PowerShell or native Windows/macOS
execution is claimed. Codec, identity implementation, provider and pins are unchanged.

## Windows reproduction — next agreed gate

These are reproduction steps, not a request to repeat acceptance now. Open
`C:\Cumpiling-shit\GhostWire` in Visual Studio Explorer → **Open in Terminal**.
The terminal is already at the source root. Confirm `Hostx64\x64`, retain original
TEMP/TMP and use the configured PowerShell policy:

```powershell
(Get-Command cl.exe).Source
foreach ($cfg in 'debug', 'release') {
    cmake --preset "portable-$cfg" "-DPython3_EXECUTABLE=C:/Python/Python313/python.exe"
    if ($LASTEXITCODE) { throw "$cfg configure failed" }
    cmake --build "out/build/portable-$cfg" --target ghostwire-cli ghostwire-desktop ghostwire-server ghostwire-relay ghostwire-tests --parallel 4
    if ($LASTEXITCODE) { throw "$cfg build failed" }
    ctest --test-dir "out/build/portable-$cfg" -R '^(core-unit|cli-identity-session|(desktop|server|relay|cli)-(help|version|check-config|unknown|invalid-config|missing-config|bootstrap|unicode-paths))$' --output-on-failure
    if ($LASTEXITCODE) { throw "$cfg checks failed" }
}
.\out\build\portable-release\bin\ghostwire-cli.exe
```

Use `1`, alias `123`, name `Johna`, then `4`. Create another profile with the same
alias/different name, use `3` to select the first, then `4` and `2` to inspect its
status/marker. At a creation prompt, `cancel` returns without creating a profile.
`0` closes with cleanup; restart starts empty. `--bootstrap` exits once, while
`--help`/`--version` do not open a menu. The established `dev-windows.ps1 -OfflineTests`
route remains available in README/BUILD for broader checkpoint acceptance.

If an IDE exposes pipes, use `--identity-session` explicitly. Console input/output
retain the UTF-8 contract; the adapter changes no code page. Native code-page/IME
and accessibility qualification is outside this pass. No new Ctrl+C handler is
installed; graceful shutdown evidence is for quit and EOF.

## Changed files and next dependency

Runtime/build: `apps/cli/main.cpp`, `identity_session.cpp/.hpp`, new
`terminal.cpp/.hpp`, `src/core/bootstrap.cpp`, and `CMakeLists.txt`.
Checks: existing `tests/unit/core_tests.cpp`, `tests/integration/cli_contract.cmake`
and `tests/integration/identity_session.cmake`.
Records: `AGENTS.md`, `README.md`, `BUILD.md`, `ARCHITECTURE.md`, `ROADMAP.md`,
`TODO.md`, `current-development-map.md`, `docs/CHANGELOG.md`, `docs/COMPLETED.md`,
this report and existing module/worker JSON records. Packaging metadata:
`tools/packaging/package_source.py`.

The existing packager creates `GHOSTWIRE-GW-APP-P01-LOCAL-CONSOLE-source.zip`, an
embedded SHA-256 inventory and `.zip.sha256` sidecar. Builds, temporary probes,
downloaded tools, caches and private runtime state are excluded. Final archive
inventory/CRC/checksum validation and cumulative source comparison precede delivery.

Implementation confidence **0.92**, a qualitative assessment, not a security
probability. Native Windows/macOS and owner acceptance of this delivery remain open.
Official completion stays null; accepted floor stays 11/110 points (about 10%).
Remaining A05 native evidence and independent parser review/owner decision precede
deferred AUTH-PRE composition. No security gate, publication or 20% acceptance is
implied by this local app delivery.
