#!/bin/bash

mingw=0
clingo=0
iclingo=0
kdev=0

options=""
while [[ $# > 0 ]]; do
	case $1 in
		"--debug")
			debug=1
			;;
		"--iclingo")
			options=" -D WITH_CLASP:BOOL=ON -D WITH_ICLASP:BOOL=ON"
			clasp=1
			iclingo=1
			;;
		"--clingo")
			options=" -D WITH_CLASP:BOOL=ON -D WITH_ICLASP:BOOL=OFF"
			clingo=1
			;;
		"--kdev") 
			kdev=1
			;;
		"--mingw") 
			mingw=1 
			;;
		"--help")
			echo "$0 [options]"
			echo
			echo "--help    : show this help"
			echo "--mingw   : crosscompile for windows"
			echo "            Note: u may have to change the file \"mingw.cmake\""
			echo "--clingo  : enable build-in clasp version"
			echo "--iclingo : enable incremental clasp interface "
			echo "--debug   : also create debug builds"
			exit 0
			;;
		*)
			echo "error: unknown option $1"
			exit 1
	esac
	shift
done

function prepare()
{
	mkdir -p $1
	cd $1

	mkdir -p release
	cd release
	cmake $2 ../../..
	cd ..
	if [[ $debug == 1 ]]; then
		mkdir -p debug
		cd debug
		cmake -D CMAKE_BUILD_TYPE:STRING=Debug $2 ../../..
		cd ..
	fi
	if [[ $kdev == 1 ]]; then
		mkdir -p kdevelop
		cd kdevelop
		cmake -G KDevelop3 $2 ../../..
		cd ..
	fi
	if [[ $mingw == 1 ]]; then
		mkdir -p win32
		cd win32
		cmake -D CMAKE_TOOLCHAIN_FILE=../../../mingw.cmake $2 ../../..
		cd ..
	fi

	cd ..
}

mkdir -p build
cd build

prepare gringo "-D WITH_CLASP:BOOL=OFF -D WITH_ICLASP:BOOL=OFF"
if [[ $clingo == 1 ]]; then
	prepare clingo "-D WITH_CLASP:BOOL=ON -D WITH_ICLASP:BOOL=OFF"
fi
if [[ $iclingo == 1 ]]; then
	prepare iclingo "-D WITH_CLASP:BOOL=ON -D WITH_ICLASP:BOOL=ON"
fi

echo
echo 
echo "To compile the project simply change to folder build/{gringo,clingo,iclingo}/{debug,release,win32} and type \"make\"."
echo "Note: You can always change the cmake options by modifying the file \"CMakeCache.txt\"."
echo 
if [[ $iclingo == 1 ]]; then
	echo "incremental clasp interface: yes"
else
	echo "incremental clasp interface: no (enable with --iclingo)"
fi
if [[ $clingo == 1 ]]; then
	echo "internal clasp support: yes"
else
	echo "internal clasp support: no (enable with --clingo)"
fi

