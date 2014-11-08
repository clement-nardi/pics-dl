#include "devicemanager.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QSettings>
#include <QCoreApplication>
#include <QMessageBox>
#include "fsutils.h"

static QString getDriveSerial(QString path);


DeviceManager::DeviceManager(DeviceConfig *deviceConfig, QObject *parent) :
    QObject(parent)
{
    dc = deviceConfig;
}

void DeviceManager::treatDrive(QString drivePath){
    QString serial = getDriveSerial(drivePath);
    qDebug() << "treating drive " << drivePath << serial;

    if (dc->conf[serial].isUndefined() || dc->conf[serial].isNull() ) {
        qDebug() << "first time";
        QJsonObject driveConfig;
        QMessageBox mb(QMessageBox::Question,
                       "New device detected!",
                       "Would you like to manage this device with " + QCoreApplication::applicationName() + "?",
                       QMessageBox::Yes | QMessageBox::No);
        int answer = mb.exec();
        qDebug() << "answer is " << answer;

        driveConfig.insert("isManaged",QJsonValue(answer == QMessageBox::Yes));
        dc->conf.insert(serial,driveConfig);
        dc->saveConfig();
        newID(serial);
    } else {
        qDebug() << "known drive";
    }

    if (dc->conf[serial].toObject()["isManaged"].toBool() == true) {
        bool isAlreadyOpened = false;
        QSetIterator<DeviceConfigView *> i(openedViews);
        while (i.hasNext()) {
            DeviceConfigView *ldcv = i.next();
            if (ldcv->id == serial) {
                isAlreadyOpened = true;
                break;
            }
        }

        if (!isAlreadyOpened) {
            qDebug() << "treat this drive now !";
            QJsonObject obj = dc->conf[serial].toObject();
            if (drivePath.startsWith("WPD:/")) {
                /* TODO: retrieve display name of the device */
                obj["path"] = drivePath.split("/").at(2);
                obj["IDPath"] = "WPD:/" + drivePath.split("/").at(1);
            } else {
                obj["path"] = drivePath;
            }
            dc->conf[serial] = obj;
            dc->saveConfig();


            DeviceConfigView *dcv = new DeviceConfigView(dc,serial);
            openedViews.insert(dcv);
            connect(dcv,SIGNAL(destroyed(QObject*)),this,SLOT(handleDestroyed(QObject*)));
            //treatDir(drivePath);
        } else {
            qDebug() << "This drive is already being treated.";
        }
    } else {
        qDebug() << "leave this drive alone...";
    }

    return;
}

void DeviceManager::handleDestroyed(QObject *object) {
    openedViews.remove((DeviceConfigView *)object);
}


#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#else // linux stuff

#endif // _WIN32


static QString getDriveSerial(QString path) {

#ifdef _WIN32

    if (path.startsWith("WPD:/")) {
        return path.mid(5);
    } else {
        // adapted from http://www.developpez.net/forums/d523835/c-cpp/cpp/utilisation-getvolumeinformation/

        TCHAR volName[MAX_PATH+1];
        DWORD volSerial;
        DWORD nameLen;
        DWORD volFlags;
        TCHAR volFS[MAX_PATH+1];

        QDir::setCurrent( path );

        GetVolumeInformation(NULL, volName, MAX_PATH+1, &volSerial, &nameLen, &volFlags, volFS, MAX_PATH+1);

        qDebug() << "Serial=" << volSerial
                 << " Name="  << QString((QChar*)volName)
                 << " FS="    << QString((QChar*)volFS)
                 << " for "   << path;
        return QString("%1;%2;%3").arg(volSerial).arg(QString((QChar*)volName)).arg(QString((QChar*)volFS)) ;
    }

#else
    //for linux implementation: http://lists.trolltech.com/qt-interest/2004-11/msg01098.html

    struct hd_driveid id;

    int fd = open("/dev/hda", O_RDONLY|O_NONBLOCK);

    if (fd < 0)
    {
        perror("/dev/hda");
    }

    if(!ioctl(fd, HDIO_GET_IDENTITY, &id)) {
        hardwareID = id.serial_no;
    }

#endif

}
