#include "geotagger.h"

#include "exiftoolperlwrapper.h"
#include <QList>
#include <QDebug>
#include <QByteArray>
#include "file.h"
#include <QBuffer>
#include <QIODevice>

Geotagger::Geotagger(File *trackFilesFolder)
{
    qDebug() << trackFilesFolder->absoluteFilePath;
    exiftool = new ExifToolPerlWrapper();

    bool theresMore = true;
    while (theresMore) {
        QList<File> contentPart = trackFilesFolder->ls(&theresMore);
        for (int i = 0; i < contentPart.size(); ++i) {
            if (!contentPart.at(i).isDir) {
                qDebug() << "Loading track file: "  << contentPart.at(i).absoluteFilePath;
                exiftool->loadTrackFile(contentPart.at(i).absoluteFilePath.toStdString().c_str());
            }
        }
    }
}

bool Geotagger::geotag(QBuffer *in, QIODevice *out) {
    char *geotaggedContent = NULL;
    qint64 geotaggedContent_size = 0;
    exiftool->geotag(in->buffer().data(),(long)in->size(),&geotaggedContent,(long*)&geotaggedContent_size);
    out->open(QIODevice::WriteOnly);
    out->seek(0);
    out->write(geotaggedContent, geotaggedContent_size);
    out->close();
    //TODO
    return true;
}
