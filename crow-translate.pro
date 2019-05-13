#-------------------------------------------------
#
# Project created by Shatur95 2018-01-03T03:19:41
#
#-------------------------------------------------

TARGET = crow
TEMPLATE = app
VERSION = 2.1.0
QT += core gui widgets dbus

# Windows specific stuff
QMAKE_TARGET_COMPANY = Hennadii Chernyshchyk
QMAKE_TARGET_DESCRIPTION = Crow Translate
QMAKE_TARGET_COPYRIGHT = Copyright © 2018-2019 Hennadii Chernyshchyk
QMAKE_TARGET_PRODUCT = Crow Translate
RC_ICONS = dist\windows\icon.ico

include(src/qonlinetranslator/qonlinetranslator.pri)
include(src/qgittag/qgittag.pri)
include(src/third-party/qhotkey/qhotkey.pri)
include(src/third-party/singleapplication/singleapplication.pri)

DEFINES += \
    QAPPLICATION_CLASS=QApplication \
    QT_DEPRECATED_WARNINGS

SOURCES += \
    src/main.cpp \
    src/cli.cpp \
    src/settingsdialog.cpp \
    src/mainwindow.cpp \
    src/popupwindow.cpp \
    src/appsettings.cpp \
    src/langbuttongroup.cpp \
    src/addlangdialog.cpp \
    src/trayicon.cpp

HEADERS += \
    src/cli.h \
    src/settingsdialog.h \
    src/mainwindow.h \
    src/popupwindow.h \
    src/appsettings.h \
    src/langbuttongroup.h \
    src/addlangdialog.h \
    src/trayicon.h

FORMS += \
    src/settingsdialog.ui \
    src/mainwindow.ui \
    src/popupwindow.ui \
    src/addlangdialog.ui

RESOURCES += \
    data/resources.qrc

win32 {
SOURCES += \
    src/updaterwindow.cpp

HEADERS += \
    src/updaterwindow.h

FORMS += \
    src/updaterwindow.ui

RESOURCES += \
    data/windows-icons.qrc
}

TRANSLATIONS += \
    data/translations/crow.ts \
    data/translations/crow_ru.ts \
    data/translations/crow_pt_BR.ts \
    data/translations/crow_uk.ts \
    data/translations/crow_zh_CN.ts

# Compile translations
system(lrelease crow-translate.pro)

# Make install
unix {
bin.path   = /usr/bin
bin.files   = crow

icons.path = /usr/share/icons/hicolor/
icons.files = dist/unix/generic/hicolor/*

desktop.path = /usr/share/applications
desktop.files = dist/unix/generic/crow-translate.desktop

INSTALLS += bin icons desktop
}

# Check with PVS Studio
#CONFIG += pvs
CONFIG(pvs) {
    pvs_studio.target = $${TARGET}
    pvs_studio.sources = $${SOURCES}
    pvs_studio.output = true
    pvs_studio.cfg_text = "analysis-mode = 0"

    include(src/third-party/pvs-studio/PVS-Studio.pri)
}
