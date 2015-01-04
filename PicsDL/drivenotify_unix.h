#ifndef DRIVENOTIFY_UDEV_H
#define DRIVENOTIFY_UDEV_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QFileSystemWatcher>
#include <QThread>

class DriveNotify_unix : public QThread
{
    Q_OBJECT
public:
    explicit DriveNotify_unix(QObject *parent = 0);

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

#endif // DRIVENOTIFY_UDEV_H
