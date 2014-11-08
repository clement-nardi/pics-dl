#ifndef OPENPATH_H
#define OPENPATH_H

#include <QObject>
#include "o1.h"
#include <QDateTime>

static const QString openPathsURL = "https://openpaths.cc/api/1";

class OpenPath : public QObject
{
    Q_OBJECT
public:
    explicit OpenPath(QString accessKey, QString secretKey, QObject *parent = 0);

    QString getDirectoryForDateRange(QDateTime from, QDateTime to);

private:
    O1 o1;
    QString accessKey;
    QString secretKey;

signals:

public slots:

};

#endif // OPENPATH_H
