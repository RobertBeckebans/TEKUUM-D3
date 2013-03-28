cd ..
del /s /q build
mkdir build
cd build
cmake -G "Visual Studio 10" -DCMAKE_USE_RELATIVE_PATHS=ON ../neo
pause