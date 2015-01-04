#include "drivenotify_unix.h"

#include <QFile>
#include <QStringList>
#include <QDebug>

static QString expandAsciiChars(QString str);

DriveNotify_unix::DriveNotify_unix(QObject *parent)
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

void DriveNotify_unix::reloadMountPoints(bool firstTime) {
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

void DriveNotify_unix::run() {
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
