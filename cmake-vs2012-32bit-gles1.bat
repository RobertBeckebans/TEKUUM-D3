cd ..
del /s /q build
mkdir build
cd build
cmake -G "Visual Studio 11" -DSTANDALONE=OFF -DGLES1=ON ../src
pause