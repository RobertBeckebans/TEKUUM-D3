
project "game"
	targetname  "game"
	targetdir 	"../.."
	language    "C++"
	kind        "SharedLib"
	flags       { "ExtraWarnings" }
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
	}
	includedirs
	{
		"../idlib",
	}
	defines
	{ 
		"_D3SDK",
		"__DOOM__",
		"GAME_DLL"
	}
	
	--
	-- Platform Configurations
	--
	configuration "x32"
		targetname  "gamex86"
	
	configuration "x64"
		targetname  "gamex86_64"
				
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
		}
		postbuildcommands
		{
		   "cd ../..",
		   "del game00.pk4",
		   "zip game00.pk4 gamex86.dll",
		   "zip game00.pk4 binary.conf",
		}
	
	configuration { "linux", "x32" }
		--targetname  "gamex86"
		targetprefix ""
	
	configuration { "linux", "x64" }
		--targetname  "gamex86_64"
		targetprefix ""