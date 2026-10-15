# Security invariants and test mapping

Current A03-HF01 addendum: SI-025/SI-026 retain owned group identity until reap.
Darwin EPERM is accepted only after bounded native inspection establishes that
the owned, unreaped leader's group contains no live members. Other failures stay
structured errors; native retesting is pending. The crypto fixture canonicalizes
its trusted TEMP base; production symlink rejection under SI-004 is unchanged.
No key, secret, private profile or process metadata persistence is introduced.
A11-PRE/AUTH-PRE and all A03 crypto limitations remain in force.

“Implemented” refers to narrow A00–A03 behavior, not a future security capability.
Future tests are named acceptance tasks, not passing tests hidden by missing code.

| ID | Invariant | State / evidence |
|---|---|---|
| SI-001 | Bootstrap requires no remote account/service/activation | A00 implemented: no network code or linked network dependency; four CLI bootstrap tests. Future disappearance integration test still pending. |
| SI-002 | Input config is bounded and fail-closed | Implemented: config budget, malformed/duplicate/schema/numeric tests; HF01 adds invalid UTF-8 CLI/config regressions including raw ANSI-style 0xF6. No replacement-character or code-page fallback. |
| SI-003 | Ordinary logs never serialize designated private fields | Implemented: sensitive-field override, escaping/bounds and CLI no-echo tests; public metadata contract still requires code review |
| SI-004 | Each role resolves isolated paths and validation performs no writes | Implemented: role isolation and check/print no-write tests |
| SI-005 | Failure to initialize is not reported as successful startup | A01: graph validation before side effects; partial-start rollback including the failing service; original startup error preserved separately from cleanup failures; process exit tests |
| SI-006 | No network event can erase another user's local keys | No such feature exists; A12 remote-lockout/erase negative tests required |
| SI-007 | Protected/privacy-first never fall back to direct | Policy parsing only; A06 full transport and DNS/ICE/file/media/AI leak tests required |
| SI-008 | Server/relay/discovery operators do not implicitly receive E2EE keys | Future: A04/A08/A14 host-compromise and membership tests |
| SI-009 | Root, device, community, session and vault keys have distinct lifecycles | A03 separates volatile AEAD/signing handle types; domain identity/session/revocation/backup lifecycles remain A04/A08/A11 work |
| SI-010 | Aliases and hardware identifiers cannot become authentication roots | Future: A04 alias-change/verification; A12 disclosure tests |
| SI-011 | No private persistence until reviewed encrypted key store is ready | A00–A03 store no private data; A11-PRE still blocks A04/A08 persistence; A11 inspect disk/WAL/temp/search/backup outputs |
| SI-012 | All messages/chunks are authenticated before exposure | Future: A08/A15 tamper/reorder/truncation tests using reviewed implementations |
| SI-013 | Replay and retry bookkeeping is bounded and consistent with crypto state | Future: A05/A08/A09 crash/replay/quota tests |
| SI-014 | Discovery records are verified, short-lived and policy-approved | Future: A07 poisoning/expiry/route-disclosure tests |
| SI-015 | Every public operation has a budget and a cancellation path | A01 bounded graph/diagnostics, active/queued/declared-memory admission and cooperative stop/deadline tests. Not OS CPU/RSS enforcement; blocking trusted hooks/filesystem calls cannot be forcibly interrupted. Future subsystems must implement their own bounded operations. |
| SI-016 | AI/plugin authority is explicit and revocable | Future: A20/A22 scope denial, prompt-injection, provider disclosure and sandbox tests |
| SI-017 | Community policy never grants blanket DM or vault access | Future: A13/A21 permission matrix and host-only compromise tests |
| SI-018 | Recovery/erasure claims account for replicated keys and copied plaintext | Future: A11/A12/A23 restore/replica inventory and deliberate authorization tests |
| SI-019 | Capability claims reflect actual platform/runtime state | A02 API-only probe; persistence_ready false; loader failure is an error. A02 Windows owner-accepted, macOS open. A03 diagnostics never claim all memory locked or persistence available. A17 capture/accessibility remain future. |
| SI-020 | Protocol changes are versioned; unsupported versions fail closed | No wire schema in A00; A05 compatibility matrix and fuzz tests pending |
| SI-021 | Runtime GUI components cannot become headless dependencies | Implemented module-boundary check, separate composition roots; future CMake dependency checks |
| SI-022 | Dependency/packaging provenance is explicit | Manifest pin, vendored SHA verification, source inventory and archive checksum; future advisory/release-signing audits pending |
| SI-023 | Lifecycle failures cannot leak service-provided private error text | A01 fixed error messages, numeric service IDs/reasons and exception redaction tests; no raw exception text logged |
| SI-024 | Unfinished service work retains its dependencies and ownership | A01 reverse cleanup, dependency retention, retry, lifetime and real joined-worker tests; no detached work or false clean completion with outstanding leases |

When adding a security subsystem, assign concrete test IDs and evidence to its
invariants. Mark failed, blocked and unverified separately. Do not label a future
invariant “passing” merely because the triggering subsystem does not exist yet.

## A02 additions

| ID | Invariant | Implementation and evidence |
|---|---|---|
| SI-025 | Only explicitly owned child resources can be controlled | No PID-adoption API; Windows suspended-child job assignment and handle allowlist; POSIX group identity retained until cleanup/reap; Linux real-child/foreign-FD tests |
| SI-026 | Child output and waits are bounded without abandoning ownership | Per-stream caps plus continued draining; timeout/cancel retains object; repeated/concurrent waits, flood and RAII tests; OS cleanup is not a hard deadline |
| SI-027 | Restricted native callbacks do not run application code | Lock-free latch only; owner dispatch requests stop; handler restoration/reinstall and actual signal-delivery tests |
| SI-028 | Secure API presence cannot authorize private persistence | Capability field explicitly API-only; no secret operation; disabled/unsupported/cancel tests; A11-PRE and AUTH-PRE stay open |

These invariants do not imply hostile process containment, forced in-process
service cancellation, complete process-memory secrecy or production encryption.
See the full platform contracts and A02 native acceptance checklist.

## A03 additions

| ID | Invariant | Implementation and evidence |
|---|---|---|
| SI-029 | Only pinned reviewed primitives; entropy failure never downgrades | Exact Botan source/generated hashes, signature/provenance review; initialization/OS entropy and private fault tests; no rand/time/fallback API |
| SI-030 | Authentication failure releases no plaintext | Clearing private decryption workspace; RFC 8439 vectors and per-byte tag/ciphertext mutations, wrong AAD/nonce/key, truncation/extension tests |
| SI-031 | Secret lifetime and purpose remain explicit | Move-only AEAD/signing types, allocation quotas, reset/move/outlive-provider and zero-before-free observer tests; borrowed spans and locking limitations documented |
| SI-032 | Backend failures cannot leak input or exception text | Enum-only errors and fixed descriptions; fault redaction and actual failed bootstrap exit 70 tests |
| SI-033 | Crypto work is bounded, cancellable and owned | Input/workspace/handle/byte caps, nonblocking busy, per-call/application/owned cancellation, in-flight close/retention and concurrent tests; no forced interruption claim |
| SI-034 | Primitive availability grants no persistence/protocol authority | No key export/store, identity, KDF or network API; nonce uniqueness belongs to caller; sign_public accepts public content; A11-PRE/AUTH-PRE unchanged |

Vector correctness and these tests do not establish production E2EE readiness,
full memory secrecy, nonce/replay policy, identity revocation or independent audit.
See CRYPTO_PROVIDER.md and the actual GW-A03 validation matrix.
