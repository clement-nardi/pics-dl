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

#ifndef DRIVEVIEW_H
#define DRIVEVIEW_H

#include <QObject>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include "deviceconfig.h"

class DriveView : public QObject
{
    Q_OBJECT
public:
    explicit DriveView(QString id, DeviceConfig *dc, QObject *parent = 0);
    DeviceConfig *dc;
    QString id;
    QLabel *label;
    QCheckBox *managedBox;
    QPushButton *removeButton;
    QPushButton *editButton;
    QPushButton *launchButton;
    QLabel *driveIcon;

private:
    void changeState(bool enabled);

signals:
    void changed();
public slots:
    void removed();
    void edit();
    void managed(bool);
    void loadName();

};

#endif // DRIVEVIEW_H
