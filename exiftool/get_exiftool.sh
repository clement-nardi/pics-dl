
current_dir=`pwd`
script_dir=$current_dir/`dirname $0`/
script_dir=`cd $script_dir;pwd`
echo current_dir=$current_dir
echo script_dir=$script_dir
echo 1=$1

cd $script_dir

if [ ! -f $script_dir/exiftool.exe ]; then
	#check for the latest version at http://www.sno.phy.queensu.ca/~phil/exiftool/
	zipfilename=exiftool-9.77.zip
	exifURL=http://www.sno.phy.queensu.ca/~phil/exiftool/$zipfilename
	echo Downloading exiftool.exe from $exifURL
	curl -o $script_dir/$zipfilename $exifURL
	unzip -o $script_dir/$zipfilename
	ls $script_dir
	mv $script_dir/exiftool\(-k\).exe exiftool.exe
fi
if [ ! -f $script_dir/exiftool.exe ]; then
	echo "MISSING FILE: $script_dir/exiftool.exe"
	echo "This file should be retrieved automatically by $script_dir/get_exiftool.sh ..."
	exit 1;
fi

if [ ! -f $1/exiftool.exe ]; then
	echo "Copying exiftool.exe into target directory"
	cp $script_dir/exiftool.exe $1/
fi
if [ ! -f $1/exiftool.exe ]; then
	echo "Unable to copy exiftool.exe into $1/ ..."
	exit 1;
fi

if [ ! -f $1/exiftool.exe.config ]; then
	echo "Copying exiftool.exe.config into target directory"
	cp $script_dir/exiftool.exe.config $1/
fi
if [ ! -f $1/exiftool.exe.config ]; then
	echo "Unable to copy exiftool.exe.config into $1/ ..."
	exit 1;
fi

exit 0;
