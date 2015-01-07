use Image::ExifTool qw(:Public);
use Data::Dumper;
use IO::Scalar;
$exifTool = new Image::ExifTool;
my @loadTrackResults = $exifTool->SetNewValue('Geotag', 'dummy_track_file');

my $destContent = '';  #init buffer
open $sourceHandle, "<", \$sourceFileData;
open $destHandle, ">", \$destContent;

$exifTool->ExtractInfo($sourceHandle);
print 'DateTimeOriginal ' . Dumper($exifTool->GetValue('DateTimeOriginal', 'PrintConv'));
print 'GPSLatitude ' . Dumper($exifTool->GetValue('GPSLatitude'));

$exifTool->SetNewValue('Geotime', $exifTool->GetValue('DateTimeOriginal'));

$exifTool->WriteInfo($sourceHandle, $destHandle);
