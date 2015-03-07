#ifdef _WIN32

#include "wpdiodevice.h"
#include "WPDInterface.h"
#include <QDebug>
#include <QMutex>

WPDIODevice::WPDIODevice(QString IDPath_)
{
    IDPath = IDPath_;
    at_end = false;
}

WPDIODevice::~WPDIODevice()
{

}

static QMutex WPDMutex;

bool WPDIODevice::open(QIODevice::OpenMode mode){
    WPDMutex.lock();
    qDebug() << QString("got Lock! - %1").arg(IDPath);
    if (mode != QIODevice::ReadOnly) {
        qWarning() << "WARNING - WPDIODevice can only be opened for reading!!";
    }
    QString deviceID = IDPath.split("/")[1];
    QString objectID = IDPath.split("/").last();
    WCHAR * deviceID_i = (WCHAR*) malloc(sizeof(WCHAR)*(deviceID.size()+1));
    WCHAR * objectID_i = (WCHAR*) malloc(sizeof(WCHAR)*(objectID.size()+1));
    deviceID.toWCharArray(deviceID_i);
    deviceID_i[deviceID.size()] = L'\0';
    objectID.toWCharArray(objectID_i);
    objectID_i[objectID.size()] = L'\0';
    DWORD optimalTransferSize;
    bool initDone = WPDI_InitTransfer(deviceID_i,objectID_i,&optimalTransferSize);
    //qDebug() << "optimalTransferSize=" << optimalTransferSize;
    free(deviceID_i);
    free(objectID_i);
    if (initDone) {
        return QIODevice::open(mode);
    } else {
        return false;
    }
}

void WPDIODevice::close(){
    WPDI_CloseTransfer();
    QIODevice::close();
    WPDMutex.unlock();
}

qint64 WPDIODevice::readData(char * data, qint64 maxSize) {
    if (maxSize > 0) {
        DWORD read = 0;
        WPDI_readNextData((unsigned char *)data,maxSize,&read);
        //qDebug() << QString("maxSize=%1 - read=%2").arg(maxSize).arg(read);
        if (read == 0) {
            return -1;
        }
        if (read < maxSize) {
            at_end = true;
        }
        return read;
    } else {
        return 0;
    }
}

bool WPDIODevice::atEnd() const {
    return at_end;
}

qint64 WPDIODevice::writeData(const char * data, qint64 maxSize){
    qWarning() << "WARNING: WPDIODevice does not support writing!!";
    return -1;
}

#endif
