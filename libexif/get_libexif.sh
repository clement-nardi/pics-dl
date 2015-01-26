
echo "get_libexif"
current_dir=`pwd`
script_dir=$current_dir/`dirname $0`/
script_dir=`cd $script_dir;pwd`

version=0.6.21
format=tar.gz

cd $script_dir
echo cd $script_dir

if [ -f /usr/local/lib/libexif.a -o -f $script_dir/libexif-$version/libexif/.libs/libexif.dll ] ; then
	echo "libexif is already installed"
	if [ ! "$1" = "-f"  ] ; then
		exit 0;
	fi
fi

if command -v make ; then
	make=make
else
	#Note: for the moment this script will not work under windows with the MSYS bundled in git
	#because it does not support parentheses in test: if [ ! ( -f toto -o -f tata ) ] ; 
	#and some of the generated shell scripts used by libexif contains such conditions
	make=mingw32-make
fi

if [ ! -f libexif-$version.$format ] ; then
	curl http://freefr.dl.sourceforge.net/project/libexif/libexif/$version/libexif-$version.$format -o libexif-$version.$format
	tar -xzvf libexif-$version.$format
fi

cd libexif-$version
./configure
$make
$make install
sudo $make install
