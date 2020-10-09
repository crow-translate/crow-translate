# Leptonica is currently placing CMake Config files in the wrong directory
# Dependency for Tesseract

find_path(Leptonica_INCLUDE_DIR
    NAMES leptonica/leptwin.h
    PATHS ${PC_Leptonica_INCLUDE_DIRS}
)
find_library(Leptonica_LIBRARY
    NAMES leptonica lept leptonica-1.78.0d leptonica-1.78.0
    PATHS ${PC_Leptonica_LIBRARY_DIRS}
)

set(Leptonica_VERSION ${PC_Leptonica_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Leptonica
    FOUND_VAR Leptonica_FOUND
    REQUIRED_VARS
        Leptonica_LIBRARY
        Leptonica_INCLUDE_DIR
    VERSION_VAR Leptonica_VERSION
)

if(Leptonica_FOUND)
    find_package(GIF REQUIRED)
    find_package(JPEG REQUIRED)
    find_package(PNG REQUIRED)
    find_package(TIFF REQUIRED)
    find_package(ZLIB REQUIRED)
    find_package(WEBP REQUIRED)
    set(Leptonica_LIBRARIES ${Leptonica_LIBRARY})
    set(Leptonica_INCLUDE_DIRS ${Leptonica_INCLUDE_DIR})
    set(Leptonica_DEFINITIONS ${PC_Leptonica_CFLAGS_OTHER})

    if(NOT TARGET Leptonica::Leptonica)
        add_library(Leptonica::Leptonica UNKNOWN IMPORTED)
        set_target_properties(Leptonica::Leptonica PROPERTIES
            IMPORTED_LOCATION "${Leptonica_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${PC_Leptonica_CFLAGS_OTHER}"
            INTERFACE_INCLUDE_DIRECTORIES "${Leptonica_INCLUDE_DIR}"
        )
        target_link_libraries(Leptonica::Leptonica INTERFACE ZLIB::ZLIB TIFF::TIFF PNG::PNG JPEG::JPEG GIF::GIF WEBP::WEBP)
    endif()
endif()

mark_as_advanced(
    Leptonica_INCLUDE_DIR
    Leptonica_LIBRARY
)
