#include "geotaggerprivate.h"

#include "exiftoolperlwrapper.h"
#include <QList>
#include <QDebug>
#include <QByteArray>
#include "file.h"
#include <QBuffer>
#include <QIODevice>
#include <QApplication>
#include <QThread>
#include <QObject>
#include <QDir>

GeotaggerWorker::GeotaggerWorker() {
    exiftool = NULL;
}
GeotaggerWorker::~GeotaggerWorker() {
    qDebug() << "deleting the worker";
    delete exiftool;
}
void GeotaggerWorker::init() {
    if (exiftool == NULL) {
        qDebug() << "initializing the worker";
        exiftool = new ExifToolPerlWrapper(QString(QCoreApplication::applicationDirPath() + "/perl").toStdString().c_str());
    }
}

void GeotaggerWorker::setTrackFilesFolder(File trackFilesFolder_) {
    init();
    qDebug() << "setTrackFilesFolder from thread " << QThread::currentThreadId();
    if (trackFilesFolder_.absoluteFilePath != trackFilesFolder.absoluteFilePath) {
        qDebug() << "loading track files from:" << trackFilesFolder.absoluteFilePath;
        trackFilesFolder = trackFilesFolder_;
        bool theresMore = true;
        while (theresMore) {
            QList<File> contentPart = trackFilesFolder.ls(&theresMore);
            for (int i = 0; i < contentPart.size(); ++i) {
                if (!contentPart.at(i).isDir) {
                    qDebug() << "Loading track file: "  << contentPart.at(i).absoluteFilePath;
                    exiftool->loadTrackFile(contentPart.at(i).absoluteFilePath.toStdString().c_str());
                }
            }
        }
    }
}

void GeotaggerWorker::geotag(File *file, QString outName){
    init();
    qDebug() << QString("[Thread %1] geotag %2 (%3)")
                .arg((long)(QThread::currentThreadId()))
                .arg(file->fileName())
                .arg(File::size2Str(file->size));
    char *geotaggedContent = NULL;
    qint64 geotaggedContent_size = 0;
    exiftool->geotag(file->getBufferredContent()->buffer().data(),(long)file->getBufferredContent()->size(),&geotaggedContent,(long*)&geotaggedContent_size);
    QDir().mkpath(QFileInfo(outName).absolutePath());
    QFile out(outName);
    out.open(QIODevice::WriteOnly);
    out.seek(0);
    out.write(geotaggedContent, geotaggedContent_size);
    out.close();
    File::setDates(outName,file->lastModified);
    emit writeFinished(file);
}



GeotaggerPrivate::GeotaggerPrivate(){
    gw = new GeotaggerWorker();
    gw->moveToThread(&thread);
    qRegisterMetaType<File>("File");
    connect(this,SIGNAL(setTrackFilesFolder(File)),gw,SLOT(setTrackFilesFolder(File)),Qt::QueuedConnection);
    connect(this,SIGNAL(geotag(File*,QString)),gw,SLOT(geotag(File*,QString)),Qt::QueuedConnection);
    thread.start();
}

GeotaggerPrivate::~GeotaggerPrivate(){
    delete gw;
    thread.quit();
    thread.wait(10000);
}
