#ifndef GEOTAG_H
#define GEOTAG_H

#include "exiftoolperlwrapper.h"

class Geotagger
{
public:
    Geotagger(char *trackFilesFolder);
    void geotag(char *in, char *out);

private:
    ExifToolPerlWrapper * exiftool;
};

#endif // GEOTAG_H
