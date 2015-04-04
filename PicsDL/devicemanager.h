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

#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QJsonObject>
#include "deviceconfigview.h"
#include <QSet>

class Config;

class DeviceManager : public QObject
{
    Q_OBJECT
public:
    explicit DeviceManager(Config *deviceConfig, QObject *parent = 0);

private:
    Config *dc;
    QSet<DeviceConfigView *> openedViews;

public slots:
    void treatDrive(QString, QString serial, QString displayName, qint64 deviceSize, qint64 bytes_available);
    void handleDestroyed(QObject*);
};

#endif // DEVICEMANAGER_H
