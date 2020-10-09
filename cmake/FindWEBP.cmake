# Dependency for Leptonica

find_path(WEBP_INCLUDE_DIR
    NAMES webp/decode.h
    PATHS ${PC_WEBP_INCLUDE_DIRS}
)
find_library(WEBP_LIBRARY
    NAMES webp
    PATHS ${PC_WEBP_LIBRARY_DIRS}
)

set(WEBP_VERSION ${PC_WEBP_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WEBP
    FOUND_VAR WEBP_FOUND
    REQUIRED_VARS
        WEBP_LIBRARY
        WEBP_INCLUDE_DIR
    VERSION_VAR WEBP_VERSION
)

if(WEBP_FOUND)
    set(WEBP_LIBRARIES ${WEBP_LIBRARY})
    set(WEBP_INCLUDE_DIRS ${WEBP_INCLUDE_DIR})
    set(WEBP_DEFINITIONS ${PC_WEBP_CFLAGS_OTHER})

    if(NOT TARGET WEBP::WEBP)
        add_library(WEBP::WEBP UNKNOWN IMPORTED)
        set_target_properties(WEBP::WEBP PROPERTIES
            IMPORTED_LOCATION "${WEBP_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${PC_WEBP_CFLAGS_OTHER}"
            INTERFACE_INCLUDE_DIRECTORIES "${WEBP_INCLUDE_DIR}"
        )
    endif()
endif()

mark_as_advanced(
    WEBP_INCLUDE_DIR
    WEBP_LIBRARY
)
