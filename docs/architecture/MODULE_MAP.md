# Module map and source ownership

## P01 protocol design only (no module yet)

`schemas/ghostwire-wire-v1.cddl` is a proposed schema artifact, not a linked
protocol implementation. ADR-009, PROTOCOL_SCHEMA.md and PROTOCOL_TEST_PLAN.md
describe a future strict `ghostwire_protocol` codec and explicit application
composition. P01 introduces no `include/ghostwire/protocol` or `src/protocol`,
no library target, service, socket, encryption or persistence.


## P03 scoped update

P03 refines IdentityService::delete_membership: invalidate matching self-trust
in the owning profile before removing its owned record; preserve other profiles.
No new module or public API is introduced. See ../checkpoints/GW-A04-P03.md.


## Implemented

| Module | Public headers | Sources and ownership |
|---|---|---|
| Core values | error, result, role, version, resource_limits | Value semantics; version implementation under src/core |
| Configuration | core/config.hpp | Pure parser and validator; bounded explicit file load |
| Logging | core/logger.hpp | Caller-owned stream, logger-owned mutex, synchronous bounded JSON records |
| Paths | core/paths.hpp | Pure resolution then explicit directory initialization |
| Lifecycle | application.hpp, application_lifecycle.hpp, application_context.hpp, service.hpp, service_manager.hpp, clock.hpp | Application-owned immutable context, stop source and service graph; injected logger/clock; bounded diagnostics |
| Admission | core/resource_admission.hpp, resource_limits.hpp | Application-owned controller; thread-safe slots/bytes; move-only shared-accounting reservations |
| Bootstrap | core/bootstrap.hpp | CLI orchestration; one root call per executable |
| Platform paths/argv | platform/arguments.hpp, filesystem.hpp, platform_info.hpp | Native types remain in src/platform; explicit native/UTF-8 conversion |
| Owned processes | platform/process.hpp | Caller-owned process/group/job, bounded capture and joined monitor |
| Process stop | platform/process_stop.hpp | Explicit owner-thread registration/dispatch, native callback latch |
| Storage capability | platform/secure_storage.hpp | API-entry-point probe only; no secret operations or persistence |
| Crypto | crypto/crypto_provider.hpp, crypto_error.hpp, key_handle.hpp | Explicit provider, purpose-specific move-only keys, clearing buffers, bounds, typed errors and cancellation; pinned static Botan private to implementation |
| Crypto lifecycle | core/crypto_service.hpp | ServiceManager-owned adapter, real initialization/stop/rollback in all roles |
| Identity values/records | identity/identity_id.hpp, identity_record.hpp, identity_error.hpp | Fixed typed IDs, bounded canonical public DeviceSet, pure Ed25519 signing/verification through crypto; enum-only errors |
| Identity state | identity/identity_service.hpp | Explicitly owned volatile profiles, independent membership/device keys, trust pins, revision/conflict/time state and scoped mutation; provider/clock borrowed |
| Identity lifecycle | composition/identity_service_adapter.hpp | Separate core Service adapter, declared admission reservation and injected CryptoService dependency; no core-to-identity edge |
| Identity console | apps/cli/identity_session.hpp | Explicit --identity-session; bounded owner-thread input, selected profile handle and public-only display; no persistence or networking |
| Tests | tests/unit, tests/security, tests/fixtures, tests/integration, tests/windows | Core/platform/crypto/identity/fault runners, Linux OS entropy failure runner, real native child/CLI/Unicode/tooling regressions |
| Build | CMakeLists.txt, cmake, presets, vcpkg manifests | Repository-local build directories |

A00 platform filesystem is suitable for nonsecret configuration and empty
bootstrap directories. It is not permission-hardened, race-proof vault storage.
`ResourceLimits` feeds real bounded admission with CPU headroom and declared
memory budgets. There are no production job queues or pools; A02 owns one
monitor thread per explicitly launched child.

## Planned only

A04-P01 supplied the owner-accepted design in
[IDENTITY_DESIGN.md](IDENTITY_DESIGN.md) and
[ADR-008](../decisions/ADR-008-IDENTITY-AND-DEVICE-TRUST.md). It covers isolated
profiles, typed scopes, separate membership credentials, device authorization,
revocation and explicit recovery limits. P02 implements the bounded volatile
portion listed above. Public record verification lives in identity_record.hpp;
there is no empty identity_verification.hpp or key_rotation.hpp placeholder.
Protected storage, root continuity/recovery, cross-device enrollment, live
remote possession and community admission remain unimplemented.

Vault, permissions, protocol, transport, discovery, messaging,
friends, groups, communities, federation, storage, files, media, moderation,
AI, plugins, diagnostics services and all ui/ and schemas/ trees are documented
but not populated with stubs. Create a directory when real functionality or a
versioned schema lands. Discovery/storage roles initially compose headless
services; their executable split is a later packaging decision.

The proposed public contracts and ownership constraints are in
[FUTURE_INTERFACES.md](FUTURE_INTERFACES.md). A01 implements only the core context/service manager/admission contracts,
explained in [CORE_LIFECYCLE.md](CORE_LIFECYCLE.md). Future browser, AI, training
and media proposals are in [FUTURE_FEATURES.md](FUTURE_FEATURES.md).

A02 implements the bounded platform subset in
[PLATFORM_ABSTRACTIONS.md](PLATFORM_ABSTRACTIONS.md); sealed-key operations,
hardware/GPU policies and capture protection remain future work.
A03 implements the bounded volatile [crypto contract](CRYPTO_PROVIDER.md), including
best-effort clearing allocation; protected persistence and full memory secrecy
remain unavailable. No general platform secure-memory API is introduced.
