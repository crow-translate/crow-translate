cmake_minimum_required(VERSION 3.16.0)

project(crow-translate VERSION 2.8.6 LANGUAGES CXX)

set(CMAKE_PROJECT_HOMEPAGE_URL crow-translate.github.io)
set(CMAKE_PROJECT_DESCRIPTION "A simple and lightweight translator that allows to translate and speak text using Google, Yandex and Bing")

set(APPLICATION_NAME "Crow Translate")
set(EXECUTABLE_NAME crow)
if(UNIX)
    set(DESKTOP_FILE io.crow_translate.CrowTranslate.desktop)
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

option(WITH_PORTABLE_MODE "Enable portable functionality" OFF)

find_package(ECM REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Qt5 REQUIRED COMPONENTS Widgets LinguistTools Concurrent)
find_package(Tesseract REQUIRED)
if(UNIX)
    find_package(Qt5 REQUIRED COMPONENTS DBus)
    find_package(XCB REQUIRED COMPONENTS XCB)
endif()

include(ExternalLibraries)
include(GNUInstallDirs)
include(CPack)
if(UNIX)
    include(ECMInstallIcons)
elseif(WIN32)
    include(ECMAddAppIcon)
endif()

set(APP_ICONS
    data/icons/freedesktop/128-apps-crow-translate.png
    data/icons/freedesktop/150-apps-crow-translate.png
    data/icons/freedesktop/16-apps-crow-translate.png
    data/icons/freedesktop/22-apps-crow-translate.png
    data/icons/freedesktop/256-apps-crow-translate.png
    data/icons/freedesktop/310-apps-crow-translate.png
    data/icons/freedesktop/32-apps-crow-translate.png
    data/icons/freedesktop/44-apps-crow-translate.png
    data/icons/freedesktop/48-apps-crow-translate.png
    data/icons/freedesktop/512-apps-crow-translate.png
    data/icons/freedesktop/64-apps-crow-translate.png
)

qt5_add_translation(QM_FILES
    data/translations/${PROJECT_NAME}_el.ts
    data/translations/${PROJECT_NAME}_es.ts
    data/translations/${PROJECT_NAME}_fi.ts
    data/translations/${PROJECT_NAME}_fr.ts
    data/translations/${PROJECT_NAME}_hi.ts
    data/translations/${PROJECT_NAME}_hu.ts
    data/translations/${PROJECT_NAME}_id.ts
    data/translations/${PROJECT_NAME}_it.ts
    data/translations/${PROJECT_NAME}_ms.ts
    data/translations/${PROJECT_NAME}_nl.ts
    data/translations/${PROJECT_NAME}_pl.ts
    data/translations/${PROJECT_NAME}_pt.ts
    data/translations/${PROJECT_NAME}_ru.ts
    data/translations/${PROJECT_NAME}_sq.ts
    data/translations/${PROJECT_NAME}_tr.ts
    data/translations/${PROJECT_NAME}_ug.ts
    data/translations/${PROJECT_NAME}_uk.ts
    data/translations/${PROJECT_NAME}_zh.ts
)

configure_file(src/cmake.h.in cmake.h)
configure_file(data/icons/flags.qrc ${CircleFlags_SOURCE_DIR}/flags/flags.qrc COPYONLY)
configure_file(data/icons/fluent-icon-theme.qrc ${FluentIconTheme_SOURCE_DIR}/src/fluent-icon-theme.qrc COPYONLY)

add_executable(${PROJECT_NAME}
    ${CircleFlags_SOURCE_DIR}/flags/flags.qrc
    ${QM_FILES}
    ${FluentIconTheme_SOURCE_DIR}/src/fluent-icon-theme.qrc
    data/icons/engines/engines.qrc
    src/addlanguagedialog.cpp
    src/addlanguagedialog.ui
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

if(UNIX)
    target_sources(${PROJECT_NAME} PRIVATE
        src/ocr/screengrabbers/dbusscreengrabber.cpp
        src/ocr/screengrabbers/waylandgnomescreengrabber.cpp
        src/ocr/screengrabbers/waylandplasmascreengrabber.cpp
        src/ocr/screengrabbers/waylandportalscreengrabber.cpp
    )
elseif(WIN32)
    ecm_add_app_icon(WINDOWS_ICON ICONS ${APP_ICONS})

    target_sources(${PROJECT_NAME} PRIVATE
        ${WINDOWS_ICON}
        data/icons/freedesktop/freedesktop.qrc
        src/updaterdialog.cpp
        src/updaterdialog.ui
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
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::DBus XCB::XCB)
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE QGitTag::QGitTag)
endif()

if(WITH_PORTABLE_MODE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE WITH_PORTABLE_MODE)
endif()

if(UNIX)
    install(TARGETS ${PROJECT_NAME})
    install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${APPLICATION_NAME}/${APPLICATION_NAME}/translations)
    install(FILES data/${DESKTOP_FILE} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)

    ecm_install_icons(ICONS
        ${APP_ICONS}
        data/icons/freedesktop/sc-apps-crow-translate.svg
        data/icons/freedesktop/16-status-crow-translate-tray-dark.png
        data/icons/freedesktop/16-status-crow-translate-tray-light.png
        data/icons/freedesktop/22-status-crow-translate-tray-dark.png
        data/icons/freedesktop/22-status-crow-translate-tray-light.png
        data/icons/freedesktop/24-status-crow-translate-tray-dark.png
        data/icons/freedesktop/24-status-crow-translate-tray-light.png
        data/icons/freedesktop/sc-status-crow-translate-tray-dark.svg
        data/icons/freedesktop/sc-status-crow-translate-tray-light.svg
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons
    )
elseif(WIN32)
    install(TARGETS ${PROJECT_NAME} DESTINATION .)
    install(FILES ${QM_FILES} DESTINATION translations)

    # Bundle Qt dependencies
    find_program(WINDEPLOYQT windeployqt PATH_SUFFIXES tools/qt5/bin)
    install(CODE "execute_process(COMMAND ${WINDEPLOYQT} --no-compiler-runtime --no-angle \${CMAKE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:${PROJECT_NAME}>)")

    # Bundle OpenSLL libraries that required for Qt networking manually because it not handled by windeployqt
    find_file(SLL_DLL NAMES libssl.dll libssl-1_1.dll libssl-1_1-x64.dll PATH_SUFFIXES tools/qt5/bin)
    find_file(CRYPTO_DLL NAMES libcrypto.dll libcrypto-1_1.dll libcrypto-1_1-x64.dll PATH_SUFFIXES tools/qt5/bin)
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
    ]])
endif()
