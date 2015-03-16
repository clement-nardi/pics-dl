
target_dir=$1
target_name=$2

current_dir=`pwd`
script_dir=$current_dir/`dirname $0`/
script_dir=`cd $script_dir;pwd`


rcfile=$script_dir/../PicsDL/PicsDL.rc
major=`grep MAJOR_VERSION $rcfile | cut -d " " -f3`
minor=`grep MINOR_VERSION $rcfile | cut -d " " -f3`
patch=`grep PATCH_NUMBER $rcfile | cut -d " " -f3`
build=`cat $script_dir/../PicsDL/build_number.txt`

version=$major.$minor.$patch.$build

echo target_dir=$target_dir
echo target_name=$target_name
echo version=$version

bundle_name=$target_name.app
bundle_path=$target_dir$bundle_name
working_dir=$script_dir/temp/

mkdir $working_dir
cp -r $bundle_path $working_dir/
app_path=$working_dir/$bundle_name/Contents/MacOS/$target_name

otool -L $app_path

mkdir $working_dir/$bundle_name/Contents/Frameworks
cp /System/Library/Perl/5.18/darwin-thread-multi-2level/CORE/libperl.dylib $working_dir/$bundle_name/Contents/Frameworks/
install_name_tool -change /System/Library/Perl/5.18/darwin-thread-multi-2level/CORE/libperl.dylib @executable_path/../Frameworks/libperl.dylib $app_path

cd $script_dir/
perl $script_dir/generate_perl_deps.pl

if [ ! -d $script_dir/perl ] ; then
    echo generate_perl_deps.pl did not execute properly.;
    echo this folder should have been created: $script_dir/perl;
    exit 1;
fi

cp -r $script_dir/perl $working_dir/$bundle_name/Contents/MacOS/

echo "launching macdeployqt"
~/Qt5.4.1/5.4/clang_64/bin/macdeployqt $working_dir/$bundle_name -dmg

if [ ! -f $working_dir/$target_name.dmg ] ; then
    echo missing file $working_dir/$target_name.dmg;
    echo "macdeployqt didn't work as expected";
    exit 1;
else
    echo "macdeployqt executed properly"
fi

otool -L $app_path

if [ ! -d output ] ; then
    mkdir output
fi
mv -f $working_dir/$target_name.dmg $script_dir/output/$target_name-$version.dmg

rm -rf $working_dir

if [ ! -f $script_dir/output/$target_name-$version.dmg ] ; then
    echo missing file $script_dir/output/$target_name-$version.dmg;
    exit 1;
else
    echo "properly created the DMG file $script_dir/output/$target_name-$version.dmg"
fi


exit 0
