#ifndef FILEINFO_H
#define FILEINFO_H

#include <QFileInfo>
#include <QDateTime>
#include <QString>
#include "libexif/exif-data.h"
#include <QStringList>
#include <QPixmap>

class FileInfo
{
public:
    FileInfo(QFileInfo qfi);
    FileInfo(uint lastModified, QString absoluteFilePath, quint64 size, bool isDir, QString IDPath = "");
    uint lastModified;
    uint dateTaken() const;
    void loadExifData();
    QString getEXIFValue(QString key) const;
    QPixmap getThumbnail();
    QStringList getEXIFTags();
    QString absoluteFilePath;
    QString IDPath;
    bool isDir;
    ExifData * exifData;
    QString fileName() const;
    QString extension() const;
    quint64 size;
    int operator<(FileInfo);
    int operator==(const FileInfo &other) const;
    bool isAttachmentOf(FileInfo other);
    bool isPicture() const;
    bool isVideo() const;
    bool isJPEG() const;
    static QString size2Str(qint64 nbBytes);
private:
    QPixmap thumbnail;
    bool exifLoadAttempted;
};

uint qHash(FileInfo fi);

#endif // FILEINFO_H
