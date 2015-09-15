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

void DeviceManager::treatDrive(QString serial, bool force){
    QJsonObject obj = dc->devices[serial].toObject();

    qDebug() << "treating drive " << serial << obj[CONFIG_PATH].toString() << obj[CONFIG_DISPLAYNAME].toString();

    if (!force) {
        if (obj[CONFIG_ISMANAGED].isUndefined() || obj[CONFIG_ISMANAGED].isNull() ) {
            qDebug() << "first time";

            /* avoid asking twice the same question */
            obj.insert(CONFIG_ISMANAGED,QJsonValue(false));
            dc->devices[serial] = obj;
            dc->deviceFieldChanged(serial);
            QString deviceDescription = obj[CONFIG_DISPLAYNAME].toString();
            if (obj[CONFIG_DISPLAYNAME].toString() != obj[CONFIG_PATH].toString()) {
                deviceDescription += " (" + obj[CONFIG_PATH].toString() + ")";
            }

            QMessageBox mb(QMessageBox::Question,
                           QCoreApplication::applicationName(),
                           QString(tr("New device detected: %1\nWould you like to manage this device with %2?"))
                                .arg(deviceDescription)
                                .arg(QCoreApplication::applicationName()),
                           QMessageBox::Yes | QMessageBox::No);
            int answer = mb.exec();
            qDebug() << "answer is " << answer;

            obj.insert(CONFIG_ISMANAGED,QJsonValue(answer == QMessageBox::Yes));
            dc->devices[serial] = obj;
            dc->deviceFieldChanged(serial);
            dc->saveDevices();
        } else {
            qDebug() << "known drive";
        }
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

