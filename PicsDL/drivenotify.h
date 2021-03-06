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

#ifndef DRIVENOTIFY_H
#define DRIVENOTIFY_H


#include <QObject>
#include <QStorageInfo>
#include <QString>
#include <QTimer>
#include <QMap>
class Config;

//moc doesn't recognize _WIN32
#if defined(_WIN32) || defined(WIN32)
#include <shlobj.h>
#include <shlwapi.h>
#include <QWidget>
#define DRIVENOTIFY_PARENT_TYPE QWidget
#else
#include <QFileSystemWatcher>
#include <QThread>
#define DRIVENOTIFY_PARENT_TYPE QThread
#endif

class DriveNotify : public DRIVENOTIFY_PARENT_TYPE
{
    Q_OBJECT
public:
    explicit DriveNotify(Config *dc_, DRIVENOTIFY_PARENT_TYPE *parent = 0);
    ~DriveNotify();
    /**
     * @brief getDeviceInfo
     * @param[in] serial
     * @return true if the device is currently plugged-in
     */
    bool isPluggedIn(QString serial);

private:
    QList<QStorageInfo> mountPoints;
    QMap<QString,QStorageInfo> serialMap;
    QString getSerial(QStorageInfo si);
    void reBuildSerialMap();
    QTimer tryAgainTimer;
    int nbTries;
    Config *dc;
    void updateConfigWithDeviceInfo(QStorageInfo si);
signals:
    void driveAdded(QString serial);

private slots:
    void reloadMountPoints(bool firstTime = false);

#if defined(_WIN32) || defined(WIN32)
protected:
    bool nativeEvent(const QByteArray & eventType, void * message, long * result);
private:
    static const unsigned int msgShellChange = WM_USER + 1;
    unsigned long id;
#else
private slots:
    void handleMountFileChange();

private:
    QFileSystemWatcher syswatch;
    QString fileToWatch;
    void run();
    void watch();

#endif
};



#endif // DRIVENOTIFY_H
