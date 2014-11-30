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

#include "fsutils.h"
#include <QStringList>
#include <QDir>
#include <WPDInterface.h>
#include <QDebug>
#include <utime.h>
#include <QElapsedTimer>

FSUtils::FSUtils()
{
}

QList<FileInfo> FSUtils::ls(FileInfo dirInfo, bool *theresMore) {
    QList<FileInfo> res;
    *theresMore = false;
    if (dirInfo.IDPath.startsWith("WPD:/")) {
        if (dirInfo.absoluteFilePath.count("/") > 5) return res;
        QString deviceID = dirInfo.IDPath.split("/")[1];
        WCHAR * deviceID_i = (WCHAR*) malloc(sizeof(WCHAR)*(deviceID.size()+1));
        QString objectID;
        if (dirInfo.IDPath.count("/") == 1) {
            /* this is the root */
            objectID = "DEVICE";
        } else {
            objectID = dirInfo.IDPath.split("/").last();
        }
        WCHAR * objectID_i = (WCHAR*) malloc(sizeof(WCHAR)*(objectID.size()+1));
        deviceID.toWCharArray(deviceID_i);
        deviceID_i[deviceID.size()] = L'\0';
        objectID.toWCharArray(objectID_i);
        objectID_i[objectID.size()] = L'\0';
        int count;
        WPDFileInfo WPDList[MAX_REQUESTED_ELEMENTS];
        /*qDebug() << "Calling WPDI_LS(" << QString::fromWCharArray(deviceID_i)
                 << "," << QString::fromWCharArray(objectID_i);*/
        qDebug() << "listing content of" << dirInfo.absoluteFilePath;
        //qDebug() << "             path=" << dirInfo.IDPath;
        WPDI_LS(deviceID_i,objectID_i,WPDList,&count);
        qDebug() << "returned" << count << "elements";
        for (int i = 0; i<count; i++) {
            //qDebug() << "appending element " << i << QString::fromWCharArray(WPDList[i].id) << QString::fromWCharArray(WPDList[i].name);
            //qDebug() << QString::fromWCharArray(WPDList[i].date);
            res.append(FileInfo(QDateTime::fromString(QString::fromWCharArray(WPDList[i].date),
                                                      "yyyy/MM/dd:HH:mm:ss.zzz").toTime_t(),
                                dirInfo.absoluteFilePath + "/" + QString::fromWCharArray(WPDList[i].name),
                                WPDList[i].size,
                                WPDList[i].isDir,
                                dirInfo.IDPath + "/" + QString::fromWCharArray(WPDList[i].id)));
            WPDI_Free(WPDList[i].id);
            WPDI_Free(WPDList[i].name);
            WPDI_Free(WPDList[i].date);
        }
        if (count >= MAX_REQUESTED_ELEMENTS) {
            *theresMore = true;
        }
        free(deviceID_i);
        free(objectID_i);
    } else {
        QDir dir = QDir(dirInfo.absoluteFilePath);
        dir.setFilter(dir.filter()|QDir::NoDotAndDotDot|QDir::System);
        for (int i = 0; i<dir.entryInfoList().size(); i++) {
            res.append(FileInfo(dir.entryInfoList().at(i)));
        }
    }
    return res;
}

bool FSUtils::copyWithDirs(FileInfo from, QString to){
    if (from.IDPath.startsWith("WPD:/")) {
        QElapsedTimer timer;
        qint64 initTime;
        qint64 transferTime = 0;
        qint64 closeTime;
        QDir().mkpath(QFileInfo(to).absolutePath());
        QFile toFile(to);
        if (toFile.exists()) return false;
        QString deviceID = from.IDPath.split("/")[1];
        QString objectID = from.IDPath.split("/").last();
        WCHAR * deviceID_i = (WCHAR*) malloc(sizeof(WCHAR)*(deviceID.size()+1));
        WCHAR * objectID_i = (WCHAR*) malloc(sizeof(WCHAR)*(objectID.size()+1));
        deviceID.toWCharArray(deviceID_i);
        deviceID_i[deviceID.size()] = L'\0';
        objectID.toWCharArray(objectID_i);
        objectID_i[objectID.size()] = L'\0';
        DWORD optimalTransferSize;
        DWORD read;
        qint64 written;
        qint64 totalWritten = 0;
        timer.start();
        if (!WPDI_InitTransfer(deviceID_i,objectID_i,&optimalTransferSize)) return false;
        initTime = timer.nsecsElapsed();
        /* Create the destination file only if InitTransfer worked */
        if (!toFile.open(QIODevice::WriteOnly)) return false;
        //qDebug() << "optimalTransferSize=" << optimalTransferSize;
        unsigned char * data = (unsigned char *)malloc(sizeof(unsigned char) * optimalTransferSize);
        timer.restart();
        while (WPDI_readNextData(data,optimalTransferSize,&read)) {
            transferTime += timer.nsecsElapsed();
            written = toFile.write((char *)data,read);
            if ( written == -1) {
                qWarning() << "Problem during toFile.write(data,"<<read<<")";
                break;
            }
            if ( written != read ) {
                qWarning() << "Problem during toFile.write(data,"<<read<<"), written=" << written;
            }
            totalWritten += written;
            //qDebug() << "Transfered " << totalWritten << "so far!";
            timer.restart();
        }
        free(data);
        free(deviceID_i);
        free(objectID_i);
        toFile.close();
        timer.restart();
        WPDI_CloseTransfer();
        closeTime = timer.nsecsElapsed();
        struct utimbuf times;
        times.actime = from.lastModified;
        times.modtime = from.lastModified;
        utime(to.toStdString().c_str(),&times);
        qint64 totalTime = initTime + transferTime + closeTime;
        qDebug() << QString("Init %1ms (%2\%)), Transfer %3ms (%4\%) %5@%6/s, Close %7ms (%8\%)")
                    .arg((double)initTime/1000000,0,'f',3)
                    .arg((double)initTime*100/totalTime,0,'f',2)
                    .arg((double)transferTime/1000000,0,'f',3)
                    .arg((double)transferTime*100/totalTime,0,'f',2)
                    .arg(FileInfo::size2Str(totalWritten))
                    .arg(FileInfo::size2Str((qint64)((double)totalWritten*1000000000/(double)transferTime)))
                    .arg((double)closeTime/1000000,0,'f',3)
                    .arg((double)closeTime*100/totalTime,0,'f',2) ;
        if ( written == -1) {
            return false;
        } else {
            return true;
        }
    } else {
        return copyWithDirs(from.absoluteFilePath,to);
    }
}

bool FSUtils::copyWithDirs(QString from, QString to){
    bool copied = QFile(from).copy(to);
    if (!copied) {
        QDir().mkpath(QFileInfo(to).absolutePath());
        copied = QFile(from).copy(to);
    }
    return copied;
}

bool FSUtils::moveWithDirs(QString from, QString to){
    bool moved = QFile(from).rename(to);
    if (!moved) {
        QDir().mkpath(QFileInfo(to).absolutePath());
        moved = QFile(from).rename(to);
    }
    return moved;
}

#include "windows.h"

bool FSUtils::setHidden(QString path) {
    WCHAR * wPath = (WCHAR*) malloc(sizeof(WCHAR)*(path.size()+1));
    path.replace("/","\\").toWCharArray(wPath);
    DWORD dwAttrs = GetFileAttributes(wPath);
    SetFileAttributes(wPath, dwAttrs | FILE_ATTRIBUTE_HIDDEN);
    free(wPath);
}
