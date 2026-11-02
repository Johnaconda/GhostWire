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

# Numbered navigation uses the real service, including duplicate display aliases.
# Status/list disclose labels only when requested; no public-key dump is implicit.
file(WRITE "${input}" "menu\n3\n1\n00123\nJörg Menu Owner\n4\n"
    "create 00123 Other Menu Profile\n2\n3\n1\n4\ndiagnostics\n6\n0\n")
execute_process(COMMAND "${EXE}" --identity-session --data-root "${root}"
    INPUT_FILE "${input}" RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE errors
    ENCODING UTF-8 TIMEOUT 25)
if(NOT "${status}" STREQUAL "0" OR errors MATCHES "Identity command error|GhostWire error")
    message(FATAL_ERROR "Guided local console workflow failed")
endif()
foreach(expected IN ITEMS
    "Local identity menu:"
    "No profiles yet. Use 1 or create to create one."
    "numeric alias (1-20 digits; back/cancel/quit)>"
    "display name (UTF-8, 1-256 bytes; back/cancel/quit)>"
    "profile number>"
    "name=Other Menu Profile devices=1 revision=1 [selected]"
    "Selected profile: alias=00123 name=Jörg Menu Owner devices=1 revision=1 prepared_memberships=0"
    "profiles=2 memberships=0 devices=2 private_keys=4"
    "Identity session closed; volatile state cleared")
    string(FIND "${output}" "${expected}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Missing guided console behavior: ${expected}")
    endif()
endforeach()
if(output MATCHES "authority_public_key=|signed_public_record=|identity=" OR
    errors MATCHES "Jörg|Menu Owner|Other Menu Profile|00123")
    message(FATAL_ERROR "Guided navigation disclosed keys or logged labels implicitly")
endif()

# Cancel, rejected labels, overflow and unavailable selections leave existing
# profiles intact. Invalid selection must stay in the session/prompt for retry.
string(REPEAT "N" 257 long_name)
file(WRITE "${input}" "create\nDO_NOT_ECHO\ncancel\n"
    "create\n${oversized}\ncancel\n"
    "create\n123\n${oversized}\n${long_name}\ncancel\n"
    "create\n123\nPRIVATE_MARKER_${invalid_utf8}\ndiagnostics\n"
    "create 456 Preserved\nselect\n0\n99\ncancel\nstatus\n"
    "select 999\ndiagnostics\nquit\n")
execute_process(COMMAND "${EXE}" --identity-session --data-root "${root}"
    INPUT_FILE "${input}" RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE errors
    ENCODING UTF-8 TIMEOUT 25)
if(NOT "${status}" STREQUAL "0" OR
    NOT output MATCHES "profiles=0 memberships=0 devices=0 private_keys=0" OR
    NOT output MATCHES "Selected profile: alias=456 name=Preserved" OR
    NOT output MATCHES "profiles=1 memberships=0 devices=1 private_keys=2" OR
    NOT errors MATCHES "Alias requires 1-20 ASCII digits" OR
    NOT errors MATCHES "Display name requires 1-256 UTF-8 bytes" OR
    NOT errors MATCHES "Selection is unavailable" OR
    NOT errors MATCHES "Input line exceeds the 4096-byte limit")
    message(FATAL_ERROR "Guided cancellation, rejection or selection recovery failed")
endif()
if("${output}${errors}" MATCHES "DO_NOT_ECHO|198\\.51\\.100\\.19|PRIVATE_MARKER")
    message(FATAL_ERROR "Rejected guided input leaked")
endif()

# EOF during either creation prompt (including an unterminated alias) never
# creates an identity. No extra read/pause is required after EOF.
foreach(partial IN ITEMS "create\n" "create\n123\n" "create\n123")
    file(WRITE "${input}" "${partial}")
    execute_process(COMMAND "${EXE}" --identity-session --data-root "${root}"
        INPUT_FILE "${input}" RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE errors
        ENCODING UTF-8 TIMEOUT 25)
    if(NOT "${status}" STREQUAL "0" OR output MATCHES "Profile created and selected" OR
        NOT output MATCHES "Identity session closed; volatile state cleared")
        message(FATAL_ERROR "EOF during guided creation failed")
    endif()
endforeach()
file(GLOB_RECURSE stored_files LIST_DIRECTORIES false "${root}/*")
if(stored_files)
    message(FATAL_ERROR "Guided workflow unexpectedly persisted a file")
endif()

# Guided membership lifecycle uses existing identity services. Label changes keep
# the same authority; deleting/recreating credentials produces a fresh authority.
file(WRITE "${input}" "create 00123 Jörg Lifecycle Owner\n7\n${community}\n00123\nMember One\n"
    "show membership 1\n8\n1\n2\n00456\nManaged Member\n3\n4\n1\n5\n2\n1\n7\n0\n"
    "show membership 1\nrename-membership 1 00789 Direct Member\nshow membership 1\n"
    "show profile\n9\n00111\nRenamed Lifecycle Owner\nshow profile\n"
    "create 00123 Other Lifecycle Owner\n7\n${community}\n00123\nOther Member\nshow membership 1\n"
    "select 1\ndelete membership 1\ncancel\nmemberships\n"
    "delete membership 1\ndelete\nmemberships\nshow profile\n"
    "membership ${community} 00123 Recreated Member\nshow membership 1\n"
    "select 2\nmemberships\ndiagnostics\nquit\n")
execute_process(COMMAND "${EXE}" --identity-session --data-root "${root}"
    INPUT_FILE "${input}" RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE errors
    ENCODING UTF-8 TIMEOUT 25)
if(NOT "${status}" STREQUAL "0" OR errors MATCHES "Identity command error|GhostWire error")
    message(FATAL_ERROR "Guided membership lifecycle failed: ${errors}")
endif()
foreach(expected IN ITEMS
    "No pin or credentials are created until all fields are submitted"
    "Managed membership: alias=00456 name=Managed Member devices=1 revision=4"
    "Membership labels changed; identity unchanged"
    "Profile labels changed; identity unchanged"
    "name=Direct Member"
    "Local membership deleted; community pin retained; other scopes unchanged"
    "Prepared memberships: 0"
    "name=Other Member"
    "profiles=2 memberships=2 devices=4 private_keys=8")
    string(FIND "${output}" "${expected}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Missing membership lifecycle behavior: ${expected}")
    endif()
endforeach()
string(REGEX MATCHALL "identity=[0-9a-f]+" ids "${output}")
string(REGEX MATCHALL "authority_public_key=[0-9a-f]+" keys "${output}")
list(LENGTH ids id_count)
list(LENGTH keys key_count)
if(NOT id_count EQUAL 9 OR NOT key_count EQUAL 9)
    message(FATAL_ERROR "Missing explicit lifecycle identity/key disclosures")
endif()
foreach(values IN ITEMS ids keys)
    list(GET ${values} 0 original)
    list(GET ${values} 1 managed)
    list(GET ${values} 2 managed_after)
    list(GET ${values} 3 renamed)
    list(GET ${values} 4 profile_before)
    list(GET ${values} 5 profile_after)
    list(GET ${values} 6 other)
    list(GET ${values} 7 profile_after_delete)
    list(GET ${values} 8 recreated)
    if(NOT original STREQUAL managed OR NOT original STREQUAL managed_after OR NOT original STREQUAL renamed OR
        NOT profile_before STREQUAL profile_after OR NOT profile_before STREQUAL profile_after_delete OR
        original STREQUAL other OR original STREQUAL recreated OR other STREQUAL recreated)
        message(FATAL_ERROR "Lifecycle changed label authority or reused another identity/key")
    endif()
endforeach()
if(errors MATCHES "Jörg|Lifecycle|00123|00456|00789|${community}|Member")
    message(FATAL_ERROR "Lifecycle labels leaked into diagnostics")
endif()

# Profile deletion clears its own keys and UI handles, preserving an observer's
# independent trust pin. That orphaned public pin remains explicitly forgettable.
file(WRITE "${input}" "create 123 Deleted Owner\nshow profile\n"
    "membership ${community} 123 Deleted Member\ncreate 123 Observer\ntrust-profile 1\n"
    "select 1\ntrust-profile 2\ndelete profile\nno\ncancel\nstatus\n"
    "10\ndelete\ndiagnostics\nstatus\nprofiles\nselect 1\ntrusts\n"
    "forget-trust 2\nforget-trust 1\nforget extra\ncancel\n"
    "forget-trust 1\nforget\ntrusts\n"
    "create 123 Replacement\nshow profile\nselect 1\nrecognize-profile 2 1\n"
    "trust-profile 2\nselect 2\ntrust-profile 1\ndelete profile\ndelete\n"
    "select 1\ntrusts\nforget-trust 1\nforget\ndiagnostics\nquit\n")
execute_process(COMMAND "${EXE}" --identity-session --data-root "${root}"
    INPUT_FILE "${input}" RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE errors
    ENCODING UTF-8 TIMEOUT 25)
if(NOT "${status}" STREQUAL "0" OR
    NOT output MATCHES "Selected profile: alias=123 name=Deleted Owner" OR
    NOT output MATCHES "Local profile and its membership keys deleted; no profile selected" OR
    NOT output MATCHES "Trust 1: subject_profile=deleted state=pinned revision=1 has_record=true" OR
    NOT output MATCHES "Console-managed trust pins for selected profile: 0" OR
    NOT output MATCHES "Trust pin forgotten locally; no private keys changed" OR
    NOT output MATCHES "profiles=1 memberships=0 devices=1 private_keys=2 trust_contexts=0" OR
    NOT errors MATCHES "Type exactly delete" OR NOT errors MATCHES "Type exactly forget" OR
    NOT errors MATCHES "Explicitly create or select a profile first" OR
    NOT errors MATCHES "Selection is unavailable" OR
    NOT errors MATCHES "No explicit trust selection exists")
    message(FATAL_ERROR "Profile cleanup, confirmation or independent trust handling failed")
endif()
string(REGEX MATCHALL "identity=[0-9a-f]+" ids "${output}")
list(LENGTH ids count)
if(NOT count EQUAL 2)
    message(FATAL_ERROR "Missing profile recreation identity checks")
endif()
list(GET ids 0 original)
list(GET ids 1 replacement)
if(original STREQUAL replacement)
    message(FATAL_ERROR "Recreated alias reused deleted profile identity")
endif()
if(errors MATCHES "Deleted Owner|Deleted Member|Observer|Replacement|${community}")
    message(FATAL_ERROR "Cleanup diagnostics leaked identity labels")
endif()

# Pending keys/labels, rejected input and cancelled confirmations have no mutation.
file(WRITE "${input}" "create 123 Preserved Owner\n"
    "membership\nDO_NOT_ECHO\n${oversized}\n${community}\nDO_NOT_ECHO\n123\n"
    "${oversized}\n${long_name}\ncancel\ndiagnostics\n"
    "rename\nDO_NOT_ECHO\ncancel\nmanage-membership 999\ndelete membership 0\n"
    "delete profile\nDO_NOT_ECHO\n${oversized}\nDELETE\ncancel\nstatus\ndiagnostics\n"
    "membership\n${community}\n123\nPRIVATE_MARKER_${invalid_utf8}\ndiagnostics\nquit\n")
execute_process(COMMAND "${EXE}" --identity-session --data-root "${root}"
    INPUT_FILE "${input}" RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE errors
    ENCODING UTF-8 TIMEOUT 25)
if(NOT "${status}" STREQUAL "0" OR
    NOT output MATCHES "Selected profile: alias=123 name=Preserved Owner" OR
    NOT output MATCHES "profiles=1 memberships=0 devices=1 private_keys=2 trust_contexts=0" OR
    NOT output MATCHES "profiles=1 memberships=0 devices=1 private_keys=2 trust_contexts=1" OR
    NOT output MATCHES "Community pin retained; membership preparation failed" OR
    output MATCHES "Local profile and its membership keys deleted|credentials prepared \\(not admitted\\)" OR
    NOT errors MATCHES "Community key requires 64 lowercase hex characters" OR
    NOT errors MATCHES "Alias requires 1-20 ASCII digits" OR
    NOT errors MATCHES "Type exactly delete")
    message(FATAL_ERROR "Pending membership/rename/deletion rejection or cancellation failed")
endif()
if("${output}${errors}" MATCHES "DO_NOT_ECHO|198\\.51\\.100\\.19|PRIVATE_MARKER")
    message(FATAL_ERROR "Rejected lifecycle input leaked")
endif()

# EOF before submission/confirmation applies no pending lifecycle action.
foreach(partial IN ITEMS
    "membership\n"
    "membership\n${community}\n"
    "membership\n${community}\n123\n"
    "rename\n123\n"
    "delete profile\n"
    "membership ${community} 123 Keep Member\nmanage-membership\n"
    "membership ${community} 123 Keep Member\nmanage-membership 1\n6\n"
    "trust-profile 1\nforget-trust 1\n")
    file(WRITE "${input}" "create 123 Preserved Owner\n${partial}")
    execute_process(COMMAND "${EXE}" --identity-session --data-root "${root}"
        INPUT_FILE "${input}" RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE errors
        ENCODING UTF-8 TIMEOUT 25)
    if(NOT "${status}" STREQUAL "0" OR
        output MATCHES "Local profile and its membership keys deleted|Local membership deleted|Trust pin forgotten|labels changed" OR
        NOT output MATCHES "Identity session closed; volatile state cleared")
        message(FATAL_ERROR "EOF applied an unconfirmed lifecycle action")
    endif()
endforeach()
file(GLOB_RECURSE stored_files LIST_DIRECTORIES false "${root}/*")
if(stored_files)
    message(FATAL_ERROR "Membership lifecycle unexpectedly persisted a file")
endif()

# Reproduce the owner's numeric-alias confusion. Invalid input stays private;
# help explains this field, back discards it, and exit closes at the name prompt.
file(WRITE "${input}" "1\nJohnaconda\nhelp\nback\ndiagnostics\ncreate\n123\nexit\n")
execute_process(COMMAND "${EXE}" --identity-session --data-root "${root}"
    INPUT_FILE "${input}" RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE errors
    ENCODING UTF-8 TIMEOUT 25)
if(NOT "${status}" STREQUAL "0" OR
    NOT errors MATCHES "Alias requires 1-20 ASCII digits" OR
    errors MATCHES "Johnaconda|Invalid command" OR
    NOT output MATCHES "Your visible name comes next" OR
    NOT output MATCHES "profiles=0 memberships=0 devices=0 private_keys=0" OR
    output MATCHES "Profile created and selected" OR
    NOT output MATCHES "Identity session closed; volatile state cleared")
    message(FATAL_ERROR "Alias guidance or owner-reported prompt escape failed")
endif()

# Exercise navigation across each pending field/action/confirmation context.
# Help must retain that context; back must discard pending work; quit must close
# rather than becoming a label, device action or deletion confirmation.
set(navigation_stages
    "create\n" "create\n123\n"
    "rename\n" "rename\n123\n" "select\n"
    "membership\n" "membership\n${community}\n" "membership\n${community}\n123\n"
    "manage-membership\n" "manage-membership 1\n"
    "manage-membership 1\n2\n" "manage-membership 1\n2\n123\n"
    "manage-membership 1\n4\n" "manage-membership 1\n5\n"
    "rename-membership 1\n" "rename-membership 1\n123\n"
    "delete profile\n" "delete membership 1\n" "forget-trust 1\n")
foreach(stage IN LISTS navigation_stages)
    foreach(navigation IN ITEMS back quit)
        file(WRITE "${input}" "create 12 Navigation Owner\n"
            "membership ${community} 12 Navigation Member\ntrust-profile 1\n"
            "${stage}help\n${navigation}\ndiagnostics\nquit\n")
        execute_process(COMMAND "${EXE}" --identity-session --data-root "${root}"
            INPUT_FILE "${input}" RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE errors
            ENCODING UTF-8 TIMEOUT 25)
        if(NOT "${status}" STREQUAL "0" OR errors MATCHES "Identity command error|GhostWire error" OR
            NOT output MATCHES "Navigation: back/cancel/menu/.." OR
            NOT output MATCHES "Identity session closed; volatile state cleared" OR
            output MATCHES "labels changed|Local profile and its membership keys deleted|Local membership deleted|Trust pin forgotten|device replaced|device revoked")
            message(FATAL_ERROR "Pending navigation changed state, failed guidance or did not close cleanly")
        endif()
        if(navigation STREQUAL "back")
            if(NOT output MATCHES "profiles=1 memberships=1 devices=2 private_keys=4 trust_contexts=2")
                message(FATAL_ERROR "Back failed to preserve existing profile/member/trust state")
            endif()
        elseif(output MATCHES "profiles=")
            message(FATAL_ERROR "Quit consumed more input instead of closing the pending console")
        endif()
    endforeach()
endforeach()

# Standalone navigation tolerates surrounding whitespace. Zero stays valid
# numeric alias data; full commands still allow navigation words as literal names.
foreach(navigation IN ITEMS cancel menu .. exit)
    file(WRITE "${input}" "create\n123\n  ${navigation} \t\ndiagnostics\nquit\n")
    execute_process(COMMAND "${EXE}" --identity-session --data-root "${root}"
        INPUT_FILE "${input}" RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE errors
        ENCODING UTF-8 TIMEOUT 25)
    if(NOT "${status}" STREQUAL "0" OR errors MATCHES "Identity command error|GhostWire error" OR
        output MATCHES "Profile created and selected" OR
        NOT output MATCHES "Identity session closed; volatile state cleared")
        message(FATAL_ERROR "Whitespace or navigation synonym handling failed")
    endif()
    if(navigation STREQUAL "exit")
        if(output MATCHES "profiles=")
            message(FATAL_ERROR "Exit did not stop input consumption")
        endif()
    elseif(NOT output MATCHES "profiles=0 memberships=0 devices=0 private_keys=0")
        message(FATAL_ERROR "Navigation retained or published pending profile fields")
    endif()
endforeach()
file(WRITE "${input}" "create\n0\nhelp\nVisible Name\nstatus\n"
    "rename 0 quit\nstatus\ncreate 1 back\nstatus\ncreate 2 help\nstatus\ndiagnostics\nquit\n")
execute_process(COMMAND "${EXE}" --identity-session --data-root "${root}"
    INPUT_FILE "${input}" RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE errors
    ENCODING UTF-8 TIMEOUT 25)
if(NOT "${status}" STREQUAL "0" OR errors MATCHES "Identity command error|GhostWire error" OR
    NOT output MATCHES "Selected profile: alias=0 name=Visible Name" OR
    NOT output MATCHES "Selected profile: alias=0 name=quit" OR
    NOT output MATCHES "Selected profile: alias=1 name=back" OR
    NOT output MATCHES "Selected profile: alias=2 name=help" OR
    NOT output MATCHES "profiles=3 memberships=0 devices=3 private_keys=6")
    message(FATAL_ERROR "Navigation stole zero alias data, help changed context or literal names regressed")
endif()
file(GLOB_RECURSE stored_files LIST_DIRECTORIES false "${root}/*")
if(stored_files)
    message(FATAL_ERROR "Prompt navigation unexpectedly persisted a file")
endif()

foreach(bad_command IN ITEMS --version --help --check-config --identity-session --bootstrap)
    execute_process(COMMAND "${EXE}" --identity-session "${bad_command}"
        RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE errors TIMEOUT 10)
    if(NOT "${status}" STREQUAL "2")
        message(FATAL_ERROR "Identity session accepted incompatible command combination")
    endif()
endforeach()
file(REMOVE "${input}")
message(STATUS "Identity session workflow, guided navigation, isolation, malformed input and cleanup passed")
