# Most distributions use the autotools build system for packaging Tesseract, which currently does not generate CMake Config files

find_package(PkgConfig)
pkg_check_modules(PC_Tesseract QUIET tesseract)

find_path(Tesseract_INCLUDE_DIR
    NAMES tesseract/baseapi.h
    PATHS ${PC_Tesseract_INCLUDE_DIRS}
)

find_library(Tesseract_LIBRARY_RELEASE
    NAMES tesseract ${PC_Tesseract_LIBRARIES}
    PATHS ${PC_Tesseract_LIBRARY_DIRS}
)
find_library(Tesseract_LIBRARY_DEBUG
    NAMES tesseractd ${PC_Tesseract_LIBRARIES}d
    PATHS ${PC_Tesseract_LIBRARY_DIRS}
)
include(SelectLibraryConfigurations)
select_library_configurations(Tesseract)

set(Tesseract_VERSION ${PC_Tesseract_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Tesseract
    FOUND_VAR Tesseract_FOUND
    REQUIRED_VARS
        Tesseract_LIBRARY
        Tesseract_INCLUDE_DIR
    VERSION_VAR Tesseract_VERSION
)

if(Tesseract_FOUND)
    find_package(Leptonica REQUIRED)
    set(Tesseract_LIBRARIES ${Tesseract_LIBRARY})
    set(Tesseract_INCLUDE_DIRS ${Tesseract_INCLUDE_DIR})
    set(Tesseract_DEFINITIONS ${PC_Tesseract_CFLAGS_OTHER})

    if(NOT TARGET Tesseract::Tesseract)
        add_library(Tesseract::Tesseract UNKNOWN IMPORTED)
        set_target_properties(Tesseract::Tesseract PROPERTIES
            IMPORTED_LOCATION "${Tesseract_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${PC_Tesseract_CFLAGS_OTHER}"
            INTERFACE_INCLUDE_DIRECTORIES "${Tesseract_INCLUDE_DIR}"
        )
        if(Tesseract_LIBRARY_RELEASE)
            set_target_properties(Tesseract::Tesseract PROPERTIES
                IMPORTED_LOCATION_RELEASE "${Tesseract_LIBRARY_RELEASE}"
            )
        endif()
        if(Tesseract_LIBRARY_DEBUG)
            set_target_properties(Tesseract::Tesseract PROPERTIES
                IMPORTED_LOCATION_DEBUG "${Tesseract_LIBRARY_DEBUG}"
            )
        endif()

        target_link_libraries(Tesseract::Tesseract INTERFACE Leptonica::Leptonica)
    endif()
endif()

mark_as_advanced(
    Tesseract_INCLUDE_DIR
    Tesseract_LIBRARY
    Tesseract_LIBRARY_RELEASE
    Tesseract_LIBRARY_DEBUG
)
