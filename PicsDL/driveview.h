#ifndef DRIVEVIEW_H
#define DRIVEVIEW_H

#include <QObject>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include "deviceconfig.h"

class DriveView : public QObject
{
    Q_OBJECT
public:
    explicit DriveView(QString id, DeviceConfig *dc, QObject *parent = 0);
    DeviceConfig *dc;
    QString id;
    QLabel *label;
    QCheckBox *managedBox;
    QPushButton *removeButton;
    QPushButton *editButton;
    QPushButton *launchButton;
    QLabel *driveIcon;

private:
    void changeState(bool enabled);

signals:
    void changed();
public slots:
    void removed();
    void edit();
    void managed(bool);
    void loadName();

};

#endif // DRIVEVIEW_H
