
current_dir=`pwd`
script_dir=$current_dir/`dirname $0`/
script_dir=`cd $script_dir;pwd`

version=0.6.21

cd $script_dir

if [ ! -f /usr/local/lib/libexif.a ] ; then
    if [ ! -f libexif-$version.tar.bz2 ] ; then
        curl http://freefr.dl.sourceforge.net/project/libexif/libexif/$version/libexif-$version.tar.bz2 -o libexif-$version.tar.bz2
    fi
    tar -xzvf libexif-$version.tar.bz2
    cd libexif-$version
    ./configure
    make
    sudo make install
fi
