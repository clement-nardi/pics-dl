/**
 * Copyright 2014 Clément Nardi
 *
 * This file is part of PicsDL.
 *
 * PicsDL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PicsDL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PicsDL.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#include "devicemanager.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QSettings>
#include <QCoreApplication>
#include <QMessageBox>

static QString getDriveSerial(QString path);


DeviceManager::DeviceManager(DeviceConfig *deviceConfig, QObject *parent) :
    QObject(parent)
{
    dc = deviceConfig;
}

void DeviceManager::treatDrive(QString driveID){
    QStringList driveIDsplit= driveID.split(";");
    QString drivePath = driveIDsplit.first();

    QString serial = getDriveSerial(driveIDsplit.last());
    qDebug() << "treating drive " << drivePath << serial;

    if (dc->conf[serial].isUndefined() || dc->conf[serial].isNull() ) {
        qDebug() << "first time";
        QJsonObject driveConfig;
        QMessageBox mb(QMessageBox::Question,
                       "New device detected!",
                       "Would you like to manage this device with " + QCoreApplication::applicationName() + "?",
                       QMessageBox::Yes | QMessageBox::No);
        int answer = mb.exec();
        qDebug() << "answer is " << answer;

        driveConfig.insert("isManaged",QJsonValue(answer == QMessageBox::Yes));
        dc->conf.insert(serial,driveConfig);
        dc->saveConfig();
        newID(serial);
    } else {
        qDebug() << "known drive";
    }

    if (dc->conf[serial].toObject()["isManaged"].toBool() == true) {
        bool isAlreadyOpened = false;
        QSetIterator<DeviceConfigView *> i(openedViews);
        while (i.hasNext()) {
            DeviceConfigView *ldcv = i.next();
            if (ldcv->id == serial) {
                isAlreadyOpened = true;
                break;
            }
        }

        if (!isAlreadyOpened) {
            qDebug() << "treat this drive now !";
            QJsonObject obj = dc->conf[serial].toObject();
            if (drivePath.startsWith("WPD:/")) {
                /* TODO: retrieve display name of the device */
                obj["path"] = drivePath.split("/").at(2);
                obj["IDPath"] = "WPD:/" + drivePath.split("/").at(1);
            } else {
                obj["path"] = drivePath;
            }
            dc->conf[serial] = obj;
            dc->saveConfig();


            DeviceConfigView *dcv = new DeviceConfigView(dc,serial);
            openedViews.insert(dcv);
            connect(dcv,SIGNAL(destroyed(QObject*)),this,SLOT(handleDestroyed(QObject*)));
            //treatDir(drivePath);
        } else {
            qDebug() << "This drive is already being treated.";
        }
    } else {
        qDebug() << "leave this drive alone...";
    }

    return;
}

void DeviceManager::handleDestroyed(QObject *object) {
    openedViews.remove((DeviceConfigView *)object);
}


#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#else // linux stuff
#include <unistd.h>      //close()
#include <fcntl.h>       //File control defs
#include <sys/ioctl.h>   //IOCTL  defs
#include <linux/hdreg.h> //Drive specific defs
#include <libudev.h>
#include <stdio.h>
#endif // _WIN32


static QString getDriveSerial(QString path) {

#ifdef _WIN32

    if (path.startsWith("WPD:/")) {
        return path.mid(5);
    } else {
        // adapted from http://www.developpez.net/forums/d523835/c-cpp/cpp/utilisation-getvolumeinformation/
        // and http://www.linuxquestions.org/questions/programming-9/problem-with-hdio_get_identity-ioctl-856753/

        TCHAR volName[MAX_PATH+1];
        DWORD volSerial;
        DWORD nameLen;
        DWORD volFlags;
        TCHAR volFS[MAX_PATH+1];

        QDir::setCurrent( path );

        GetVolumeInformation(NULL, volName, MAX_PATH+1, &volSerial, &nameLen, &volFlags, volFS, MAX_PATH+1);

        qDebug() << "Serial=" << volSerial
                 << " Name="  << QString((QChar*)volName)
                 << " FS="    << QString((QChar*)volFS)
                 << " for "   << path;
        return QString("%1;%2;%3").arg(volSerial).arg(QString((QChar*)volName)).arg(QString((QChar*)volFS)) ;
    }

#else
    QString res;

    // inspired from http://www.signal11.us/oss/udev/
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;

    int fd;

    /* Create the udev object */
    udev = udev_new();
    if (!udev) {
        printf("Can't create udev\n");
        exit(1);
    }

    /* Create a list of the devices in the 'hidraw' subsystem. */
    enumerate = udev_enumerate_new(udev);
    //udev_enumerate_add_match_subsystem(enumerate, "hidraw");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);
    /* For each item enumerated, print out its information.
       udev_list_entry_foreach is a macro which expands to
       a loop. The loop will be executed for each member in
       devices, setting dev_list_entry to a list entry
       which contains the device's path in /sys. */
    qDebug() << "Looking for " << path;
    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *syspath;

        /* Get the filename of the /sys entry for the device
           and create a udev_device object (dev) representing it */
        syspath = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, syspath);

        /* usb_device_get_devnode() returns the path to the device node
           itself in /dev. */
        printf("Device Node Path: %s\n", udev_device_get_devnode(dev));
        if (udev_device_get_devnode(dev) == path) {
            printf("Match!");
            res = QString(udev_device_get_property_value(dev,"ID_SERIAL"));
            break;
        }

        udev_device_unref(dev);
    }

    /* Free the enumerator object */
    udev_enumerate_unref(enumerate);
    udev_unref(udev);

    return res;
#endif

}
