# Copy-paste continuation prompt: GW-A01

Use the GHOSTWIRE MASTER DEVELOPMENT CONTRACT in the supplied source.

CHECKPOINT: GW-A01-CORE-LIFECYCLE-SERVICES
AUTHORITATIVE INPUT: GHOSTWIRE-GW-A00-FOUNDATION-BOOTSTRAP-source.zip, or a newer
explicitly supplied A00 acceptance fix. Verify its SHA-256 and inspect the actual
repository. Do not reconstruct source from memory or restart the project.
A00's report distinguishes Linux validation from unverified Windows/macOS paths.

Implement A01 only after the user authorizes this prompt. Preserve the four
console targets, strict CLI/config, privacy-aware logs, path behavior and build
modes. No networking, identity generation, crypto/vault implementation or GUI.

1. Introduce an explicit ApplicationContext holding references to owned core
   services and immutable configuration/path snapshots. No global service locator.
2. Add a small owned service lifecycle mechanism with declared dependencies,
   deterministic initialization, cycle/missing/duplicate detection, rollback after
   partial failure, reverse-order shutdown and testable state transitions.
3. Inject a clock where deadlines/backoff require it. Use std::stop_token and
   bounded deadlines; define ownership of asynchronous completions before adding
   any worker. Do not introduce a heavyweight task framework.
4. Define bounded service work/admission and diagnostic snapshots where real
   behavior exists. If a job executor is needed, keep it small and owner-scoped;
   do not create fake unused subsystem services or detached background threads.
5. Make graceful process-stop integration explicit through a platform facade if
   this pass adds a long-running lifecycle mode. Keep ordinary A00 one-shot CLI
   behavior and documented exit codes. No busy polling or fabricated daemon.
6. Add meaningful tests for dependency cycles, failure after partial startup,
   cancellation, timeout/cleanup, repeated shutdown, ownership and log redaction.
   Use controlled failure fixtures, not simulated production network services.
7. Run available Debug/Release CTest and relevant sanitizer checks. Report Windows
   10/VS2026 and macOS verification honestly. Preserve vcpkg pins unless a scoped
   documented change is necessary. Do not disable warnings to conceal failures.
8. Update ARCHITECTURE/module graph/interface contracts, TODO, COMPLETED, CHANGELOG,
   source inventory and checkpoint report. Keep A11-PRE and AUTH-PRE prerequisites.
9. Deliver cumulative GHOSTWIRE-GW-A01-CORE-LIFECYCLE-SERVICES-source.zip with
   SHA-256, changed files, actual commands/results, security impact, limitations
   and an A02 proposal. Stop and await acceptance. Do not begin A02 automatically.

No remote repository creation, push, publication or external messages are
implicitly authorized. Windows final acceptance remains on the user's actual
host; a Linux build is not proof of Windows/MSVC runtime behavior.
