#!/bin/sh
cd ..
rm -rf build
mkdir build
cd build
cmake -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_USE_RELATIVE_PATHS=ON ../src