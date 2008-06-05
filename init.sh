mkdir -p build
cd build

mkdir -p debug
cd debug
cmake -D CMAKE_BUILD_TYPE:STRING=Debug ../..
cd ..

mkdir -p release
cd release
cmake ../..
cd ..

mkdir -p win32
cd win32
cmake -DCMAKE_TOOLCHAIN_FILE=../../mingw.cmake ../..

echo "To compile the project simply change to folder build/debug or build/release and type make."

