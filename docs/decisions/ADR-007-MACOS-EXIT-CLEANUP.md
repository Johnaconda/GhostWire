# ADR-007: Preserve owned-group cleanup on Darwin

Date: 2026-10-14. Status: implemented candidate; native acceptance pending.
This refines ADR-005 without changing its public API or ownership guarantees.

## Evidence and diagnosis

A03's first inspected macOS26 ARM64/Intel Debug/Release runs compiled, but each
failed eleven platform wait/cleanup cases. The descendant case passed. The
original assertions did not print the structured failure reason, so the precise
native errno was not observed in that log.

Apple's published XNU `killpg1` implementation excludes zombie group members,
then returns EPERM when its POSIX path signals no member. GhostWire deliberately
retains the exited group leader using waitid(WNOWAIT) to protect the PID/PGID
until cleanup. Signaling such a zombie-only group was therefore incorrectly
treated as a control failure. This source-level diagnosis matches the observed
case pattern; the repaired native branch still needs execution evidence.

## Decision

Keep group cleanup before reaping. Do not switch to reaping first, adopt PIDs,
use a legacy signaling call that hides permission failures, or accept every EPERM.
The synchronized POSIX adapter keeps ordinary kill success and ESRCH handling.
Only on Darwin and only after EPERM:

1. Confirm the exact owned leader has exited using waitid with WNOWAIT/WNOHANG.
2. Request KERN_PROC_PGRP records for that owned group into 64 fixed-size native
   records. There is no allocation retry, global process scan result or queue.
3. Require a complete successful result, correctly sized records, matching group
   IDs, the unreaped leader's presence and zombie state for every member.
4. Only that condition counts as already-clean. Any live process, missing leader,
   query error or over-capacity result preserves process_control_failed.

The ownership mutex serializes this decision with control and reaping. The
existing no-external-reaper contract remains mandatory. Native structures stay
inside process.cpp and are never returned, logged, saved or used as identities.
There is no new dependency; sysctl uses the existing Darwin system runtime.

The 64-record cap applies only to the exceptional inspection, not normal launch
or successful group signaling. A larger zombie-only group conservatively reports
failure. Native kernel calls remain subject to the existing cooperative deadline
limits. This adapter still supervises trusted local executables, not hostile
children that escape groups or change credentials.

## Separate test-fixture correction

The crypto lifecycle fixture used temp_directory_path directly, unlike the
existing core/platform fixtures. macOS TEMP commonly traverses /var's symlink.
The application correctly rejects a symlink component in application directories.
Resolve only the trusted fixture base with filesystem::canonical before composing
its fresh test path. Do not relax or silently canonicalize production inputs.

A Linux subprocess using a symlink TMPDIR reproduces the original failure.
The new Unix CTest entry passes the same layout to the real lifecycle case and
checks that the intended case actually executed. A new real-child case exercises
capture/discard, repeated short-lived launches, exact exit status, failure state,
repeated cleanup/wait and reaping. Existing descendant, signal, cancellation and
foreign-descriptor tests remain mandatory. No native test is skipped to obtain a pass.

## Primary source review

Reviewed 2026-10-14; these explain the implementation, not a native test result:

- [Apple XNU kern_sig.c: kill and killpg1](https://github.com/apple-oss-distributions/xnu/blob/main/bsd/kern/kern_sig.c)
- [Apple XNU kern_exit.c: waitid_nocancel](https://github.com/apple-oss-distributions/xnu/blob/main/bsd/kern/kern_exit.c)
- [Apple XNU kern_sysctl.c: sysctl_prochandle, group filter and zombie enumeration](https://github.com/apple-oss-distributions/xnu/blob/main/bsd/kern/kern_sysctl.c)
- [Apple XNU sysctl.h: KERN_PROC_PGRP and kinfo_proc](https://github.com/apple-oss-distributions/xnu/blob/main/bsd/sys/sysctl.h)
