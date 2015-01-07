#-------------------------------------------------
#
# Project created by QtCreator 2014-01-13T23:16:19
#
#-------------------------------------------------

#note: by default $$DESTDIR is empty, and qmake decides to create "release" and "debug" directories
#This is useless since QT Creator creates different directories to Release and Debug anyway
#plus, if this variable is not set, there's no way to know the final location of the application
DESTDIR = bin

rcfile.target = PicsDL.rc
rcfile.commands = ../PicsDL/increment_build_number.sh \"$$OUT_PWD/$$DESTDIR\"
QMAKE_EXTRA_TARGETS += rcfile
PRE_TARGETDEPS += PicsDL.rc

exiftool.target = exiftool_app
exiftool.commands = ../exiftool/get_exiftool.sh \"$$OUT_PWD/$$DESTDIR\"
QMAKE_EXTRA_TARGETS += exiftool
PRE_TARGETDEPS += exiftool_app

#with "/" it doesn't work under win7
win32: include( ..\o2-master\src\src.pri )
unix: include( ../o2-master/src/src.pri )


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
    file.cpp

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
    file.h

FORMS    += mainwindow.ui \
            deviceconfigview.ui \
            dcomdialog.ui \
            exifdialog.ui

win32{
    LIBS += -L"C:/Program Files (x86)/Windows Kits/8.1/Lib/winv6.3/um/x86/" -lshell32

    INCLUDEPATH   += "$$PWD/../WPDInterface/WPDInterface"
    LIBS += -L"$$PWD/../WPDInterface/Release" -lWPDInterface

    INCLUDEPATH += $$PWD/../libexif-0.6.21/
    LIBS      += -L$$PWD/../libexif-0.6.21/libexif/.libs/ -lexif
} else {
    LIBS      += -lexif -ludev
}

QMAKE_CXXFLAGS += -DUSETHREADS -DUSEITHREADS -DMULTIPLICITY
win32 {
    INCLUDEPATH += "C:/Strawberry/perl/lib/CORE"
    LIBS        += -LC:/Strawberry/perl/lib/CORE/ -lperl520
    #Path to the DLL. Without this line, the compilation is fine, but QT Creator is unable to launch the app:
    #An exception was triggered: Exception at 0x77e98f05, code: 0xc0000139: DLL entry point not found, flags=0x0. During startup program exited with code 0xc0000139.
    LIBS        += -LC:/Strawberry/perl/bin -lperl520
}
unix {
    INCLUDEPATH += "/usr/lib/perl/5.18.2/CORE/"
    LIBS        += -L/usr/lib/ -lperl
}

RESOURCES += \
    resources.qrc

RC_FILE = PicsDL.rc

QMAKE_POST_LINK += ../PicsDL-win32-installer/create-installer.sh \"$$OUT_PWD/$$DESTDIR/\" \"$$TARGET\"
