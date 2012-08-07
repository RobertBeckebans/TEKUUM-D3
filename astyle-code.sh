#!/bin/sh

# ../bin/linux-x86_64/astyle
 ./astyle -v --options=astyle-options.ini --exclude="libs" --recursive *.h
 ./astyle -v --options=astyle-options.ini --exclude="libs" --exclude="idlib/math/Simd_3DNow.cpp" --exclude="idlib/math/Simd_AltiVec.cpp" --exclude="idlib/math/Simd_MMX.cpp" --exclude="idlib/math/Simd_SSE.cpp" --exclude="idlib/math/Simd_SSE2.cpp" --exclude="idlib/math/Simd_SSE3.cpp" --exclude="game/gamesys/SysCvar.cpp" --exclude="game/gamesys/Callbacks.cpp" --exclude="sys/win32/win_cpu.cpp" --exclude="sys/win32/win_main.cpp" --exclude="sys/win32/win_shared.cpp"  --recursive *.cpp
 #--exclude="sys/android/android_main.cpp" --exclude="sys/linux/main.cpp"
 
 ./astyle -v --options=astyle-options.ini --recursive ../android/src/*.java
 

