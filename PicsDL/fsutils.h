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
