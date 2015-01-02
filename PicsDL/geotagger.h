#ifndef GEOTAG_H
#define GEOTAG_H

#include <QString>

class File;
class ExifToolPerlWrapper;
class QBuffer;
class QIODevice;

class Geotagger
{
public:
    Geotagger(File *trackFilesFolder);
    bool geotag(QBuffer *in, QIODevice *out);

private:
    ExifToolPerlWrapper * exiftool;
};

#endif // GEOTAG_H
