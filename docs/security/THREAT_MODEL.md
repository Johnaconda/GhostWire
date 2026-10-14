# Initial threat model

Status: design baseline, not an audited secure product. A00/A01 accept local CLI
and nonsecret configuration. A02 adds owned native processes and API discovery.
A03 adds volatile cryptographic operations; no persistent identity, private store,
message protocol or network connection exists. Historical sections retain scope.

## Assets, principals and scope

Assets: root identity and recovery keys, device keys, session/epoch state,
plaintext messages/files, encrypted vaults, contact graph, routing metadata,
resource budgets, independently hosted availability and trustworthy updates.
Principals: local owner, approved devices/contacts, community admins, relays,
discovery/mailbox/storage operators, plugins/AI providers, software distributors
and untrusted peers. None automatically inherits another principal's authority.

Trust boundaries are detailed in BOUNDARY_MAP.md. The user's uncompromised
endpoint and authorized UI are trusted for plaintext access while unlocked.
Attackers can send arbitrary packets/attachments, run servers/relays/discovery
nodes, create unlimited identities, steal encrypted backups, replay old state,
or observe some network paths. A malicious local administrator, kernel or
compromised unlocked client can observe keys/plaintext; module separation cannot
promise otherwise. External cameras and already-copied plaintext are outside
technical erasure/capture guarantees.

## Threats and required responses

| Threat | Attacker capability / consequence | Design response and acceptance gate |
|---|---|---|
| Malicious peer | Malformed input, malicious commands, flood | Strict versioned bounded parsers, deny-by-default authorization, fuzzing; A05/A06 |
| Compromised community server | Substitute identities, withhold/reorder traffic | Verified identities, membership checks, E2EE where selected; availability loss remains possible; A04/A13/A14 |
| Malicious relay | Observe timing/addresses, drop/replay traffic | End-to-end authentication, bounded retries, route trust separation; no full anonymity claim; A06/A23 |
| Offline password attack | Unlimited trials on copied vault | Reviewed memory-hard KDF with versioned salt/parameters, password policy and protected device keys; A11 |
| Impersonation | Duplicate aliases/names, substitute public keys | Cryptographic ID verification, out-of-band contact verification, explicit rotation trust; A04/A10 |
| Key theft | Stolen file/backup or exposed memory | Distinct root/device/vault lifecycles, protected key handles, revocation/recovery design; A03/A04/A11 |
| Device compromise | Read unlocked process and input | Revoke affected device, rekey, disclose limits; no promise to protect an already compromised endpoint; A12/A23 |
| Malicious attachment | Path traversal, archive/decode bombs, executable payload | Inert storage, bounded parsing/decoding, quotas, quarantine, no auto-execution; A15/A19 |
| Denial of service | Queue/memory/disk/bandwidth exhaustion | Per-peer and global limits, reservations, fair backpressure, cancellation; A01 onward |
| Replay/dedup abuse | Repeat old messages, exhaust dedup store | Authenticated message IDs/epochs, bounded retention and replay windows, atomic durable ratchet advancement; A05/A08 |
| Traffic analysis | Correlate times, sizes and endpoints | Minimize metadata; document relay observer model, future independent multi-hop evaluation; A06/A24 |
| Discovery poisoning | Bogus/expired reachability, malicious bootstrap | Signature/trust checks, expiries, clock-skew bounds, bounded search, manual peer escape hatch; A07 |
| Sybil/eclipse attack | Many keys/nodes, isolate a target | Diverse independent bootstrap, budgets, scoped authorization/invites, detect inconsistency; no “one human per key” claim; A07/A21 |
| Lockout abuse | Bad unlock attempts or rolled-back counters | Local-only policy, recoverable cooldown, monotonic timing, tamper/rollback analysis; remote messages cannot wipe keys; A12 |
| Backup theft/rollback | Old encrypted state or copied wrapping keys | Versioned encrypted backups, clear key replication/recovery policy, atomic restore and rollback detection limits; A11/A23 |
| Metadata correlation | Stable identity/hardware/IP linkage | No serial tracking; separate per-device credentials and coarse local capabilities; minimal retention; A02/A12 |
| Remote AI exposure | Authorized content retained by provider or prompt injection | Explicit scoped context, provider disclosure, separate tool authority, no transitive permission from message text; A20 |
| Supply chain | Malicious dependency/update/build script | Pins, license/advisory review, least-privilege CI, signed release policy, independent audit; A03/A24/A25 |
| Privilege escalation | Plugin/admin bypasses private content authorization | Capability interfaces, process isolation where needed, audit allowed actions; A13/A20/A21/A22 |
| Capture leakage | OS screenshots, privileged capture, camera | Best-effort OS adapters and truthful supported/active/unavailable state; A17/A24 |

## A00 attack surface and limits

Config is capped at 64 KiB; numeric conversion rejects overflow; unsupported
keys/schema, duplicate keys, control bytes and malformed UTF-8 paths fail. CLI
options and arguments have count/length bounds. Error messages never echo raw
values. Log fields default to sensitive; private categories stay redacted even
if incorrectly marked public. Static public metadata is escaped and clipped;
no regex is presented as universal secret detection.

Filesystem creation rejects file collisions and symlink ancestors, but this
check is not race-proof against a malicious local user replacing directories.
Windows ACL hardening, POSIX ownership/mode checks, secure open-by-handle,
secret-memory locking, dump suppression and atomic secret writes are not
implemented. No secrets may use this API as a vault. Configuration symlinks
may be followed for explicit reads; config files are trusted local inputs for
settings, not sources of executable instructions. Standard stderr/stdout may
be captured by a shell; redaction must occur before writing them.

The logger borrows the stream and requires exclusive external ownership:
other code must not concurrently write to that same stream outside the logger.
The optional explicit path display is a local disclosure choice, not normal
logging. File errors expose categories but not underlying private filenames.

## A01 lifecycle and resource boundaries

Registered services are trusted in-process code with explicit ownership and
constructor-injected dependencies. Numeric IDs order cleanup; they are not
identities or authorization capabilities. The context contains no private data
store or network provider. Graph/diagnostic sizes are bounded; lifecycle errors
discard arbitrary service text and exception messages. Logs keep A00 redaction.

Admission serializes slot and declared-memory reservations and rejects capacity
overflow, cancellation and closed admission. It does not isolate malicious code,
meter total RSS/CPU, guarantee scheduling fairness or allocate GPU resources.
Stop requests close admission first. Hooks must cooperate with one monotonic
deadline; incomplete work and its dependencies remain owned for retry. A rogue
blocking hook, stop callback, filesystem operation or destructor can exceed the
deadline. Forcibly killing threads would not provide safe in-process cleanup.
Destructors must join residual work before dependencies/context disappear.

No listeners, telemetry, key material, private cache, crash dump or new persistent
record is introduced. A11-PRE and AUTH-PRE remain unchanged prerequisites. Local
web/model/training proposals add no execution authority in this checkpoint.

## Security validation programme

A03: upstream vectors, entropy failure, key lifetime, provider errors and license
review. A05: corpus/fuzz parsers for truncation, oversized lengths and unknown
versions. A06: whole-subsystem route leak tests including DNS/ICE/media/files/AI.
A08/A14: reviewed protocol interop, crash/rollback/epoch/replay tests. A11/A12:
inspect temporary files, WAL, journals, indexes, dumps and backups for plaintext;
fault-inject transactions and locked-device behavior. A23: partitions, disk-full,
clock skew, key revocation and restoration. A24: independent review of the
protocol composition, implementation and supported endpoint protections.

Threats remain open until their gate is evidenced. No A00/A01 test claims to prove
forward secrecy, post-compromise recovery, E2EE, secure erasure or anonymity.

## A02 process and capability boundary

A02 accepts a trusted caller's executable, native cwd, UTF-8 argv and selected
environment; production CLI input does not expose arbitrary process launching.
No shell/PATH interpolation occurs. Explicit handle/fd isolation avoids accidental
inheritance of unrelated application capabilities. Replacement environment is the
default; inherited values and captured raw bytes must be treated as private.
They are never automatically logged, written to a cache/temp file or uploaded.
Tests use synthetic data only, and directories are owned temporary fixtures.

A process group or Windows job is a cleanup mechanism. It cannot constrain
malicious code running with the user's OS authority; POSIX descendants can escape
a group, external reapers can violate ownership, and parent crash can orphan work.
No model/plugin sandbox is claimed. Owners must reserve resource budgets before
launching children; capture bounds do not enforce child CPU, memory or disk limits.
Deadline expiry retains ownership; RAII cleanup may wait for the OS.

Native stop handlers only latch atomic state. Owner dispatch runs cancellation
callbacks outside restricted signal/control context. Close/logoff/hard-kill/crash
recovery is not guaranteed. Secure-storage probing loads trusted native system
libraries and checks entry points without secret reads/writes or persistence.
A present DPAPI/Keychain API does not prove an accessible account or a secure vault.
No private backups, journals, indexes or key temporary files are introduced.
A11-PRE, AUTH-PRE and future isolation/provider review remain mandatory.


## A03 volatile cryptography boundary

| Threat | Implemented response | Remaining limit |
|---|---|---|
| Entropy failure / predictable substitute | Actual OS RNG probe and typed failure, bounded random requests, no fallback; production Linux getrandom failure test | OS RNG call is not forcibly interruptible; native BCrypt/CommonCrypto failures unexecuted here |
| Modified ciphertext or signature | Fixed reviewed AEAD/signature suites; strict sizes, known-answer and mutation tests; decryption only publishes authenticated bytes | No protocol, replay defense, identity authorization, channel binding or forward secrecy |
| Memory exhaustion / concurrent overload | Message/AAD/random caps, live secret count/byte accounting, conservative workspace admission, immediate busy | Not an OS RSS/CPU cap; independent providers and caller-owned public output need composition-level budgets |
| Key misuse or accidental copy | Distinct move-only AEAD/signing handles; no serializer/export/logger; clearing allocation and scoped borrowing | Imports can contain equal bytes; C++ types cannot prove historical purpose; caller copies/borrow misuse remain caller obligations |
| Plaintext remaining in memory | Clearing private decrypt/RNG work and secure scrub on returned-secret release | Locking best effort; registers/stack/compiler copies, swap, dumps, hibernation, VM snapshots and endpoint compromise remain |
| Signing-message confidentiality confusion | sign_public/verify_public explicitly public-content only because upstream Pure Ed25519 buffers ordinary heap bytes | No private-message signing-memory guarantee; identity records must use approved public domains |
| Nonce reuse | Exact RFC 8439 nonce shape and prominent per-key uniqueness contract | Stateless primitive cannot enforce uniqueness; future storage/protocol design must do so before use |
| Supply-chain drift / test bypass | Signed official source, precise version/hash, generated-source checks, reproducible generator, test faults in separate library | Same-origin signing-key acquisition is not independent trust certification; audits remain version/platform-specific |
| Shutdown / diagnostic disclosure | Owned cancellation, nonblocking close, consumer-join contract, enum-only errors and fixed lifecycle message | Destruction may not race calls; no hard preemption or memory-confidential diagnostic dump |

The accepted security ordering is unchanged: no persistence before A11-PRE,
no privileged remote listener/action before AUTH-PRE, and no production E2EE
claim from primitive tests. New module dependencies confer no authority to keys.
