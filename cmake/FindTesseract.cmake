# Most distributions use the autotools build system for packaging Tesseract, which currently does not generate CMake Config files

find_package(PkgConfig)
pkg_check_modules(PC_Tesseract QUIET tesseract)

find_path(Tesseract_INCLUDE_DIR
    NAMES tesseract/baseapi.h
    PATHS ${PC_Tesseract_INCLUDE_DIRS}
)

if(PC_Tesseract_VERSION)
    # Parse version numbers as possible postfixes in the library name
    string(REGEX MATCHALL [0-9]+ Tesseract_VERSION_NUMBERS ${PC_Tesseract_VERSION})
    list(GET Tesseract_VERSION_NUMBERS 0 Tesseract_MAJOR_VERSION)
    list(GET Tesseract_VERSION_NUMBERS 1 Tesseract_MINOR_VERSION)

    set(Tesseract_VERSION ${PC_Tesseract_VERSION})
endif()

find_library(Tesseract_LIBRARY
    NAMES tesseract tesseract${Tesseract_MAJOR_VERSION}${Tesseract_MINOR_VERSION}d tesseract${Tesseract_MAJOR_VERSION}${Tesseract_MINOR_VERSION}
    PATHS ${PC_Tesseract_LIBRARY_DIRS}
)

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
        target_link_libraries(Tesseract::Tesseract INTERFACE Leptonica::Leptonica)
    endif()
endif()

mark_as_advanced(
    Tesseract_INCLUDE_DIR
    Tesseract_LIBRARY
)
