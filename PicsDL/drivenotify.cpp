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

#include "drivenotify.h"


#ifdef _WIN32

#include <QLibrary>
#include <QDebug>
#include "dbt.h"
#include "shobjidl.h"
#include "WPDInterface.h"

static QChar DriveMaskToLetter(int mask);

DriveNotify::DriveNotify(QWidget *parent) :
    QWidget(parent), id(0) {

    ITEMIDLIST* pidl;

    if (!SHChangeNotifyRegister) {
        qDebug() << "Could not get SHChangeNotifyRegister entry point.";
        return;
    }
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
}

DriveNotify::~DriveNotify() {
    if (id) {
        if (!SHChangeNotifyDeregister) {
            qDebug() << "Could not get SHChangeNotifyDeregister entry point.";
            return;
        }
        SHChangeNotifyDeregister(id);
    }
}

bool DriveNotify::nativeEvent(const QByteArray & eventType, void * message, long * result) {

    MSG* msg = reinterpret_cast<MSG*> (message);

    if (msg->message == WM_DEVICECHANGE || msg->message == msgShellChange) {
        qDebug() << "DriveNotify::nativeEvent " << msg->message << msg->wParam << msg->lParam << msg->time << msg->pt.x << msg->pt.y ;
        if (msg->wParam == DBT_DEVICEARRIVAL) {
            DEV_BROADCAST_HDR *deviceInfo = reinterpret_cast<DEV_BROADCAST_HDR*> (msg->lParam);
            qDebug() << "DriveNotify::nativeEvent DBT_DEVICEARRIVAL devicetype = " << deviceInfo->dbch_devicetype ;
            if (deviceInfo->dbch_devicetype == DBT_DEVTYP_VOLUME) {
                DEV_BROADCAST_VOLUME *volumeInfo = reinterpret_cast<DEV_BROADCAST_VOLUME*> (msg->lParam);
                QChar driveLetter = DriveMaskToLetter(volumeInfo->dbcv_unitmask);
                qDebug() << "DriveNotify::nativeEvent DBT_DEVICEARRIVAL DBT_DEVTYP_VOLUME " << driveLetter << volumeInfo->dbcv_flags ;
                driveAdded(QString("%1:/").arg(driveLetter));
            }
        }
        if (msg->wParam == DBT_DEVICEREMOVECOMPLETE) {
            DEV_BROADCAST_HDR *deviceInfo = reinterpret_cast<DEV_BROADCAST_HDR*> (msg->lParam);

            qDebug() << "DriveNotify::nativeEvent DBT_DEVICEREMOVECOMPLETE devicetype = " << deviceInfo->dbch_devicetype ;
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
                driveAdded("WPD:/" + id + "/" + name);
            }
        }
    }

    if (msg->message != msgShellChange) return false;
    HANDLE lock;
    long event;
    ITEMIDLIST** items;
    if (SHChangeNotification_Lock)
        lock = SHChangeNotification_Lock((HANDLE) msg->wParam, msg->lParam, &items, &event);
    else {
        event = msg->lParam;
        items = (ITEMIDLIST**) msg->wParam;
    }
    QString n1 = getPidlPath(items[0]);
    QString n2 = getPidlPath(items[1]);
    /*
    WCHAR path[MAX_PATH];
    QString n1 = "?";
    QString n2 = "?";
    if (items[0]) {
        DWORD r1 = SHGetPathFromIDList(items[0], path);
        if (r1) {
            n1 = QString::fromWCharArray(path);
        }
    }
    if (items[1]) {
        DWORD r2 = SHGetPathFromIDList(items[1], path);
        if (r2) {
            n2 = QString::fromWCharArray(path);
        }
    }
    */

    switch (event) {
    case SHCNE_ATTRIBUTES: {
        qDebug() << QString("Got change ATTRIBUTES for %1.").arg(n1);
        break;
    }
    case SHCNE_CREATE: {
        qDebug() << QString("Got change CREATE for %1.").arg(n1);
        break;
    }
    case SHCNE_DELETE: {
        qDebug() << QString("Got change DELETE %1.").arg(n1);
        break;
    }
    case SHCNE_DRIVEADD: {
        qDebug() << QString("Got change DRIVEADD %1.").arg(n1);
        break;
    }
    case SHCNE_DRIVEADDGUI: {
        qDebug() << QString("Got change DRIVEADDGUI %1.").arg(n1);
        break;
    }
    case SHCNE_DRIVEREMOVED: {
        qDebug() << QString("Got change DRIVEREMOVED %1.").arg(n1);
        break;
    }
    case SHCNE_MEDIAINSERTED: {
        qDebug() << QString("Got change MEDIAINSERTED %1.").arg(n1);
        break;
    }
    case SHCNE_MEDIAREMOVED: {
        qDebug() << QString("Got change MEDIAREMOVED %1.").arg(n1);
        break;
    }
    case SHCNE_MKDIR: {
        qDebug() << QString("Got change MKDIR %1.").arg(n1);
        break;
    }
    case SHCNE_RENAMEFOLDER: {
        qDebug() << QString("Got change RENAMEFOLDER %1 to %2.").arg(n1).arg(n2);
        break;
    }
    case SHCNE_RENAMEITEM: {
        qDebug() << QString("Got change RENAMEITEM %1 to %2.").arg(n1).arg(n2);
        break;
    }
    case SHCNE_RMDIR: {
        qDebug() << QString("Got change RMDIR %1.").arg(n1);
        break;
    }
    case SHCNE_UPDATEDIR: {
        qDebug() << QString("Got change UPDATEDIR %1.").arg(n1);
        break;
    }
    case SHCNE_UPDATEITEM: {
        qDebug() << QString("Got change UPDATEITEM %1.").arg(n1);
        break;
    }
    default: qDebug() << "Got unrecognized change.";
    }
    if (SHChangeNotification_Lock) SHChangeNotification_Unlock(lock);
    result = 0;
    return true;


    return false;
}


QString DriveNotify::getPidlPath(ITEMIDLIST* pidl) {
    if (!pidl) return "";
    SHFILEINFO shfi;
    QString name =
       ( SHGetFileInfo((WCHAR*) pidl, 0, &shfi, sizeof(shfi), SHGFI_PIDL | SHGFI_DISPLAYNAME) )
       ? QString::fromWCharArray(shfi.szDisplayName) : "<unknown>";
    ITEMIDLIST* p = pidl;
    if (p->mkid.cb) {
        for (;;) {
            ITEMIDLIST* q = (ITEMIDLIST*) (((char*) p) + p->mkid.cb);
            if (q->mkid.cb == 0) break;
            p = q;
        }
        int n = (char*)p - (char*)pidl;
        char *s = new char[n+2];
        memcpy(s, pidl, n);
        s[n] = s[n+1] = 0;
        name = getPidlPath((ITEMIDLIST*) s) + "\\" + name;
    }
    return name;
}

static QChar DriveMaskToLetter(int mask)
{
  QChar letter;
  QString drives = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  // 1 = A
  // 2 = B
  // 4 = C...
  int cnt = 0;
  int pom = mask / 2;
  while (pom != 0)
  {
    // while there is any bit set in the mask
    // shift it to the righ...
    pom = pom / 2;
    cnt++;
  }

  if (cnt < drives.length())
    letter = drives[cnt];
  else
    letter = '?';

  return letter;
}

#else


#include <QFile>
#include <QStringList>
#include <QDebug>

static QString expandAsciiChars(QString str);

DriveNotify::DriveNotify(QObject *parent)
    :QThread(parent)
{

    fileToWatch = "/etc/mtab";

    /** For QFileSystemWatcher to work properly, I need to add the file to the watcher after each notification...
     * But inotify seems to do the same...
     * */
    useQFileSystemWatcher = true;

    reloadMountPoints(true);

    if (useQFileSystemWatcher) {
        connect(&syswatch,SIGNAL(directoryChanged(QString)),this,SLOT(reloadMountPoints()),Qt::QueuedConnection);
        connect(&syswatch,SIGNAL(fileChanged(QString))     ,this,SLOT(reloadMountPoints()),Qt::QueuedConnection);
    } else {
        start();
    }
}

void DriveNotify::reloadMountPoints(bool firstTime) {
    qDebug() << "Reloading mount points by reading /proc/mounts...";

    QMap<QString,QString> newMountList;
    QFile mountList("/proc/mounts");
    mountList.open(QIODevice::ReadOnly);
    while (1) {
        QByteArray line = mountList.readLine();
        if (line.isEmpty()) break;
        QStringList mountInfo = QString(line).split(" ");
        if (!mountPoints.contains(mountInfo.at(1))) {
            qDebug() << "New mount point: " << mountInfo.at(0) << mountInfo.at(1);
            if (!firstTime) {
                emit driveAdded(QString("%1;%2").arg(expandAsciiChars(mountInfo.at(1))).arg(mountInfo.at(0)));
            }
        }
        newMountList[mountInfo.at(1)] = mountInfo.at(0);
    }
    mountList.close();
    qDebug() << QString("Reload Mount Points. Count: %1 -> %2").arg(mountPoints.size()).arg(newMountList.size());
    mountPoints = newMountList;
    if (useQFileSystemWatcher) {
        /* sometimes the watcher stops... */
        if (syswatch.addPath(fileToWatch)) {
            qDebug() << "Now watching " << fileToWatch;
        } else {
            qDebug() << "Problem watching " << fileToWatch;
        }
    }
}

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <unistd.h>
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

void DriveNotify::run() {
    int length, i = 0;
    int fd;
    int wd;
    char buffer[EVENT_BUF_LEN];

    /*creating the INOTIFY instance*/
    fd = inotify_init();

    /*checking for error*/
    if ( fd < 0 ) {
      perror( "inotify_init" );
    }

    /*adding the “/tmp” directory into watch list. Here, the suggestion is to validate the existence of the directory before adding into monitoring list.*/
    wd = inotify_add_watch( fd, fileToWatch.toStdString().c_str(), IN_MODIFY);

    qDebug() << "inotifying " << fileToWatch;
    /*read to determine the event change happens on “/tmp” directory. Actually this read blocks until the change event occurs*/

    while (1) {
        length = read( fd, buffer, EVENT_BUF_LEN );
        qDebug() << "inotify: " << fileToWatch << " was modified";
        if (!useQFileSystemWatcher) {
            reloadMountPoints();
        }
    }

    /*removing the “/tmp” directory from the watch list.*/
     inotify_rm_watch( fd, wd );

    /*closing the INOTIFY instance*/
     close( fd );

}


QString expandAsciiChars(QString str) {
    QString res = str;
    for (int i = 0; i< str.size(); i++) {
        QChar c = str.at(i);
        if (c == '\\') {
            bool ok;
            int asciiCode = str.mid(i+1,3).toInt(&ok,8);
            res.replace(str.mid(i,4),QString(QLatin1Char(asciiCode)));
        }
    }
    return res;
}


#endif
