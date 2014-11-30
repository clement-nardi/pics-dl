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

#include "driveview.h"
#include <QJsonObject>
#include <QDebug>
#include "deviceconfigview.h"

DriveView::DriveView(QString id_, DeviceConfig *dc_, QObject *parent) :
    QObject(parent)
{
    id = id_;
    dc = dc_;
    QJsonObject obj = dc->conf[id].toObject();

    removeButton = new QPushButton(QIcon(":/icons/remove"),"Remove");
    editButton = new QPushButton(QIcon(":/icons/edit"),"Edit");
    launchButton = new QPushButton(QIcon(":/icons/play"),"Launch");
    label = new QLabel();
    driveIcon = new QLabel();
    managedBox = new QCheckBox("Manage this drive");

    label->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    label->setMinimumWidth(200);

    loadName();

    changeState(obj["isManaged"].toBool());
    connect(managedBox, SIGNAL(clicked(bool)), this, SLOT(managed(bool)));
    connect(removeButton, SIGNAL(clicked()), this, SLOT(removed()));
    connect(editButton, SIGNAL(clicked()), this, SLOT(edit()));
}

void DriveView::loadName() {
    QJsonObject obj = dc->conf[id].toObject();
    QString deviceName;
    if (obj["IDPath"].toString().startsWith("WPD:/")) {
        deviceName = obj["path"].toString();
    } else {
        if (id.split(";").size() > 1) {
            QString name = id.split(";").at(1);
            if (name.length() > 0) {
                deviceName = name;
            } else {
                deviceName = id;
            }
        } else {
            deviceName = id;
        }
        if (obj["path"].toString().size()>0) {
            deviceName.append(" (" + obj["path"].toString() + ")");
        }
    }
    if (obj["CameraName"].toString().size()>0) {
        deviceName.append(" (" + obj["CameraName"].toString() + ")");
    }
    label->setText(deviceName);
}

void DriveView::changeState(bool enabled){
    if (enabled) {
        managedBox->setCheckState(Qt::Checked);
        driveIcon->setPixmap(QIcon(":/icons/drive").pixmap(30,30, QIcon::Normal));
    } else {
        managedBox->setCheckState(Qt::Unchecked);
        driveIcon->setPixmap(QIcon(":/icons/drive").pixmap(30,30, QIcon::Disabled));
    }
    label->setEnabled(enabled);
}

void DriveView::edit(){
    DeviceConfigView *dcv = new DeviceConfigView(dc,id,true);
    dcv->show();
}

void DriveView::removed(){
    dc->conf.remove(id);
    dc->saveConfig();
    label->deleteLater();
    driveIcon->deleteLater();
    managedBox->deleteLater();
    removeButton->deleteLater();
    editButton->deleteLater();
    deleteLater();
}

void DriveView::managed(bool checked){
    QJsonObject obj = dc->conf[id].toObject();
    obj.insert("isManaged",QJsonValue(checked));
    dc->conf[id] = obj;
    dc->saveConfig();
    changeState(checked);
}
