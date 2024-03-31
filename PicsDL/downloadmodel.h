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

#ifndef DOWNLOADMODEL_H
#define DOWNLOADMODEL_H

#include <QAbstractTableModel>
#include "config.h"
#include "file.h"
#include <QList>
#include <QProgressDialog>
#include <QItemDelegate>
#include <QElapsedTimer>
#include <QMutex>
#include <QHash>
#include <QMap>
class Geotagger;

extern QString ExifToolPath;

#define NB_COLUMNS      8

#define COL_THUMBNAIL   0
#define COL_FILEPATH    1
#define COL_FILENAME    2
#define COL_SIZE        3
#define COL_MODIFIED    4
#define COL_DATE        5
#define COL_NEWPATH     6
#define COL_GPS         7

class DownloadModel : public QAbstractTableModel
{
    friend class TransferManager;
    friend class TransferWorker;
    Q_OBJECT
public:
    explicit DownloadModel(Config *dc, bool editMode = false, QObject *parent = 0);
    ~DownloadModel();
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    void sort(int column, Qt::SortOrder order);
    Qt::ItemFlags flags(const QModelIndex & index) const;
    void loadPreview(QString id);
    bool launchDownload();
    void moveToFinalLocation();
    QString getTrackingFolder();
    void showEXIFTags(int row);
    QString guessCameraName();
    void getStats(qint64 *totalSize, int *nbFiles);
    qint64 averagePicSize;
    void freeUpSpace(bool isFakeRun, qint64 *targetAvailable, qint64 *bytesDeleted, int *nbFilesDeleted);
    QList<File*> deletedFiles;
    QSize thumbnailSize(int row);
    QList<int> sortColumnOrder;
    bool sortIsAscending[NB_COLUMNS];
    QString &newPath(File *fi) const;
    QString newPath(File *fi, bool keepDComStr) const;
    void markForDownload(QModelIndexList rows, bool mark);
private:
    Config *dc;
    bool editMode;
    QString id;
    QList<File> blacklistedDirectories;
    QList<File*> completeFileList;
    QList<File*> completeFileList_byDate;
    QList<File*> selectedFileList;
    QSet<File*> excludedFiles;
    bool isBlacklisted(File info);
    void treatDir(File dirInfo, QProgressDialog *pd, QElapsedTimer *pdTimer);
    int itemBeingDownloaded;
    QString getDCom(File *fi) const;
    QString sessionComment;
    int discoveredFolders;
    int browsedFolders;
    int browsedFiles;
    void emptyFileList();
    QString DLTempFolder;
    QString tempPath(File *fi) const;
    mutable QHash<File*,QString> newPathCache;
    mutable QMap<QString,int> newPathCollisions;
    mutable QSet<File*> GPSRequested;
    QDateTime dateForNewName(File *fi) const;

    void sortSelection();
signals:
    void itemOfInterest(QModelIndex);
    void reloaded();
    void selectionModified();
    void EXIFLoadCanceled(bool);
    void downloadFinished();
    void requestGPSCoord(File *) const;
public slots:
    void reloadSelection(bool firstTime = false);
    bool getAllCom();
    void updateNewPaths();
    void receiveGPSCoord(File*fi);
    void updateGPS();
private slots:
    void readStarted(File * file);
    void writeFinished(File * file);

};


#endif // DOWNLOADMODEL_H
