cmake_minimum_required(VERSION 3.14.0)

project(crow-translate VERSION 2.3.2 LANGUAGES CXX)

set(CMAKE_PROJECT_HOMEPAGE_URL crow-translate.github.io)
set(CMAKE_PROJECT_DESCRIPTION "A simple and lightweight translator that allows to translate and speak text using Google, Yandex and Bing")

set(APPLICATION_NAME "Crow Translate")

set(CPACK_PACKAGE_VENDOR ${APPLICATION_NAME})
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${APPLICATION_NAME})
set(CPACK_PACKAGE_CONTACT genaloner@gmail.com)
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/COPYING)

set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS_POLICY >=)
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)
set(CPACK_RPM_PACKAGE_LICENSE GPLv3)
set(CPACK_RPM_PACKAGE_GROUP Applications/Text)
set(CPACK_RPM_COMPRESSION_TYPE lzma)
set(CPACK_NSIS_DISPLAY_NAME ${APPLICATION_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_WIN32_EXECUTABLE ON)

set(QAPPLICATION_CLASS QApplication)
option(QHOTKEY_INSTALL OFF)

add_subdirectory(src/third-party/singleapplication)
add_subdirectory(src/third-party/qhotkey)
add_subdirectory(src/third-party/qtaskbarcontrol)
add_subdirectory(src/qonlinetranslator)
if(WIN32)
    add_subdirectory(src/qgittag)
endif()

find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
if(UNIX)
    find_package(Qt5 COMPONENTS DBus REQUIRED)
endif()
find_package(ECM REQUIRED NO_MODULE)

list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

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
    data/icons/freedesktop/sc-apps-crow-translate.svg
)

qt5_add_translation(QM_FILES
    data/translations/${PROJECT_NAME}_es_ES.ts
    data/translations/${PROJECT_NAME}_fr_FR.ts
    data/translations/${PROJECT_NAME}_ms_MY.ts
    data/translations/${PROJECT_NAME}_pl_PL.ts
    data/translations/${PROJECT_NAME}_pt_BR.ts
    data/translations/${PROJECT_NAME}_ru_RU.ts
    data/translations/${PROJECT_NAME}_tr_TR.ts
    data/translations/${PROJECT_NAME}_uk_UA.ts
    data/translations/${PROJECT_NAME}_zh_CN.ts
)

configure_file(src/cmake.h.in cmake.h)

add_executable(${PROJECT_NAME}
    ${QM_FILES}
    data/icons/engines/engines.qrc
    data/icons/flags/flags.qrc
    src/addlangdialog.cpp
    src/addlangdialog.ui
    src/cli.cpp
    src/langbuttongroup.cpp
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.ui
    src/popupwindow.cpp
    src/popupwindow.ui
    src/selection.cpp
    src/settings/appsettings.cpp
    src/settings/settingsdialog.cpp
    src/settings/settingsdialog.ui
    src/settings/shortcutsmodel/shortcutitem.cpp
    src/settings/shortcutsmodel/shortcutsmodel.cpp
    src/settings/shortcutsmodel/shortcutsview.cpp
    src/sourcetextedit.cpp
    src/speakbuttons.cpp
    src/speakbuttons.ui
    src/transitions/languagedetectedtransition.cpp
    src/transitions/playerstoppedtransition.cpp
    src/transitions/retranslationtransition.cpp
    src/transitions/textemptytransition.cpp
    src/transitions/translatorabortedtransition.cpp
    src/transitions/translatorerrortransition.cpp
    src/translationedit.cpp
    src/trayicon.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME crow)

if(WIN32)
    ecm_add_app_icon(WINDOWS_ICON ICONS ${APP_ICONS})
    target_sources(${PROJECT_NAME} PRIVATE
        ${WINDOWS_ICON}
        data/icons/windows/windows.qrc
        src/updaterdialog.cpp
        src/updaterdialog.ui
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE SingleApplication::SingleApplication QHotkey::QHotkey QTaskbarControl::QTaskbarControl QOnlineTranslator::QOnlineTranslator)
if(UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::DBus)
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE QGitTag::QGitTag)
endif()
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

option(PORTABLE_MODE "Enable portable functionality" OFF)
if(${PORTABLE_MODE})
    target_compile_definitions(${PROJECT_NAME} PRIVATE PORTABLE_MODE)
endif()

install(TARGETS ${PROJECT_NAME})
if(UNIX)
    install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${APPLICATION_NAME}/${APPLICATION_NAME}/translations)
    install(FILES data/io.crow_translate.CrowTranslate.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
    ecm_install_icons(ICONS
        ${APP_ICONS}
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
    find_program(WINDEPLOYQT windeployqt)
    install(CODE "execute_process(COMMAND ${WINDEPLOYQT} \${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>)")

    file(GLOB OPENSSL_DLLS ${OPENSSL_ROOT_DIR}/bin/libssl-*.dll ${OPENSSL_ROOT_DIR}/bin/libcrypto-*.dll)
    install(FILES ${OPENSSL_DLLS} TYPE BIN)

    install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_BINDIR}/translations)
endif()
