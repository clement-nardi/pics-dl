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
class Geotagger;

extern QString ExifToolPath;

class DownloadModel : public QAbstractTableModel
{
    friend class TransferManager;
    friend class TransferWorker;
    Q_OBJECT
public:
    explicit DownloadModel(Config *dc, QProgressDialog *pd, bool editMode = false, QObject *parent = 0);
    ~DownloadModel();
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
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
private:
    QElapsedTimer pdTimer;
    Config *dc;
    bool editMode;
    QString id;
    QList<File*> blacklistedDirectories;
    QList<File*> completeFileList;
    QList<File*> completeFileList_byDate;
    QList<File*> selectedFileList;
    bool isBlacklisted(File info);
    void treatDir(File dirInfo);
    QString newPath(File *fi, bool keepDComStr = false) const;
    int itemBeingDownloaded;
    QString getDCom(File *fi, bool forceQuery = false) const;
    QString sessionComment;
    QProgressDialog *pd;
    int discoveredFolders;
    int browsedFolders;
    int browsedFiles;
    void emptyFileList();
    QString DLTempFolder;
    QString tempPath(File *fi) const;

signals:
    void itemOfInterest(QModelIndex);
    void reloaded();
    void EXIFLoadCanceled(bool);
    void downloadFinished();
public slots:
    void reloadSelection(bool firstTime = false);
    bool getAllCom();
private slots:
    void readStarted(File * file);
    void writeFinished(File * file);

};


#endif // DOWNLOADMODEL_H
