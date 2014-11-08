#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include "drivenotify.h"
#include "devicemanager.h"
#include "deviceconfig.h"
#include <QDir>

QString ExifToolPath;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);

    QCoreApplication::setOrganizationName("PicsDL");
    QCoreApplication::setApplicationName("PicsDL");
    QCoreApplication::setApplicationVersion("0.1");
    QCoreApplication::setOrganizationDomain("picsdl.com");

    qDebug() << QDir::current().absolutePath();
    ExifToolPath = QDir::current().absolutePath() + "/exiftool.exe";

    DeviceConfig *dc = new DeviceConfig();
    MainWindow w(dc);
    DeviceManager *dm = new DeviceManager(dc);
    DriveNotify *dn = new DriveNotify();

    dm->connect(dn,SIGNAL(driveAdded(QString)), dm, SLOT(treatDrive(QString)));
    dm->connect(dm, SIGNAL(newID(QString)), &w, SLOT(insertDrive(QString)));
    //w.show();

    return a.exec();
}
