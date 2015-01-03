current_dir=`pwd`
script_dir=$current_dir/`dirname $0`/
script_dir=`cd $script_dir;pwd`

cd $script_dir

bn=`cat build_number.txt`
bn=$((bn + 1))
echo $bn > build_number.txt

sed "s/__BUILD_NUMBER__/$bn/g" PicsDL.rc.template > PicsDL.rc

