#
# Copyright 2014-2015 Clément Nardi
#
# This file is part of PicsDL.
#
# PicsDL is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# PicsDL is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with PicsDL.  If not, see <http://www.gnu.org/licenses/>.
#

#-------------------------------------------------
#
# Project created by QtCreator 2014-01-13T23:16:19
#
#-------------------------------------------------

#note: by default $$DESTDIR is empty, and qmake decides to create "release" and "debug" directories
#This is useless since QT Creator creates different directories to Release and Debug anyway
#plus, if this variable is not set, there's no way to know the final location of the application
DESTDIR = bin


MAJOR=0
MINOR=4
PATCH=0


!contains(BUILD_NUMBER_UPDATE,false) {
    rcfile.target = PicsDL.rc
    rcfile.commands = ../PicsDL/set_version_numbers.sh \"$$MAJOR\" \"$$MINOR\" \"$$PATCH\"
    QMAKE_EXTRA_TARGETS += rcfile
    PRE_TARGETDEPS += PicsDL.rc
}
BUILD=$$cat(../PicsDL/build_number.txt)

exiftool.target = exiftool_app
exiftool.commands = ../exiftool/get_exiftool.sh \"$$OUT_PWD/$$DESTDIR\"
QMAKE_EXTRA_TARGETS += exiftool
PRE_TARGETDEPS += exiftool_app

#doesn't work from here. Under windows, the sh.exe bundled with git doesn't support parentheses in test,
#so you need to launch the script from an MSYS shell (see README)
#on linux and mac, sudo is needed to install the lib, so you need to launch the script manually
#win32 {
#    libexif.target = libexif_lib
#    libexif.commands = ../libexif/get_libexif.sh \"$$OUT_PWD/$$DESTDIR\"
#    QMAKE_EXTRA_TARGETS += libexif
#    PRE_TARGETDEPS += libexif_lib
#}

#with "/" it doesn't work under win7
#and with "\" it doesn't work on my brand new windows 8.1 ...
#win32: include( ..\o2-master\src\src.pri )
include( ../o2-master/src/src.pri )


QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PicsDL
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    drivenotify.cpp \
    devicemanager.cpp \
    deviceconfig.cpp \
    driveview.cpp \
    deviceconfigview.cpp \
    dcomdialog.cpp \
    exifdialog.cpp \
    downloadmodel.cpp \
    openpath.cpp \
    autocheckbox.cpp \
    verticalscrollarea.cpp \
    instancemanager.cpp \
    geotagger.cpp \
    exiftoolperlwrapper.cpp \
    file.cpp \
    geotaggerprivate.cpp \
    transfermanager.cpp \
    progressbarlabel.cpp \
    wpdiodevice.cpp \
    about.cpp \
    transferdialog.cpp \
    globals.cpp

HEADERS  += mainwindow.h \
    drivenotify.h \
    devicemanager.h \
    deviceconfig.h \
    driveview.h \
    deviceconfigview.h \
    dcomdialog.h \
    exifdialog.h \
    downloadmodel.h \
    openpath.h \
    autocheckbox.h \
    verticalscrollarea.h \
    instancemanager.h \
    geotagger.h \
    exiftoolperlwrapper.h \
    file.h \
    geotaggerprivate.h \
    transfermanager.h \
    progressbarlabel.h \
    wpdiodevice.h \
    about.h \
    transferdialog.h \
    globals.h


FORMS    += mainwindow.ui \
            deviceconfigview.ui \
            dcomdialog.ui \
            exifdialog.ui \
    about.ui \
    transferdialog.ui

win32{
    LIBS += -L"C:/Program Files (x86)/Windows Kits/8.1/Lib/winv6.3/um/x86/" -lshell32

    INCLUDEPATH   += "$$PWD/../WPDInterface/WPDInterface"
    LIBS += -L"$$PWD/../WPDInterface/Release" -lWPDInterface
}
win32|mac {
    INCLUDEPATH += $$PWD/../libexif/libexif-0.6.21/
    LIBS      += -L$$PWD/../libexif/libexif-0.6.21/libexif/.libs/ -lexif
} else {
    LIBS      += -lexif -ludev
}

QMAKE_CXXFLAGS += -DUSETHREADS -DUSEITHREADS -DMULTIPLICITY -DMAJOR=$$MAJOR -DMINOR=$$MINOR -DPATCH=$$PATCH -DBUILD=$$BUILD
win32 {
    INCLUDEPATH += "C:/Strawberry/perl/lib/CORE"
    LIBS        += -LC:/Strawberry/perl/lib/CORE/ -lperl520
    #Path to the DLL. Without this line, the compilation is fine, but QT Creator is unable to launch the app:
    #An exception was triggered: Exception at 0x77e98f05, code: 0xc0000139: DLL entry point not found, flags=0x0. During startup program exited with code 0xc0000139.
    LIBS        += -LC:/Strawberry/perl/bin -lperl520
}
unix:!mac {
    INCLUDEPATH += "/usr/lib/perl/5.18.2/CORE/"
    LIBS        += -L/usr/lib/ -lperl
}
mac {
    INCLUDEPATH += "/System/Library/Perl/5.18/darwin-thread-multi-2level/CORE/"
    LIBS        += -L/System/Library/Perl/5.18/darwin-thread-multi-2level/CORE/ -lperl
}

RESOURCES += \
    resources.qrc

RC_FILE = PicsDL.rc

#This is needed for debian packaging. It only works if INSTALL_ROOT is passed as parameter of make
binaries.path = /usr/bin
binaries.files = $$DESTDIR/$$TARGET
INSTALLS += binaries

#win32: QMAKE_POST_LINK += ../package-win32/create-installer.sh \"$$OUT_PWD/$$DESTDIR/\" \"$$TARGET\"
#unix: QMAKE_POST_LINK += ../package-debian/create-debian-package.sh \"$$OUT_PWD/$$DESTDIR/\" \"$$TARGET\"
#mac: QMAKE_POST_LINK += ../package-macos/make_dmg.sh \"$$OUT_PWD/$$DESTDIR/\" \"$$TARGET\"

