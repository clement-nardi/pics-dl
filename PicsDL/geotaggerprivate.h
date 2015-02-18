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
