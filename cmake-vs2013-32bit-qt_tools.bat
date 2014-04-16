cd ..
del /s /q build
mkdir build
cd build
cmake -G "Visual Studio 12" -DUSE_QT_TOOLS=ON -DUSE_QT_WINDOWING=ON -DSTANDALONE=ON ../neo
pause