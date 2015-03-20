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

#ifndef GEOTAGGERPRIVATE_H
#define GEOTAGGERPRIVATE_H

#include <QObject>
#include "file.h"
#include <QList>
#include <QThread>
class ExifToolPerlWrapper;

class GeotaggerWorker: public QObject {
    Q_OBJECT
public:
    GeotaggerWorker();
    ~GeotaggerWorker();

public slots:
    void setTrackFilesFolder(File trackFilesFolder);
    void geotag(File *file);
    void getGeotags(File *file);

private:
    void init();
    ExifToolPerlWrapper * exiftool;
    File trackFilesFolder;
    QList<File> loadedtrackFiles;

private slots:
    void handleWriteFinished(File* file);

signals:
    void writeFinished(File*);
    void getGeotagsFinished(File*);

};


class GeotaggerPrivate: public QObject {
    Q_OBJECT

public:
    GeotaggerPrivate();
    ~GeotaggerPrivate();
    GeotaggerWorker * gw;
    QThread thread;

signals:
    void setTrackFilesFolder(File trackFilesFolder_);
    bool geotag(File *in);
    void getGeotags(File *);

};

#endif // GEOTAGGERPRIVATE_H
