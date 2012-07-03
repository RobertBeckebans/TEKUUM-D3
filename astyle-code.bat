REM ..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --exclude="libs" --recursive *.h
REM ..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --exclude="libs" --exclude="idlib/math/Simd_3DNow.cpp" --exclude="idlib/math/Simd_AltiVec.cpp" --exclude="idlib/math/Simd_MMX.cpp" --exclude="idlib/math/Simd_SSE.cpp" --exclude="idlib/math/Simd_SSE2.cpp" --exclude="idlib/math/Simd_SSE3.cpp" --exclude="sys/win32/win_cpu.cpp" --exclude="sys/win32/win_main.cpp" --exclude="sys/win32/win_shared.cpp" --exclude="sys/android/android_main.cpp" --exclude="sys/linux/main.cpp" --recursive *.cpp

..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive ui/*.h
..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive ui/*.cpp

REM ..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive game/*.h
REM ..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --exclude="game/gamesys/SysCvar.cpp" --exclude="game/gamesys/Callbacks.cpp" --recursive game/*.cpp

..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive tools/*.h
..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive tools/*.cpp

..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive sys/qt/*.h
..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive sys/qt/*.cpp
pause