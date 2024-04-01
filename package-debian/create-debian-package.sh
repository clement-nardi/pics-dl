
#if git diff --quiet; then
if echo; then
	current_dir=`pwd`
	script_dir=$current_dir/`dirname $0`/
	script_dir=`cd $script_dir;pwd`


	rcfile=$script_dir/../PicsDL/PicsDL.rc
	major=`grep MAJOR_VERSION $rcfile | cut -d " " -f3`
	minor=`grep MINOR_VERSION $rcfile | cut -d " " -f3`
	patch=`grep PATCH_NUMBER $rcfile | cut -d " " -f3`

	version=$major.$minor.$patch
	workingDir=$script_dir/build-debian-package
	packageName=pics-dl
	upstreamTarballName=$packageName"_"$version.orig.tar.gz

	rm -rf $workingDir
	mkdir $workingDir
	cd $script_dir/..
	git archive --format tar.gz --prefix $packageName-$version/ -o $workingDir/$upstreamTarballName HEAD
	cd $workingDir
	tar -zxvf $upstreamTarballName
	cd $packageName-$version
	
	mkdir -p ~/.config/qtchooser
	#cp -f $script_dir/qt5.5.0-static.conf ~/.config/qtchooser/
	
	#This is just to make sure the conf file is properly set
	#QT_SELECT should also be set in the "debian/rules" file
	qtchooser -install QT6.4.2 /bin/qmake6
	export QT_SELECT=QT6.4.2
	qtchooser -print-env
	
	debuild -us -uc

else
	echo "You need to commit you latest changes before creating the debian package"
fi

exit 0;
