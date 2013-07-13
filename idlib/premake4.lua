
project "idlib"
	targetname  "idlib"
	language    "C++"
	kind        "StaticLib"
	flags       { "ExtraWarnings" }
	files
	{
		--"**.cpp", "**.h",
		"*.cpp", "*.h",
				
		"bv/Bounds.cpp", "bv/Bounds.h",
		"bv/Box.cpp", "bv/Box.h",
		"bv/Sphere.cpp", "bv/Sphere.h",
		
		"containers/*.cpp", "containers/*.h",
		"geometry/*.cpp", "geometry/*.h",
		"hashing/*.cpp", "hashing/*.h",
		
		"math/*.cpp", "math/*.h",
		
		"sys/*.cpp", "sys/*.h",
	}
	excludes
	{
		--"math/Simd_AltiVec.cpp", "math/Simd_AltiVec.h",
		--"bv/Frustum_gcc.cpp",
	}
	includedirs
	{
		"."
		--"idlib",
	}
	defines
	{
		"__IDLIB__",
		"__DOOM_DLL__",
		"USE_EXCEPTIONS",
	}
	
	--
	-- Platform Configurations
	-- 	
		
	--
	-- Options Configurations
	--
	configuration "standalone"
		defines
		{
			"STANDALONE",
		}
	
	configuration "debug-memory"
		defines
		{
			"ID_DEBUG_MEMORY",
			"ID_REDIRECT_NEWDELETE",
		}
		
	configuration { "mfc-tools", "vs*" }
		flags       { "MFC" }
	
	-- 
	-- Project Configurations
	-- 
	configuration "vs*"
		defines
		{
			"WIN32",
			"_WINDOWS",
			"_CRT_SECURE_NO_DEPRECATE",
			"_CRT_NONSTDC_NO_DEPRECATE",
			--"_CRT_SECURE_NO_WARNINGS",
			"_USE_32BIT_TIME_T",
			"_MBCS",
		}
		--pchheader "precompiled.h"
		--pchsource "precompiled.cpp"
		
		
	configuration { "linux" }
		files
		{
			"sys/posix/posix_thread.cpp",
		}
			
	if not _OPTIONS["android"] then
		defines
		{
			"ID_PC"
		}
	end
		
	
		
		