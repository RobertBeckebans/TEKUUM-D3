REM ..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --exclude="libs" --recursive *.h
REM ..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --exclude="libs" --exclude="idlib/math/Simd_3DNow.cpp" --exclude="idlib/math/Simd_AltiVec.cpp" --exclude="idlib/math/Simd_MMX.cpp" --exclude="idlib/math/Simd_SSE.cpp" --exclude="idlib/math/Simd_SSE2.cpp" --exclude="idlib/math/Simd_SSE3.cpp" --exclude="sys/win32/win_cpu.cpp" --exclude="sys/win32/win_main.cpp" --exclude="sys/win32/win_shared.cpp" --exclude="sys/android/android_main.cpp" --exclude="sys/linux/main.cpp" --recursive *.cpp

..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive cm/*.h
..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive cm/*.cpp

..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive framework/*.h
..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive framework/*.cpp

..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive game/*.h
..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --exclude="game/gamesys/SysCvar.cpp" --exclude="game/gamesys/Callbacks.cpp" --recursive game/*.cpp

..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive idlib/*.h
..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --exclude="idlib/math/Simd_3DNow.cpp" --exclude="idlib/math/Simd_AltiVec.cpp" --exclude="idlib/math/Simd_MMX.cpp" --exclude="idlib/math/Simd_SSE.cpp" --exclude="idlib/math/Simd_SSE2.cpp" --exclude="idlib/math/Simd_SSE3.cpp" --recursive idlib/*.cpp

..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive renderer/*.h
..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive renderer/*.cpp

REM ..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive sound/*.h
REM ..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive sound/*.cpp

..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive sys/qt/*.h
..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive sys/qt/*.cpp

..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive tools/*.h
..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive tools/*.cpp

..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive ui/*.h
..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive ui/*.cpp

..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive libs/lua/src/*.h
..\bin\win32\UniversalIndentGUI\indenters\astyle.exe -v --options=astyle-options.ini --recursive libs/lua/src/*.c

pause