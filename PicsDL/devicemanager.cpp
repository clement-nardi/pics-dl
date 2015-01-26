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


DeviceManager::DeviceManager(DeviceConfig *deviceConfig, QObject *parent) :
    QObject(parent)
{
    dc = deviceConfig;
}

void DeviceManager::treatDrive(QString drivePath, QString serial, QString displayName){

    qDebug() << "treating drive " << drivePath << serial << displayName;

    if (dc->conf[serial].isUndefined() || dc->conf[serial].isNull() ) {
        qDebug() << "first time";
        QJsonObject driveConfig;

        /* avoid asking twice the same question */
        driveConfig.insert("isManaged",QJsonValue(false));
        dc->conf.insert(serial,driveConfig);
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
                obj["path"] = displayName;
                obj["IDPath"] = drivePath;
            } else {
                obj["path"] = drivePath;
            }
            obj["displayName"] = displayName;
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

