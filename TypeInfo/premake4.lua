
project "TypeInfo"
	targetname  "TypeInfo"
	language    "C++"
	kind        "ConsoleApp"
	flags       { "ExtraWarnings", "NoManifest" }
	files
	{
		"*.cpp", "*.h",
	
		"../framework/CmdSystem.cpp", "../framework/CmdSystem.h",
		"../framework/CVarSystem.cpp", "../framework/CVarSystem.h",
		"../framework/File.cpp", "../framework/File.h",
		"../framework/FileSystem.cpp", "../framework/FileSystem.h",
		"../framework/Licensee.h",
		"../framework/Unzip.cpp", "../framework/Unzip.h",
	}
	includedirs
	{
		"../idlib",
	}
	defines
	{ 
		"__DOOM__",
		"__DOOM_DLL__",
		"ID_ENABLE_CURL=0",
		"ID_TYPEINFO",
		"USE_EXCEPTIONS",
	}
	links
	{
		"idlib",
	}
	
	--
	-- Platform Configurations
	-- 	
		
	--
	-- Options Configurations
	--
	configuration "debug-memory"
		defines
		{
			"ID_DEBUG_MEMORY",
			"ID_REDIRECT_NEWDELETE",
		}
	
	-- 
	-- Project Configurations
	-- 
	configuration "vs*"
		flags       { "MFC" }
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
		
		
	-- configuration { "vs*", "Debug" }
		-- links
		-- { 
			-- "libcmtd",
		-- }
		
	configuration { "vs*", "x32" }
		targetdir 	"../../bin/win32"
		
	configuration { "vs*", "x64" }
		targetdir 	"../../bin/win64"
		
	
	configuration "linux"
		targetname  "typeinfo"
			
	configuration { "linux", "x32" }
		targetdir 	"../bin/linux-x86"
		
	configuration { "linux", "x64" }
		targetdir 	"../bin/linux-x86_64"

	
		
		