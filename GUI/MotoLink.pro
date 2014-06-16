#-------------------------------------------------
#
# Project created by QtCreator 2014-04-08T14:53:12
#
#-------------------------------------------------

QT += core gui xml network script scripttools

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MotoLink
TEMPLATE = app
win32:CONFIG += console

VERSION = 0.1
message(Version $$VERSION)

VERSTR = '\\"$${VERSION}\\"'  # place quotes around the version string
DEFINES += __MTL_VER__=\"$${VERSTR}\" # create a VER macro containing the version string

INCLUDEPATH += inc ../code/common/

include(QtUsb/QtUsb.pri)

SOURCES += src/main.cpp \
    src/mainwindow.cpp \
    src/bootloader.cpp \
    src/hrc.cpp \
    src/updatewizard.cpp \
    src/motolink.cpp \
    src/helpviewer.cpp \
    src/commands.cpp \
    src/tablemodel.cpp \
    src/spinbox.cpp \
    src/update.cpp

HEADERS  += \
    inc/compat.h \
    inc/mainwindow.h \
    inc/bootloader.h \
    inc/datastructures.h \
    inc/hrc.h \
    inc/updatewizard.h \
    inc/motolink.h \
    inc/helpviewer.h \
    inc/commands.h \
    inc/tablemodel.h \
    inc/spinbox.h \
    inc/update.h

FORMS    += ui/main.ui \
    ui/updatewizard.ui \
    ui/helpviewer.ui

RESOURCES += \
    res/oxygen.qrc \
    res/binaries.qrc \
    res/images/images.qrc \
    res/translations.qrc \
    res/doc/doc.qrc

TRANSLATIONS = res/motolink_fr.ts
CODECFORTR = UTF-8

buildscripts.target = .buildscripts
buildscripts.commands = cd $$_PRO_FILE_PWD_/res && python makefw.py -v 0.1

QMAKE_EXTRA_TARGETS += buildscripts
PRE_TARGETDEPS += .buildscripts

# Icon for windows
win32:RC_FILE = res/motolink.rc
# OSX
macx:ICON = res/images/icon.icns

target.path = /usr/bin
INSTALLS += target

conf.path = /etc/udev/rules.d
conf.files = 49-motolink.rules
INSTALLS += conf

unix:!macx {
    icon.path = /usr/share/pixmaps
    icon.files = res/images/motolink.png
    INSTALLS += icon

    launcher.path = /usr/share/applications
    launcher.files = res/motolink.desktop
    INSTALLS += launcher
}

