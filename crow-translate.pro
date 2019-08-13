#-------------------------------------------------
#
# Project created by Shatur95 2018-01-03T03:19:41
#
#-------------------------------------------------

TARGET = crow
TEMPLATE = app
VERSION = 2.1.0
INCLUDEPATH = src
QT += core gui widgets dbus
win32:QT += winextras

# Windows specific stuff
QMAKE_TARGET_COMPANY = Hennadii Chernyshchyk
QMAKE_TARGET_DESCRIPTION = Crow Translate
QMAKE_TARGET_COPYRIGHT = Copyright © 2018-2019 Hennadii Chernyshchyk
QMAKE_TARGET_PRODUCT = Crow Translate
RC_ICONS = dist\windows\icon.ico

include(src/qonlinetranslator/qonlinetranslator.pri)
include(src/qgittag/qgittag.pri)
include(src/third-party/qhotkey/qhotkey.pri)
include(src/third-party/qtaskbarcontrol/qtaskbarcontrol.pri)
include(src/third-party/singleapplication/singleapplication.pri)

DEFINES += \
    QAPPLICATION_CLASS=QApplication \
    QT_DEPRECATED_WARNINGS

SOURCES += \
    src/main.cpp \
    src/cli.cpp \
    src/playerbuttons.cpp \
    src/transitions/playerstoppedtransition.cpp \
    src/settings/settingsdialog.cpp \
    src/settings/appsettings.cpp \
    src/settings/shortcutsmodel/shortcutitem.cpp \
    src/settings/shortcutsmodel/shortcutsmodel.cpp \
    src/settings/shortcutsmodel/shortcutsview.cpp \
    src/mainwindow.cpp \
    src/popupwindow.cpp \
    src/langbuttongroup.cpp \
    src/addlangdialog.cpp \
    src/sourcetextedit.cpp \
    src/translationedit.cpp \
    src/trayicon.cpp

HEADERS += \
    src/cli.h \
    src/playerbuttons.h \
    src/transitions/conditiontransition.h \
    src/transitions/playerstoppedtransition.h \
    src/settings/settingsdialog.h \
    src/settings/appsettings.h \
    src/settings/shortcutsmodel/shortcutitem.h \
    src/settings/shortcutsmodel/shortcutsmodel.h \
    src/settings/shortcutsmodel/shortcutsview.h \
    src/mainwindow.h \
    src/popupwindow.h \
    src/langbuttongroup.h \
    src/addlangdialog.h \
    src/sourcetextedit.h \
    src/translationedit.h \
    src/trayicon.h

FORMS += \
    src/playerbuttons.ui \
    src/settings/settingsdialog.ui \
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
    data/translations/crow_tr.ts \
    data/translations/crow_zh_CN.ts

# Compile translations
qtPrepareTool(LRELEASE, lrelease)
system($$LRELEASE crow-translate.pro) | error("Failed to run lrelease")

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
