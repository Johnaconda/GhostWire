# GW-A02 — Platform abstractions

Checkpoint: **GW-A02-PLATFORM-ABSTRACTIONS**. Version: **0.2.0**.
Date: 2026-10-12. **Implemented; Linux validation recorded; independent review
and new Windows/macOS acceptance pending. Stop after A02.**

## Source continuity and authorization

The owner accepted HF01 Windows10 x64/VS2026 portable Debug and Release, 34/34
CTest each with original TEMP/TMP, Unicode/tooling passes and a clean CLI exit.
The owner explicitly authorized A02 and excluded persistent profiles, identities,
cryptography and networking. The bundled A02 prompt is retained unchanged.

Accepted input:
`GHOSTWIRE-GW-A01-HF01-WINDOWS-UNICODE-RECOVERY-source(1).zip`.
Verified SHA-256:
`751583206efef4170b9d28addf6c6d0f203b466c2fcb9f3aff33ac36c5da2c41`.
CRC, safe paths and all 106 embedded source hashes passed before extraction.
Untouched input Debug configure/build/CTest passed 33/33 locally.

During implementation the owner supplied the updated public GitHub repository.
Commit `713750550d6fa7bbde7200c494554d1c4f9e008f` was inspected and reconciled
against the accepted archive and ongoing A02 edits. Its HF02 workflow, three
unsigned-byte fixes, historical records and owner README changes are preserved.
Version-only overlaps advance to A02. No repository was reconstructed, restarted,
pushed or published. [Input evidence](GW-A02-INPUT-EVIDENCE.md) records exact
Git hashes and the prior successful hosted jobs; none are attributed to A02.

## Implemented functionality

1. **Owned real processes:** absolute native executable/cwd, validated UTF-8 argv,
   explicit replace/inherit environment, no shell/PATH interpolation, null stdin,
   discard or bounded raw stdout/stderr capture, exit/signal snapshots, cancellable
   0..60-second waits, explicit graceful/forced stop, repeat/concurrent waits and
   destruction cleanup. One joined monitor per child; no detached threads/queue.
2. **Native ownership:** Windows explicit Unicode CreateProcess, inherited-handle
   allowlist, suspended launch assigned to a kill-on-close job before resume;
   POSIX spawn cwd actions, descriptor isolation and owned process group. POSIX
   retains leader identity until descendant cleanup and direct-child reaping.
   Timeouts/cancellation never silently abandon a child. Configuration rejects
   platforms lacking required safe spawn facilities.
3. **Process-stop bridge:** explicit single owner, duplicate rejection, repeated
   close, prior POSIX handler restoration and reinstall. Restricted native callbacks
   only latch lock-free state; owner dispatch requests std::stop_token cancellation.
   All four existing one-shot bootstraps forward this token into Application at
   safe boundaries. They create no background child or long-running service.
4. **Truthful capability discovery:** local DPAPI/Keychain API entry points only,
   separate supported/available/unavailable/unsupported/error states, explicit
   disabled policy and cancellation. Linux reports unsupported. No secret operation,
   vault, persistent profile or security-gate completion; persistence_ready is false.
5. **Regression coverage:** real child/descendant tests for Unicode/quoting, invalid
   input, environment replacement/inheritance, exit/failure, bounded output, wait
   cancellation/deadline, repeated/concurrent cleanup, native stop/lifetimes,
   foreign descriptor/handle exclusion, capability states and a real child-owned
   A01 service. All previous A00/A01/HF01 tests and build routes remain.

Full operation/ownership/thread/error/resource/platform contracts:
[PLATFORM_ABSTRACTIONS](../architecture/PLATFORM_ABSTRACTIONS.md) and
[ADR-005](../decisions/ADR-005-PLATFORM-OWNERSHIP.md). The target DAG is unchanged;
no third-party runtime dependency or language change was introduced. Warnings
remain errors. vcpkg/doctest pins and all existing presets are retained.

## Actual local validation

Executor: Ubuntu 24.04.3 LTS x64; GCC 13.3.0; CMake 4.4.4;
Ninja 1.13.2.git.kitware.jobserver-pipe-1; Python 3.12.14;
clang-format 23.1.3. doctest 2.4.12, pinned vcpkg baseline
`9e593bb18ea69cc5095e012465dcd675a822ed0d`.

| Check | Actual result |
|---|---|
| Untouched accepted HF01 Debug | Configure/build PASS; 33/33 CTest |
| A02 portable Debug | Configure/build PASS; 34/34 CTest |
| A02 portable Release | Configure/build PASS; 34/34 CTest |
| A02 Linux vcpkg Debug | Dependency configure/build PASS; 34/34 CTest |
| Unit coverage in each ordinary suite | Core 67 cases / 640 assertions plus platform 18 / 191; total 85 / 831, no skips |
| ASan/UBSan, leaks explicitly disabled | Configure/build PASS; 34/34 CTest; no reported address/undefined-behavior finding |
| Default leak-enabled core test | NOT PASS: all core assertions passed, then LeakSanitizer failed to inspect /proc task state; full A02 leak check remains blocked |
| GCC ThreadSanitizer | Configure/build PASS; both complete unit suites PASS (2/2), including instrumented child fixtures; no reported race; role CLI suite not claimed under TSan |
| Format / module boundaries | PASS; existing checks and warning policy retained |
| JSON/YAML / local documentation links | PASS: 4 JSON and 6 YAML source files; all local links in 48 Markdown files resolve |
| Fresh cumulative ZIP extraction, Release build/CTest | PASS: clean configure/build; 34/34 CTest, 85 cases / 831 assertions; final compile inputs identical to the tested extraction |
| A02 Windows / macOS / hosted CI / clang-tidy | Not executed here; exact gates below |

Commands are retained in [VALIDATION](../development/VALIDATION.md); ordinary
portable Debug/Release and linux-debug each use configure, serial build, then
`ctest --preset <name> --verbose`. Windows/macOS procedures are in
[GW-A02-NATIVE-ACCEPTANCE](GW-A02-NATIVE-ACCEPTANCE.md). Expected registrations
are 34 Linux/macOS and 35 Windows; native A02 counts have not been observed.

Two initial parallel-link attempts locally left some produced executables at
mode 0644 despite successful linking. CTest reported permission denied (24 role
failures on one incremental run, one unit BAD_COMMAND on a clean parallel run).
A serial clean rebuild produced normal executables and passed all 34 checks;
subsequent validation uses serial builds. No chmod workaround, warning relaxation,
test skip or source behavioral change was used to turn that into a pass.

Default LeakSanitizer reported “Can't open /proc/.../task for reading” and a fatal
runtime error, not an application leak finding or a passing leak audit. The
separate limited run uses `ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1`.
No sanitizer suppression/disable setting is baked into project presets or CI.

## Security and known limits

- Only owned processes can be controlled. Native handles/PIDs are private;
  diagnostic IDs are not authority. No shell execution or peer-input launch path.
- Process groups/jobs manage ordinary descendants; they are not hostile-code
  containment. Child code has ordinary OS authority; POSIX escape/parent crash,
  externally stolen reaping and kernel stalls are explicit limits. Future model
  and plugin execution requires separately approved isolation and permissions.
- Captured bytes and inherited environment can contain private data. Capture is
  opt-in/bounded and never logged automatically. Error messages are fixed. No
  new private disk state, backups, temp keys, indexes, telemetry or cloud service.
- Wait deadlines/cancellation bound the caller's wait, not the OS or child CPU/RSS.
  Destruction forces and joins/reaps instead of detaching; it may wait for kernel
  cleanup. Callers retain responsibility for A01 admission and escalation policy.
- Windows graceful delivery needs a shared console. Close/logoff/shutdown and
  SIGKILL/crashes do not promise cooperative dispatch. Owners must pump dispatch
  in future event loops; A02 retains existing one-shot roles.
- Linux spawn support currently requires glibc 2.34+ facilities. musl/older libc
  and older macOS deployment targets are not established; CMake fails required
  feature checks instead of silently weakening descriptor isolation.
- Native secure API presence is not a working unlocked store, encryption service
  or vault. A11-PRE and AUTH-PRE remain mandatory and unimplemented. No A03 code.

## Outstanding acceptance

New Windows10 x64/VS2026 Debug/Release with original TEMP/TMP, macOS x64/ARM64
build/runtime, A02 hosted CI, full A02 leak checking and clang-tidy remain open.
The owner's Windows vcpkg/doctest issue is unresolved; prior hosted MSVC19.44 and
current Linux vcpkg passes do not resolve their MSVC19.51 host configuration.
No independent security/host acceptance is inferred from local test success.

The supplied HF02 commit now has actual successful Linux, Windows and macOS26
hosted jobs, unlike its preserved pre-rerun reports. That prior evidence is useful
and explicitly attributed in the input record. It does not validate new A02 code.

## Delivery and continuation

Package: `GHOSTWIRE-GW-A02-PLATFORM-ABSTRACTIONS-source.zip`, cumulative GhostWire
source with regenerated `SOURCE_MANIFEST.json`; builds, binaries, caches, .git,
credentials and temporary fixtures excluded by the existing allowlisted packager.
The ZIP contains 125 source files plus its SHA-256 manifest. CRC and every
manifest entry were verified; the final build inputs match the fresh tested
extraction. ZIP SHA-256 is supplied beside the downloadable artifact, not embedded
inside itself. [Changed files](GW-A02-CHANGED-FILES.md) compare both the accepted HF01
archive and supplied repository commit.

TODO/roadmap distinguish implementation, measured evidence and pending acceptance.
[GW-A03-PROMPT](GW-A03-PROMPT.md) is a proposal only. Stop after A02 for independent
review and Windows host acceptance; advance only after explicit new authorization.
