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

#include "devicemodel.h"
#include "config.h"
#include "driveview.h"
#include <QDebug>

static QList<int> sortColumnOrder;
static bool sortIsAscending[12] = {true};

DeviceModel::DeviceModel(Config *dc_, DriveNotify *dn_){
    dc = dc_;
    dn = dn_;
    sortColumnOrder.append(COL_LAUNCH); /* show first connected devices */
    sortColumnOrder.append(COL_MANAGE); /* then sort by managed */
    sortColumnOrder.append(COL_LASTTRANSFER); /* then by last transfer date*/
    sortColumnOrder.append(COL_NAME); /* then by name */
    sortColumnOrder.append(COL_TOTAL); /* then by total space */
    for (int i = 0; i<NB_COLUMNS; i++) {
        sortIsAscending[i] = true;
    }
    sortIsAscending[COL_MANAGE] = false;
    sortIsAscending[COL_LASTTRANSFER] = false;
    sortIsAscending[COL_LAUNCH] = false;
    sortIsAscending[COL_EDIT] = false;
    sortIsAscending[COL_REMOVE] = false;
    reload();
    connect(dc,SIGNAL(configStructuralChange(QString)),this,SLOT(reload()));
}

#define IF_DIFFERENT_RETURN(a,b,asc) {if ((a) != (b)) {return (asc)?((a)<(b)):((b)<(a));}}

bool driveLessThan(DriveView *a, DriveView *b) {
    QJsonObject objA = a->dc->devices[a->id].toObject();
    QJsonObject objB = b->dc->devices[b->id].toObject();
    for (int i = 0; i < sortColumnOrder.size(); i++) {
        int column = sortColumnOrder.at(i);
        switch (column) {
        case COL_NAME: IF_DIFFERENT_RETURN(objA[CONFIG_DISPLAYNAME].toString(),objB[CONFIG_DISPLAYNAME].toString(),sortIsAscending[column]) break;
        case COL_AVAILABLE: IF_DIFFERENT_RETURN(objA[CONFIG_BYTESAVAILABLE].toString().toLongLong(),objB[CONFIG_BYTESAVAILABLE].toString().toLongLong(),sortIsAscending[column]) break;
        case COL_TOTAL: IF_DIFFERENT_RETURN(objA[CONFIG_DEVICESIZE].toString().toLongLong(),objB[CONFIG_DEVICESIZE].toString().toLongLong(),sortIsAscending[column]) break;
        case COL_CAMERA: IF_DIFFERENT_RETURN(objA[CONFIG_CAMERANAME].toString(),objB[CONFIG_CAMERANAME].toString(),sortIsAscending[column]) break;
        case COL_MANAGE: IF_DIFFERENT_RETURN(objA[CONFIG_ISMANAGED].toBool(),objB[CONFIG_ISMANAGED].toBool(),sortIsAscending[column]) break;
        case COL_LASTTRANSFER: IF_DIFFERENT_RETURN(objA[CONFIG_LASTTRANSFER].toString().toUInt(),objB[CONFIG_LASTTRANSFER].toString().toUInt(),sortIsAscending[column]) break;
        case COL_LAUNCH: IF_DIFFERENT_RETURN(a->launchButton->isEnabled(),b->launchButton->isEnabled(),sortIsAscending[column]) break;
        case COL_EDIT: IF_DIFFERENT_RETURN(a->editButton->isEnabled(),b->editButton->isEnabled(),sortIsAscending[column]) break;
        case COL_REMOVE: IF_DIFFERENT_RETURN(a->removeButton->isEnabled(),b->removeButton->isEnabled(),sortIsAscending[column]) break;
        }
    }
    return 0;
}

void DeviceModel::reload() {
    beginResetModel();
    emptyList();

    QJsonObject::iterator i;
    for (i = dc->devices.begin(); i != dc->devices.end(); ++i) {
        DriveView *dv = new DriveView(i.key(),dc,dn);
        deviceList.append(dv);
        connect(dv,SIGNAL(changed(int)),this,SLOT(driveChanged(int)));
    }

    std::stable_sort(deviceList.begin(),deviceList.end(),driveLessThan);
    setIndexes();

    endResetModel();
}

void DeviceModel::sort(int column, Qt::SortOrder order){
    qDebug() << QString("sort(%1,%2)").arg(column).arg(order);
    sortColumnOrder.removeAll(column);
    sortColumnOrder.prepend(column);
    sortIsAscending[column] = (order == Qt::AscendingOrder);
    reload();
}

void DeviceModel::emptyList() {
    for (int i = 0; i < deviceList.size(); i++) {
        deviceList.at(i)->deleteLater();
    }
    deviceList.clear();
}

void DeviceModel::setIndexes(){
    for (int i = 0; i < deviceList.size(); i++) {
        deviceList.at(i)->row = i;
    }
}

DeviceModel::~DeviceModel(){
    emptyList();
}

QVariant DeviceModel::headerData(int section, Qt::Orientation orientation, int role) const{
    switch (role) {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
        switch (orientation) {
        case Qt::Horizontal:
            switch (section) {
            case COL_NAME: return tr("Device Name");
            case COL_AVAILABLE: return tr("available");
            case COL_TOTAL: return tr("total");
            case COL_CAMERA: return tr("Camera");
            case COL_MANAGE: return tr("Manage");
            case COL_EDIT: return tr("Edit");
            case COL_LAUNCH: return tr("Launch");
            case COL_REMOVE: return tr("Remove");
            case COL_LASTTRANSFER: return tr("Last Transfer");
            }
            break;
        case Qt::Vertical:
            break;
        }
    }
    return QVariant();
}


int DeviceModel::rowCount(const QModelIndex & parent __attribute__ ((unused))) const{
    return dc->devices.count();
}

int DeviceModel::columnCount(const QModelIndex & parent __attribute__ ((unused))) const{
    return NB_COLUMNS;
}

QVariant DeviceModel::data(const QModelIndex & index, int role) const{
    DriveView *dv = deviceList.at(index.row());
    QJsonObject obj = dc->devices[dv->id].toObject();

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case COL_NAME: return obj[CONFIG_DISPLAYNAME].toString();
        case COL_AVAILABLE: return File::size2Str(obj[CONFIG_BYTESAVAILABLE].toString().toLongLong());
        case COL_TOTAL: return File::size2Str(obj[CONFIG_DEVICESIZE].toString().toLongLong());
        case COL_CAMERA: return obj[CONFIG_CAMERANAME].toString();
        case COL_MANAGE: break;
        case COL_EDIT: break;
        case COL_LAUNCH: break;
        case COL_REMOVE: break;
        case COL_LASTTRANSFER: return obj[CONFIG_LASTTRANSFER].isNull()?tr("Never"):
                                                                        (QDateTime::fromSecsSinceEpoch(obj[CONFIG_LASTTRANSFER].toString().toUInt()).toString("yyyy/MM/dd hh:mm:ss"));
        }
        break;
    case Qt::ToolTipRole:
        switch (index.column()) {
        case COL_NAME: return dv->id;
        case COL_AVAILABLE: return QString(tr("Available space on this device: %1 (%2 Bytes)"))
                       .arg(File::size2Str(obj[CONFIG_BYTESAVAILABLE].toString().toLongLong()))
                       .arg(obj[CONFIG_BYTESAVAILABLE].toString());
        case COL_TOTAL: return QString(tr("You can store up to %1 (%2 Bytes) of data on this device."))
                       .arg(File::size2Str(obj[CONFIG_DEVICESIZE].toString().toLongLong()))
                       .arg(obj[CONFIG_DEVICESIZE].toString());
        case COL_CAMERA: return QString(tr("Some pictures stored on this device\nwhere taken with a %1")).arg(obj[CONFIG_CAMERANAME].toString());
        case COL_MANAGE: return (obj[CONFIG_ISMANAGED].toBool()?tr("This device is managed by PicsDL"):tr("Click here to manage this device with PicsDL."))
                                + "\n" + tr("Managed devices are treated as soon as they are plugged in.");
        case COL_EDIT: return tr("Edit the configuration of this device");
        case COL_LAUNCH: return tr("Launch the transfer of files from this device");
        case COL_REMOVE: return tr("Remove this device from this list");
        case COL_LASTTRANSFER: return obj[CONFIG_LASTTRANSFER].isNull()?tr("No file was ever downloaded from this device"):
                                                                        (QString(tr("The last time some files were transfered from this device was on %1"))
                                                                         .arg(QDateTime::fromSecsSinceEpoch(obj[CONFIG_LASTTRANSFER].toString().toUInt()).toString()));
        }
        break;
    case Qt::DecorationRole:
        if (index.column() == 0) {
            return QIcon(":/icons/drive");
        }
        break;
    case Qt::SizeHintRole:
        break;
    }
    return QVariant();
}

Qt::ItemFlags DeviceModel::flags(const QModelIndex & index) const{
    DriveView *dv = deviceList.at(index.row());
    QJsonObject obj = dc->devices[dv->id].toObject();
    return obj[CONFIG_ISMANAGED].toBool()?Qt::ItemIsEnabled:Qt::NoItemFlags;
}

void DeviceModel::driveChanged(int row){
    emit dataChanged(createIndex(row,0),createIndex(row,columnCount()));
}
