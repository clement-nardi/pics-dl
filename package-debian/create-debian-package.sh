
if git diff --quiet; then
	current_dir=`pwd`
	script_dir=$current_dir/`dirname $0`/
	script_dir=`cd $script_dir;pwd`


	rcfile=$script_dir/../PicsDL/PicsDL.rc
	major=`grep MAJOR_VERSION $rcfile | cut -d " " -f3`
	minor=`grep MINOR_VERSION $rcfile | cut -d " " -f3`
	patch=`grep PATCH_VERSION $rcfile | cut -d " " -f3`

	version=$major.$minor.$patch
	workingDir=$script_dir/Build-debian-package
	packageName=pics-dl
	upstreamTarballName=$packageName_$version.orig.tar.gz

	rm -rf $workingDir
	mkdir $workingDir
	git archive --format tar.gz --prefix $packageName-$version/ -o $workingDir/$upstreamTarballName HEAD
	cd $workingDir
	tar -zxvf $upstreamTarballName
	cd $packageName-$version
	debuild -us -uc


else
	echo "You need to commit you latest changes before creating the debian package"
fi

exit 0;
