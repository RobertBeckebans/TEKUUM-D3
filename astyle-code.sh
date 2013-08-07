#!/bin/sh

# ../bin/linux-x86_64/astyle
 ./astyle.exe -v -Q --options=astyle-options.ini --exclude="libs" --recursive *.h
 ./astyle.exe -v -Q --options=astyle-options.ini --exclude="libs" --exclude="idlib/math/Simd_SSE.cpp" --exclude="game/gamesys/SysCvar.cpp" --exclude="game/gamesys/Callbacks.cpp" --exclude="sys/win32/win_cpu.cpp" --exclude="sys/win32/win_main.cpp" --exclude="sys/win32/win_shared.cpp"  --recursive *.cpp
 #--exclude="sys/android/android_main.cpp" --exclude="sys/linux/main.cpp"
 
 ./astyle.exe -v -Q --options=astyle-options.ini --recursive ../android/src/*.java
 

