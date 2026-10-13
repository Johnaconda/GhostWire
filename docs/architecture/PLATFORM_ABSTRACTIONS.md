# A02 platform contracts

Status: implemented C++20 source; Linux execution is recorded in
[GW-A02](../checkpoints/GW-A02.md). New Windows/macOS branches need native A02
acceptance. Earlier HF02 CI evidence is not A02 validation. Public headers expose
standard C++ values and owned objects; native handles/types remain in adapter
implementation files. Test-only fixtures may use native APIs to verify behavior.

## Owned processes

`Process::launch` creates one direct child and returns a `unique_ptr<Process>`.
The object owns its native process/group/job, output pipes and one monitor
`std::jthread`. It cannot adopt a PID, launch by PATH lookup, or use a shell.
The caller owns the object; a service may own it and declare its own lifecycle
dependencies. The real child-service test exercises A01 startup/cancellation/
shutdown with this API. Ordinary executables do not launch fixture processes.

Inputs are copied before launch. `executable` and `working_directory` are
absolute native filesystem paths. Arguments and environment values are validated
UTF-8 text; the existing strict path-text validator also excludes NUL, ASCII
controls and DEL. Empty arguments/values are supported. Limits: 128 arguments,
64 environment overrides, 128 ASCII characters per environment name, 4,096
bytes per input value/path, and 64 KiB aggregate path/argument/value bytes.
Names use letters/underscore then letters/digits/underscore and duplicates are
rejected case-insensitively for portability. Windows additionally enforces its
32,767 UTF-16 command-line limit after quoting. Invalid options fail before spawn.
This bounded text interface intentionally does not represent arbitrary byte argv.

Environment replacement is the default: only explicit variables are supplied.
Inheritance requires an explicit enum and snapshots the parent environment before
overrides. An inherited snapshot is limited to 1 MiB of bytes on POSIX, or
1 Mi UTF-16 code units on Windows; explicit overrides have the separate limits
above. POSIX names remain case-sensitive at the OS boundary; Windows comparisons
are ordinal and case-insensitive. The API never changes the parent environment.
Callers must not mutate POSIX process environment concurrently with inheritance.
The executable may require explicit system/runtime variables when replacement
is chosen. Neither mode is an authorization boundary against a hostile child.

stdin is the native null device (EOF). stdout/stderr default to discard.
Opt-in capture has separate stream limits (64 KiB default, 1 MiB maximum each,
including a valid zero-byte budget). The monitor continues draining after the
limit, marks truncation and avoids pipe backpressure deadlock. Each drain slice
is bounded so an active writer does not starve exit observation. Captured bytes
are raw, potentially private, and never automatically decoded or logged.
Snapshots copy bounded retained output; callers control snapshot frequency.

Operations on a live Process are synchronized, including concurrent waits and
monitor joining; destruction must not race callers. `snapshot().identifier` is
diagnostic only and grants no process-control authority. `running` covers native
execution plus final output collection. `exit` becomes available when collection
finishes; `failure` is independently observable. `wait_for` returns a fixed,
structured error if monitoring failed, never substitutes a successful exit.
POSIX signaled exits have nonzero `signal` and code zero; normal exits preserve
the native status. Windows preserves the 32-bit exit code.

`wait_for` accepts 0..60,000 ms and a standard stop token. Expiry/cancellation
interrupts the wait only: the object still owns the child. Successful waits are
repeatable and join the completed monitor. Completed state wins over a later
cancel request. Launch checks cancellation before work and after native spawn;
if cancellation wins after spawn, RAII terminates/reaps before returning failure.
Cancellation is cooperative, not a promise that native spawn is interruptible.

`request_stop` is idempotent after successful delivery: SIGTERM to the owned
POSIX group, CTRL_BREAK to the owned Windows console group. Windows requires a
shared console; console-less children can reject graceful delivery. `terminate`
explicitly forces the owned group/job and is also repeat-safe. Owners implement
their policy: request stop, wait within the service's remaining deadline, then
choose retry or force termination. A wait timeout does not discard ownership.
Destruction forces unfinished owned work and joins/reaps; it never detaches.
Destruction/OS cleanup is not a hard wall-clock guarantee under an unresponsive
kernel. A01's cooperative shutdown and retained-dependency contracts still apply.

### Native implementation and limits

- Windows uses `CreateProcessW` with an explicit executable and CRT-compatible
  argument quoting, Unicode environment, a 64 KiB buffer hint per capture pipe,
  a three-handle inheritance allowlist,
  and a suspended child. An owned kill-on-close job is assigned before resume.
  Assignment/resume failure cleans up the suspended child. Job membership follows
  ordinary descendants; completion forces remaining job members and closes handles.
- POSIX uses `posix_spawn`, an explicit working-directory file action, an empty
  signal mask and default INT/TERM/PIPE/QUIT dispositions. Each child has its own
  process group. Linux closes foreign descriptors using spawn `addclosefrom`;
  macOS uses `POSIX_SPAWN_CLOEXEC_DEFAULT`. No fork-and-execute C++ child path is
  used in the production adapter. Linux requires the tested spawn extensions
  (glibc 2.34+); CMake fails if required functions are absent.
- A single joined monitor drains and observes exit with blocking/timed OS waits
  (20 ms idle observation interval). POSIX retains the exited leader with
  `waitid(WNOWAIT)` until group cleanup, then reaps with `waitpid`. No unrelated
  SIGCHLD handler/reaper may consume owned children; SIG_IGN/SA_NOCLDWAIT is
  rejected at launch. Unexpected ECHILD reports a failure without signaling a
  potentially reused PID. Windows retains the owned kernel handle.
- After leader exit, final pipe draining is bounded to 200 ms; still-open pipes
  are closed with truncation reported. Ordinary group/job descendants are stopped.
  A hostile POSIX child can escape a group or delegate work; parent crash can
  orphan POSIX children. This API is for trusted local executables, **not a sandbox**
  for downloaded models, plugins, attachments or peer instructions.
- Launch errors, wait errors, control errors and output failures have stable
  `ErrorReason` values and fixed messages. Paths, argv, environment, raw OS error
  strings and captured output are not inserted in errors or bootstrap logs.
  OS resource exhaustion can fail launch. Fatal process-wide allocation failure
  during monitoring is not a recoverable or hard-real-time guarantee.

Each active object costs one parent monitor thread and a child plus bounded pipe
and capture memory. There is no global process pool, queue or automatic CPU/RSS
limit. Future service owners must apply A01 admission before creating work and
account for child/monitor costs; declared budgets do not constrain a child OS.

## Process-stop bridge

`ProcessStopBridge` explicitly owns one process-wide native registration.
Install, dispatch, close and destruction belong to its owner thread. Duplicate
installation fails; repeated close succeeds. Callers may retain/copy its stop
token beyond the bridge, but cannot access its stop source. The bridge must be
destroyed before process teardown and must not race method callers. No other
component may replace the same native handlers while it owns registration.

Native POSIX INT/TERM or Windows CTRL_C/CTRL_BREAK callbacks only touch a
statically verified lock-free atomic latch. They allocate nothing, lock no mutex,
invoke no lifecycle/logger code and hold no object pointer. The owner calls
`dispatch` at safe event-loop boundaries; only there does `request_stop` invoke
C++ callbacks. Repeated events coalesce into the first event; this is not a queued
signal transport. Generation tags reject updates already based on an older latch;
teardown resets latched state. OS events concurrently in flight at reinstall are
not promised ordering. POSIX close restores previous dispositions; Windows removes
the owned handler. Restoration failure remains an error; RAII cannot silently
abandon a registration. Destruction on the wrong thread is a contract violation.

Callbacks installed on the stop token must be bounded/nonblocking and must not
destroy the bridge while dispatch is active. Windows close/logoff/shutdown events
are deliberately not acknowledged as asynchronously handled: native hard deadlines
cannot be satisfied by simply returning from a handler. POSIX SIGKILL and crashes
are also outside cooperative cleanup. No hidden dispatcher thread is created.

All four bootstrap roles install the bridge only for their existing one-shot
startup path, forward its token to Application and dispatch at safe boundaries.
Help/version/config validation/path printing retain their earlier no-write behavior.
There is no daemon loop or GUI. A future long-running owner must pump dispatch;
a signal does not automatically interrupt a blocking filesystem call.

## Secure-storage capability discovery

`probe_secure_storage` is a synchronous local native-API probe with pre/post
cancellation checks. Its result explicitly separates the provider, whether this
build supports probing it, API availability and a reason. `disabled` reports
unavailable without loading a provider. Invalid policy and loader failures are
Result errors; missing entry points report unavailable. Linux reports unsupported
because no Secret Service adapter/dependency has been adopted.

Windows loads only the system-directory crypt32 module and discovers DPAPI entry
points; macOS opens the absolute system Security framework and discovers SecItem
entry points. No secret operation is called. `available` means **API present**,
not an unlocked/keychain-ready session or a proven encrypted persistence path.
`persistence_ready` is always false in A02. There is no plaintext fallback,
credential prompt, network call, persistent profile, identity generation or vault.
A11-PRE and AUTH-PRE remain unimplemented mandatory gates.

Existing path/argv adapters, strict UTF-8 rejection, role isolation and privacy
logging remain intact. Public native filesystem paths are not serialized through
locale-dependent `path::string()`; use the existing explicit UTF-8 interface.
