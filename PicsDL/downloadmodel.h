#ifndef DOWNLOADMODEL_H
#define DOWNLOADMODEL_H

#include <QAbstractTableModel>
#include "deviceconfig.h"
#include "fileinfo.h"
#include <QList>
#include <QProgressDialog>
#include <QItemDelegate>
#include <QElapsedTimer>

extern QString ExifToolPath;

class DownloadModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit DownloadModel(DeviceConfig *dc, QProgressDialog *pd, bool editMode = false, QObject *parent = 0);
    ~DownloadModel();
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    void loadPreview(QString id);
    bool downloadToTemp();
    void moveToFinalLocation();
    bool geoTag();
    void showEXIFTags(int row);
    QString guessCameraName();
private:
    QElapsedTimer pdTimer;
    DeviceConfig *dc;
    bool editMode;
    QString id;
    QList<FileInfo*> *blacklistedDirectories;
    QList<FileInfo*> *completeFileList;
    QList<FileInfo*> *selectedFileList;
    bool isBlacklisted(FileInfo info);
    void treatDir(FileInfo dirInfo);
    QString newPath(FileInfo *fi, bool keepDComStr = false) const;
    int itemBeingDownloaded;
    QString getDCom(FileInfo *fi, bool forceQuery = false) const;
    QString sessionComment;
    QProgressDialog *pd;
    int discoveredFolders;
    int browsedFolders;
    int browsedFiles;
    void emptyFileList();
    QString DLTempFolder;
    QString tempPath(FileInfo *fi) const;
signals:
    void itemOfInterest(QModelIndex);
    void reloaded();
    void EXIFLoadCanceled(bool);
public slots:
    void reloadSelection();
    bool getAllCom();

};


#endif // DOWNLOADMODEL_H
