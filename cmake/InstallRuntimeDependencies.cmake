# Installs all required DLLs and Qt translations
# Only works for VCPKG
function(install_runtime_dependencies TARGET)
    if(VCPKG_TOOLCHAIN AND _VCPKG_TARGET_TRIPLET_PLAT STREQUAL windows)
        install(CODE "execute_process(COMMAND
            powershell -noprofile -executionpolicy Bypass -file \"${_VCPKG_TOOLCHAIN_DIR}/msbuild/applocal.ps1\"
            -targetBinary \"\${CMAKE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:${TARGET}>\"
            -installedDir \"${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}$<$<CONFIG:Debug>:/debug>/bin\"
            -OutVariable out)"
        )
        file(GLOB QT_TRANSLATIONS ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/qt5/translations/qtbase_*.qm)
        install(FILES ${QT_TRANSLATIONS} DESTINATION translations)
    endif()
endfunction()
