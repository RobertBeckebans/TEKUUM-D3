cd ..
del /s /q build
mkdir build
cd build
cmake -G "Visual Studio 12" -DUSE_MFC_TOOLS=ON -DSTANDALONE=ON ../neo
pause