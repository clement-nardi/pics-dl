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
    void geotag(File *file, QString outName);

private:
    void init();
    ExifToolPerlWrapper * exiftool;
    File trackFilesFolder;
    QList<File> loadedtrackFiles;

signals:
    void writeFinished(File*);

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
    bool geotag(File *in, QString out);

};

#endif // GEOTAGGERPRIVATE_H
