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


ExifToolPerlWrapper::ExifToolPerlWrapper(const char *includeDir) {
    if (!globalInitDone) {
        globalInitDone = true;
        PERL_SYS_INIT3(&argc,(char ***)&argv,&env);
    }
    my_perl = perl_alloc();

    PERL_SET_CONTEXT(my_perl);

    perl_construct(my_perl);
    PL_exit_flags |= PERL_EXIT_DESTRUCT_END;
    perl_parse(my_perl, xs_init, argc, argv, (char **)NULL);
    perl_run(my_perl);


    SV* includeDir_SV = get_sv("includeDir", GV_ADD);
    sv_setpv(includeDir_SV, includeDir);
    eval_pv("BEGIN { push @INC, $includeDir }", TRUE);
    eval_pv("use Image::ExifTool qw(:Public);", TRUE);
    eval_pv("use Data::Dumper;", TRUE);
    eval_pv("use IO::Scalar;", TRUE);

    eval_pv("$exifTool = new Image::ExifTool;", TRUE);

}

ExifToolPerlWrapper::~ExifToolPerlWrapper(){
    PERL_SET_CONTEXT(my_perl);
    perl_destruct(my_perl);
    perl_free(my_perl);
}


void ExifToolPerlWrapper::loadTrackFile(const char * trackFilePath) {
    PERL_SET_CONTEXT(my_perl);
    SV* trackFilePath_SV = get_sv("trackFilePath", GV_ADD);
    sv_setpv(trackFilePath_SV, trackFilePath);
    eval_pv("my @loadTrackResults = $exifTool->SetNewValue('Geotag', $trackFilePath);", TRUE);
    eval_pv("print Dumper(@loadTrackResults);", TRUE);
}

void ExifToolPerlWrapper::geotag(char *in, long in_size, char **out, long *out_size){

    PERL_SET_CONTEXT(my_perl);

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
void ExifToolPerlWrapper::exitPerl(){
    PERL_SYS_TERM();
}
