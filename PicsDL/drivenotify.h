#ifndef DRIVENOTIFY_H
#define DRIVENOTIFY_H

#include <QObject>

class DriveNotify : public QObject
{
    Q_OBJECT
public:
    explicit DriveNotify(QObject *parent = 0);

signals:
    void driveAdded(QString);

public slots:
private slots:
};

#endif // DRIVENOTIFY_H
