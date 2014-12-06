
current_dir=`pwd`
script_dir=$current_dir/`dirname $0`/
script_dir=`cd $script_dir;pwd`

cd $script_dir

ls

vcred=vcredist_2013_x86.exe

if [ ! -f $vcred ] ; then
	curl -o $vcred http://download.microsoft.com/download/2/E/6/2E61CFA4-993B-4DD4-91DA-3737CD5CD6E3/vcredist_x86.exe
fi

if [ ! -f $vcred ] ; then
	echo MISSING FILE: $script_dir/$vcred
	echo "This file should be retrieved automatically by $script_dir/build-installer.sh ..."
	exit 1;
fi

echo "C:\Program Files (x86)\Inno Setup 5\ISCC.exe" "/dMyAppPath=$1" "/dMyAppName=$2" "$script_dir/PicsDL-installer-script.iss"

ISCC="C:\Program Files (x86)\Inno Setup 5\ISCC.exe"
if [ ! -f "$ISCC" ]; then
	echo "ISCC.exe not found in C:\\Program Files (x86)\\Inno Setup 5\\"
	echo "trying C:\\Program Files\\Inno Setup 5\\"
	ISCC="C:\Program Files\Inno Setup 5\ISCC.exe"
fi

"$ISCC" "-dMyAppPath=$1" "-dMyAppName=$2" "$script_dir/PicsDL-installer-script.iss"
	
	
	