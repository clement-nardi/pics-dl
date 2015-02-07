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
    void geotag(File *in, QString out);

private:
    GeotaggerPrivate *p;

signals:
    void writeFinished(File *);
};

#endif // GEOTAG_H
