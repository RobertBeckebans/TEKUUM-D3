cd ..
del /s /q build
mkdir build
cd build
cmake -G "Visual Studio 11" -DUSE_QT_TOOLS=ON -DMONOLITH=ON ../src
pause