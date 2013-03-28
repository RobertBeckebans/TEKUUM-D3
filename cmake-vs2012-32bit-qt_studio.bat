cd ..
del /s /q build
mkdir build
cd build
cmake -G "Visual Studio 11" -DUSE_QT_TOOLS=ON -DUSE_QT_WINDOWING=ON -DMONOLITH=ON ../neo
pause