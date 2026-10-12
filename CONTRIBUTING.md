# Contributing

Read AGENTS.md, ARCHITECTURE.md and the current checkpoint report first. The
current source is authoritative; never reconstruct another project's code from
memory. Work in feature/*, fix/* or security/* branches based on develop.
main contains user-accepted checkpoints only. Tag an accepted checkpoint with
its exact identifier; A00 delivery alone is not a claim of user acceptance.

Keep patches scoped, document ownership and errors, and implement complete
behavior within the authorized checkpoint. Use RAII, explicit dependencies,
C++20 and no uncontrolled globals. Do not introduce GUI/crypto/network stubs
that report success. Runtime changes require appropriate tests; protocol,
crypto, schemas, platform foundations and broad core changes require full
relevant regression validation. Report unavailable platforms honestly.

Before requesting review: build and test the relevant presets, run formatting
and module-boundary checks, update TODO/ADRs and the checkpoint report, and
identify test commands and known gaps in the PR template. Reviewers check
security invariants, input limits, cancellation, paths/encoding, failure states,
private-data logging, dependency direction and actual user-visible behavior.

Dependency changes must specify upstream source, immutable pin, hash/license,
advisory review, integration impact and rollback. Do not update all dependencies
as a side effect of a feature. GitHub workflow pins are reviewed similarly.
No real secrets or private user fixtures belong in tests or issue reports.

Do not create/push a remote, send reports to other people, or publish/tag a
release without the required owner authorization. Once a remote exists,
maintainers should require Windows/Linux/macOS build checks on main/develop,
review before merge and private security reporting. These protections are
recommendations to configure, not remotely applied settings in A00.
