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

#include "drivenotify.h"
#include <QDebug>
#include "file.h"
#include "config.h"

#ifdef _WIN32

#include <QLibrary>
#include "dbt.h"
#include "shobjidl.h"
#include "WPDInterface.h"

#else

#endif


DriveNotify::DriveNotify(Config *dc_, DRIVENOTIFY_PARENT_TYPE *parent) :
    DRIVENOTIFY_PARENT_TYPE(parent) {
    dc = dc_;

    tryAgainTimer.setSingleShot(true);
    tryAgainTimer.setInterval(2000);
    connect(&tryAgainTimer,SIGNAL(timeout()),this,SLOT(reloadMountPoints()));

    reloadMountPoints(true);

#ifdef _WIN32

    id = 0;
    nbTries = 0;

    ITEMIDLIST* pidl;

    /*
    if (!SHChangeNotifyRegister) {
        qDebug() << "Could not get SHChangeNotifyRegister entry point.";
        return;
    }*/
    HRESULT res = SHGetSpecialFolderLocation(0, CSIDL_DESKTOP, &pidl);

    qDebug() << "pidl=" << pidl << "res=" << ((res==S_OK)?"OK":"NOT OK");
    /*STRRET *pName;
    GetDisplayNameOf(pidl,SHGDN_NORMAL,pName);
    qDebug() << pidl->mkid.cb << pidl->mkid.abID << pName;*/
    SHChangeNotifyEntry ne = {pidl, true};
    //show();
    id = SHChangeNotifyRegister((HWND)winId(), 0x8000 ,
                                SHCNE_DRIVEADD,
                                msgShellChange, 1, &ne);
    qDebug() << "SHChangeNotifyRegister id =" << id << " msgShellChange=" << msgShellChange;


    WPDI_Init();

#else


#ifdef __APPLE__
    fileToWatch = "/Volumes/";
#else
    fileToWatch = "/etc/mtab";
#endif

    watch();
    connect(&syswatch,SIGNAL(directoryChanged(QString)),this,SLOT(handleMountFileChange()),Qt::QueuedConnection);
    connect(&syswatch,SIGNAL(fileChanged(QString))     ,this,SLOT(handleMountFileChange()),Qt::QueuedConnection);

#endif
}

DriveNotify::~DriveNotify() {

}

QString DriveNotify::getSerial(QStorageInfo si){
    return QString("%1;%2;%3").arg(si.name()).arg(QString(si.fileSystemType())).arg(si.bytesTotal());
}

void DriveNotify::reBuildSerialMap(){
    serialMap.clear();
    for (int i = 0; i< mountPoints.size(); i++) {
        QStorageInfo si = mountPoints.at(i);
        serialMap.insert(getSerial(si),si);
    }
}

void DriveNotify::updateConfigWithDeviceInfo(QStorageInfo si)
{
    QJsonObject obj = dc->devices[getSerial(si)].toObject();
    obj[CONFIG_DISPLAYNAME]     = si.displayName();
    obj[CONFIG_PATH]            = si.rootPath();
    obj[CONFIG_DEVICESIZE]      = QString("%1").arg(si.bytesTotal());
    obj[CONFIG_BYTESAVAILABLE]  = QString("%1").arg(si.bytesAvailable());
    dc->devices[getSerial(si)] = obj;
}

bool DriveNotify::isPluggedIn(QString serial){
    QStorageInfo si = serialMap.value(serial);
    return si.isValid();
}

void DriveNotify::reloadMountPoints(bool firstTime) {
    qDebug() << "Reloading mount points...";

    nbTries++;
    QList<QStorageInfo> oldList = mountPoints;
    mountPoints = QStorageInfo::mountedVolumes();
    reBuildSerialMap();
    qDebug() << "Number of mount points: " << oldList.size() << " -> " << mountPoints.size();

    /* Detect new devices */
    bool newFound = false;
    for (int i = 0; i< mountPoints.size(); i++) {
        QStorageInfo si = mountPoints.at(i);
        if (!oldList.contains(si)) {
            qDebug() << "New Mount Point: " << si.rootPath() << si.displayName();
            if (!firstTime) {
                updateConfigWithDeviceInfo(si);
                dc->deviceFieldChanged(getSerial(si));
                driveAdded(getSerial(si));
                newFound = true;
            }
        }
    }
    /* Detect unplugged devices */
    for (int i = 0; i< oldList.size(); i++) {
        QStorageInfo si = oldList.at(i);
        if (!mountPoints.contains(si)) {
            qDebug() << "Device unplugged: " << si.rootPath() << si.displayName();
            dc->deviceFieldChanged(getSerial(si));
        }
    }

    if (!newFound && oldList.size() == mountPoints.size() && nbTries < 10) {
        qDebug() << QString("Tentative #%1. Will try again in 2s.").arg(nbTries);
        tryAgainTimer.start();
    } else {
        dc->saveDevices();
    }
}


#ifdef _WIN32

bool DriveNotify::nativeEvent(const QByteArray & eventType, void * message, long * result __attribute__ ((unused))) {

    MSG* msg = reinterpret_cast<MSG*> (message);
    qDebug() << eventType << " " << msg->message;

    if (msg->message == WM_DEVICECHANGE) {
        qDebug() << "DriveNotify::nativeEvent " << msg->message << msg->wParam << msg->lParam << msg->time << msg->pt.x << msg->pt.y ;
        if (msg->wParam == DBT_DEVICEARRIVAL) {
            DEV_BROADCAST_HDR *deviceInfo = reinterpret_cast<DEV_BROADCAST_HDR*> (msg->lParam);
            qDebug() << "DriveNotify::nativeEvent DBT_DEVICEARRIVAL devicetype = " << deviceInfo->dbch_devicetype ;
            if (deviceInfo->dbch_devicetype == DBT_DEVTYP_VOLUME) {
                nbTries = 0;
                reloadMountPoints();
            }
        }
        if (msg->wParam == DBT_DEVICEREMOVECOMPLETE) {
            DEV_BROADCAST_HDR *deviceInfo = reinterpret_cast<DEV_BROADCAST_HDR*> (msg->lParam);
            qDebug() << "DriveNotify::nativeEvent DBT_DEVICEREMOVECOMPLETE devicetype = " << deviceInfo->dbch_devicetype ;
            nbTries = 0;
            reloadMountPoints();
        }
        if (msg->wParam == DBT_DEVNODES_CHANGED) {
            WCHAR * id_             = NULL;
            WCHAR * displayName_    = NULL;
            WCHAR * manufacturer_   = NULL;
            WCHAR * description_    = NULL;
            WPDI_LookForNewDevice(&id_,&displayName_,&manufacturer_,&description_);
            QString id              = QString::fromWCharArray(id_);
            QString displayName     = QString::fromWCharArray(displayName_);
            QString manufacturer    = QString::fromWCharArray(manufacturer_);
            QString description     = QString::fromWCharArray(description_);
            qDebug() << "id=" << id << "displayName=" << displayName << "manufacturer=" << manufacturer << "description=" << description;
            WPDI_Free(id_);
            WPDI_Free(displayName_);
            WPDI_Free(manufacturer_);
            WPDI_Free(description_);
            if (id.size() > 0) {
                QString name;
                if (displayName.size()>0) {
                    name = displayName;
                } else {
                    name = manufacturer + " " + description;
                }

                File fi(0,name,0,true,"WPD:/" + id);
                bool more;
                bool isStandard = false;
                QList<File> content = fi.ls(&more);
                qDebug() << "Listing content at the root of this drive: " << content.size() << " elements";
                for (int i = 0; i<content.size(); i++) {
                    qDebug() << content.at(i).absoluteFilePath();
                    if (content.at(i).absoluteFilePath().endsWith(':')) {
                        /* This is a standard drive, do not treat it as a WPD drive */
                        qDebug() << "ignoring a WPD drive because it is a standard drive";
                        isStandard = true;
                        //reloadMountPoints(); //just in case
                        break;
                    }
                }
                if (!isStandard) {
                    if (content.size() == 0) {
                        qDebug() << "ignoring a WPD drive because it is empty";
                    } else {
                        QJsonObject obj = dc->devices[id].toObject();
                        obj[CONFIG_DISPLAYNAME] = name;
                        obj[CONFIG_PATH]        = name;
                        obj[CONFIG_IDPATH]      = "WPD:/" + id;
                        dc->devices[id] = obj;
                        driveAdded(id);
                    }
                }
            }
        }
    }
    return true;
}


#else


void DriveNotify::run() {
    /** If QFileSystem doesn't work, implement here any blocking API like inotifiy
      * See previous versions of this file for an example */
}

void DriveNotify::handleMountFileChange() {
    nbTries = 0;
    tryAgainTimer.stop();
    reloadMountPoints(false);
    /* sometimes the watcher stops... */
    watch();
}

void DriveNotify::watch() {
    if (syswatch.addPath(fileToWatch)) {
        qDebug() << "Now watching " << fileToWatch;
    } else {
        qDebug() << "Problem watching " << fileToWatch << " (already under watch?)";
    }
}


#endif


