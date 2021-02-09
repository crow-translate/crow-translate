cmake_minimum_required(VERSION 3.15.0)

project(crow-translate VERSION 2.7.0 LANGUAGES CXX)

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
endif()

include(IgnoreExternalCodeChecks)
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
    data/translations/${PROJECT_NAME}_es.ts
    data/translations/${PROJECT_NAME}_fr.ts
    data/translations/${PROJECT_NAME}_ms.ts
    data/translations/${PROJECT_NAME}_nl.ts
    data/translations/${PROJECT_NAME}_pl.ts
    data/translations/${PROJECT_NAME}_pt.ts
    data/translations/${PROJECT_NAME}_ru.ts
    data/translations/${PROJECT_NAME}_tr.ts
    data/translations/${PROJECT_NAME}_ug.ts
    data/translations/${PROJECT_NAME}_uk.ts
    data/translations/${PROJECT_NAME}_zh.ts
)

configure_file(src/cmake.h.in cmake.h)
configure_file(data/icons/flags.qrc ${CircleFlags_SOURCE_DIR}/flags/flags.qrc COPYONLY)

add_executable(${PROJECT_NAME}
    ${QM_FILES}
    data/icons/engines/engines.qrc
    ${CircleFlags_SOURCE_DIR}/flags/flags.qrc
    src/addlanguagedialog.cpp
    src/addlanguagedialog.ui
    src/cli.cpp
    src/languagebuttonswidget.cpp
    src/languagebuttonswidget.ui
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.ui
    src/ocr/ocr.cpp
    src/ocr/screengrabber.cpp
    src/popupwindow.cpp
    src/popupwindow.ui
    src/selection.cpp
    src/settings/appsettings.cpp
    src/settings/ocrlanguageslistwidget.cpp
    src/settings/settingsdialog.cpp
    src/settings/settingsdialog.ui
    src/settings/tesseractparameterstablewidget.cpp
    src/settings/shortcutsmodel/shortcutitem.cpp
    src/settings/shortcutsmodel/shortcutsmodel.cpp
    src/settings/shortcutsmodel/shortcutsview.cpp
    src/sourcetextedit.cpp
    src/speakbuttons.cpp
    src/speakbuttons.ui
    src/transitions/languagedetectedtransition.cpp
    src/transitions/ocruninitializedtransition.cpp
    src/transitions/playerstoppedtransition.cpp
    src/transitions/retranslationtransition.cpp
    src/transitions/textemptytransition.cpp
    src/transitions/translatorabortedtransition.cpp
    src/transitions/translatorerrortransition.cpp
    src/translationedit.cpp
    src/trayicon.cpp
)

if(WIN32)
    ecm_add_app_icon(WINDOWS_ICON ICONS ${APP_ICONS})
    configure_file(data/icons/we10x.qrc ${We10X_SOURCE_DIR}/src/we10x.qrc COPYONLY)

    target_sources(${PROJECT_NAME} PRIVATE
        ${WINDOWS_ICON}
        ${We10X_SOURCE_DIR}/src/we10x.qrc
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
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::DBus)
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

    include(InstallRuntimeDependencies)
    install_runtime_dependencies(${PROJECT_NAME})
endif()
