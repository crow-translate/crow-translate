# Dependency for Leptonica

find_package(PkgConfig)
pkg_check_modules(PC_WEBP QUIET webp)

find_path(WEBP_INCLUDE_DIR
    NAMES webp/decode.h
    PATHS ${PC_WEBP_INCLUDE_DIRS}
)

find_library(WEBP_LIBRARY_RELEASE
    NAMES webp
    PATHS ${PC_WEBP_LIBRARY_DIRS}
)
find_library(WEBP_LIBRARY_DEBUG
    NAMES webpd
    PATHS ${PC_WEBP_LIBRARY_DIRS}
)
include(SelectLibraryConfigurations)
select_library_configurations(WEBP)

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
            IMPORTED_LOCATION_RELEASE "${WEBP_LIBRARY_RELEASE}"
            IMPORTED_LOCATION_DEBUG "${WEBP_LIBRARY_DEBUG}"
            INTERFACE_COMPILE_OPTIONS "${PC_WEBP_CFLAGS_OTHER}"
            INTERFACE_INCLUDE_DIRECTORIES "${WEBP_INCLUDE_DIR}"
        )
    endif()
endif()

mark_as_advanced(
    WEBP_INCLUDE_DIR
    WEBP_LIBRARY
    WEBP_LIBRARY_RELEASE
    WEBP_LIBRARY_DEBUG
)
