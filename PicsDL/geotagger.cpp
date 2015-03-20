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

#include "geotagger.h"
#include "geotaggerprivate.h"
#include "file.h"
#include <QDebug>


Geotagger::Geotagger()
{
    p = new GeotaggerPrivate();
    connect(p->gw,SIGNAL(writeFinished(File*)),this,SIGNAL(writeFinished(File*)));
}

Geotagger::~Geotagger()
{
    delete p;
}


void Geotagger::setTrackFilesFolder(File trackFilesFolder) {    
    //qDebug() << "emit setTrackFilesFolder from thread " << QThread::currentThreadId();
    emit p->setTrackFilesFolder(trackFilesFolder);
}

void Geotagger::geotag(File *in) {
    //qDebug() << "emit geotag from thread " << QThread::currentThreadId();
    emit p->geotag(in);
}

void Geotagger::getGeotags(File *file) {
    emit p->getGeotags(file);
}
