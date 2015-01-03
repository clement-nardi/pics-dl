#ifndef DRIVENOTIFY_UDEV_H
#define DRIVENOTIFY_UDEV_H

#include <QThread>

class DriveNotify_udev : public QThread
{
    Q_OBJECT
public:
    explicit DriveNotify_udev(QObject *parent = 0);
    void run();

signals:
    void driveAdded(QString);

public slots:

};

#endif // DRIVENOTIFY_UDEV_H
