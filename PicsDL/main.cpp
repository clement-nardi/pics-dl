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

#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include "drivenotify.h"
#include "devicemanager.h"
#include "deviceconfig.h"
#include <QDir>
#include "instancemanager.h"
#include <iostream>
#include <geotagger.h>

QString ExifToolPath;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);

    QCoreApplication::setOrganizationName("PicsDL");
    QCoreApplication::setApplicationName("PicsDL");
    QCoreApplication::setApplicationVersion("0.1");
    QCoreApplication::setOrganizationDomain("picsdl.com");

    InstanceManager *im = new InstanceManager();
    if (!im->isFirstInstance) {
        qDebug() << "Already running";
        exit(0);
        return 0;
    }

    qDebug() << QDir::current().absolutePath();
    qDebug() << QCoreApplication::applicationDirPath();
    ExifToolPath = QCoreApplication::applicationDirPath() + "/exiftool.exe";

    DeviceConfig *dc = new DeviceConfig();
    MainWindow w(dc);
    DeviceManager *dm = new DeviceManager(dc);
    DriveNotify *dn = new DriveNotify();

    im->connect(im, SIGNAL(applicationLaunched()),&w, SLOT(applicationLaunched()));

    dm->connect(dn,SIGNAL(driveAdded(QString)), dm, SLOT(treatDrive(QString)));
    dm->connect(dm, SIGNAL(newID(QString)), &w, SLOT(insertDrive(QString)));
    //w.show();
    if (!QFile(ExifToolPath).exists()) {
        w.sysTray.showMessage("PicsDL may not work properly",
                              "A file is missing: " + ExifToolPath + "\n" + "You can retrieve it from here: http://www.sno.phy.queensu.ca/~phil/exiftool/",
                              QSystemTrayIcon::Warning);
    }

    int res = a.exec();
    delete im;
    delete dn;
    delete dm;
    delete dc;
    return res;
}
