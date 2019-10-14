#-------------------------------------------------
#
# Project created by Shatur95 2018-01-03T03:19:41
#
#-------------------------------------------------

TARGET = crow
TEMPLATE = app
VERSION = 2.2.2
INCLUDEPATH = src
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
include(src/third-party/qtaskbarcontrol/qtaskbarcontrol.pri)
include(src/third-party/singleapplication/singleapplication.pri)

DEFINES += \
    QAPPLICATION_CLASS=QApplication \
    QT_DEPRECATED_WARNINGS

SOURCES += \
    src/main.cpp \
    src/cli.cpp \
    src/playerbuttons.cpp \
    src/transitions/languagedetectedtransition.cpp \
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
    src/transitions/retranslationtransition.cpp \
    src/transitions/textemptytransition.cpp \
    src/transitions/translatorabortedtransition.cpp \
    src/transitions/translatorerrortransition.cpp \
    src/translationedit.cpp \
    src/trayicon.cpp

HEADERS += \
    src/cli.h \
    src/playerbuttons.h \
    src/transitions/languagedetectedtransition.h \
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
    src/transitions/retranslationtransition.h \
    src/transitions/textemptytransition.h \
    src/transitions/translatorabortedtransition.h \
    src/transitions/translatorerrortransition.h \
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
        src/updaterdialog.cpp

    HEADERS += \
        src/updaterdialog.h

    FORMS += \
        src/updaterdialog.ui

    RESOURCES += \
        data/windows-icons.qrc
}

TRANSLATIONS += \
    data/translations/crow.ts \
    data/translations/crow_ru.ts \
    data/translations/crow_pt_BR.ts \
    data/translations/crow_pl.ts \
    data/translations/crow_uk.ts \
    data/translations/crow_tr.ts \
    data/translations/crow_zh_CN.ts

# Compile translations
qtPrepareTool(LRELEASE, lrelease)
system($$LRELEASE crow-translate.pro) | error("Failed to run lrelease")

# Make install
unix {
    target.path = /usr/bin

    desktop.path = /usr/share/applications
    desktop.files = dist/unix/generic/crow-translate.desktop

    # App icons
    icon16.path = /usr/share/icons/hicolor/16x16/apps/
    icon16.files = dist/unix/generic/hicolor/16x16/apps/crow-translate.png
    icon22.path = /usr/share/icons/hicolor/22x22/apps/
    icon22.files = dist/unix/generic/hicolor/22x22/apps/crow-translate.png
    icon24.path = /usr/share/icons/hicolor/24x24/apps/
    icon24.files = dist/unix/generic/hicolor/24x24/apps/crow-translate.png
    icon32.path = /usr/share/icons/hicolor/32x32/apps/
    icon32.files = dist/unix/generic/hicolor/32x32/apps/crow-translate.png
    icon36.path = /usr/share/icons/hicolor/36x36/apps/
    icon36.files = dist/unix/generic/hicolor/36x36/apps/crow-translate.png
    icon48.path = /usr/share/icons/hicolor/48x48/apps/
    icon48.files = dist/unix/generic/hicolor/48x48/apps/crow-translate.png
    icon64.path = /usr/share/icons/hicolor/64x64/apps/
    icon64.files = dist/unix/generic/hicolor/64x64/apps/crow-translate.png
    icon72.path = /usr/share/icons/hicolor/72x72/apps/
    icon72.files = dist/unix/generic/hicolor/72x72/apps/crow-translate.png
    icon128.path = /usr/share/icons/hicolor/128x128/apps/
    icon128.files = dist/unix/generic/hicolor/128x128/apps/crow-translate.png
    icon192.path = /usr/share/icons/hicolor/192x192/apps/
    icon192.files = dist/unix/generic/hicolor/192x192/apps/crow-translate.png
    icon256.path = /usr/share/icons/hicolor/256x256/apps/
    icon256.files = dist/unix/generic/hicolor/256x256/apps/crow-translate.png
    iconScalable.path = /usr/share/icons/hicolor/scalable/apps/
    iconScalable.files = dist/unix/generic/hicolor/scalable/apps/crow-translate.png

    # Status icons
    statusIcons16.path = /usr/share/icons/hicolor/16x16/status/
    statusIcons16.files = dist/unix/generic/hicolor/16x16/status/*
    statusIcons22.path = /usr/share/icons/hicolor/22x22/status/
    statusIcons22.files = dist/unix/generic/hicolor/22x22/status/*
    statusIcons24.path = /usr/share/icons/hicolor/24x24/status/
    statusIcons24.files = dist/unix/generic/hicolor/24x24/status/*
    statusIcons32.path = /usr/share/icons/hicolor/32x32/status/
    statusIcons32.files = dist/unix/generic/hicolor/32x32/status/*
    statusIcons36.path = /usr/share/icons/hicolor/36x36/status/
    statusIcons36.files = dist/unix/generic/hicolor/36x36/status/*
    statusIcons48.path = /usr/share/icons/hicolor/48x48/status/
    statusIcons48.files = dist/unix/generic/hicolor/48x48/status/*
    statusIcons64.path = /usr/share/icons/hicolor/64x64/status/
    statusIcons64.files = dist/unix/generic/hicolor/64x64/status/*
    statusIcons72.path = /usr/share/icons/hicolor/72x72/status/
    statusIcons72.files = dist/unix/generic/hicolor/72x72/status/*
    statusIcons128.path = /usr/share/icons/hicolor/128x128/status/
    statusIcons128.files = dist/unix/generic/hicolor/128x128/status/*
    statusIcons192.path = /usr/share/icons/hicolor/192x192/status/
    statusIcons192.files = dist/unix/generic/hicolor/192x192/status/*
    statusIcons256.path = /usr/share/icons/hicolor/256x256/status/
    statusIcons256.files = dist/unix/generic/hicolor/256x256/status/*
    statusIconsScalable.path = /usr/share/icons/hicolor/scalable/status/
    statusIconsScalable.files = dist/unix/generic/hicolor/scalable/status/*


    INSTALLS += target \
        desktop \
        icon16 \
        icon22 \
        icon24 \
        icon32 \
        icon36 \
        icon48 \
        icon64 \
        icon72 \
        icon128 \
        icon192 \
        icon256 \
        iconScalable \
        statusIcons16 \
        statusIcons22 \
        statusIcons24 \
        statusIcons32 \
        statusIcons36 \
        statusIcons48 \
        statusIcons64 \
        statusIcons72 \
        statusIcons128 \
        statusIcons192 \
        statusIcons256 \
        statusIconsScalable
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
