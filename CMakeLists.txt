cmake_minimum_required(VERSION 3.25)

option(BUILD_TESTING "Build GhostWire tests" ON)
set(GW_TEST_DEPENDENCY "vendored" CACHE STRING "Test dependency provider: vendored or vcpkg")
set_property(CACHE GW_TEST_DEPENDENCY PROPERTY STRINGS vendored vcpkg)
if(NOT GW_TEST_DEPENDENCY MATCHES "^(vendored|vcpkg)$")
    message(FATAL_ERROR "GW_TEST_DEPENDENCY must be vendored or vcpkg")
endif()
set(VCPKG_MANIFEST_FEATURES "")
if(BUILD_TESTING AND GW_TEST_DEPENDENCY STREQUAL "vcpkg")
    list(APPEND VCPKG_MANIFEST_FEATURES tests)
endif()
set(VCPKG_MANIFEST_NO_DEFAULT_FEATURES ON)

project(GhostWire VERSION 0.4.1 LANGUAGES CXX)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Use an out-of-source build: cmake --preset portable-debug")
endif()
option(GW_WARNINGS_AS_ERRORS "Treat project warnings as errors" ON)
option(GW_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(GW_ENABLE_CLANG_TIDY "Run clang-tidy during compilation" OFF)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
include(cmake/ProjectOptions.cmake)
if(GW_ENABLE_CLANG_TIDY)
    find_program(GW_CLANG_TIDY NAMES clang-tidy REQUIRED)
    set(CMAKE_CXX_CLANG_TIDY "${GW_CLANG_TIDY}")
endif()
find_package(Threads REQUIRED)

# Shared source contracts, not a stable binary ABI. Keep platform below core.
add_library(ghostwire_contracts INTERFACE)
target_compile_features(ghostwire_contracts INTERFACE cxx_std_20)
target_include_directories(ghostwire_contracts INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

include(cmake/VendoredBotan.cmake)
add_library(ghostwire_crypto STATIC src/crypto/crypto_provider.cpp)
target_link_libraries(ghostwire_crypto PUBLIC ghostwire_contracts
    PRIVATE ghostwire_botan ghostwire_options Threads::Threads)
add_library(GhostWire::crypto ALIAS ghostwire_crypto)

add_library(ghostwire_platform STATIC src/platform/arguments.cpp src/platform/filesystem.cpp
    src/platform/platform_info.cpp src/platform/process.cpp src/platform/process_stop.cpp
    src/platform/secure_storage.cpp)
target_link_libraries(ghostwire_platform PUBLIC ghostwire_contracts PRIVATE ghostwire_options Threads::Threads)
if(WIN32)
    target_link_libraries(ghostwire_platform PRIVATE shell32 ole32)
else()
    include(CheckCXXSymbolExists)
    check_cxx_symbol_exists(posix_spawn_file_actions_addchdir "spawn.h" GW_HAVE_SPAWN_CHDIR)
    if(GW_HAVE_SPAWN_CHDIR)
        target_compile_definitions(ghostwire_platform PRIVATE GW_SPAWN_CHDIR=posix_spawn_file_actions_addchdir)
    else()
        check_cxx_symbol_exists(posix_spawn_file_actions_addchdir_np "spawn.h" GW_HAVE_SPAWN_CHDIR_NP)
        if(NOT GW_HAVE_SPAWN_CHDIR_NP)
            message(FATAL_ERROR "Owned child working directories require posix_spawn addchdir support (glibc 2.29+ / modern macOS SDK).")
        endif()
        target_compile_definitions(ghostwire_platform PRIVATE GW_SPAWN_CHDIR=posix_spawn_file_actions_addchdir_np)
    endif()
    if(NOT APPLE)
        check_cxx_symbol_exists(posix_spawn_file_actions_addclosefrom_np "spawn.h" GW_HAVE_SPAWN_CLOSEFROM)
        if(NOT GW_HAVE_SPAWN_CLOSEFROM)
            message(FATAL_ERROR "Safe owned-process descriptor isolation requires posix_spawn addclosefrom (glibc 2.34+). No inheriting fallback is used.")
        endif()
    endif()
endif()

configure_file(cmake/build_version.hpp.in generated/ghostwire/build_version.hpp @ONLY)
add_library(ghostwire_core STATIC
    src/core/application.cpp src/core/bootstrap.cpp src/core/config.cpp src/core/crypto_service.cpp
    src/core/clock.cpp src/core/resource_admission.cpp src/core/service_manager.cpp
    src/core/logger.cpp src/core/paths.cpp src/core/version.cpp)
target_link_libraries(ghostwire_core PUBLIC ghostwire_contracts ghostwire_platform GhostWire::crypto
                      PRIVATE ghostwire_options Threads::Threads)
target_include_directories(ghostwire_core PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
add_library(GhostWire::core ALIAS ghostwire_core)

# Identity uses only crypto and header-only value/clock contracts. Lifecycle
# composition links both libraries; neither lower layer links back to it.
set(GW_IDENTITY_SOURCES src/identity/identity_record.cpp src/identity/identity_service.cpp)
add_library(ghostwire_identity STATIC ${GW_IDENTITY_SOURCES})
target_link_libraries(ghostwire_identity PUBLIC GhostWire::crypto ghostwire_contracts
    PRIVATE ghostwire_options Threads::Threads)
add_library(GhostWire::identity ALIAS ghostwire_identity)
# In-memory A05 v1.0 strict public wire codec; no crypto, sockets or identity trust dependencies.
add_library(ghostwire_protocol STATIC src/protocol/wire_codec.cpp)
target_link_libraries(ghostwire_protocol PUBLIC ghostwire_contracts PRIVATE ghostwire_options)
add_library(GhostWire::protocol ALIAS ghostwire_protocol)

add_library(ghostwire_composition STATIC src/composition/identity_service_adapter.cpp)
target_link_libraries(ghostwire_composition PUBLIC GhostWire::core GhostWire::identity
    PRIVATE ghostwire_options)
add_library(GhostWire::composition ALIAS ghostwire_composition)

foreach(role IN ITEMS desktop server relay cli)
    add_executable(ghostwire-${role} apps/${role}/main.cpp)
    target_link_libraries(ghostwire-${role} PRIVATE GhostWire::core ghostwire_options)
    install(TARGETS ghostwire-${role} RUNTIME DESTINATION bin)
endforeach()
target_sources(ghostwire-cli PRIVATE apps/cli/identity_session.cpp)
target_link_libraries(ghostwire-cli PRIVATE GhostWire::composition)
install(FILES LICENSE README.md BUILD.md THIRD_PARTY_NOTICES.md DESTINATION share/ghostwire)
install(FILES third_party/botan/LICENSE.txt DESTINATION share/ghostwire/licenses RENAME Botan-LICENSE.txt)

include(CTest)
if(BUILD_TESTING)
    if(GW_TEST_DEPENDENCY STREQUAL "vcpkg")
        find_package(doctest 2.4.12 EXACT CONFIG REQUIRED)
    else()
        include(cmake/VendoredDoctest.cmake)
    endif()
    add_executable(ghostwire-tests tests/unit/core_tests.cpp tests/unit/lifecycle_tests.cpp)
    target_link_libraries(ghostwire-tests PRIVATE GhostWire::core ghostwire_options doctest::doctest Threads::Threads)
    add_test(NAME core-unit COMMAND ghostwire-tests --no-colors)
    set_tests_properties(core-unit PROPERTIES TIMEOUT 30 LABELS "unit;security")
    add_executable(ghostwire-platform-fixture tests/fixtures/platform_child.cpp)
    target_link_libraries(ghostwire-platform-fixture PRIVATE ghostwire_platform ghostwire_options)
    add_executable(ghostwire-platform-tests tests/unit/platform_tests.cpp)
    target_link_libraries(ghostwire-platform-tests PRIVATE GhostWire::core ghostwire_options doctest::doctest Threads::Threads)
    add_test(NAME platform-unit COMMAND ghostwire-platform-tests "$<TARGET_FILE:ghostwire-platform-fixture>")
    set_tests_properties(platform-unit PROPERTIES TIMEOUT 30 LABELS "unit;platform;security")
    add_executable(ghostwire-crypto-tests tests/unit/crypto_tests.cpp)
    target_link_libraries(ghostwire-crypto-tests PRIVATE GhostWire::core ghostwire_options doctest::doctest Threads::Threads)
    add_test(NAME crypto-unit COMMAND ghostwire-crypto-tests --no-colors)
    set_tests_properties(crypto-unit PROPERTIES TIMEOUT 30 LABELS "unit;crypto;security")
    if(UNIX)
        add_test(NAME crypto-symlink-temp
            COMMAND "${CMAKE_COMMAND}" "-DEXE=$<TARGET_FILE:ghostwire-crypto-tests>"
            "-DWORK=${CMAKE_BINARY_DIR}/test-runtime/crypto-symlink-temp"
            -P "${CMAKE_SOURCE_DIR}/tests/integration/crypto_temp_path.cmake")
        set_tests_properties(crypto-symlink-temp PROPERTIES TIMEOUT 30 LABELS "integration;crypto;platform")
    endif()
    # Production targets never link this library. No test fault API is present in
    # ghostwire_crypto; private hooks use the same implementation in isolation.
    add_library(ghostwire_crypto_faults STATIC src/crypto/crypto_provider.cpp)
    target_compile_definitions(ghostwire_crypto_faults PRIVATE GW_CRYPTO_TESTING=1)
    target_link_libraries(ghostwire_crypto_faults PUBLIC ghostwire_contracts
        PRIVATE ghostwire_botan ghostwire_options Threads::Threads)
    add_executable(ghostwire-crypto-fault-tests tests/unit/crypto_fault_tests.cpp)
    target_compile_definitions(ghostwire-crypto-fault-tests PRIVATE GW_CRYPTO_TESTING=1)
    target_include_directories(ghostwire-crypto-fault-tests PRIVATE "${CMAKE_SOURCE_DIR}/src/crypto")
    target_link_libraries(ghostwire-crypto-fault-tests PRIVATE ghostwire_crypto_faults ghostwire_options doctest::doctest Threads::Threads)
    add_test(NAME crypto-faults COMMAND ghostwire-crypto-fault-tests --no-colors)
    set_tests_properties(crypto-faults PROPERTIES TIMEOUT 30 LABELS "unit;crypto;security")
    add_executable(ghostwire-protocol-tests tests/unit/protocol_tests.cpp)
    target_link_libraries(ghostwire-protocol-tests PRIVATE GhostWire::protocol GhostWire::identity
        ghostwire_options doctest::doctest)
    # Test-only cross-language probe: consumes stdin hex, never opens a socket.
    add_executable(ghostwire-protocol-probe tests/fixtures/protocol/probe.cpp)
    target_link_libraries(ghostwire-protocol-probe PRIVATE GhostWire::protocol ghostwire_options)
    # Compile the identical codec in an isolated test executable with a
    # two-site allocation fault hook. Never override process-wide operator new
    # (MSVC CRT allocations can enter that override). Production protocol has
    # no hook and never links the fault test.
    add_executable(ghostwire-protocol-allocation-fault
        tests/fixtures/protocol/allocation_fault.cpp src/protocol/wire_codec.cpp)
    target_compile_definitions(ghostwire-protocol-allocation-fault PRIVATE GW_PROTOCOL_FAULT_TESTING=1)
    target_link_libraries(ghostwire-protocol-allocation-fault PRIVATE ghostwire_contracts ghostwire_options)
    add_test(NAME protocol-allocation-fault COMMAND ghostwire-protocol-allocation-fault)
    set_tests_properties(protocol-allocation-fault PROPERTIES TIMEOUT 15 LABELS "unit;protocol;security")
    # Preserve HF07's stronger *global* allocation count/preflight regression
    # where its platform-wide allocator replacement is already known to work.
    # Never run it with MSVC or sanitize builds: the portable fault fixture
    # above is the native Windows safety regression.
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT GW_ENABLE_SANITIZERS)
        add_executable(ghostwire-protocol-global-allocation-linux
            tests/fixtures/protocol/allocation_fault_global_linux.cpp)
        target_link_libraries(ghostwire-protocol-global-allocation-linux PRIVATE
            GhostWire::protocol ghostwire_options)
        add_test(NAME protocol-global-allocation-linux COMMAND ghostwire-protocol-global-allocation-linux)
        set_tests_properties(protocol-global-allocation-linux PROPERTIES TIMEOUT 15 LABELS "unit;protocol;security")
    endif()
    add_test(NAME protocol-unit COMMAND ghostwire-protocol-tests --no-colors)
    set_tests_properties(protocol-unit PROPERTIES TIMEOUT 30 LABELS "unit;protocol;security")
    add_executable(ghostwire-identity-tests tests/unit/identity_record_tests.cpp
        tests/unit/identity_service_tests.cpp tests/unit/identity_lifecycle_tests.cpp)
    target_link_libraries(ghostwire-identity-tests PRIVATE GhostWire::composition
        ghostwire_options doctest::doctest Threads::Threads)
    add_test(NAME identity-unit COMMAND ghostwire-identity-tests --no-colors)
    set_tests_properties(identity-unit PROPERTIES TIMEOUT 60 LABELS "unit;identity;security")
    # Compile the identical identity implementation against the isolated fault
    # provider. Production libraries never link fault hooks or substituted RNG.
    add_executable(ghostwire-identity-fault-tests tests/unit/identity_fault_tests.cpp
        ${GW_IDENTITY_SOURCES})
    target_compile_definitions(ghostwire-identity-fault-tests PRIVATE GW_CRYPTO_TESTING=1)
    target_include_directories(ghostwire-identity-fault-tests PRIVATE "${CMAKE_SOURCE_DIR}/src/crypto")
    target_link_libraries(ghostwire-identity-fault-tests PRIVATE ghostwire_crypto_faults
        ghostwire_options doctest::doctest Threads::Threads)
    add_test(NAME identity-faults COMMAND ghostwire-identity-fault-tests --no-colors)
    set_tests_properties(identity-faults PROPERTIES TIMEOUT 30 LABELS "unit;identity;security")
    add_test(NAME cli-identity-session
        COMMAND "${CMAKE_COMMAND}" "-DEXE=$<TARGET_FILE:ghostwire-cli>"
        "-DWORK=${CMAKE_BINARY_DIR}/test-runtime/identity-session"
        -P "${CMAKE_SOURCE_DIR}/tests/integration/identity_session.cmake")
    set_tests_properties(cli-identity-session PROPERTIES TIMEOUT 30 LABELS "integration;identity;unicode")
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang)$")
        add_executable(ghostwire-crypto-entropy-tests tests/security/crypto_entropy_failure.cpp)
        target_link_libraries(ghostwire-crypto-entropy-tests PRIVATE GhostWire::core ghostwire_options doctest::doctest)
        target_link_options(ghostwire-crypto-entropy-tests PRIVATE -Wl,--wrap=getrandom)
        add_test(NAME crypto-system-entropy-failure COMMAND ghostwire-crypto-entropy-tests --no-colors)
        set_tests_properties(crypto-system-entropy-failure PROPERTIES TIMEOUT 30 LABELS "unit;crypto;security")
    endif()
    foreach(role IN ITEMS desktop server relay cli)
        foreach(case IN ITEMS help version check-config unknown invalid-config missing-config bootstrap)
            add_test(NAME ${role}-${case}
                COMMAND "${CMAKE_COMMAND}" "-DEXE=$<TARGET_FILE:ghostwire-${role}>" "-DROLE=${role}"
                "-DCASE=${case}" "-DWORK=${CMAKE_BINARY_DIR}/test-runtime/${role}-${case}"
                -P "${CMAKE_SOURCE_DIR}/tests/integration/cli_contract.cmake")
            set_tests_properties(${role}-${case} PROPERTIES TIMEOUT 15 LABELS integration)
        endforeach()
        add_test(NAME ${role}-unicode-paths
            COMMAND "${CMAKE_COMMAND}" "-DEXE=$<TARGET_FILE:ghostwire-${role}>" "-DROLE=${role}"
            "-DWORK=${CMAKE_BINARY_DIR}/test-runtime/${role}-unicode"
            -P "${CMAKE_SOURCE_DIR}/tests/integration/unicode_paths.cmake")
        set_tests_properties(${role}-unicode-paths PROPERTIES TIMEOUT 30 LABELS "integration;unicode")
    endforeach()
    if(WIN32)
        find_program(GW_TEST_POWERSHELL NAMES powershell.exe pwsh.exe REQUIRED)
        add_test(NAME windows-tooling
            COMMAND "${GW_TEST_POWERSHELL}" -NoProfile -ExecutionPolicy Bypass -File
            "${CMAKE_SOURCE_DIR}/tests/windows/tooling-tests.ps1")
        set_tests_properties(windows-tooling PROPERTIES TIMEOUT 30 LABELS "tooling;unicode")
    endif()
endif()


# Optional upstream interoperability gate, NEVER a default/product dependency.
# Only explicitly imported, SHA-256 verified TinyCBOR v7.0 can be used.
option(GW_A05_ENABLE_TINYCBOR_INTEROP "Test-only pinned TinyCBOR comparison (requires separate verified import)" OFF)
if(GW_A05_ENABLE_TINYCBOR_INTEROP)
    if(NOT BUILD_TESTING)
        message(FATAL_ERROR "TinyCBOR interop requires BUILD_TESTING=ON")
    endif()
    find_package(Python3 3.9 REQUIRED COMPONENTS Interpreter)
    execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/protocol/import_tinycbor.py"
        --source "${CMAKE_SOURCE_DIR}/third_party/tinycbor/verified"
        RESULT_VARIABLE GW_TINYCBOR_VERIFY_RESULT OUTPUT_QUIET ERROR_QUIET)
    if(NOT GW_TINYCBOR_VERIFY_RESULT EQUAL 0)
        message(FATAL_ERROR "TinyCBOR v7.0 verified sources unavailable or modified; run the explicit offline import verifier first")
    endif()
    enable_language(C)
    set(GW_TINYCBOR_GEN "${CMAKE_BINARY_DIR}/gw-tinycbor-headers")
    file(MAKE_DIRECTORY "${GW_TINYCBOR_GEN}")
    file(WRITE "${GW_TINYCBOR_GEN}/tinycbor-export.h" "#pragma once\n#define CBOR_API\n")
    file(WRITE "${GW_TINYCBOR_GEN}/tinycbor-version.h"
        "#pragma once\n#define TINYCBOR_VERSION_MAJOR 7\n#define TINYCBOR_VERSION_MINOR 0\n#define TINYCBOR_VERSION_PATCH 0\n")
    add_library(ghostwire_test_tinycbor STATIC
        third_party/tinycbor/verified/src/cborparser.c
        third_party/tinycbor/verified/src/cborvalidation.c
        third_party/tinycbor/verified/src/cborparser_dup_string.c
        third_party/tinycbor/verified/src/cborerrorstrings.c)
    set_target_properties(ghostwire_test_tinycbor PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED ON)
    target_compile_definitions(ghostwire_test_tinycbor PUBLIC CBOR_NO_FLOATING_POINT CBOR_PARSER_MAX_RECURSIONS=8)
    target_include_directories(ghostwire_test_tinycbor PUBLIC
        "${CMAKE_SOURCE_DIR}/third_party/tinycbor/verified/src" "${GW_TINYCBOR_GEN}")
    add_executable(ghostwire-tinycbor-probe tests/interop/tinycbor_probe.cpp)
    target_link_libraries(ghostwire-tinycbor-probe PRIVATE ghostwire_test_tinycbor ghostwire_options)
    add_test(NAME protocol-tinycbor-interop
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/protocol/differential_check.py"
        "$<TARGET_FILE:ghostwire-protocol-probe>"
        --upstream-probe "$<TARGET_FILE:ghostwire-tinycbor-probe>")
    set_tests_properties(protocol-tinycbor-interop PROPERTIES TIMEOUT 120 LABELS "security;protocol;interop")
    add_test(NAME protocol-tinycbor-stdin-bounds
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/protocol/test_probe_bounds.py"
        "$<TARGET_FILE:ghostwire-tinycbor-probe>" --upstream)
    set_tests_properties(protocol-tinycbor-stdin-bounds PROPERTIES TIMEOUT 35 LABELS "security;protocol;interop")
endif()

find_package(Python3 3.9 QUIET COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND AND BUILD_TESTING)
    add_test(NAME protocol-upstream-import-guard
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/protocol/test_tinycbor_import.py")
    set_tests_properties(protocol-upstream-import-guard PROPERTIES TIMEOUT 15 LABELS "tooling;protocol;security")
    add_test(NAME protocol-upstream-fetch-guard
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/protocol/test_tinycbor_fetch.py")
    set_tests_properties(protocol-upstream-fetch-guard PROPERTIES TIMEOUT 15 LABELS "tooling;protocol;security")
    add_test(NAME protocol-upstream-comparison-guard
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/protocol/test_interop_gate.py")
    set_tests_properties(protocol-upstream-comparison-guard PROPERTIES TIMEOUT 15 LABELS "tooling;protocol;security")
    add_test(NAME protocol-probe-stdin-bounds
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/protocol/test_probe_bounds.py"
        "$<TARGET_FILE:ghostwire-protocol-probe>")
    set_tests_properties(protocol-probe-stdin-bounds PROPERTIES TIMEOUT 35 LABELS "tooling;protocol;security")
    add_test(NAME development-module-registry
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/development/test_module_registry.py")
    set_tests_properties(development-module-registry PROPERTIES TIMEOUT 15 LABELS "tooling;security")
    add_test(NAME development-effort-forecast
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/development/test_effort_forecast.py")
    set_tests_properties(development-effort-forecast PROPERTIES TIMEOUT 15 LABELS "tooling;development")
    add_test(NAME development-windows-docs
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/development/test_windows_docs.py")
    set_tests_properties(development-windows-docs PROPERTIES TIMEOUT 15 LABELS "tooling;security")
    add_test(NAME development-worker-merge
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/development/test_worker_merge.py")
    set_tests_properties(development-worker-merge PROPERTIES TIMEOUT 15 LABELS "tooling;security")
endif()
if(Python3_Interpreter_FOUND)
    add_custom_target(format-check COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/format.py" --check
                      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()
