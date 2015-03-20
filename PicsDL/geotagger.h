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

#ifndef GEOTAG_H
#define GEOTAG_H

#include <QString>
#include <QObject>

class File;
class ExifToolPerlWrapper;
class QBuffer;
class QIODevice;
class GeotaggerPrivate;

class Geotagger: public QObject
{
    Q_OBJECT
public:
    Geotagger();
    ~Geotagger();
    void setTrackFilesFolder(File trackFilesFolder);
    void geotag(File *in);
    void getGeotags(File *);

private:
    GeotaggerPrivate *p;

signals:
    void getGeotagsFinished(File *);
    void writeFinished(File *);
};

#endif // GEOTAG_H
