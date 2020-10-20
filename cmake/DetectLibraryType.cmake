# Detect library type of a library
# Returns one of `SHARED`, `STATIC`, or `UNKNOWN` into the variable passed as the first argument

function(detect_library_type LIBRARY_TYPE)
    cmake_parse_arguments(PARSE_ARGV 1 PARAMS "" "PATH" "")

    if(NOT DEFINED PARAMS_PATH)
        message(FATAL_ERROR "The `PATH` argument is required.")
    endif()

    if(DEFINED PARAMS_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unparsed arguments for detect_library_type: " "${PARAMS_UNPARSED_ARGUMENTS}")
    endif()

    if(NOT PARAMS_PATH)
        message(FATAL_ERROR "The `PATH` argument is empty.")
    endif()

    set(${LIBRARY_TYPE} UNKNOWN PARENT_SCOPE)
    # Windows libraries all end with `.lib`. We need to detect the type based on
    # the contents of the library. However, MinGW does use different extensions.
    if(MSVC)
        get_filename_component(CXX_COMPILER_FOLDER ${CMAKE_CXX_COMPILER} DIRECTORY CACHE)
        find_program(DUMPBIN_EXECUTABLE
            NAMES dumpbin
            HINTS ${CXX_COMPILER_FOLDER}
        )
        mark_as_advanced(DUMPBIN_EXECUTABLE)
        execute_process(COMMAND "${DUMPBIN_EXECUTABLE}" /HEADERS "${PARAMS_PATH}"
            OUTPUT_VARIABLE DUMPBIN_OUTPUT
            ERROR_VARIABLE  DUMPBIN_ERROR
            RESULT_VARIABLE DUMPBIN_RESULT)
        if(DUMPBIN_RESULT EQUAL 0)
            if(DUMPBIN_OUTPUT MATCHES "DLL name     :")
                set(${LIBRARY_TYPE} SHARED PARENT_SCOPE)
            else()
                set(${LIBRARY_TYPE} STATIC PARENT_SCOPE)
            endif()
        else()
            message(WARNING "Failed to run `dumpbin` on ${PARAMS_PATH}. Cannot determine shared/static library type: ${DUMPBIN_ERROR}")
        endif()
    else()
        string(LENGTH "${PARAMS_PATH}" PATH_LENGTH)

        string(LENGTH "${CMAKE_SHARED_LIBRARY_SUFFIX}" SHARED_SUFFIX_LENGTH)
        math(EXPR SHARED_INDEX "${PATH_LENGTH} - ${SHARED_SUFFIX_LENGTH}")
        string(SUBSTRING "${PARAMS_PATH}" "${SHARED_INDEX}" -1 SHARED_SUFFIX)

        string(LENGTH "${CMAKE_STATIC_LIBRARY_SUFFIX}" STATIC_SUFFIX_LENGTH)
        math(EXPR STATIC_INDEX "${PATH_LENGTH} - ${STATIC_SUFFIX_LENGTH}")
        string(SUBSTRING "${PARAMS_PATH}" "${STATIC_INDEX}" -1 STATIC_SUFFIX)

        if(SHARED_SUFFIX STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
            set(${LIBRARY_TYPE} SHARED PARENT_SCOPE)
        elseif(STATIC_SUFFIX STREQUAL CMAKE_STATIC_LIBRARY_SUFFIX)
            set(${LIBRARY_TYPE} STATIC PARENT_SCOPE)
        endif()

        # When import suffix != static suffix, we can disambiguate static and import
        if(WIN32 AND NOT CMAKE_IMPORT_LIBRARY_SUFFIX STREQUAL CMAKE_STATIC_LIBRARY_SUFFIX)
            string(LENGTH "${CMAKE_IMPORT_LIBRARY_SUFFIX}" IMPORT_SUFFIX_LENGTH)
            math(EXPR IMPORT_SUFFIX_INDEX "${PATH_LENGTH} - ${IMPORT_SUFFIX_LENGTH}")
            string(SUBSTRING "${PARAMS_PATH}" "${IMPORT_SUFFIX_INDEX}" -1 IMPORT_SUFFIX)
            if(IMPORT_SUFFIX STREQUAL CMAKE_IMPORT_LIBRARY_SUFFIX)
                set(${LIBRARY_TYPE} SHARED PARENT_SCOPE)
            endif()
        endif()
    endif()
endfunction()
