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

#ifndef DEVICEMODEL_H
#define DEVICEMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QObject>
#include "drivenotify.h"

class Config;
class DriveView;

#define NB_COLUMNS 9

#define COL_NAME        0
#define COL_AVAILABLE   1
#define COL_TOTAL       2
#define COL_CAMERA      3
#define COL_MANAGE      4
#define COL_EDIT        5
#define COL_LAUNCH      6
#define COL_REMOVE      7
#define COL_LASTTRANSFER 8

class DeviceModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    DeviceModel(Config *dc_, DriveNotify *dn_);
    ~DeviceModel();
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex & index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
    QList<DriveView*> deviceList;

private:
    Config *dc;
    DriveNotify *dn;
    void emptyList();
    void setIndexes();

public slots:
    void driveChanged(int row);
    void reload();
};

#endif // DEVICEMODEL_H
