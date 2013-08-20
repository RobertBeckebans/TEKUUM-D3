cd ..
del /s /q build
mkdir build
cd build
cmake -G "Visual Studio 11" -DSTANDALONE=OFF -DGLES2=ON ../neo
pause