REM cd ../Bin/Win32
REM ARTISTIC_STYLE_OPTIONS=
REM OPTIONS="--style=allman --indent=tab=4"

REM AStyle.exe --options=indent-options.ini game\Game.cpp
REM AStyle.exe --options=indent-options.ini Game/Game.h
REM AStyle.exe --options=indent-options.ini --recursive *.cpp
REM AStyle.exe --options=indent-options.ini --exclude="cm framework game libs renderer sound sys tools ui" --recursive *.h

astyle.exe -v --options=astyle-options.ini --exclude="libs" --recursive *.h
astyle.exe -v --options=astyle-options.ini --exclude="libs" --exclude="idlib/math/Simd_3DNow.cpp" --exclude="idlib/math/Simd_AltiVec.cpp" --exclude="idlib/math/Simd_MMX.cpp" --exclude="idlib/math/Simd_SSE.cpp" --exclude="idlib/math/Simd_SSE2.cpp" --exclude="idlib/math/Simd_SSE3.cpp" --recursive *.cpp
pause