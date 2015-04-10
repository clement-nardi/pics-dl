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

#ifndef DRIVEVIEW_H
#define DRIVEVIEW_H

#include <QObject>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include "config.h"
class DriveNotify;

class DriveView : public QObject
{
    Q_OBJECT
public:
    explicit DriveView(QString id, Config *dc, DriveNotify *dn_, QObject *parent = 0);
    Config *dc;
    DriveNotify *dn;
    QString id;
    QCheckBox *managedBox;
    QWidget *managedBox_centered;
    QPushButton *removeButton;
    QPushButton *editButton;
    QPushButton *launchButton;
    int row;

signals:
    void changed(int);
    void launchTransfer(QString path,QString serial,QString name, qint64 device_size, qint64 bytes_available, bool force);
public slots:
    void removed();
    void edit();
    void launch();
    void managed(bool);

};

#endif // DRIVEVIEW_H
