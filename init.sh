mingw=0
clasp=0
options=""
while [[ $# > 0 ]]; do
	case $1 in
		"--clasp")
			options=" -D WITH_CLASP:BOOL=ON"
			clasp=1
			;;
		"--mingw") 
			mingw=1 
			;;
		"--help")
			echo "$0 [options]"
			echo
			echo "--help :  show this help"
			echo "--mingw : crosscompile for windows"
			echo "          Note: u may have to change the file \"mingw.cmake\""
			echo "--clasp : enable build-in clasp version" 
			exit 0
			;;
		*)
			echo "error: unknown option $1"
			exit 1
	esac
	shift
done

mkdir -p build
cd build

mkdir -p debug
cd debug
cmake -D CMAKE_BUILD_TYPE:STRING=Debug $options ../..
cd ..

mkdir -p release
cd release
cmake $options ../..
cd ..

if [[ $mingw == 1 ]]; then
	mkdir -p win32
	cd win32
	cmake $options -D CMAKE_TOOLCHAIN_FILE=../../mingw.cmake ../..
	cd ..
fi

echo
echo 
echo "To compile the project simply change to folder build/{debug,release,win32} and type \"make\"."
echo "Note: You can always change the cmake options by modifying the file \"CMakeCache.txt\"."
echo 
if [[ $clasp == 1 ]]; then
	echo "internal clasp support: yes"
else
	echo "internal clasp support: no (enable with --clasp)"
fi

