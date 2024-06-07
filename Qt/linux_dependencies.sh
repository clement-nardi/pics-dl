sudo apt install -y git qt6-base-dev qt6-tools-dev-tools qmake6 qt6-positioning-dev qtcreator  curl g++ libexif-dev libperl-dev libudev-dev debhelper cdbs devscripts build-essential geany
wget -O - http://cpanmin.us | perl - --self-upgrade --sudo
cpanm Image::ExifTool --sudo
cpanm IO::Scalar --sudo

sudo apt install -y libfontconfig1-dev libfreetype6-dev libx11-dev libxext-dev libxfixes-dev  libxcb1-dev libxcb-glx0-dev libdbus-1-dev xz-utils
sudo apt install -y '^libxcb.*-dev' libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev libxkbcommon-dev libxkbcommon-x11-dev
sudo apt install libx11-* libx11* libxcb-* libxcb* libxkbcommon-dev libxkbcommon-x11-dev libxkb* libsm-dev libpthread* libglib2.0-dev*
sudo apt install -y clang libclang-18-dev libmd4c-dev libmd4c-html0-dev

exit 0

QTVER=6.6.3
QTVERMINOR=6.6

curl http://ftp.fau.de/qtproject/archive/qt/$QTVERMINOR/$QTVER/single/qt-everywhere-src-$QTVER.tar.xz -o qt-everywhere-src-$QTVER.tar.xz
tar -xf qt-everywhere-src-$QTVER.tar.xz
cd qt-everywhere-src-$QTVER

# Note: this doesn't work, xcb is reset, some dependency must be missing
./configure -static -xcb -opensource -confirm-license
time cmake --build . --parallel
sudo cmake --install .
