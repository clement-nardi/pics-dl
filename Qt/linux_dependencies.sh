sudo apt-get install -y git qt5-default qtcreator qtscript5-dev curl g++ libexif-dev libperl-dev libudev-dev debhelper cdbs devscripts build-essential geany qttools5-dev-tools
wget -O - http://cpanmin.us | perl - --self-upgrade --sudo
cpanm Image::ExifTool --sudo
cpanm IO::Scalar --sudo

sudo apt-get install -y libfontconfig1-dev libfreetype6-dev libx11-dev libxext-dev libxfixes-dev libxi-dev libxrender-dev libxcb1-dev libx11-xcb-dev libxcb-glx0-dev libdbus-1-dev xz-utils

curl http://ftp.fau.de/qtproject/archive/qt/5.5/5.5.0/single/qt-everywhere-opensource-src-5.5.0.tar.xz -o qt-everywhere-opensource-src-5.5.0.tar.xz
tar -xf qt-everywhere-opensource-src-5.5.0.tar.xz
cd qt-everywhere-opensource-src-5.5.0
./configure -static -qt-xcb -opensource -confirm-license
make
sudo make install
