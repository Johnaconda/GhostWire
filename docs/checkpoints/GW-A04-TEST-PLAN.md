# A04 implementation acceptance plan

P02 update: the owner accepted the P01 design and authorized implementation.
The original plan below is retained for traceability. Its implemented tests are
in identity_record_tests.cpp, identity_service_tests.cpp,
identity_fault_tests.cpp, identity_lifecycle_tests.cpp and identity_session.cmake.
See [P02 execution and coverage](GW-A04-P02.md) for actual results and the
explicit limitations on collision/uint64-exhaustion and allocation coverage.
The historical P01 status below does not describe the current implementation.

Status: **planned, not executed**. These cases apply after approval of
[ADR-008](../decisions/ADR-008-IDENTITY-AND-DEVICE-TRUST.md) and its
[module contracts](../architecture/IDENTITY_DESIGN.md). There is no identity
implementation, new CTest entry or passing identity case in A04-P01.

Use real accepted CryptoProvider operations and synthetic explicit test keys.
Existing private failure libraries may support narrowly isolated faults; never
add production entropy substitution or export APIs for test convenience.

| Test ID | Behavior to exercise | Required observable result |
|---|---|---|
| ID-01 | Create two profiles; same numerical alias and Unicode display name; operate on each | Distinct independent roots and generation-checked handles; no shared trust/membership/key state |
| ID-02 | Two profiles prepare memberships for the same pinned community with identical aliases | Distinct membership/device keys and IDs; resolving an alias returns candidates rather than silently choosing/merging |
| ID-03 | One profile prepares credentials for two different communities | Different authority/device keys; one community's record cannot authorize the other's operation; no profile root/device in exported membership bytes |
| ID-04 | Copy another member's public IDs, name, device set and public keys | Record parsing may succeed; private signing authority, admission and permissions are never obtained |
| ID-05 | Delete a profile, reuse its storage slot, submit its old handle or another service's handle | invalid_handle; remaining profiles still work; slot/generation overflow is refused |
| ID-06 | Change alias or display name | Cryptographic principal, trust, membership identity and revocation state unchanged; no role/ban reset side effect |
| ID-07 | Verify a valid self-signed record for an unknown root; then explicitly pin exact authority in profile A | First verification changes no trust; pin permits scoped recognition only in A, not profile B |
| ID-08 | Wrong key, correct key but wrong kind/community, cross-purpose signature and replaced issuer | Rejected independently of a matching human alias; no untrusted key lookup/adoption |
| ID-09 | Known synthetic signing vectors for the reviewed public frame, byte/field mutation, independent fixed expected encodings | Real provider verification succeeds only for exact canonical bytes; schema arithmetic/endian/width checked, not just serializer round trips |
| ID-10 | Every truncation of a small valid record, appended bytes, oversized count/envelope, unknown suite/version/kind | Rejected before retained-state allocation; no parser overread or accepted prefix |
| ID-11 | Duplicate/unsorted entries, altered count, reversed integer endian, interval inversion/overflow | Explicit malformed-record error; receiver never repairs ambiguity before verification |
| ID-12 | Device authorization at from-1, from, until-1, until; expired outer set and narrower inner validity | Only the half-open valid interval succeeds; no expiry grace; enclosing/entry policy enforced |
| ID-13 | Inject missing/untrusted time, backward/forward jumps and native-width boundary values | Safe time errors/expiry; no subtraction overflow or steady-clock/wall-clock confusion; local revalidation never resets revision |
| ID-14 | Accept revision N then replay N-1; accept byte-identical N twice; accept a valid complete N+2 | Old denied, duplicate idempotent, full newer state accepted without missing-delta ambiguity |
| ID-15 | Two valid but different records at revision N | Context becomes conflict and denies recognition; another incoming record cannot clear it without explicit local resolution |
| ID-16 | Scoped device removal/rotation followed by use of a previously verified snapshot; empty set | Old device denied in that authority at point of use; other scopes/profiles continue; empty set denies its devices; no global-revocation success claim |
| ID-17 | Wrong owner/device attempts to issue a set, delegate another key or change a pin | Denied; device membership is not authority ownership or action permission |
| ID-18 | Root or membership-authority replacement, lost private key, attempted backup/export | No implicit trust/account/history continuity; unsupported recovery/persistence cannot report success |
| ID-19 | Full profile/membership/trust/byte/private-key limits including checked-arithmetic extremes | Immediate structured rejection, no eviction of pinned revision/conflict state, no partial object or quota leak |
| ID-20 | Entropy/provider/allocation/busy errors at staged profile creation and device rotation | Existing state/revision remains intact; staged keys cleared; no spin or partial-success handle |
| ID-21 | Cancellation before work, between crypto calls and immediately before commit; deadline expiry | No partial published result; owned cleanup; cooperative race after commit documented rather than timing-based flaky assertions |
| ID-22 | Real lifecycle adapter startup/rollback/stop/repeated shutdown and provider failure | Registration ordering honored; identity consumers gone before provider teardown; no stale borrowed pointers or detached work |
| ID-23 | Distinctive synthetic aliases, keys and record bytes passed through every error/diagnostic path | No sensitive values in stdout/stderr/log/config; only fixed enums and bounded structural counts |
| ID-24 | Normal four-executable startup/help/version/config/path checks in an isolated temp tree | Existing exit/path/UTF-8 behavior retained; no implicit identity creation, key files, private state, listener or new permissions |
| ID-25 | Same root/device signing key presented in incompatible roles through test fixtures | Rejected by role/context policy where applicable; typed authority does not expose a generic sign-anything method |
| ID-26 | Explicit trust forgetting/resolution and deletion of local profile | Local handles invalidated; no remote-triggered deletion of another profile's private keys or data |

After implementation, full Debug/Release regression runs are required because
identity is security-sensitive shared functionality. Retain A00–A03-HF01 cases,
RFC vectors, crypto faults, native processes, symlink TEMP, Unicode and Windows
tooling. Run available ASan/UBSan and targeted ownership/race checks, format,
module boundaries and source-package verification. Record actual case/assertion
totals and skips; expected registrations are not evidence.

Windows owner acceptance uses C:\Cumpiling-shit\GhostWire and the existing
ordinary PowerShell tooling with original TEMP/TMP. The owner has no personal
Mac; inspect native hosted Intel/ARM64 Debug/Release CI for the exact submitted
source instead. The owner controls pushes. Never trigger publication to obtain
validation without authorization, and never infer a later implementation's CI
result from HF01's successful commit.

Later, separately gated acceptance must test network possession/replay/channel
binding, AUTH-PRE action denial, A11-PRE protected persistence/restore/rollback,
root continuity/recovery, membership admission and multi-device synchronization.
A04's local recognition tests cannot certify those missing systems.
