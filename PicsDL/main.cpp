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
#include <QFile>
#include <QDateTime>

static QFile * debugFile;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString prefix;
    switch (type) {
    case QtDebugMsg:
        prefix = "Debug   ";
        break;
    case QtWarningMsg:
        prefix = "Warning ";
        break;
    case QtCriticalMsg:
        prefix = "Critical";
        break;
    case QtFatalMsg:
        prefix = "Fatal   ";
        break;
    }
    debugFile->write(QString("%1: %2 (%3:%4, %5)\n")
                     .arg(prefix)
                     .arg(msg)
                     .arg(context.file)
                     .arg(context.line)
                     .arg(context.function)
                     .toStdString().c_str());
    debugFile->flush();
}

const char *perlIncludeDir;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    perlIncludeDir = QString(QCoreApplication::applicationDirPath() + "/perl").toStdString().c_str();
    if (argc > 1 && QString(argv[1]) == "-v") {
        debugFile = new QFile(QCoreApplication::applicationDirPath() + "/debug" + QDateTime::currentDateTime().toString("_yyyy-MM-dd_hh-mm-ss") + ".txt");
        debugFile->open(QIODevice::WriteOnly);
        qInstallMessageHandler(myMessageOutput);
        qDebug() << "Verbose Mode ON";
    }

    if (0) {
        Geotagger *gt = new Geotagger(new File("./track"));
        gt->geotag(new QBuffer(new QByteArray("toto_data")),new QFile("toto"));
    }


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

    DeviceConfig *dc = new DeviceConfig();
    MainWindow w(dc);
    DeviceManager *dm = new DeviceManager(dc);
    DriveNotify *dn = new DriveNotify();

    im->connect(im, SIGNAL(applicationLaunched()),&w, SLOT(applicationLaunched()));

    dm->connect(dn,SIGNAL(driveAdded(QString)), dm, SLOT(treatDrive(QString)));
    dm->connect(dm, SIGNAL(newID(QString)), &w, SLOT(insertDrive(QString)));
    //w.show();

    int res = a.exec();
    delete im;
    delete dn;
    delete dm;
    delete dc;
    if (argc > 1 && argv[1] == "-v") {
        debugFile->close();
    }
    return res;
}
