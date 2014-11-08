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


include( ../o2-master/src/src.pri )

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = StarPicsDL
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

RC_FILE = icon.rc




