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
