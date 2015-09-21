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
/*
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
*/
#endif // OPENPATH_H
