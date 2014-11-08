#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <deviceconfig.h>
#include "deviceconfigview.h"

class DeviceManager : public QObject
{
    Q_OBJECT
public:
    explicit DeviceManager(DeviceConfig *deviceConfig, QObject *parent = 0);

private:
    DeviceConfig *dc;
    QSet<DeviceConfigView *> openedViews;

signals:
    void newID(QString);

public slots:
    void treatDrive(QString);
    void handleDestroyed(QObject*);
};

#endif // DEVICEMANAGER_H
