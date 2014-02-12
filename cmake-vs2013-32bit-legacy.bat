cd ..
del /s /q build
mkdir build
cd build
cmake -G "Visual Studio 12" -DSTANDALONE=OFF ../neo
pause