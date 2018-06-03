#-------------------------------------------------
#
# Project created by Shatur95 2018-01-03T03:19:41
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++14
TARGET = crow
TEMPLATE = app
VERSION = 0.9.9

# Windows specific stuff
QMAKE_TARGET_COMPANY = Gennady Chernyshchuk
QMAKE_TARGET_DESCRIPTION = Simple and lightweight translator
QMAKE_TARGET_COPYRIGHT = Copyright © 2018 Gennady Chernyshchuk
QMAKE_TARGET_PRODUCT = Crow Translate
RC_ICONS = dist\windows\icon.ico

include(src/qonlinetranslator/qonlinetranslator.pri)
include(src/third-party/qhotkey/qhotkey.pri)
include(src/third-party/singleapplication/singleapplication.pri)
DEFINES += QAPPLICATION_CLASS=QApplication

DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += \
    src/main.cpp \
    src/settingsdialog.cpp \
    src/mainwindow.cpp \
    src/popupwindow.cpp

HEADERS += \
    src/settingsdialog.h \
    src/mainwindow.h \
    src/popupwindow.h

FORMS += \
    src/settingsdialog.ui \
    src/mainwindow.ui \
    src/popupwindow.ui

RESOURCES += \
    data/resources.qrc

win32 {
RESOURCES += data/windows-icons.qrc
}

TRANSLATIONS += $$files(data/translations/crow_*.ts)

# For make install
unix {
bin.path   = /usr/bin
bin.files   = crow

icons.path = /usr/share/icons/hicolor/
icons.files = dist/unix/generic/menuicons/*

desktop.path = /usr/share/applications
desktop.files = dist/unix/generic/crow-translate.desktop

INSTALLS += bin icons desktop
}
