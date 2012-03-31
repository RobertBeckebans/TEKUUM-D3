
project "game"
	targetname  "game"
	targetdir 	"../../basety"
	language    "C++"
	kind        "SharedLib"
	flags       { "ExtraWarnings", "NoManifest" }
	files
	{
		"../idlib/**.cpp", "../idlib/**.h",
		"**.c", "**.cpp", "**.h",
		
		"../cm/**.h",
		"../framework/**.h",
		"../renderer/**.h",
		"../sound/**.h",
		"../tools/**.h",
		"../ui/**.h",
	}
	excludes
	{
		"../idlib/math/Simd_AltiVec.cpp", "../idlib/math/Simd_AltiVec.h",
		"../idlib/bv/Frustum_gcc.cpp",
		"gamesys/Callbacks.cpp",
		"EndLevel.cpp", "EndLevel.h",
	}
	includedirs
	{
		"../idlib",
	}
	defines
	{ 
		--"_D3SDK",
		"__DOOM__",
		"GAME_DLL",
		"USE_EXCEPTIONS",
	}
	
	--
	-- Platform Configurations
	--
	configuration { "vs*", "x32" }
		targetname  "gamex86"
		
	configuration { "vs*", "x64" }
		targetname  "gamex86_64"
	
	--
	-- Options Configurations
	--
	configuration "debug-memory"
		defines
		{
			"ID_DEBUG_MEMORY",
			"ID_REDIRECT_NEWDELETE",
		}
		
	configuration { "vs*", "x32", "debug-memory" }
		prebuildcommands
		{
		   "cd ../../bin/win32",
		   "TypeInfo.exe",
		}
	
	configuration { "vs*", "x64", "debug-memory" }
		prebuildcommands
		{
		   "cd ../../bin/win64",
		   "TypeInfo.exe",
		}
	
	configuration "lightmaps"
		defines
		{
			"USE_LIGHTMAPS",
		}
	
	-- 
	-- Project Configurations
	-- 
	configuration "vs*"
		linkoptions
		{
			"/DEF:Game.def",
		}
		defines
		{
			"WIN32",
			"_WINDOWS",
			"_CRT_SECURE_NO_WARNINGS",
			"_CRT_SECURE_NO_DEPRECATE",
			"_CRT_NONSTDC_NO_DEPRECATE",
			"_USE_32BIT_TIME_T",
			"_MBCS",
			"_WINDLL"
		}
		postbuildcommands
		{
		   "cd ../../basety",
		   "del game00.pk4",
		   "zip game00.pk4 gamex86.dll",
		   "zip game00.pk4 binary.conf",
		}
		links
		{
			"TypeInfo"
		}
	
	configuration { "linux", "x32" }
		targetname  "gamex86"
		targetprefix ""
	
	configuration { "linux", "x64" }
		targetname  "gamex86_64"
		targetprefix ""