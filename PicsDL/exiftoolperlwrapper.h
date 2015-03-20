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

#ifndef EXIFTOOLPERLWRAPPER_H
#define EXIFTOOLPERLWRAPPER_H

typedef struct interpreter PerlInterpreter;

class ExifToolPerlWrapper
{
public:
    ExifToolPerlWrapper(const char *);
    ~ExifToolPerlWrapper();
    void loadTrackFile(const char *trackFilePath);
    void loadTrackContent(const char * trackFileContent, long size);
    void geotag(char *in, long in_size, char **out, long *out_size);
    void geotagPipe(char *in, long in_size, char **out, long *out_size);
    void getGeotags(const char *geotime, int *nbTags, char keys[10][200], char values[10][200]);
    static void exitPerl();
private:
    PerlInterpreter *my_perl;
};

#endif // EXIFTOOLPERLWRAPPER_H
