#include "openpath.h"

OpenPath::OpenPath(QString accessKey_, QString secretKey_, QObject *parent) :
    QObject(parent) {
    accessKey = accessKey_;
    secretKey = secretKey_;

}

QString OpenPath::getDirectoryForDateRange(QDateTime from, QDateTime to) {

}
