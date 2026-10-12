# ADR-004: Owned service graph, cooperative cleanup and admission

Status: implemented for authorized GW-A01; native acceptance pending.
Date: 2026-10-11. Input A00 SHA-256:
`d07d38825ceb2c73a9ddb0eee429d42e8b85ad64439ba08fd7079399021d0025`.

Use a bounded ServiceManager with unique_ptr ownership and declared numeric
IDs. Prefer this name to service_registry because the object supplies lifecycle
ordering, not ambient lookup. Services receive explicit constructor dependencies.
The ApplicationContext owns immutable configuration/path snapshots and borrows
only documented facilities. Existing Application constructors and CLI commands
remain usable. Path initialization is a real managed service in every executable.

Use std::stop_source/token directly. A cancellation wrapper, job framework,
synthetic network service or process-stop facade would add no A01 functionality.
Use an injected monotonic Clock for deadlines and deterministic failure tests.
The one-shot command still exits; it is not converted into a fake daemon.

Shutdown requests cancellation after closing admission. Stop hooks share one
absolute deadline. Failed cleanup retains the service and its dependencies;
independent branches may finish and the caller may retry. Destruction preserves
dependency lifetime and never detaches a worker. This deliberately prioritizes
safe ownership over claiming a hard wall-clock bound on arbitrary C++ code.
A service success result is a contract that its asynchronous work is finished.

Use a small ResourceAdmission facility: capped active/queued reservations,
declared-byte accounting, move-only RAII leases and immediate backpressure.
CPU headroom is a concurrency cap; memory accounting is not allocator isolation.
Do not add a thread pool, scheduler, GPU allocation or payload queue until a
real workload requires one. The production runtime has no background workers.

No runtime dependencies, language, vcpkg baseline, warning policies, config
schema or process exit codes change. Version advances to 0.1.0 and the compiled
checkpoint identity to GW-A01-CORE-LIFECYCLE-SERVICES.

## C++ source review

WG21's [P0660R10](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0660r10.pdf)
describes cooperative stop requests, synchronous callback execution and joining
thread ownership. It does not provide arbitrary thread preemption. The later
[P1869R1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1869r1.html)
clarifies the interruptible condition-variable method names used in C++20.
These primary sources were checked during A01. The implementation uses C++20,
not the later std::execution framework. See CORE_LIFECYCLE.md for GhostWire's
specific timeout, retry and destructor contracts and tests for actual evidence.
