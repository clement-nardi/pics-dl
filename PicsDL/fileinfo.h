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
