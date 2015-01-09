
current_dir=`pwd`
script_dir=$current_dir/`dirname $0`/
script_dir=`cd $script_dir;pwd`


rcfile=$script_dir/../PicsDL/PicsDL.rc
major=`grep MAJOR_VERSION $rcfile | cut -d " " -f3`
minor=`grep MINOR_VERSION $rcfile | cut -d " " -f3`
patch=`grep PATCH_VERSION $rcfile | cut -d " " -f3`

version=$major.$minor.$patch
workingDir=Build-debian-package

git archive --format tar.gz --prefix pics-dl-$version/ -o $workingDir/pics-dl_$version.orig.tar.gz HEAD
