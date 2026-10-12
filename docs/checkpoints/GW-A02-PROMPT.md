# Proposed continuation prompt: GW-A02

**Proposal only. Do not execute before the owner accepts A01 HF01 on Windows
with original TEMP/TMP and explicitly authorizes A02.**

CHECKPOINT: GW-A02-PLATFORM-ABSTRACTIONS

AUTHORITATIVE INPUT: `GHOSTWIRE-GW-A01-HF01-WINDOWS-UNICODE-RECOVERY-source.zip`, or a
newer explicitly supplied A01 acceptance fix. Obtain the expected SHA-256 from
the owner's delivered checksum/sidecar. Verify it before extraction. The archive
does not embed its own checksum because that would be self-referential.

Continue the actual cumulative source. Do not reconstruct, reset or replace
working implementations with stubs. Read AGENTS.md, MASTER_CONTRACT.md,
ARCHITECTURE.md, ROADMAP.md, TODO.md, DEPENDENCY_GRAPH.md, CORE_LIFECYCLE.md,
GW-A00.md, GW-A01.md, GW-A01-HF01.md, GW-A01-HF01-WINDOWS-ACCEPTANCE.md,
SECURITY_INVARIANTS.md and ADR-004. Establish which HF01 native
acceptance results the owner has actually supplied; authorization is not proof
that a platform test passed.

The owner previously validated A01 portable/vendored Debug/Release with an
ASCII TEMP workaround. HF01 fixes fixture encoding and development tooling;
verify its original-TEMP acceptance evidence rather than applying the workaround
again. Keep native paths separate from UTF-8 CLI/config strings. Preserve the
Unicode and Windows tooling tests. Windows vcpkg/doctest configuration, macOS,
full LSan, clang-tidy and hosted CI remain open unless actual new evidence closes
them; Linux or portable Windows success cannot close the vcpkg gate.

Implement a bounded platform abstraction checkpoint in C++20:

1. Review the existing real path/argument/platform adapters and preserve their
   CLI, UTF-8, role isolation and no-write validation contracts. Add concrete
   native coverage and close evidenced defects without weakening checks.
2. Define an owned process API with explicit executable/argument handling,
   working directory/environment policy, bounded output if captured, cancellation,
   exit status, deadline and handle/child cleanup contracts. Avoid shell command
   interpolation. Implement and test real owned-child behavior on available
   platforms; report unavailable runtime validation separately.
3. Add a platform process-stop bridge suitable for future long-running modes,
   with explicit installation/uninstallation ownership and testable cancellation.
   Signal/control callbacks must obey their native restrictions and marshal stop
   requests safely. Do not call logging, graph mutation or blocking C++ lifecycle
   operations from restricted native callbacks. Keep ordinary CLI one-shot
   behavior unless a separately reviewed requirement changes it.
4. Provide truthful secure-storage capability results at the platform boundary:
   distinguish supported/available/unavailable/unsupported and failure. Do not
   manufacture secure-storage success or save plaintext keys as a fallback.
   Capability discovery is not an encrypted vault or proof of A11-PRE completion.
   Concrete secret persistence needs its own approved provider/threat-model slice.
5. Keep all native types/handles in platform implementation files behind explicit
   RAII ownership. Preserve the module DAG, headless core, service dependency
   lifetimes, cancellation/admission semantics and privacy-aware diagnostics.
6. Test actual child launch/failure/cancellation/timeout/reaping, platform stop
   bridge teardown and repeated use, Unicode/spaces/invalid paths, and secure-store
   unavailable behavior. Preserve all A00/A01 tests. Run available Debug/Release,
   CTest, sanitizers and static checks. Record exact Windows 10 x64/VS2026 MSVC,
   Linux x64 and macOS x64/ARM64 evidence rather than inferring portability.
7. Update architecture/module contracts, relevant ADRs, security boundaries,
   TODO, roadmap, changelog, completed records, changed-file inventory, source
   manifest and checkpoint validation. Preserve A00/A01 history and document
   unresolved acceptance gates explicitly.
8. Deliver the complete cumulative source archive, SHA-256, actual build/test
   results, security considerations, limitations and a proposed A03 prompt.
   Stop after A02 and await owner acceptance.

No GUI, networking/listeners, discovery, messaging, crypto algorithms, identity
generation, inference/training, browser gateway or media implementation is
authorized by this proposal. Future proposals remain in FUTURE_FEATURES.md.
A11-PRE and AUTH-PRE still gate private persistence and listener/admin authority.
No new dependency, language change, mandatory cloud service, telemetry, remote
repository or push without its required justification/explicit authorization.
