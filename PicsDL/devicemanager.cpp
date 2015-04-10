/**
 * Copyright 2014-2015 Clément Nardi
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
#include "config.h"


DeviceManager::DeviceManager(Config *deviceConfig, QObject *parent) :
    QObject(parent)
{
    dc = deviceConfig;
}

void DeviceManager::treatDrive(QString drivePath, QString serial, QString displayName, qint64 deviceSize, qint64 bytes_available, bool force){

    qDebug() << "treating drive " << drivePath << serial << displayName;

    if (!force) {
        if (dc->devices[serial].isUndefined() || dc->devices[serial].isNull() ) {
            qDebug() << "first time";
            QJsonObject driveConfig;

            /* avoid asking twice the same question */
            driveConfig.insert(CONFIG_ISMANAGED,QJsonValue(false));
            driveConfig[CONFIG_DISPLAYNAME] = displayName;
            driveConfig[CONFIG_DEVICESIZE] = QString("%1").arg(deviceSize);
            driveConfig[CONFIG_BYTESAVAILABLE] = QString("%1").arg(bytes_available);
            dc->devices.insert(serial,driveConfig);
            dc->deviceFieldChanged(serial);
            QString deviceDescription = displayName;
            if (displayName != drivePath) {
                deviceDescription += " (" + drivePath + ")";
            }

            QMessageBox mb(QMessageBox::Question,
                           QCoreApplication::applicationName(),
                           "New device detected: " + deviceDescription +  "\n" +
                           "Would you like to manage this device with " + QCoreApplication::applicationName() + "?",
                           QMessageBox::Yes | QMessageBox::No);
            int answer = mb.exec();
            qDebug() << "answer is " << answer;

            driveConfig.insert(CONFIG_ISMANAGED,QJsonValue(answer == QMessageBox::Yes));
            dc->devices.insert(serial,driveConfig);
            dc->saveDevices();
        } else {
            qDebug() << "known drive";
        }
        /* Notify each time a drive is plugged, to update the "launch" button */
        dc->deviceFieldChanged(serial);
    }

    if (dc->devices[serial].toObject()[CONFIG_ISMANAGED].toBool() == true || force) {
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
            QJsonObject obj = dc->devices[serial].toObject();
            if (drivePath.startsWith("WPD:/")) {
                obj[CONFIG_PATH] = displayName;
                obj[CONFIG_IDPATH] = drivePath;
            } else {
                obj[CONFIG_PATH] = drivePath;
            }
            obj[CONFIG_DISPLAYNAME] = displayName;
            obj[CONFIG_DEVICESIZE] = QString("%1").arg(deviceSize);
            obj[CONFIG_BYTESAVAILABLE] = QString("%1").arg(bytes_available);
            dc->devices[serial] = obj;
            dc->saveDevices();


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

