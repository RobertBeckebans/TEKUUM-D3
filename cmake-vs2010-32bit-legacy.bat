cd ..
del /s /q build
mkdir build
cd build
cmake -G "Visual Studio 10" -DSTANDALONE=OFF ../src
pause