# Security policy

GhostWire A01 is a development foundation. It does not implement cryptographic
identity, E2EE, secure persistence or anonymous networking. Do not use it to
protect sensitive communication. Claims and open work are in docs/security.

## Reporting

No public remote repository or monitored security mailbox is configured yet.
Do not invent a contact address. When a remote is created, maintainers must
enable GitHub private vulnerability reporting before public release and update
this file with the verified repository URL and response ownership.

Until that channel is established, report privately to the project owner through
the channel used to obtain this source. Do not publish private keys, passwords,
private conversations, full endpoints or exploitable user data in a public issue.
Reports should identify checkpoint/hash, affected platform, minimal redacted
reproduction, impact and a proposed mitigation if known.

## Maintenance requirements

Review direct/transitive dependencies and licenses at introduction and upgrades;
record pins and provenance. Schedule advisory checks before each release and
periodically during active maintenance. Use least-privilege CI, no secrets on
untrusted PR jobs, no pull_request_target execution of submitted code, reviewed
workflow pins and reproducible source packages. Track disclosure, fixes and
regressions privately until the agreed release. Never claim an external audit
or CVE review occurred without evidence.

A00/A01 validation is functional, lifecycle and bounded-input testing, not cryptographic
certification. Independent protocol, key-management and implementation review
is mandatory before any security-ready release claim.
