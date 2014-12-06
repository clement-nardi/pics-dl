#-------------------------------------------------
#
# Project created by QtCreator 2014-01-13T23:16:19
#
#-------------------------------------------------

#TODO: notification "you can unplug the device"
#TODO: bug with microSD when files can't be read
#TODO: bug with exiftool final output reading
#TODO: start downloading files as soon as card is inserted
#TODO: when file is downloaded completely for EXIF reading purpose, store it locally
#TODO: create reorganize mode: select a folder as input, and option to move instead of copy
#TODO: change default location of temp folder: D:/temp iso dest/temp
#TODO: change visible pictures when editing "dayly comments" (when clicking on a line)
#TODO: fix issue with DST


#note: by default $$DESTDIR is empty, and qmake decides to create "release" and "debug" directories
#This is useless since QT Creator creates different directories to Release and Debug anyway
#plus, if this variable is not set, there's no way to know the final location of the application
DESTDIR = bin

rcfile.target = PicsDL.rc
rcfile.commands = ..\PicsDL\increment_build_number.bat
QMAKE_EXTRA_TARGETS += rcfile
PRE_TARGETDEPS += PicsDL.rc

exiftool.target = exiftool_app
exiftool.commands = ../exiftool/get_exiftool.sh \"$$OUT_PWD/$$DESTDIR\"
QMAKE_EXTRA_TARGETS += exiftool
PRE_TARGETDEPS += exiftool_app


include( ..\o2-master\src\src.pri )

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PicsDL
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    drivenotify.cpp \
    drivenotofy_win.cpp \
    devicemanager.cpp \
    deviceconfig.cpp \
    driveview.cpp \
    deviceconfigview.cpp \
    fsutils.cpp \
    fileinfo.cpp \
    dcomdialog.cpp \
    exifdialog.cpp \
    downloadmodel.cpp \
    openpath.cpp \
    autocheckbox.cpp \
    verticalscrollarea.cpp

HEADERS  += mainwindow.h \
    drivenotify.h \
    drivenotofy_win.h \
    devicemanager.h \
    deviceconfig.h \
    driveview.h \
    deviceconfigview.h \
    fsutils.h \
    fileinfo.h \
    dcomdialog.h \
    exifdialog.h \
    downloadmodel.h \
    openpath.h \
    autocheckbox.h \
    verticalscrollarea.h

FORMS    += mainwindow.ui \
            deviceconfigview.ui \
            dcomdialog.ui \
            exifdialog.ui

win32: LIBS += -L"C:/Program Files (x86)/Windows Kits/8.1/Lib/winv6.3/um/x86/" -lshell32


INCLUDEPATH   += "$$PWD/../WPDInterface/WPDInterface"
win32: LIBS += -L"$$PWD/../WPDInterface/Release" -lWPDInterface


INCLUDEPATH += $$PWD/../libexif-0.6.21/
LIBS      += -L$$PWD/../libexif-0.6.21/libexif/.libs/ -lexif

RESOURCES += \
    resources.qrc

RC_FILE = PicsDL.rc

QMAKE_POST_LINK += ../PicsDL-win32-installer/create-installer.sh \"$$OUT_PWD/$$DESTDIR/\" \"$$TARGET\"
