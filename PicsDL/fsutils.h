#ifndef FSUTILS_H
#define FSUTILS_H
#include <QFileInfoList>
#include <QString>
#include "fileinfo.h"

class FSUtils
{
public:
    FSUtils();
    static QList<FileInfo> ls(FileInfo dirInfo, bool *theresMore) ;
    static bool copyWithDirs(FileInfo from, QString to);
    static bool copyWithDirs(QString from, QString to);
    static bool moveWithDirs(QString from, QString to);
    static bool setHidden(QString path);
};

#endif // FSUTILS_H
