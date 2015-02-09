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
    qDebug() << "emit setTrackFilesFolder from thread " << QThread::currentThreadId();
    emit p->setTrackFilesFolder(trackFilesFolder);
}

void Geotagger::geotag(File *in, QString out) {
    qDebug() << "emit geotag from thread " << QThread::currentThreadId();
    emit p->geotag(in,out);
}

void Geotagger::getGeotags(File *file) {
    emit p->getGeotags(file);
}
