#include "exiftoolperlwrapper.h"

#include "EXTERN.h"
#include "perl.h"

EXTERN_C void boot_DynaLoader (pTHX_ CV* cv);
EXTERN_C void
xs_init(pTHX)
{
    char *file = __FILE__;
    /* DynaLoader is a special case */
    newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, file);
}


static int argc = 3;
static char* argv[3] = {"perl", "-e" , "0"};
static char** env = NULL;
static bool globalInitDone = false;


ExifToolPerlWrapper::ExifToolPerlWrapper() {
    if (!globalInitDone) {
        globalInitDone = true;
        PERL_SYS_INIT3(&argc,(char ***)&argv,&env);
    }
    perl_interpreter = perl_alloc();

    PERL_SET_CONTEXT(perl_interpreter);

    perl_construct(my_perl);
    PL_exit_flags |= PERL_EXIT_DESTRUCT_END;
    perl_parse(my_perl, xs_init, argc, argv, (char **)NULL);
    perl_run(my_perl);

    eval_pv("use Image::ExifTool qw(:Public);", TRUE);
    eval_pv("use Data::Dumper;", TRUE);
    eval_pv("use IO::Scalar;", TRUE);

    eval_pv("$exifTool = new Image::ExifTool;", TRUE);

    eval_pv("$exifTool->SetNewValue('Geotag', 'E:/Documents/My Dropbox/Apps/GPSLogger for Android/20141213.gpx');", TRUE);
}

~ExifToolPerlWrapper(){

}


void ExifToolPerlWrapper::loadTrackFile(char * trackFilePath) {
    SV* trackFilePath_SV = get_sv("trackFilePath", GV_ADD);
    sv_setpv(trackFilePath_SV, trackFilePath);
    eval_pv("$exifTool->SetNewValue('Geotag', $trackFilePath);", TRUE);
}

void ExifToolPerlWrapper::geotag(char *in, int in_size, char **out, int *out_size){

    PERL_SET_CONTEXT(perl_interpreter);

    SV* source = get_sv("sourceFileData", GV_ADD);
    sv_setpvn(source, in, in_size);

    eval_pv("my $destContent = '';  #init buffer", TRUE);
    eval_pv("open $sourceHandle, \"<\", \\$sourceFileData;", TRUE);
    eval_pv("open $destHandle, \">\", \\$destContent;", TRUE);

    eval_pv("$exifTool->ExtractInfo($sourceHandle);", TRUE);
    eval_pv("print 'DateTimeOriginal ' . Dumper($exifTool->GetValue('DateTimeOriginal', 'PrintConv'));", TRUE);
    eval_pv("print 'GPSLatitude ' . Dumper($exifTool->GetValue('GPSLatitude'));", TRUE);

    eval_pv("$exifTool->SetNewValue('Geotime', $exifTool->GetValue('DateTimeOriginal'));", TRUE);

    eval_pv("$exifTool->WriteInfo($sourceHandle, $destHandle);", TRUE);

    STRLEN len;
    char * res = SvPV(get_sv("destContent", 0), len);

    /* TODO: check if SvPV allocates memory for res */
    *out_size = len;
    *out = res;
}

/**
 * @brief exitPerl
 * exits Perl properly. To be called only once when closing application.
 */
static ExifToolPerlWrapper::exitPerl(){
    PERL_SYS_TERM();
}
