# Module map and source ownership

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
| Tests | tests/unit, tests/fixtures/platform_child.cpp, tests/integration, tests/windows | Two doctest runners, compiled native child fixture, real CLI and Unicode/tooling regressions |
| Build | CMakeLists.txt, cmake, presets, vcpkg manifests | Repository-local build directories |

A00 platform filesystem is suitable for nonsecret configuration and empty
bootstrap directories. It is not permission-hardened, race-proof vault storage.
`ResourceLimits` feeds real bounded admission with CPU headroom and declared
memory budgets. There are no production job queues or pools; A02 owns one
monitor thread per explicitly launched child.

## Planned only

Identity, crypto, vault, permissions, protocol, transport, discovery, messaging,
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
hardware/GPU policies, secure memory and capture protection remain future work.
