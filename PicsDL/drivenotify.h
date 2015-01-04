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

#ifndef DRIVENOTIFY_H
#define DRIVENOTIFY_H

#if defined(_WIN32) || defined(WIN32)

#include <shlobj.h>
#include <shlwapi.h>
#include <QWidget>
#include <QObject>

class DriveNotify : public QWidget
{
    Q_OBJECT
public:
    explicit DriveNotify(QWidget *parent = 0);
    ~DriveNotify();
protected:
    bool nativeEvent(const QByteArray & eventType, void * message, long * result);
private:
    static const unsigned int msgShellChange = WM_USER + 1;
    unsigned long id;
    static QString getPidlPath(ITEMIDLIST* pidl);
signals:
    void driveAdded(QString);
};

#else

#include <QObject>
#include <QMap>
#include <QString>
#include <QFileSystemWatcher>
#include <QThread>

class DriveNotify : public QThread
{
    Q_OBJECT
public:
    explicit DriveNotify(QObject *parent = 0);

signals:
    void driveAdded(QString);

public slots:
    void reloadMountPoints(bool firstTime = false);

private:
    QMap<QString,QString> mountPoints;
    QFileSystemWatcher syswatch;
    QString fileToWatch;
    bool useQFileSystemWatcher;
    void run();
};
#endif


#endif // DRIVENOTIFY_H
