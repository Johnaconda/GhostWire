# Building GhostWire A04-P03

Current runtime: **0.4.1 / GW-A04-P03-IDENTITY-HARDENING**. P03 preserves the
volatile identity console and repairs local membership deletion/revocation
semantics. The owner supplied P02 Windows Debug/Release 40/40 each and authorized
continuation. Those results are not new P03 validation.

Follow [P03 Windows acceptance](docs/checkpoints/GW-A04-P03-WINDOWS-ACCEPTANCE.md)
and [actual P03 results](docs/checkpoints/GW-A04-P03.md). Keep original TEMP/TMP.
The portable/vcpkg presets, warning policy and tool discovery remain unchanged.
`python3 tools/packaging/package_source.py` produces the cumulative
`dist/GHOSTWIRE-GW-A04-P03-IDENTITY-HARDENING-source.zip` plus SHA-256 sidecar,
excluding build products, caches and private runtime state.

## Tool requirements

C++20 compiler; CMake 3.25+; Ninja; Git for vcpkg. Python 3.9+ is needed only
for formatting, boundary checks and packaging. Test dependency: doctest 2.4.12.
The vcpkg baseline is `9e593bb18ea69cc5095e012465dcd675a822ed0d`
(upstream tag 2026.07.29). Runtime dependencies are standard C++, OS APIs and
the bundled static Botan 3.13.0 subset. Normal builds remain fully offline in
portable mode. No Python, extra Botan DLL, Perl or assembly tool is needed to
compile the included provider. Python 3.12+ on a POSIX host is needed only when
regenerating the pinned provider sources; see third_party/botan/README.md.

Presets use single-configuration Ninja. Debug and Release have separate build
directories; no Ninja Multi-Config or configuration subdirectory is required.
Do not reuse a CMake cache from another source checkout or toolchain.

## Windows 10 x64 / Visual Studio 2026 Community

Install the Desktop development with C++ workload, x64 MSVC, Windows SDK,
CMake and Ninja (Visual Studio's bundled copies are detected). The script uses
`vswhere -products * -prerelease` and `VsDevCmd.bat` without restricting the VS
major version. It imports the toolchain environment into its own process only.
An existing Hostx64/x64 Developer PowerShell is also supported. CTest is discovered
beside the selected CMake before PATH fallback, with a matching version check.
`-CTestPath` can override discovery. MSVC environment import explicitly decodes
Unicode and preserves the original process TEMP/TMP. No permanent environment
or execution-policy change is made.

A04-P02 acceptance starts with the portable/vendored mode already accepted by
the owner through A03-HF01 with original TEMP/TMP. Project root:
`C:\Cumpiling-shit\GhostWire`. Keep the actual account's default temporary paths.
Open a new ordinary PowerShell from Start (not a child
of the old workaround shell), leave TEMP/TMP unchanged, then from the source root:

```powershell
Set-Location 'C:\Cumpiling-shit\GhostWire'
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\windows\dev-windows.ps1 -Action diagnostics -OfflineTests
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\windows\dev-windows.ps1 -Action all -Configuration Debug -OfflineTests -Fresh
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\windows\dev-windows.ps1 -Action all -Configuration Release -OfflineTests -Fresh
.\out\build\portable-release\bin\ghostwire-cli.exe --version
.\out\build\portable-release\bin\ghostwire-server.exe --help
.\out\build\portable-release\bin\ghostwire-cli.exe --check-config --config .\examples\config\ghostwire.conf
```

`-Action` accepts `configure`, `build`, `test`, `run`, `clean`, `diagnostics`,
and `all` (configure + build + test). `-Configuration` accepts Debug/Release.
`-Target` accepts desktop/server/relay/cli. `clean` asks CMake to clean compiler
outputs; it does not recursively erase directories or caches. `-Fresh` is only
valid with configure/all and asks CMake to recreate its cache/CMakeFiles for the
selected build directory. It does not delete application data. Use it when
recovering from a wrong compiler, source root or architecture.

For example, in an already configured shell:

```powershell
.\tools\windows\dev-windows.ps1 -Action run -Configuration Release -OfflineTests -Target cli -AppArgs @('--version')
```

If script execution is blocked, use the child-process command above. No script
changes the global or current-user execution policy. If organization policy
blocks scripts, use the manual CMake route from an authorized developer shell.

### vcpkg selection

**Windows status: unresolved.** The owner reports doctest 2.4.12 configuration
failure on the standard vcpkg route; no complete diagnostic log establishing its
cause accompanies the acceptance report. The earlier x86 SDK/link mismatch was
recovered separately. Do not assume the two have the same cause or call vcpkg
fixed because the portable route passes or the owner created a new account.
A04-P02 preserves the pin and both modes; its crypto source is identical in both.

Precedence: explicit `-VcpkgRoot`, process `VCPKG_ROOT`, then VS's `VC/vcpkg`.
The root must contain `scripts/buildsystems/vcpkg.cmake` and `vcpkg.exe`.
For a dedicated pinned checkout (choose an appropriate directory):

```powershell
git clone https://github.com/microsoft/vcpkg.git .\out\tools\vcpkg
git -C .\out\tools\vcpkg checkout --detach 9e593bb18ea69cc5095e012465dcd675a822ed0d
.\out\tools\vcpkg\bootstrap-vcpkg.bat -disableMetrics
$env:VCPKG_ROOT = (Resolve-Path .\out\tools\vcpkg).Path
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\windows\dev-windows.ps1 -Action all -Configuration Debug -VcpkgRoot $env:VCPKG_ROOT
```

Do not update somebody else's vcpkg checkout automatically. The manifest pins
port versions; reproducible tool behavior additionally requires recording the
vcpkg tool and compiler versions. Workflows use the pinned checkout above.

### Offline test dependency

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\windows\dev-windows.ps1 -Action all -Configuration Debug -OfflineTests
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\windows\dev-windows.ps1 -Action all -Configuration Release -OfflineTests
.\out\build\portable-release\bin\ghostwire-cli.exe --version
```

This is an explicit supported mode. It verifies the included doctest header's
SHA-256; it does not silently replace a failing vcpkg build. The compiler,
CMake and Ninja still have to be installed.

### Manual Windows commands

In an x64 Developer PowerShell, with `VCPKG_ROOT` set and Ninja on PATH:

```powershell
$env:VCPKG_DISABLE_METRICS = '1'
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug --parallel 4
ctest --preset windows-msvc-debug --output-on-failure
cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release --parallel 4
ctest --preset windows-msvc-release --output-on-failure
```

The script passes the discovered absolute Ninja path as `CMAKE_MAKE_PROGRAM`
and selects the resolved MSVC compiler and linker. If CMake cannot find Ninja, use the script or pass
`-DCMAKE_MAKE_PROGRAM=<full path to ninja.exe>`. Compiler/SDK detection errors
are not fixed by changing application code. Do not mix GCC or old build caches
into the MSVC directories. VS 2026 toolset support by the chosen vcpkg tool must
be checked on the actual host; the bundled test route is independent of it.

### CTest and architecture recovery

Ordinary PowerShell need not recognize `ctest` globally. Re-run registered tests
through the discovery script:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\windows\dev-windows.ps1 -Action test -Configuration Debug -OfflineTests
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\windows\dev-windows.ps1 -Action test -Configuration Release -OfflineTests
```

HF01 checks host/target x64, compiler/linker directories, foreign linker shadowing,
incompatible x86/ARM LIB entries and required MSVC/SDK library availability before
CMake. These checks diagnose common environment mistakes; they do not validate
the machine type of every third-party library. A mixed environment must be
replaced by a fresh shell and a fresh CMake configuration. Do not manually append
SDK paths until linking happens to succeed. The script passes absolute compiler,
linker and Ninja paths; `-VisualStudioRoot` remains an explicit override.

When reproducing the unresolved vcpkg failure, omit `-OfflineTests`, run Debug and
Release separately with `-Fresh`, and retain the console error plus the relevant
doctest config logs under the selected build's `vcpkg_installed/vcpkg/blds` (or
the tool-reported buildtrees path). Record compiler/CMake/Ninja/vcpkg versions and
triplets; do not report success until both build and CTest actually pass.

## Linux and macOS

For current CI macOS support, use Xcode 26 and its C++20 libc++ with
`std::stop_source`/`std::stop_token`; the historical macos-14 AppleClang 15
runner lacks those required library types. HF02 uses hosted macos-26 ARM64/Intel
for CI runs, preserved in A02. Do not turn off `-Werror` or silently substitute a
nonstandard cancellation API to get older SDKs building. The supplied HF02 repository commit passed that hosted matrix; A02 introduces
new native code and A03 adds crypto. Both require a new native run on both
architectures. See the A03 report; prior HF02 CI is not current validation.


Install a C++20 compiler (GCC/Clang or AppleClang), CMake and Ninja through your
chosen package manager. For the exact offline dependency:

```bash
bash tools/dev.sh all portable-debug
bash tools/dev.sh all portable-release
bash tools/dev.sh all portable-sanitize  # GCC/Clang ASan+UBSan
python3 tools/check_boundaries.py
python3 tools/crypto/generate_botan.py --check # regeneration host: POSIX Python 3.12+
```

For vcpkg:

```bash
git clone https://github.com/microsoft/vcpkg.git out/tools/vcpkg
git -C out/tools/vcpkg checkout --detach 9e593bb18ea69cc5095e012465dcd675a822ed0d
out/tools/vcpkg/bootstrap-vcpkg.sh -disableMetrics
export VCPKG_ROOT="$PWD/out/tools/vcpkg"
bash tools/dev.sh all linux-debug   # macos-debug on macOS
bash tools/dev.sh all linux-release # macos-release on macOS
```

Host architecture is inferred by vcpkg on Linux/macOS. Windows presets target
x64. Other architectures are unverified. `portable-*` means no vcpkg toolchain,
not a portable-data mode. Override data locations using `--data-root`.

## Configuration and storage

`examples/config/ghostwire.conf` documents the grammar. UTF-8 text, optional
BOM, LF or CRLF; at most 64 KiB. `key=value`, surrounding whitespace trimmed;
full-line comments start with `#`. Values are literal: no quotes, shell,
environment or tilde expansion; no inline comments. Unknown keys, duplicates,
empty values, controls, invalid path UTF-8 and unsupported schema fail.

| Key | Default | Valid values |
|---|---|---|
| schema_version | 1 | 1 |
| log_level | info | debug, info, warning, error, off |
| privacy_mode | protected | direct, protected, privacy-first |
| worker_limit | 2 | 1..64 |
| queue_limit | 1024 | 16..65536 |
| data_root | OS defaults | Absolute UTF-8 path when present |

Precedence: defaults, explicit config, then `--data-root`. An invalid config
is rejected before any CLI override is applied. Privacy mode remains policy
configuration only. Worker and queue limits now bound logical work admission:
active slots respect CPU headroom, queued slots apply backpressure, and declared
memory reservations share an application budget. No worker pool is created.
The C++ composition policy defaults to 20% CPU headroom and 64 MiB of reserved
work memory; these are not new config keys or total CPU/RSS guarantees. See
[the lifecycle contract](docs/architecture/CORE_LIFECYCLE.md).
The config file must contain no passwords, keys, contacts or IP lists.

Default data/config/cache bases: Windows LocalAppData under Data/Config/Cache;
Linux XDG_DATA_HOME/XDG_CONFIG_HOME/XDG_CACHE_HOME, with absolute HOME fallbacks;
macOS Library/Application Support and Library/Caches. Each app adds
`GhostWire/<role>`. An explicit root uses `<root>/<role>/{data,config,cache}`.
Logs and runtime directories are nested under data and cache respectively.
The config directory is reserved and is not automatically read.

Directories containing symbolic links are rejected during initialization.
On systems where a chosen ancestor is a symlink, explicitly choose its real
absolute path. Existing files are never replaced. Failed initialization may
leave empty directories; shutdown does not delete them or claim atomic creation.
Permissions inherit platform policy: no secrets may be stored here until the
secure storage checkpoint supplies ACL/permission, race and recovery controls.

`--help`, `--version`, `--check-config` and `--print-paths` do not create app
directories. `--print-paths` is deliberate local disclosure. CLI diagnostics do
not echo invalid input values. No implicit file logging is enabled.

| Exit | Meaning |
|---|---|
| 0 | Command or one-shot lifecycle completed |
| 2 | Unknown/duplicate/conflicting option or malformed argument |
| 3 | Invalid/unsupported/oversized configuration |
| 4 | Config read or directory filesystem failure |
| 70 | Internal/lifecycle/output failure |

## Windows acceptance checklist

Run both `all` commands above. Record MSVC, CMake, Ninja and vcpkg versions and
the archive SHA. CTest must pass all registered tests in each configuration.
Then check normal startup and an explicit bad configuration:

```powershell
$exe = '.\out\build\portable-release\bin\ghostwire-cli.exe'
$root = Join-Path (Get-Location) 'out\acceptance\data with spaces'
& $exe --data-root $root
if ($LASTEXITCODE -ne 0) { throw 'Bootstrap failed' }
& $exe --check-config --data-root $root
if ($LASTEXITCODE -ne 0) { throw 'Config validation failed' }
$bad = Join-Path (Get-Location) 'out\acceptance\bad.conf'
'schema_version=2' | Set-Content -LiteralPath $bad -Encoding UTF8
& $exe --check-config --config $bad
if ($LASTEXITCODE -ne 3) { throw 'Expected configuration exit code 3' }
& $exe --unknown-option
if ($LASTEXITCODE -ne 2) { throw 'Expected usage exit code 2' }
```

Also validate a path containing Swedish letters using `--data-root`, check
`--print-paths` explicitly, and repeat from a fresh ordinary PowerShell to test
tool discovery. Windows Known Folder APIs, Unicode argv and PowerShell setup
remain host acceptance items. The console windows exiting immediately is
correct A01 behavior; no GUI or long-lived server is expected. The checkpoint
report preserves the original pre-acceptance checklist. Actual HF01 owner
Windows results are in
[GW-A01-HF01-HOST-ACCEPTANCE.md](docs/checkpoints/GW-A01-HF01-HOST-ACCEPTANCE.md).
The complete Unicode/default-TEMP checklist and A01 owner-reported results are
in [GW-A01-HF01-WINDOWS-ACCEPTANCE.md](docs/checkpoints/GW-A01-HF01-WINDOWS-ACCEPTANCE.md).

## Development and packaging

```bash
python3 tools/format.py --check
python3 tools/check_boundaries.py
cmake --preset portable-debug -DGW_ENABLE_CLANG_TIDY=ON # requires clang-tidy
cmake --install out/build/portable-release --prefix out/stage
python3 tools/packaging/package_source.py
```

Formatting uses the project's clang-format configuration; record your tool
version. Tests use real doctest assertions and real subprocess exit checks.
A default build can disable tests with `-DBUILD_TESTING=OFF`; production code
never links doctest. Sanitizers supplement tests; they are not a security audit.
The package script allowlists source paths, rejects symlinks/unexpected files,
verifies archive bytes, and writes a `.zip.sha256` sidecar. No build output,
local toolchain, .git directory, caches or private state is packaged.

## Retained A02 platform requirements and A03 native acceptance

The [A03 checklist](docs/checkpoints/GW-A03-NATIVE-ACCEPTANCE.md) supplements the
preserved A02/HF01 procedures. Expected A03 CTest counts are 37 on Linux/Windows
and 36 on macOS. Linux has an OS getrandom failure test; Windows has its tooling
test instead. Owner A02 Windows results were 35/35 each; that is not A03 evidence.
The owner root is `C:\Cumpiling-shit\GhostWire`; keep original TEMP/TMP.
The new platform suite launches a compiled test fixture, not a shell. It covers
Unicode paths/argv/environment, output limits, cancellation, deadlines, reaping,
handle/descriptor isolation, stop delivery and an actual child-owning service.
The Windows test runner temporarily owns a console only when needed by CTRL_BREAK
coverage. That test setup does not change the production process API contract.

The POSIX process implementation requires spawn working-directory actions and
foreign-descriptor isolation. Linux is currently supported with glibc 2.34+;
CMake detects required functions and fails clearly instead of choosing an unsafe
fallback. musl/older libc support is unvalidated and may fail this feature gate.
macOS requires the supported C++20 library and modern spawn APIs; minimum deployment
versions below the tested prior macOS 26 CI matrix are not established.

For a serial Linux validation (used in the A02 packaging environment):

```bash
cmake --preset portable-debug
cmake --build --preset portable-debug --parallel 1
ctest --preset portable-debug
cmake --preset portable-release
cmake --build --preset portable-release --parallel 1
ctest --preset portable-release
```

Normal presets and both dependency routes are retained. No process-control CLI
flag, daemon, GUI, secret-store operation or network listener was added. All four
roles remain one-shot. See [platform contracts](docs/architecture/PLATFORM_ABSTRACTIONS.md)
for bounded-wait versus destruction semantics and API-only storage capabilities.

## A03 provider and acceptance

Every configuration verifies the bundled Botan source SHA-256 and the selected
platform's generated files. No download or system-library substitution occurs.
The Windows static provider links the system bcrypt library; macOS uses the
system CommonCrypto RNG; Linux uses getrandom. Existing compiler/C++ runtime
deployment requirements still apply. Only 64-bit native MSVC Windows, Linux and
macOS are selected by this integration. Generic CPU code avoids assembly and
hardware acceleration. Native Windows/macOS A03 validation remains outstanding.

Ordinary bootstrap now initializes real entropy/crypto and fails with exit 70
if unavailable. Help/version/config/path-only commands do not initialize it.
CLI options, schema, exits, Unicode validation and directory behavior are retained.
There is no new crypto CLI or key file; test seeds are published vectors only.

Run clean Debug/Release and all registered tests; retain compiler/tool versions,
archive SHA, CTest output and unit case/assertion counts. Expected Windows:
**37/37 per configuration**, all four binaries built and CLI version 0.3.0 /
GW-A03-CRYPTOGRAPHIC-PROVIDER. Record results rather than copying that expectation.
Use [GW-A03.md](docs/checkpoints/GW-A03.md) for actual Linux commands and limits.
Do not accept A04 or persistent identities merely because primitive tests pass.


## A03-HF01 recovery acceptance

Current version: 0.3.1 / GW-A03-HF01-MACOS-RECOVERY. See
[the recovery checklist](docs/checkpoints/GW-A03-HF01-HANDOVER.md) for exact
Windows/macOS commands. Keep original TEMP/TMP. Expected CTest registrations:
Linux38, macOS37, Windows37; the Unix-only extra check verifies the crypto
lifecycle fixture under a symlink TEMP base. The native child cleanup case is
inside platform-unit. No dependency pin, preset, warning or toolchain is relaxed.

A03 owner Windows Debug/Release passed 37/37 each. Published A03 Windows CI
also passed pinned-vcpkg doctest2.4.12 on MSVC19.44; this does not close the owner's
separate local VS2026 configuration failure. All four published A03 macOS builds
compiled, then failed 34/36; this recovery's macOS branch needs a fresh run.
Historical A03 instructions/counts above describe 0.3.0. New expected counts
are not claims of native success. A11-PRE and AUTH-PRE stay mandatory.
