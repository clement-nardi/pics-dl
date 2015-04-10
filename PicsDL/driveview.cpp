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

#include "driveview.h"
#include <QJsonObject>
#include <QDebug>
#include "deviceconfigview.h"
#include <QHBoxLayout>
#include "drivenotify.h"

DriveView::DriveView(QString id_, Config *dc_, DriveNotify *dn_, QObject *parent) :
    QObject(parent)
{
    id = id_;
    dc = dc_;
    dn = dn_;
    row = 0;
    QJsonObject obj = dc->devices[id].toObject();

    removeButton = new QPushButton(QIcon(":/icons/remove"),"");
    editButton = new QPushButton(QIcon(":/icons/edit"),"");
    launchButton = new QPushButton(QIcon(":/icons/play"),"");
    managedBox_centered = new QWidget();
    managedBox = new QCheckBox(managedBox_centered);

    editButton->setEnabled(false);
    launchButton->setEnabled(false);

    QHBoxLayout *managedLayout = new QHBoxLayout(managedBox_centered);
    managedLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    managedLayout->addWidget(managedBox);
    managedLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    managedLayout->setSpacing(0);
    managedLayout->setMargin(0);
    managedBox_centered->setLayout(managedLayout);

    connect(managedBox,SIGNAL(toggled(bool)),editButton,SLOT(setEnabled(bool)));
    managedBox->setChecked(obj[CONFIG_ISMANAGED].toBool());
    launchButton->setEnabled(dn->getDeviceInfo(id));

    connect(managedBox, SIGNAL(clicked(bool)), this, SLOT(managed(bool)));
    connect(removeButton, SIGNAL(clicked()), this, SLOT(removed()));
    connect(editButton, SIGNAL(clicked()), this, SLOT(edit()));
    connect(launchButton, SIGNAL(clicked()), this, SLOT(launch()));
}


void DriveView::edit(){
    DeviceConfigView *dcv = new DeviceConfigView(dc,id,true);
    dcv->show();
}

void DriveView::removed(){
    dc->devices.remove(id);
    dc->deviceFieldChanged(id);
    dc->saveDevices();
}

void DriveView::managed(bool checked){
    QJsonObject obj = dc->devices[id].toObject();
    obj.insert(CONFIG_ISMANAGED,QJsonValue(checked));
    dc->devices[id] = obj;
    dc->saveDevices();
    changed(row);
}

void DriveView::launch(){
    QString path;
    QString name;
    qint64 total;
    qint64 available;
    if (dn->getDeviceInfo(id,&path,&name,&total,&available)) {
        emit launchTransfer(path,id,name,total,available,true);
    }
}
