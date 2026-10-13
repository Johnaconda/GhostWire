# ADR-005: Owned native processes and explicit stop dispatch

Status: implemented A02 decision; checkpoint/native acceptance pending.
Date: 2026-10-12. Predecessors: ADR-004 and the accepted HF01/HF02 source.

## Context and decision

Future headless services need real process ownership, cancellation and truthful
platform capabilities without adding networking, identities or a job framework.
The adapter must not make a child executable into ambient process-control authority.

Use a noncopyable Process owned by unique_ptr, standard input values and private
native RAII handles. Windows supplies an explicit executable, quoted Unicode argv,
an inherited-handle allowlist, and a suspended process assigned to a kill-on-close
job before resume. POSIX uses posix_spawn with cwd/descriptor actions and a new
process group. Fail configure if safe spawn extensions are unavailable; do not
fall back to arbitrary inherited descriptors or post-fork C++ in a threaded host.
Dependency pins, selected language, compiler warnings and the module DAG remain.

One joined monitor per process provides bounded stdout/stderr capture and safe
reaping using ordinary blocking OS waits. No detached worker, shell interpolation,
foreign PID adoption, global process registry or unbounded output queue is added.
Wait cancellation retains ownership; forced termination is a separate action.
RAII guarantees owned cleanup rather than promising a hard bound on kernel waits.
Process groups/jobs manage ordinary descendants, not adversarial containment.

Native stop callbacks cannot safely run arbitrary C++ lifecycle callbacks.
Use one explicit registration and a private lock-free event latch, then request
standard cancellation on the owner thread during dispatch. The process-wide
native registration is an unavoidable OS boundary, not a singleton service lookup.
The existing one-shot bootstrap gains safe dispatch points without becoming a daemon.

Secure-storage discovery tests native API entry-point presence only. Its type
names the result `api_availability` and keeps persistence readiness false.
Windows DPAPI/macOS Keychain APIs being present says nothing about a future
vault's policy, unlocked account, confidentiality, backup or crash guarantees.
Linux stays unsupported until a reviewed adapter is selected. No secret writes.

## Consequences and acceptance

Callers must select trusted executables, budget child/monitor resources, avoid
concurrent environment mutation, and own deadlines/termination policy. Windows
graceful console delivery requires a shared console. POSIX requires exclusive
reaping authority for each owned child; escaped descendants are outside scope.
Native Windows/macOS A02 tests remain required even though the preceding HF02
commit now has successful hosted CI. Full contracts and evidence:
[PLATFORM_ABSTRACTIONS](../architecture/PLATFORM_ABSTRACTIONS.md),
[GW-A02](../checkpoints/GW-A02.md).

## Primary references reviewed

- https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessw
- https://learn.microsoft.com/en-us/windows/win32/procthread/job-objects
- https://learn.microsoft.com/en-us/windows/console/handlerroutine
- https://learn.microsoft.com/en-us/windows/console/setconsolectrlhandler
- https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibraryexw
- https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/posix_spawn.2.html

Linux spawn functions were also checked against the installed headers/library
through CMake's feature checks. References inform design; they do not replace
native tests. These are not cryptographic production-readiness claims.
