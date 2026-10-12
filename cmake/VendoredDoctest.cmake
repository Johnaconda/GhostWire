# Exact upstream v2.4.12 fallback for disconnected development.
file(SHA256 "${CMAKE_SOURCE_DIR}/third_party/doctest/doctest/doctest.h" doctest_hash)
if(NOT doctest_hash STREQUAL "94029a7d32da24a56249658147dbd2b33ff0b9ed665295cbbaf19aafff5b0ced")
    message(FATAL_ERROR "Vendored doctest checksum mismatch. Restore from the pinned upstream revision.")
endif()
add_library(ghostwire_doctest INTERFACE)
target_include_directories(ghostwire_doctest SYSTEM INTERFACE "${CMAKE_SOURCE_DIR}/third_party/doctest")
add_library(doctest::doctest ALIAS ghostwire_doctest)
