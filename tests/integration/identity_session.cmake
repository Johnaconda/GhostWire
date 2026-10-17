if(NOT DEFINED EXE OR NOT DEFINED WORK)
    message(FATAL_ERROR "Missing identity session test inputs")
endif()
file(MAKE_DIRECTORY "${WORK}")
# Resolve only the trusted test base, including native macOS /var TEMP links.
file(REAL_PATH "${WORK}" WORK)
set(root "${WORK}/identity data ö")
set(input "${WORK}/commands.txt")
set(community "d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a")
file(WRITE "${input}"
    "diagnostics\ncreate 00123 Jörg Owner\nprofiles\n"
    "membership ${community} 00123 Community User\nmemberships\n"
    "create 00123 Other Profile\nprofiles\n"
    "trust-profile 1\nrecognize-profile 1 1\nblock-profile 1\nunblock-profile 1\n"
    "select 1\nrename 00456 Renamed Owner\nrotate profile 1\n"
    "rotate membership 1 1\nadd membership 1\nrevoke membership 1 2\n"
    "show membership 1\nshow profile\nselect 2\ntrust-profile 1\nrecognize-profile 1 1\n"
    "select 1\nrevoke profile 1\nshow profile\nprofiles\nmemberships\ndiagnostics\nquit\n")
execute_process(COMMAND "${EXE}" --identity-session --data-root "${root}"
    INPUT_FILE "${input}" RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE errors
    ENCODING UTF-8 TIMEOUT 25)
if(NOT "${status}" STREQUAL "0" OR errors MATCHES "Identity command error|GhostWire error")
    message(FATAL_ERROR "Identity workflow failed: status=${status}; errors=${errors}")
endif()
foreach(expected IN ITEMS
    "profiles=0 memberships=0 devices=0 private_keys=0"
    "Local profiles: 2"
    "name=Jörg Owner"
    "name=Renamed Owner"
    "independent membership credentials prepared (not admitted)"
    "Profile labels changed; identity unchanged"
    "Profile device replaced; old device revoked in this scope"
    "Membership device replaced; old device revoked in this scope"
    "Profile authority blocked locally"
    "Profile authority unblocked locally"
    "Device recognized by current local trust"
    "Profile device revoked; independent membership devices are unchanged"
    "signed_public_record="
    "revision=3 devices=0"
    "persistence_available=false"
    "Identity session closed; volatile state cleared")
    string(FIND "${output}" "${expected}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Missing observable identity workflow behavior: ${expected}")
    endif()
endforeach()
if(errors MATCHES "Jörg|Owner|Community User|Other Profile|00123|00456|${community}|identity data")
    message(FATAL_ERROR "Identity input leaked into lifecycle diagnostics")
endif()
file(GLOB_RECURSE stored_files LIST_DIRECTORIES false "${root}/*")
if(stored_files)
    message(FATAL_ERROR "Identity session unexpectedly persisted a file")
endif()

# Unknown/oversized input, bad UTF-8 and missing selections are recoverable and
# never echoed. Bounds are checked even when stdin is redirected from a file.
string(REPEAT "DO_NOT_ECHO_198.51.100.19" 200 oversized)
string(ASCII 246 invalid_utf8)
file(WRITE "${input}" "recognize-profile 1 1\nselect 0\n"
    "unknown DO_NOT_ECHO_198.51.100.19\n${oversized}\n"
    "create 12 PRIVATE_MARKER_${invalid_utf8}\n"
    "membership DO_NOT_ECHO_198.51.100.19 12 PRIVATE_MARKER\n"
    "create 12 Valid\nrecognize-profile 1 1\ndiagnostics\nquit\n")
execute_process(COMMAND "${EXE}" --identity-session --data-root "${root}"
    INPUT_FILE "${input}" RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE errors
    ENCODING UTF-8 TIMEOUT 25)
if(NOT "${status}" STREQUAL "0" OR NOT errors MATCHES "Input line exceeds the 4096-byte limit" OR
    NOT errors MATCHES "No explicit trust selection exists" OR
    NOT output MATCHES "profiles=1 memberships=0 devices=1 private_keys=2")
    message(FATAL_ERROR "Identity malformed-input handling failed")
endif()
if("${output}${errors}" MATCHES "DO_NOT_ECHO|198\\.51\\.100\\.19|PRIVATE_MARKER")
    message(FATAL_ERROR "Rejected identity input leaked")
endif()

# Restart creates no profile, including EOF without quit. Previous state is gone.
file(WRITE "${input}" "diagnostics\n")
execute_process(COMMAND "${EXE}" --identity-session --data-root "${root}"
    INPUT_FILE "${input}" RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE errors
    ENCODING UTF-8 TIMEOUT 25)
if(NOT "${status}" STREQUAL "0" OR NOT output MATCHES "profiles=0 memberships=0 devices=0 private_keys=0" OR
    NOT output MATCHES "Identity session closed; volatile state cleared")
    message(FATAL_ERROR "Identity EOF/restart cleanup failed")
endif()
file(GLOB_RECURSE stored_files LIST_DIRECTORIES false "${root}/*")
if(stored_files)
    message(FATAL_ERROR "Identity restart unexpectedly persisted a file")
endif()

foreach(bad_command IN ITEMS --version --help --check-config --identity-session)
    execute_process(COMMAND "${EXE}" --identity-session "${bad_command}"
        RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE errors TIMEOUT 10)
    if(NOT "${status}" STREQUAL "2")
        message(FATAL_ERROR "Identity session accepted incompatible command combination")
    endif()
endforeach()
file(REMOVE "${input}")
message(STATUS "Identity session workflow, isolation, malformed input and cleanup passed")
