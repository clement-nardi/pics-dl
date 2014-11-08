#include "drivenotify.h"
#include <QWidget>
#include <QDebug>


#include "drivenotofy_win.h"
DriveNotify::DriveNotify(QObject *parent) :
    QObject(parent)
{
    DriveNotofy_win *dnw = new DriveNotofy_win();
    connect(dnw,SIGNAL(driveAdded(QString)),this,SIGNAL(driveAdded(QString)));
}
