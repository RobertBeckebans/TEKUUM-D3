
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
		"bv/Frustum.cpp", "bv/Frustum.h",
		"bv/Sphere.cpp", "bv/Sphere.h",
		
		"containers/*.cpp", "containers/*.h",
		"geometry/*.cpp", "geometry/*.h",
		"hashing/*.cpp", "hashing/*.h",
		
		"math/*.cpp", "math/*.h",
	}
	excludes
	{
		"math/Simd_AltiVec.cpp", "math/Simd_AltiVec.h",
		--"bv/Frustum_gcc.cpp",
	}
	includedirs
	{
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
	configuration "lightmaps"
		defines
		{
			"USE_LIGHTMAPS",
		}
	
	-- 
	-- Project Configurations
	-- 
	configuration "vs*"
		flags       { "MFC" }
		buildoptions
		{
			--"/MT"
		}
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
		
		
	configuration { "linux" }
		files
		{
			"bv/Frustum_gcc.cpp",
		}
		
	
		
		