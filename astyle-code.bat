astyle.exe -v --options=astyle-options.ini --exclude="libs" --recursive *.h
astyle.exe -v --options=astyle-options.ini --exclude="libs" --exclude="idlib/math/Simd_SSE.cpp" --exclude="game/gamesys/SysCvar.cpp" --exclude="game/gamesys/Callbacks.cpp" --exclude="sys/win32/win_cpu.cpp" --exclude="sys/win32/win_main.cpp"  --recursive *.cpp

astyle.exe -v --options=astyle-options.ini --recursive ../android/src/*.java
REM style.exe -v --options=astyle-options.ini --recursive ../android/jni/*.h
astyle.exe -v --options=astyle-options.ini --recursive ../android/jni/*.c

pause