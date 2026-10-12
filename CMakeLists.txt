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

project(GhostWire VERSION 0.1.1 LANGUAGES CXX)
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

add_library(ghostwire_platform STATIC src/platform/arguments.cpp src/platform/filesystem.cpp src/platform/platform_info.cpp)
target_link_libraries(ghostwire_platform PUBLIC ghostwire_contracts PRIVATE ghostwire_options Threads::Threads)
if(WIN32)
    target_link_libraries(ghostwire_platform PRIVATE shell32 ole32)
endif()

configure_file(cmake/build_version.hpp.in generated/ghostwire/build_version.hpp @ONLY)
add_library(ghostwire_core STATIC
    src/core/application.cpp src/core/bootstrap.cpp src/core/config.cpp
    src/core/clock.cpp src/core/resource_admission.cpp src/core/service_manager.cpp
    src/core/logger.cpp src/core/paths.cpp src/core/version.cpp)
target_link_libraries(ghostwire_core PUBLIC ghostwire_contracts ghostwire_platform
                      PRIVATE ghostwire_options Threads::Threads)
target_include_directories(ghostwire_core PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
add_library(GhostWire::core ALIAS ghostwire_core)

foreach(role IN ITEMS desktop server relay cli)
    add_executable(ghostwire-${role} apps/${role}/main.cpp)
    target_link_libraries(ghostwire-${role} PRIVATE GhostWire::core ghostwire_options)
    install(TARGETS ghostwire-${role} RUNTIME DESTINATION bin)
endforeach()
install(FILES LICENSE README.md BUILD.md THIRD_PARTY_NOTICES.md DESTINATION share/ghostwire)

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

find_package(Python3 3.9 QUIET COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
    add_custom_target(format-check COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/format.py" --check
                      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()
