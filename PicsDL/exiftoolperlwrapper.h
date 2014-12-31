#ifndef EXIFTOOLPERLWRAPPER_H
#define EXIFTOOLPERLWRAPPER_H

typedef struct interpreter PerlInterpreter;

class ExifToolPerlWrapper
{
public:
    ExifToolPerlWrapper();
    ~ExifToolPerlWrapper();
    void loadTrackFile(char * trackFilePath);
    void geotag(char *in, int in_size, char **out, int *out_size);
    static exitPerl();
private:
    PerlInterpreter *perl_interpreter;
};

#endif // EXIFTOOLPERLWRAPPER_H
