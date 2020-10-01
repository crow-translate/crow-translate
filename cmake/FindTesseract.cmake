# Most distributions use autotools build system for Tesseract packaging.
# Therefore, I wrote my own using the official guide:
# https://cmake.org/cmake/help/latest/manual/cmake-developer.7.html#find-modules

find_path(Tesseract_INCLUDE_DIR
    NAMES tesseract/baseapi.h
    PATHS ${PC_Tesseract_INCLUDE_DIRS}
)
find_library(Tesseract_LIBRARY
    NAMES tesseract tesseract41
    PATHS ${PC_Tesseract_LIBRARY_DIRS}
)

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
    endif()
endif()

mark_as_advanced(
    Foo_INCLUDE_DIR
    Foo_LIBRARY
)
