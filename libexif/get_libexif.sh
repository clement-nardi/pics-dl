
echo "get_libexif"
current_dir=`pwd`
script_dir=$current_dir/`dirname $0`/
script_dir=`cd $script_dir;pwd`

version=0.6.21
format=tar.gz

cd $script_dir
echo cd $script_dir

if [ ! -f /usr/local/lib/libexif.a -o "$1" = "-f" ] ; then
    if [ ! -f libexif-$version.$format ] ; then
        curl http://freefr.dl.sourceforge.net/project/libexif/libexif/$version/libexif-$version.$format -o libexif-$version.$format
    fi
    tar -xzvf libexif-$version.$format
    cd libexif-$version
    ./configure
    make
	make install
    sudo make install
else
	echo "libexif is already installed"
fi
