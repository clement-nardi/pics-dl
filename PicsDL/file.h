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
#include <QBuffer>
#include <QObject>
#include <QThread>
#include <QMap>
#include <QSemaphore>

class File : public QObject
{
    Q_OBJECT
public:
    File();
    File(QFileInfo qfi);
    File(QString path);
    File(const File &fi);
    File &operator=(const File &fi);
    void shallowCopy(const File *fi);

    File(uint lastModified, QString absoluteFilePath, quint64 size, bool isDir, QString IDPath = "");
    ~File();
    uint lastModified;
    uint dateTaken() const;
    QString dateTakenRaw() const;
    void loadExifData();
    QString getEXIFValue(QString key) const;
    QPixmap getThumbnail();
    QStringList getEXIFTags();
    QString absoluteFilePath;
    QString IDPath;
    bool isDir;
    QString fileName() const;
    QString extension() const;
    quint64 size;
    int operator<(File);
    int operator==(const File &other) const;
    bool isAttachmentOf(File other);
    bool isPicture() const;
    bool isVideo() const;
    bool isJPEG() const;
    static QString size2Str(qint64 nbBytes);

    QList<File> ls(bool *theresMore);
    bool copyWithDirs(QString to);
    bool moveWithDirs(QString to);
    bool setHidden();
    QBuffer *getBufferredContent();

    void launchReadToCache();
    void launchWrite(QString dest, bool geotag = true);
    void writeHeader(QString dest = "");
    void writeContent(QString dest = "");

    static void setDates(QString fileName,uint date);
    void pipe(QString to);
    void deleteBuffer();

    QMap<QString,QString> geotags;

private:
    QPixmap thumbnail;
    bool exifLoadAttempted;
    QBuffer *buffer;
    ExifData *exifData;
    void init(QFileInfo qfi);
    void constructCommonFields();
    bool FillIODeviceWithContent(QIODevice *out);
    void pipe(QIODevice *in, QIODevice *out);
    QString pipedTo;
    QSemaphore readSemaphore;
private slots:
    void writeFinished();
signals:
    readFinished(File *);
    writeFinished(File *);
};

uint qHash(File fi);

class FileReader : public QThread
{
    Q_OBJECT
public:
    FileReader(File *file_);
    void run();
signals:
    void done(File *);
private:
    File *file;
};

class IOReader : public QThread
{
    Q_OBJECT
public:
    IOReader(QIODevice *device, QSemaphore *s);
    void run();
signals:
    void dataChunk(QByteArray data, bool theresMore);
private:
    QIODevice *device;
    QSemaphore *s;
};

class IOWriter : public QObject
{
    Q_OBJECT
public:
    IOWriter(QIODevice *device, QSemaphore *s);
signals:
    writeFinished();
public slots:
    void dataChunk(QByteArray data, bool theresMore);
private:
    QIODevice *device;
    QSemaphore *s;
    void init();
    bool initDone;
};

#endif // FILEINFO_H
