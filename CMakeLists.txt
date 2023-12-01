cmake_minimum_required(VERSION 3.16.0)

project(crow-translate
    VERSION 2.11.0
    DESCRIPTION "A simple and lightweight translator that allows to translate and speak text using Google, Yandex, Bing, LibreTranslate and Lingva"
    HOMEPAGE_URL crow-translate.github.io
    LANGUAGES CXX
)

set(APPLICATION_NAME "Crow Translate")
set(APPLICATION_ID "io.crow_translate.CrowTranslate")
set(EXECUTABLE_NAME crow)
if(UNIX)
    set(DESKTOP_FILE ${APPLICATION_ID}.desktop)
    set(METAINFO_FILE ${APPLICATION_ID}.metainfo.xml)
endif()

set(CPACK_PACKAGE_VENDOR ${APPLICATION_NAME})
set(CPACK_PACKAGE_EXECUTABLES ${EXECUTABLE_NAME} ${APPLICATION_NAME})
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${APPLICATION_NAME})
set(CPACK_PACKAGE_CONTACT genaloner@gmail.com)
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/COPYING)
set(CPACK_PROJECT_CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPackProjectConfig.cmake)

set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS_POLICY >=)
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "qtgstreamer-plugins-qt5, gstreamer1.0-plugins-good, gstreamer1.0-alsa, gstreamer1.0-pulseaudio, libqt5multimedia5-plugins")
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)
set(CPACK_RPM_PACKAGE_LICENSE GPLv3)
set(CPACK_RPM_PACKAGE_GROUP Applications/Text)
set(CPACK_RPM_COMPRESSION_TYPE lzma)
set(CPACK_NSIS_DISPLAY_NAME ${APPLICATION_NAME})
set(CPACK_NSIS_INSTALLED_ICON_NAME ${EXECUTABLE_NAME}.exe)
set(CPACK_NSIS_EXECUTABLES_DIRECTORY .)
set(CPACK_NSIS_MUI_FINISHPAGE_RUN ${EXECUTABLE_NAME}.exe)
set(CPACK_EXTERNAL_PACKAGE_SCRIPT ${CMAKE_SOURCE_DIR}/cmake/AppImage.cmake)
set(CPACK_EXTERNAL_ENABLE_STAGING YES)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_WIN32_EXECUTABLE ON)

if(CMAKE_CXX_CLANG_TIDY)
    # Exclude generated files
    # We generate a dummy .clang-tidy file in the binary directory that disables
    # all clang-tidy checks except one that will never match. This one check is
    # necessary; clang-tidy reports an error when no checks are enabled.
    # https://gitlab.kitware.com/cmake/cmake/-/issues/19772
    file(WRITE ${CMAKE_BINARY_DIR}/.clang-tidy "Checks: '-*,llvm-twine-local'")
endif()

option(WITH_PORTABLE_MODE "Enable portable functionality" OFF)
option(WITH_KWAYLAND "Use KWayland for better Wayland integration" ON)

find_package(ECM REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(QAPPLICATION_CLASS QApplication)
option(QHOTKEY_INSTALL OFF)

add_subdirectory(src/third-party/singleapplication)
add_subdirectory(src/third-party/qhotkey)
add_subdirectory(src/third-party/qtaskbarcontrol)
add_subdirectory(src/qonlinetranslator)
if(WIN32)
    add_subdirectory(src/qgittag)
endif()

find_package(Qt5 REQUIRED COMPONENTS Widgets LinguistTools Concurrent)
find_package(Tesseract REQUIRED)
if(UNIX)
    find_package(Qt5 REQUIRED COMPONENTS DBus)
    if(NOT APPLE)
        find_package(XCB REQUIRED COMPONENTS XCB)
        if(WITH_KWAYLAND)
            find_package(KF5Wayland REQUIRED COMPONENTS Client)
        endif()
    endif()
endif()

include(GNUInstallDirs)
include(CPack)
if(UNIX)
    include(ECMInstallIcons)
endif()
if(WIN32 OR APPLE)
    include(ECMAddAppIcon)
endif()

set(APP_ICONS
    data/icons/app/128-apps-crow-translate.png
    data/icons/app/150-apps-crow-translate.png
    data/icons/app/16-apps-crow-translate.png
    data/icons/app/22-apps-crow-translate.png
    data/icons/app/256-apps-crow-translate.png
    data/icons/app/310-apps-crow-translate.png
    data/icons/app/32-apps-crow-translate.png
    data/icons/app/44-apps-crow-translate.png
    data/icons/app/48-apps-crow-translate.png
    data/icons/app/512-apps-crow-translate.png
    data/icons/app/64-apps-crow-translate.png
)

qt5_add_translation(QM_FILES
    data/translations/${PROJECT_NAME}_ar_SA.ts
    data/translations/${PROJECT_NAME}_az_AZ.ts
    data/translations/${PROJECT_NAME}_de_DE.ts
    data/translations/${PROJECT_NAME}_el_GR.ts
    data/translations/${PROJECT_NAME}_es_ES.ts
    data/translations/${PROJECT_NAME}_eu_ES.ts
    data/translations/${PROJECT_NAME}_fi_FI.ts
    data/translations/${PROJECT_NAME}_fr_FR.ts
    data/translations/${PROJECT_NAME}_hi_IN.ts
    data/translations/${PROJECT_NAME}_hu_HU.ts
    data/translations/${PROJECT_NAME}_id_ID.ts
    data/translations/${PROJECT_NAME}_it_IT.ts
    data/translations/${PROJECT_NAME}_ko_KR.ts
    data/translations/${PROJECT_NAME}_ms_MY.ts
    data/translations/${PROJECT_NAME}_nl_NL.ts
    data/translations/${PROJECT_NAME}_pl_PL.ts
    data/translations/${PROJECT_NAME}_pt_BR.ts
    data/translations/${PROJECT_NAME}_pt_PT.ts
    data/translations/${PROJECT_NAME}_ru_RU.ts
    data/translations/${PROJECT_NAME}_sq_AL.ts
    data/translations/${PROJECT_NAME}_tr_TR.ts
    data/translations/${PROJECT_NAME}_ug_CN.ts
    data/translations/${PROJECT_NAME}_uk_UA.ts
    data/translations/${PROJECT_NAME}_zh_CN.ts
    data/translations/${PROJECT_NAME}_zh_TW.ts
)

configure_file(src/cmake.h.in cmake.h)

add_executable(${PROJECT_NAME}
    data/icons/flags.qrc
    ${QM_FILES}
    data/icons/icon-theme.qrc
    data/icons/engines/engines.qrc
    src/languagesdialog.cpp
    src/languagesdialog.ui
    src/cli.cpp
    src/contextmenu.cpp
    src/languagebuttonswidget.cpp
    src/languagebuttonswidget.ui
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.ui
    src/ocr/ocr.cpp
    src/ocr/screengrabbers/abstractscreengrabber.cpp
    src/ocr/screengrabbers/genericscreengrabber.cpp
    src/ocr/snippingarea.cpp
    src/popupwindow.cpp
    src/popupwindow.ui
    src/screenwatcher.cpp
    src/selection.cpp
    src/settings/appsettings.cpp
    src/settings/ocrlanguageslistwidget.cpp
    src/settings/settingsdialog.cpp
    src/settings/settingsdialog.ui
    src/settings/shortcutsmodel/shortcutitem.cpp
    src/settings/shortcutsmodel/shortcutsmodel.cpp
    src/settings/shortcutsmodel/shortcutsview.cpp
    src/settings/tesseractparameterstablewidget.cpp
    src/settings/autostartmanager/abstractautostartmanager.cpp
    src/sourcetextedit.cpp
    src/speakbuttons.cpp
    src/speakbuttons.ui
    src/transitions/languagedetectedtransition.cpp
    src/transitions/ocruninitializedtransition.cpp
    src/transitions/playerstoppedtransition.cpp
    src/transitions/retranslationtransition.cpp
    src/transitions/snippingvisibletransition.cpp
    src/transitions/textemptytransition.cpp
    src/transitions/translatorabortedtransition.cpp
    src/transitions/translatorerrortransition.cpp
    src/translationedit.cpp
    src/trayicon.cpp
)

if(UNIX AND NOT APPLE)
    target_sources(${PROJECT_NAME} PRIVATE
        src/xdgdesktopportal.cpp
        src/ocr/screengrabbers/dbusscreengrabber.cpp
        src/ocr/screengrabbers/waylandgnomescreengrabber.cpp
        src/ocr/screengrabbers/waylandplasmascreengrabber.cpp
        src/ocr/screengrabbers/waylandportalscreengrabber.cpp
        src/settings/autostartmanager/unixautostartmanager.cpp
        src/settings/autostartmanager/portalautostartmanager.cpp
    )
    if(WITH_KWAYLAND)
        target_sources(${PROJECT_NAME} PRIVATE
            src/waylandhelper.cpp
        )
    endif(WITH_KWAYLAND)
elseif(APPLE)
    ecm_add_app_icon(MACOS_ICON ICONS ${APP_ICONS})

    target_sources(${PROJECT_NAME} PRIVATE
        ${MACOS_ICON}
        data/icons/app/app.qrc
        src/settings/autostartmanager/macosautostartmanager.cpp
    )
elseif(WIN32)
    ecm_add_app_icon(WINDOWS_ICON ICONS ${APP_ICONS})

    target_sources(${PROJECT_NAME} PRIVATE
        ${WINDOWS_ICON}
        data/icons/app/app.qrc
        src/updaterdialog.cpp
        src/updaterdialog.ui
        src/settings/autostartmanager/windowsautostartmanager.cpp
    )
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${EXECUTABLE_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE
    SingleApplication::SingleApplication
    QHotkey::QHotkey
    QTaskbarControl::QTaskbarControl
    QOnlineTranslator::QOnlineTranslator
    Qt5::Concurrent
    Tesseract::Tesseract
)

if(UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::DBus)
    if(NOT APPLE)
        target_link_libraries(${PROJECT_NAME} PRIVATE XCB::XCB)
        if(WITH_KWAYLAND)
            target_link_libraries(${PROJECT_NAME} PRIVATE KF5::WaylandClient)
            target_compile_definitions(${PROJECT_NAME} PRIVATE WITH_KWAYLAND)
        endif()
    endif()
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE QGitTag::QGitTag)
endif()

if(WITH_PORTABLE_MODE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE WITH_PORTABLE_MODE)
endif()

if(UNIX AND NOT APPLE)
    install(TARGETS ${PROJECT_NAME})
    install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/${PROJECT_NAME}/translations)
    install(FILES data/${DESKTOP_FILE} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
    install(FILES data/${METAINFO_FILE} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo)

    ecm_install_icons(ICONS
        ${APP_ICONS}
        data/icons/app/sc-apps-crow-translate.svg
        data/icons/app/16-status-crow-translate-tray-dark.png
        data/icons/app/16-status-crow-translate-tray-light.png
        data/icons/app/22-status-crow-translate-tray-dark.png
        data/icons/app/22-status-crow-translate-tray-light.png
        data/icons/app/24-status-crow-translate-tray-dark.png
        data/icons/app/24-status-crow-translate-tray-light.png
        data/icons/app/sc-status-crow-translate-tray-dark.svg
        data/icons/app/sc-status-crow-translate-tray-light.svg
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons
    )
elseif(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_BUNDLE_NAME ${APPLICATION_NAME}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_GUI_IDENTIFIER ${APPLICATION_ID}
        MACOSX_BUNDLE_INFO_STRING ${PROJECT_DESCRIPTION}
        RESOURCE "${QM_FILES}"
    )
elseif(WIN32)
    install(TARGETS ${PROJECT_NAME} DESTINATION .)
    install(FILES ${QM_FILES} DESTINATION translations)

    # Bundle Qt dependencies
    find_program(WINDEPLOYQT windeployqt PATH_SUFFIXES tools/qt5/bin)
    install(CODE "execute_process(COMMAND ${WINDEPLOYQT} --no-compiler-runtime --no-angle \${CMAKE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:${PROJECT_NAME}>)")

    # Bundle OpenSLL libraries that required for Qt networking manually because it not handled by windeployqt
    find_file(SLL_DLL NAMES libssl-1_1-x64.dll libssl-1_1.dll libssl.dll PATH_SUFFIXES tools/qt5/bin)
    find_file(CRYPTO_DLL NAMES libcrypto-1_1-x64.dll libcrypto-1_1.dll libcrypto.dll PATH_SUFFIXES tools/qt5/bin)
    install(FILES ${SLL_DLL} ${CRYPTO_DLL} DESTINATION .)

    # Install other non-Qt runtime depenencies
    install(CODE [[
        file(GET_RUNTIME_DEPENDENCIES
            EXECUTABLES $<TARGET_FILE:crow-translate>
            PRE_EXCLUDE_REGEXES api-ms qt5
            POST_EXCLUDE_REGEXES C:[/\\]Windows[/\\]
            RESOLVED_DEPENDENCIES_VAR RESOLVED_DEPENDENCIES
            UNRESOLVED_DEPENDENCIES_VAR UNRESOLVED_DEPENDENCIES
        )
        foreach(DEPENDENCY ${UNRESOLVED_DEPENDENCIES})
            message(WARNING "Unable to resolve dependency ${DEPENDENCY}")
        endforeach()
        foreach(DEPENDENCY ${RESOLVED_DEPENDENCIES})
            message("Copying ${DEPENDENCY}")
            file(INSTALL
                DESTINATION "${CMAKE_INSTALL_PREFIX}"
                FILES "${DEPENDENCY}"
            )
        endforeach()
        file(WRITE ${CMAKE_INSTALL_PREFIX}/qt.conf "[Paths]") # Setup Qt libraries paths
    ]])
endif()
