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
