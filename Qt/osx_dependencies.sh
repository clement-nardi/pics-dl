

curl -L https://cpanmin.us | perl - --sudo App::cpanminus
cpanm Image::ExifTool --sudo

cd ../libexif
./get_libexif.sh


exit 0

#the below doesn’t work, see https://bugreports.qt.io/browse/QTBUG-44925

version=5.4.1

curl http://ftp.fau.de/qtproject/archive/qt/5.4/$version/single/qt-everywhere-opensource-src-$version.tar.xz -o qt-everywhere-opensource-src-$version.tar.xz
tar -xf qt-everywhere-opensource-src-$version.tar.xz
cd qt-everywhere-opensource-src-$version
./configure -static -opensource -confirm-license
make module-qtscript
make
sudo make install
sudo ln -s /usr/local/Qt-$version/ ~/Qt-$version

