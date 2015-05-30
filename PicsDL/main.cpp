/**
 * Copyright 2014-2015 Clément Nardi
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
#include "config.h"
#include <QDir>
#include "instancemanager.h"
#include <iostream>
#include <geotagger.h>
#include <QFile>
#include <QDateTime>
#include "globals.h"
#include <QAbstractNativeEventFilter>

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

class EventFilter: public QAbstractNativeEventFilter{
    bool nativeEventFilter(const QByteArray & eventType, void * message, long * result){
#ifdef _WIN32
        MSG* msg = reinterpret_cast<MSG*> (message);
        //qDebug() << "main thread nativeEvent " << msg->message << msg->wParam << msg->lParam << msg->time << msg->pt.x << msg->pt.y ;
        if (msg->message == WM_QUIT ||
            msg->message == WM_CLOSE || // sent here as soon as a window is closed...
            msg->message == WM_QUERYENDSESSION ||
            msg->message == WM_ENDSESSION) {
            qDebug() << "main thread nativeEvent " << msg->message << msg->wParam << msg->lParam << msg->time << msg->pt.x << msg->pt.y ;

            //QCoreApplication::exit(0);
        }
#endif
        return false;
    }
};

const char *perlIncludeDir;

int main(int argc, char *argv[])
{
    QApplication *a = new QApplication(argc, argv);
    EventFilter *ef = new EventFilter();
    a->installNativeEventFilter(ef);
    perlIncludeDir = QString(QCoreApplication::applicationDirPath() + "/perl").toStdString().c_str();
    if (argc > 1 && QString(argv[1]) == "-v") {
        debugFile = new QFile(QCoreApplication::applicationDirPath() + "/debug" + QDateTime::currentDateTime().toString("_yyyy-MM-dd_hh-mm-ss") + ".txt");
        debugFile->open(QIODevice::WriteOnly);
        qInstallMessageHandler(myMessageOutput);
        qDebug() << "Verbose Mode ON";
    }

    if (0) {
        Geotagger *gt = new Geotagger();
        gt->setTrackFilesFolder(File(QCoreApplication::applicationDirPath()+"/test/track"));
        //gt->geotag(new QBuffer(new QByteArray("toto_data")),new QFile("toto"));
        File *file = new File(QCoreApplication::applicationDirPath()+"/test/DSCF8472.JPG");
        file->loadExifData();
        //file->writeHeader();
        //file->writeContent();
        qDebug() << file->dateTakenRaw();
        gt->getGeotags(file);
        gt->geotag(file);
        QThread::sleep(10);
        exit(0);
        return 0;
    }


    a->setQuitOnLastWindowClosed(false);

    QCoreApplication::setOrganizationName("PicsDL");
    QCoreApplication::setApplicationName("PicsDL");
    QCoreApplication::setApplicationVersion(QString("%1.%2.%3").arg(major).arg(minor).arg(patch));
    QCoreApplication::setOrganizationDomain("picsdl.com");

    InstanceManager *im = new InstanceManager();
    if (!im->isFirstInstance) {
        qDebug() << "Already running";
        exit(0);
        return 0;
    }

    qDebug() << QDir::current().absolutePath();
    qDebug() << QCoreApplication::applicationDirPath();

    Config *dc = new Config();
    DriveNotify *dn = new DriveNotify(dc);
    DeviceManager *dm = new DeviceManager(dc);
    MainWindow *w = new MainWindow(dc,dn,dm);

    im->connect(im, SIGNAL(applicationLaunched()),w, SLOT(applicationLaunched()));

    dm->connect(dn,SIGNAL(driveAdded(QString)), dm, SLOT(treatDrive(QString)));

    //w.show();

    int res = a->exec();
    qDebug() << "Exiting";
    delete w;
    delete dm;
    delete dn;
    delete dc;
    delete im;
    if (argc > 1 && QString(argv[1]) == "-v") {
        debugFile->close();
    }
    delete a;
    return res;
}
/*
#include "signal.h"

void signalhandler(int sig){
    qDebug() << "signalhandler" << sig;
    if(sig==SIGINT){
        //qApp->quit();
    }
}*/
