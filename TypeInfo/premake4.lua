
project "TypeInfo"
	targetname  "TypeInfo"
	language    "C++"
	kind        "ConsoleApp"
	flags       { "ExtraWarnings" }
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
		
	configuration { "vs*", "Release" }
		buildoptions
		{
			-- Produces a program database (PDB) that contains type information and symbolic debugging information for use with the debugger
			-- /Zi does imply /debug
			"/Zi",
			
			-- turn off Whole Program Optimization
			"/GL-",
			
			-- Inline Function Expansion: Any Suitable (/Ob2)
			"/Ob2",
			
			-- enable Intrinsic Functions
			"/Oi",
			
			-- Omit Frame Pointers
			"/Oy",
		}
		-- linkoptions
		-- {
			-- -- turn off Whole Program Optimization
			-- "/LTCG-",
		-- }
		-- links
		-- { 
			-- "libcmt",
		-- }
		
	configuration { "vs*", "x32" }
		targetdir 	"../../bin/win32"
		
	configuration { "vs*", "x64" }
		targetdir 	"../../bin/win64"

	configuration { "linux", "gmake" }
		buildoptions
		{
			--"`pkg-config --cflags x11`",
			--"`pkg-config --cflags xext`",
			--"`pkg-config --cflags xxf86dga`",
			--"`pkg-config --cflags xxf86vm`",
			"`pkg-config --cflags sdl`",
			"`pkg-config --cflags libcurl`",
		}
		linkoptions
		{
			--"`pkg-config --libs x11`",
			--"`pkg-config --libs xext`",
			--"`pkg-config --libs xxf86dga`",
			--"`pkg-config --libs xxf86vm`",
			"`pkg-config --libs sdl`",
			"`pkg-config --libs libcurl`",
		}
		links
		{ 
			--"libcurl",
			"openal",
		}
	
	configuration "linux"
		targetname  "xreal"
		files
		{
			"sys/sys_main.c",
			"sys/sys_unix.c",
			"sys/con_log.c",
			"sys/con_passive.c",
			"sys/sdl_gamma.c",
			"sys/sdl_glimp.c",
			"sys/sdl_input.c",
			"sys/sdl_snd.c",
			"../libs/glew/src/glew.c",
		}
		--buildoptions
		--{
		--	"-pthread"
		--}
		links
		{
			"GL",
		}
		defines
		{
            "PNG_NO_ASSEMBLER_CODE",
		}
		
		