astyle.exe -v -Q --options=astyle-options.ini --exclude="libs" --recursive *.h
astyle.exe -v -Q --options=astyle-options.ini --exclude="libs" --exclude="idlib/math/Simd_SSE.cpp" --exclude="game/gamesys/SysCvar.cpp" --exclude="game/gamesys/Callbacks.cpp" --exclude="sys/win32/win_cpu.cpp" --exclude="sys/win32/win_main.cpp"  --recursive *.cpp

astyle.exe -v -Q --options=astyle-options.ini --recursive ../android/src/*.java
REM style.exe -v --options=astyle-options.ini --recursive ../android/jni/*.h
astyle.exe -v -Q --options=astyle-options.ini --recursive ../android/jni/*.c

astyle.exe -v -Q --options=astyle-options.ini ../base/scriptsharp/weapon_base.cs
astyle.exe -v -Q --options=astyle-options.ini ../base/scriptsharp/weapon_bfg.cs
astyle.exe -v -Q --options=astyle-options.ini ../base/scriptsharp/map_marscity1.cs

REM astyle.exe -v -Q --options=astyle-options.ini ../assets/renderprogs/custom/raymarch_road_to_hell.pixel
REM astyle.exe -v -Q --options=astyle-options.ini ../assets/renderprogs/custom/voronoi_digital.pixel

pause