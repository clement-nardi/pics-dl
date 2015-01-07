#ifndef EXIFTOOLPERLWRAPPER_H
#define EXIFTOOLPERLWRAPPER_H

typedef struct interpreter PerlInterpreter;

class ExifToolPerlWrapper
{
public:
    ExifToolPerlWrapper(const char *);
    ~ExifToolPerlWrapper();
    void loadTrackFile(const char *trackFilePath);
    void geotag(char *in, long in_size, char **out, long *out_size);
    static void exitPerl();
private:
    PerlInterpreter *my_perl;
};

#endif // EXIFTOOLPERLWRAPPER_H
