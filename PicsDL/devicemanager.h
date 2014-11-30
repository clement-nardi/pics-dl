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

#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <deviceconfig.h>
#include "deviceconfigview.h"

class DeviceManager : public QObject
{
    Q_OBJECT
public:
    explicit DeviceManager(DeviceConfig *deviceConfig, QObject *parent = 0);

private:
    DeviceConfig *dc;
    QSet<DeviceConfigView *> openedViews;

signals:
    void newID(QString);

public slots:
    void treatDrive(QString);
    void handleDestroyed(QObject*);
};

#endif // DEVICEMANAGER_H
