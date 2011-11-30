
project "idlib"
	targetname  "idlib"
	language    "C++"
	kind        "StaticLib"
	flags       { "ExtraWarnings" }
	files
	{
		"**.cpp", "**.h",
	}
	excludes
	{
		"math/Simd_AltiVec.cpp", "math/Simd_AltiVec.h",
		"bv/Frustum_gcc.cpp",
	}
	includedirs
	{
		--"idlib",
	}
	defines
	{ 
		"__IDLIB__",
		"__DOOM_DLL__",
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
		
	
		
		