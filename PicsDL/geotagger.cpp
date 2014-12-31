#include "geotagger.h"

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


static PerlInterpreter *my_perl;  /***    The Perl interpreter    ***/

Geotagger::Geotagger(char * trackFilesFolder)
{
    int argc = 3;
    char* argv[3] = {"perl", "-e" , "0"};
    char** env = NULL;
    fprintf(stderr,"toto 0.2\n");fflush(stderr);
    PERL_SYS_INIT3(&argc,(char ***)&argv,&env);
    fprintf(stderr,"toto 0.3\n");fflush(stderr);
    my_perl = perl_alloc();
    fprintf(stderr,"toto 0.4\n");fflush(stderr);

    perl_construct(my_perl);
    fprintf(stderr,"toto 0.5\n");fflush(stderr);

    PL_exit_flags |= PERL_EXIT_DESTRUCT_END;
    fprintf(stderr,"toto 0.6\n");fflush(stderr);
    fprintf(stderr,"%s %s %s\n", argv[0], argv[1], argv[2]);fflush(stderr);

    perl_parse(my_perl, xs_init, argc, argv, (char **)NULL);
    fprintf(stderr,"toto 0.7\n");fflush(stderr);

    perl_run(my_perl);
    fprintf(stderr,"toto 0.8\n");fflush(stderr);


    eval_pv("$a = 3; $a **= 2", TRUE);
    eval_pv("use Image::ExifTool qw(:Public);", TRUE);
    eval_pv("use Data::Dumper;", TRUE);
    eval_pv("use IO::Scalar;", TRUE);

    eval_pv("$exifTool = new Image::ExifTool;", TRUE);

    fprintf(stderr,"toto 0.8.1\n");fflush(stderr);

    FILE *sourceHandle = fopen("E:/Documents/Projets/PicsDL/exiftool/test/toto.jpg","rb");
    char *sourceContent = (char*)malloc(10000000); // 10Mo
    int sourceFileSize = fread(sourceContent,1,10000000, sourceHandle);

    fprintf(stderr,"sourceFileSize=%d\n", sourceFileSize);fflush(stderr);
    fwrite(sourceContent, sizeof(char), 100, stdout);
    fclose(sourceHandle);

    SV* source = get_sv("sourceFileData", GV_ADD);
    sv_setpvn(source, sourceContent, sourceFileSize);

    eval_pv("my $destContent = '';  #init buffer", TRUE);
    eval_pv("open $sourceHandle, \"<\", \\$sourceFileData;", TRUE);
    eval_pv("open $destHandle, \">\", \\$destContent;", TRUE);
    fprintf(stderr,"toto 0.8.2\n");fflush(stderr);

    eval_pv("$exifTool->ExtractInfo($sourceHandle);", TRUE);
    eval_pv("print 'DateTimeOriginal ' . Dumper($exifTool->GetValue('DateTimeOriginal', 'PrintConv'));", TRUE);
    eval_pv("print 'DateTimeOriginal ' . Dumper($exifTool->GetValue('DateTimeOriginal', 'ValueConv'));", TRUE);
    eval_pv("print 'DateTimeOriginal ' . Dumper($exifTool->GetValue('DateTimeOriginal', 'Raw'));", TRUE);
    eval_pv("print 'DateTimeOriginal ' . Dumper($exifTool->GetValue('DateTimeOriginal', 'Both'));", TRUE);
    eval_pv("print 'DateTimeOriginal ' . Dumper($exifTool->GetValue('DateTimeOriginal', 'Rational'));", TRUE);
    eval_pv("print 'GPSLatitude ' . Dumper($exifTool->GetValue('GPSLatitude'));", TRUE);

    eval_pv("$exifTool->SetNewValue('Geotag', 'E:/Documents/My Dropbox/Apps/GPSLogger for Android/20141213.gpx');", TRUE);
    eval_pv("$exifTool->SetNewValue('Geotag', 'E:/Documents/My Dropbox/Apps/GPSLogger for Android/20141210.gpx');", TRUE);
    eval_pv("$exifTool->SetNewValue('Geotime', $exifTool->GetValue('DateTimeOriginal'));", TRUE);
    eval_pv("$exifTool->SetNewValue('Make','Nardi Corp.');", TRUE);

    eval_pv("$exifTool->WriteInfo($sourceHandle, $destHandle);", TRUE);

    fprintf(stderr,"toto 0.8.3\n");fflush(stderr);fflush(stdout);

    //eval_pv("", TRUE);

    eval_pv("open $destReader, \"<\", \\$destContent;", TRUE);
    eval_pv("$info2 = ImageInfo($destReader);", TRUE);
    fprintf(stderr,"toto 0.8.4\n");fflush(stderr);fflush(stdout);
    eval_pv("$make1 = $exifTool->GetValue('Make');", TRUE);
    eval_pv("$make2 = $$info2{Make};", TRUE);
    printf("make1 = %s\n", SvPV_nolen(get_sv("make1", 0)));
    printf("make2 = %s\n", SvPV_nolen(get_sv("make2", 0)));

    //eval_pv("print \"Hello, world!\n\";", TRUE);

    eval_pv("print 7 . Dumper($$info2{GPSLatitude});", TRUE);
    //eval_pv("print Dumper($destContent);", TRUE);

    STRLEN len;
    char * res = SvPV(get_sv("destContent", 0), len);

    fwrite(res, sizeof(char), 100, stdout);
    printf("len = %d\n",len);


    FILE *outputFile = fopen("E:/Documents/Projets/PicsDL/exiftool/test/tata.jpg","wb");
    fwrite(res, sizeof(char), len, outputFile);
    fclose(outputFile);


    //eval_pv("print Dumper($info);", TRUE);
    //eval_pv("print Dumper($info2);", TRUE);

    perl_destruct(my_perl);
    fprintf(stderr,"toto 0.9\n");fflush(stderr);

    fflush(stdout);
    perl_free(my_perl);
    fprintf(stderr,"toto 0.10\n");fflush(stderr);

    PERL_SYS_TERM();
    fprintf(stderr,"toto 0.11\n");fflush(stderr);
}

void Geotagger::geotag(char *in, char *out) {

}
