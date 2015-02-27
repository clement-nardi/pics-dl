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
class TransferManager;

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

    void launchTransferTo(QString to, TransferManager *tm_, bool geotag);
    QBuffer * buffer;
    QBuffer * geotaggedBuffer;
    void setGeotaggedBuffer(QBuffer *geotaggedBuffer_);

    static void setDates(QString fileName,uint date);

    QMap<QString,QString> geotags;
    QString transferTo;
    int modelRow;
    bool transferOnGoing;

private:
    QPixmap thumbnail;
    bool exifLoadAttempted;
    ExifData *exifData;
    void init(QFileInfo qfi);
    void constructCommonFields();
    bool FillIODeviceWithContent(QIODevice *out);
    void pipe(QIODevice *in, QIODevice *out);
    QString pipedTo;
    QSemaphore readSemaphore;
    void launchWrite(QString dest, bool geotag = true);
    void writeHeader(QString dest = "");
    void writeContent(QString dest = "");
    void pipe(QString to);
    void pipeToBuffer();
    TransferManager *tm;
    QIODevice *getReadDevice();

private slots:
    void readStarted();
    void writeFinished();
signals:
    void readStarted(File *);
    void writeFinished(File *);
};

uint qHash(File fi);

class IOReader : public QThread
{
    Q_OBJECT
public:
    IOReader(QIODevice *device, QSemaphore *s, TransferManager *tm_);
    void run();
signals:
    void dataChunk(QByteArray data, bool theresMore);
    void readStarted();
private:
    QIODevice *device;
    QSemaphore *s;
    TransferManager *tm;
};

class IOWriter : public QObject
{
    Q_OBJECT
public:
    IOWriter(QIODevice *device, QSemaphore *s, TransferManager *tm_);
signals:
    void writeFinished();
public slots:
    void dataChunk(QByteArray data, bool theresMore);
private:
    QIODevice *device;
    QSemaphore *s;
    TransferManager *tm;
    void init();
    bool initDone;
    bool deviceIsFile;
};

#endif // FILEINFO_H
