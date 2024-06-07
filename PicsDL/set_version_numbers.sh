#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $SCRIPT_DIR

bn=`cat build_number.txt`
bn=$((bn + 1))
echo $bn > build_number.txt

echo "Setting version numbers to $1.$2.$3-$bn"

sed -e "s/__MAJOR_VERSION__/$1/g;s/__MINOR_VERSION__/$2/g;s/__PATCH_NUMBER__/$3/g;s/__BUILD_NUMBER__/$bn/g" PicsDL.rc.template > PicsDL.rc
sed -e "s/__MAJOR_VERSION__/$1/g;s/__MINOR_VERSION__/$2/g;s/__PATCH_NUMBER__/$3/g;s/__BUILD_NUMBER__/$bn/g" globals.cpp.template > globals.cpp
