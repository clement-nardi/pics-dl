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
#include <QDateTime>

GeotaggerWorker::GeotaggerWorker() {
    wasStopped = false;
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

void GeotaggerWorker::stop() {
    wasStopped = true;
}

void GeotaggerWorker::setTrackFilesFolder(File trackFilesFolder_) {
    init();
    qDebug() << "setTrackFilesFolder from thread " << QThread::currentThreadId();
    if (trackFilesFolder_.absoluteFilePath != trackFilesFolder.absoluteFilePath) {
        int fileIdx = 0;
        int nbFiles = 0;
        trackFilesFolder = trackFilesFolder_;
        qDebug() << "loading track files from:" << trackFilesFolder.absoluteFilePath;
        bool theresMore = true;
        while (theresMore) {
            QList<File> contentPart = trackFilesFolder.ls(&theresMore);
            nbFiles += contentPart.size();
            for (int i = 0; i < contentPart.size(); ++i) {
                if (wasStopped) {
                    qDebug() << "Stop loading track files";
                    return;
                }
                fileIdx++;
                if (!contentPart.at(i).isDir) {
                    qDebug() << "Loading track file: "  << contentPart.at(i).absoluteFilePath;
                    emit loadingTrackFile(fileIdx,nbFiles);
                    QFile file(contentPart.at(i).absoluteFilePath);
                    file.open(QIODevice::ReadOnly);
                    QByteArray ba = file.readAll();
                    exiftool->loadTrackContent(ba.data(),ba.size());
                    file.close();
                }
            }
        }
        emit loadingTrackFilesFinished();
    }
}

void GeotaggerWorker::handleWriteFinished(File *file){
    emit writeFinished(file);
}

void GeotaggerWorker::geotag(File *file){
    init();
    qDebug() << QString("[Thread %1] geotag %2 (%3)")
                .arg((long)(QThread::currentThreadId()))
                .arg(file->fileName())
                .arg(File::size2Str(file->size));
    QBuffer *out = new QBuffer();
    out->open(QIODevice::WriteOnly);
    out->seek(0);
    char *geotaggedContent = NULL;
    qint64 geotaggedContent_size = 0;
    exiftool->geotag(file->buffer->buffer().data(),(long)file->buffer->size(),&geotaggedContent,(long*)&geotaggedContent_size);
    out->write(geotaggedContent, geotaggedContent_size);
    out->close();
    file->setGeotaggedBuffer(out);
}


void GeotaggerWorker::getGeotags(File *file) {
    int nbTags = 0;
    char keys[10][200] = {0};
    char values[10][200] = {0};
    qDebug() << QString("[Thread %1] getGeotags date=%3 (%2)")
                .arg((long)(QThread::currentThreadId()))
                .arg(file->fileName())
                .arg(file->dateTakenRaw());
    exiftool->getGeotags(file->dateTakenRaw().toStdString().c_str(),&nbTags,keys,values);

    file->geotags.clear();
    for (int i = 0; i< nbTags; i++) {
        file->geotags.insert(QString(keys[i]),QString(values[i]));
    }
    //qDebug() << file->geotags;

    emit getGeotagsFinished(file);
}


GeotaggerPrivate::GeotaggerPrivate(){
    gw = new GeotaggerWorker();
    thread = new QThread();
    gw->moveToThread(thread);
    qRegisterMetaType<File>("File");
    connect(this,SIGNAL(setTrackFilesFolder(File)),gw,SLOT(setTrackFilesFolder(File)),Qt::QueuedConnection);
    connect(this,SIGNAL(geotag(File*)),gw,SLOT(geotag(File*)),Qt::QueuedConnection);
    connect(this,SIGNAL(getGeotags(File*)),gw,SLOT(getGeotags(File*)),Qt::QueuedConnection);
    thread->start();
}

GeotaggerPrivate::~GeotaggerPrivate(){
    gw->stop();
    thread->quit();
    thread->connect(thread,SIGNAL(finished()),gw,SLOT(deleteLater()));
    thread->connect(thread,SIGNAL(finished()),thread,SLOT(deleteLater()));
}
