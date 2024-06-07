#!/bin/bash

echo $0

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

echo current_dir=$current_dir
echo SCRIPT_DIR=$SCRIPT_DIR
echo 1=$1

cd $SCRIPT_DIR

missingPerlPackage=0;

if perl -MImage::ExifTool -e 1 2>/dev/null; then 
	echo "INFO: the perl module Image::ExifTool is properly installed"; 
else 
	echo "ERROR: the perl module Image::ExifTool is not installed"; 
	missingPerlPackage=1;
fi

if perl -MIO::Scalar -e 1 2>/dev/null; then 
	echo "INFO: the perl module IO::Scalar is properly installed"; 
else 
	echo "ERROR: the perl module IO::Scalar is not installed"; 
	missingPerlPackage=1;
fi

if [ "$missingPerlPackage" = "1" ]; then
	echo "Please launch these comands in a terminal:";
        echo "";
	echo "wget -O - http://cpanmin.us | perl - --self-upgrade --sudo";
	echo "cpanm Image::ExifTool --sudo";
	echo "cpanm IO::Scalar --sudo";
	echo “”;
	echo “if you don’t have wget, you can try with curl:”
        echo "curl -L https://cpanmin.us | perl - --sudo App::cpanminus";
	exit 1;
fi


if [ ! -f $1/exiftool.exe.config ]; then
	echo "Copying exiftool.exe.config into target directory"
	cp $SCRIPT_DIR/exiftool.exe.config $1/
fi
if [ ! -f $1/exiftool.exe.config ]; then
	echo "Unable to copy exiftool.exe.config into $1/ ..."
	exit 1;
fi

exit 0;
