# Proposed GW-A04 — Identity & Device Trust

Status: continuation proposal only. A03 must first receive independent review,
native Windows acceptance and explicit owner authorization. This file does not
authorize execution. A03 stops with a volatile primitive provider, not identities.

## Authoritative continuation

Use the owner-accepted cumulative
GHOSTWIRE-GW-A03-CRYPTOGRAPHIC-PROVIDER-source.zip and its separately delivered
SHA-256. Verify archive bytes, CRC, safe paths and every SOURCE_MANIFEST hash.
Read AGENTS, MASTER_CONTRACT, CHECKPOINT_RULES, architecture/module/dependency
maps, crypto/platform/lifecycle contracts, security invariants, ADR-003–006,
the A03 report/native checklist and actual owner acceptance. Reconcile a supplied
newer GitHub commit read-only; preserve owner changes and all historical records.
Never reconstruct or reset the repository. The owner controls GitHub pushes.

Primary owner host remains Windows10 x64, win10dev,
C:\Cumpiling-shit\GhostWire, VS2026 Community/MSVC19.51.36260, Ninja and
PowerShell5.1. Keep original TEMP/TMP. Carry unresolved Windows vcpkg/doctest,
native macOS, leak, clang-tidy and hosted CI gates forward until actual evidence.

## First pass: approved identity design

Read the owner's current identity discussion before defining persistent formats.
Resolve and document root/device separation, local profiles, alias namespace and
collisions, key rotation/recovery and trust-record semantics. Do not invent prior
agreement or treat a numerical alias, display name, server account, IP address or
hardware identifier as an authentication root. State which decisions still need
owner review. Avoid a universal identity registry or mandatory hosted service.

Document an ADR for canonical public records, bounded encodings, signature-domain
separation, versioning, expiry/clock policy, authorization/revocation checks and
recovery limits. A raw successful signature is not an authorized device. Botan's
pure Ed25519 facade signs public bytes only; private profile content must never
be passed to that interface on the assumption that it has clearing message storage.
Do not silently broaden the provider or invent a new encryption protocol.

## Bounded implementation proposal

After design approval, implement real headless identity/device contracts under
include/ghostwire/identity and src/identity with explicit service ownership and
constructor injection of crypto/clock/budgets. Generate only volatile test/runtime
identities until the separate protected-store gate is met. No identity creation
should be an incidental side effect of --help, --version, config validation or
ordinary diagnostic startup without an approved product contract.

Separate immutable public identifiers and verified records from private key
handles. Bound record sizes, counts, verification work and trust-state retention.
Use the accepted provider; deny malformed, wrong-purpose, unsupported-version,
expired, unauthorized and revoked records. Specify what local revocation means
without a network distribution mechanism. Keep error/log data fixed and nonsecret.
No generic key export, plaintext temporary files or success-shaped backup API.

**A11-PRE remains mandatory before persistent identity, credentials, private
trust/session state or key files.** A02 DPAPI/Keychain discovery is not a vault.
Persistent A04 work requires an explicitly authorized, reviewed minimum store
with authenticated atomic writes, permissions, crash/rollback/recovery tests,
bounded KDF policy where applicable and no plaintext spill. If that scope is not
approved, remain volatile and report the blocked persistence requirement.
AUTH-PRE stays mandatory before privileged listeners and remote actions.

## Validation and handover

Test actual identity verification/authorization failures, canonical ambiguity,
alias collision/change, wrong keys/purpose/domain/version, expiry boundaries,
rotation/revocation policy, count/size budgets, cancellation, key ownership and
service shutdown. Preserve A00–A03 crypto vectors, failure injection, process,
Unicode, CLI and tooling suites. Run full supported Debug/Release, sanitizers,
format/boundary and package checks; record actual platform/tool/dependency and
case/assertion counts. Never infer native Windows/macOS results from Linux.

Update architecture, invariants, threat model, TODO/ROADMAP, decisions, changelog,
completed records, measured validation, changed files and source manifest.
Deliver a cumulative A04 source ZIP/checksum, native checklist and proposed A05
handover; distinguish implementation from user acceptance. Stop for owner review.
Do not implement networking, discovery, messaging, vault UI, GUI, AI, remote
enrollment or A05 wire protocol beyond explicitly approved public identity values.
