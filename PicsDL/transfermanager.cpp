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

#include "transfermanager.h"
#include "downloadmodel.h"
#include <QDebug>
#include <QDir>
#include "geotagger.h"



TransferWorker::TransferWorker(TransferManager *tm_, bool geotag_) {
    tm=tm_;
    geotag = geotag_;
    if (geotag) {
        fileList = &(tm->filesToGeotag);
        semaphore = &(tm->geotagSemaphore);
    } else {
        fileList = &(tm->filesToTransfer);
        semaphore = &(tm->directSemaphore);
    }
}

void TransferWorker::processList() {
    for (int i = 0; i< fileList->size(); i++) {
        semaphore->acquire();
        if (tm->wasStopped) return;
        File * file = fileList->at(i);
        qDebug() << QString("worker %1: %2/%3 - %4")
                    .arg(geotag)
                    .arg(i)
                    .arg(fileList->size())
                    .arg(file->absoluteFilePath);
        emit launchFile(file,geotag);
    }
    qDebug() << QString("Worker %1 finished looping on %2 files").arg(geotag).arg(fileList->size());
}


TransferManager::TransferManager(QObject *parent, DownloadModel *dm_) : QObject(parent)
{
    dm = dm_;
    geotagger = NULL;
    udpateGeoTagger();
    resetStats();

    initWorker(0);
    initWorker(1);
}


void TransferManager::initWorker(int idx) {
    tw[idx] = new TransferWorker(this,idx==0);
    tw[idx]->moveToThread(&wt[idx]);
    connect(this, SIGNAL(startWorkers()),tw[idx],SLOT(processList()));
    connect(tw[idx],SIGNAL(launchFile(File*,bool)),this,SLOT(launchFile(File*,bool)));
    wt[idx].start();
}

void TransferManager::launchFile(File *file, bool geotag) {
    QString fi_newPath = dm->newPath(file);
    connect(file,SIGNAL(writeFinished(File*)),this,SLOT(handleWriteFinished(File*)),Qt::UniqueConnection);
    file->launchTransferTo(fi_newPath,this,geotag);
}

void TransferManager::resetStats() {
    totalRead = 0;
    totalToCache = 0;
    totalWritten = 0;
    totalToWrite = 0;
    nbFilesToTransfer = 0;
    nbFilesTransfered = 0;
}

TransferManager::~TransferManager()
{
    delete geotagger;
    delete tw[0];
    delete tw[1];
}


void TransferManager::launchDownloads() {
    qDebug() << "TransferManager::launchDownloads";

    filesToTransfer.clear();
    filesToGeotag.clear();
    wasStopped = false;

    if (dm->selectedFileList.size() > 0) {

        udpateGeoTagger();
        resetStats();

        totalToWrite = 0;
        for (int i = 0; i < dm->selectedFileList.size(); i++) {
            if (wasStopped) return;
            File * file = dm->selectedFileList.at(i);
            QString fi_newPath = dm->newPath(file);
            if (QFile(fi_newPath).exists()) {
                dm->dc->knownFiles.insert(*file);
                qDebug() << "Will not overwrite " << fi_newPath;
            } else {
                totalToWrite += file->size;
                nbFilesToTransfer++;
                if (geotagger != NULL && file->size < 50*1024*1024) {
                    filesToGeotag.append(file);
                } else {
                    filesToTransfer.append(file);
                }
            }
        }
        directSemaphore.release(1-directSemaphore.available());
        geotagSemaphore.release(2-geotagSemaphore.available());
        emit startWorkers();
    }
    if (nbFilesToTransfer == 0) {
        qDebug() << "Nothing to transfer...";
        emit downloadFinished();
    }
}

void TransferManager::stopDownloads() {
    wasStopped = true;
    dm->dc->saveKnownFiles();
}

void TransferManager::handleWriteFinished(File *file) {
    dm->dc->knownFiles.insert(*file);
    qDebug() << "copied " << file->absoluteFilePath << " to " << dm->newPath(file);
    nbFilesTransfered++;
    if (nbFilesToTransfer == nbFilesTransfered) {
        qDebug() << "That was the last file!";
        dm->dc->saveKnownFiles();
        emit downloadFinished();
    }
}

void TransferManager::udpateGeoTagger() {
    QString tf = dm->getTrackingFolder();

    if (tf == "" || !QDir(tf).exists()) {
        delete geotagger;
        geotagger = NULL;
    } else {
        if (geotagger == NULL) {
            geotagger = new Geotagger();
        }
        geotagger->setTrackFilesFolder(File(tf));
    }
}
