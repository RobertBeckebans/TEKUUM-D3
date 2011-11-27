--
-- Techyon build configuration script
-- 
solution "Techyon"
	--configurations { "Release", "ReleaseWithSymbols", "Debug" }
	configurations { "Release", "Debug" }
	platforms {"x32", "x64"}
	
	--
	-- Release/Debug Configurations
	--
	configuration "Release"
		defines     "NDEBUG"
		flags      
		{
			"OptimizeSpeed",
			"EnableSSE",
			--"StaticRuntime"
		}
		
	--configuration "ReleaseReleaseWithSymbols"
	--	defines     "NDEBUG"
	--	flags
	--	{
	--		"OptimizeSpeed",
	--		"EnableSSE",
	--		"Symbols",
	--		"StaticRuntime"
	--	}
	
	configuration "Debug"
		defines     "_DEBUG"
		flags
		{
			"Symbols",
			--"StaticRuntime",
			--"NoRuntimeChecks"
		}
	
--
-- Options
--
newoption
{
	trigger = "debug-memory",
	description = "Enables memory logging to file"
}

newoption
{
	trigger = "no-lanaddress",
	description = "Don't recognize any IP as LAN address. This is useful when debugging network	code where LAN / not LAN influences application behaviour"
}

-- newoption
-- {
	-- trigger = "memcheck"
	-- description = "Perform heap consistency checking",
	-- value = "TYPE",
	-- allowed = 
	-- {
		-- { "0", "on in Debug / off in Release" },
		-- { "1", "forces on" },
		-- { "2", "forces off" },
	-- }
-- }

--newoption
--{
--	trigger = "with-freetype",
--	description = "Compile with freetype support"
--}
		
--newoption
--{
--	trigger = "with-openal",
--	value = "TYPE",
--	description = "Specify which OpenAL library",
--	allowed = 
--	{
--		{ "none", "No support for OpenAL" },
--		{ "dlopen", "Dynamically load OpenAL library if available" },
--		{ "link", "Link the OpenAL library as normal" },
--		{ "openal-dlopen", "Dynamically load OpenAL library if available" },
--		{ "openal-link", "Link the OpenAL library as normal" }
--	}
--}

--		
-- Platform specific defaults
--

-- We don't support freetype on VS platform
--if _ACTION and string.sub(_ACTION, 2) == "vs" then
--	_OPTIONS["with-freetype"] = false
--end

-- Default to dlopen version of OpenAL
--if not _OPTIONS["with-openal"] then
--	_OPTIONS["with-openal"] = "dlopen"
--end
--if _OPTIONS["with-openal"] then
--	_OPTIONS["with-openal"] = "openal-" .. _OPTIONS["with-openal"]
--end

project "Techyon"
	targetname  "Techyon"
	language    "C++"
	kind        "WindowedApp"
	flags       { "ExtraWarnings", "StaticRuntime" }
	files
	{
		"idlib/**.cpp", "idlib/**.h",
	
		"cm/*.cpp", "cm/*.h",
		"framework/**.cpp", "framework/**.h",
		"renderer/**.c", "renderer/**.cpp", "renderer/**.h",
		
		--"libs/glew/src/glew.c",
		--"libs/glew/src/glew.h",
		
		"sound/*.cpp", "sound/*.h",
		
		"sound/OggVorbis/ogg/*.h",
		"sound/OggVorbis/oggsrc/bitwise.c",
		"sound/OggVorbis/oggsrc/framing.c",
		
		"sound/OggVorbis/vorbis/*.h",
		"sound/OggVorbis/vorbissrc/mdct.c",
		"sound/OggVorbis/vorbissrc/smallft.c",
		"sound/OggVorbis/vorbissrc/block.c",
		"sound/OggVorbis/vorbissrc/envelope.c",
		--"sound/OggVorbis/vorbissrc/window.c",
		"sound/OggVorbis/vorbissrc/lsp.c",
		"sound/OggVorbis/vorbissrc/lpc.c",
		"sound/OggVorbis/vorbissrc/analysis.c",
		"sound/OggVorbis/vorbissrc/synthesis.c",
		"sound/OggVorbis/vorbissrc/psy.c",
		"sound/OggVorbis/vorbissrc/info.c",
		"sound/OggVorbis/vorbissrc/floor1.c",
		"sound/OggVorbis/vorbissrc/floor0.c",
		"sound/OggVorbis/vorbissrc/res0.c",
		"sound/OggVorbis/vorbissrc/mapping0.c",
		"sound/OggVorbis/vorbissrc/registry.c",
		"sound/OggVorbis/vorbissrc/codebook.c",
		"sound/OggVorbis/vorbissrc/sharedbook.c",
		"sound/OggVorbis/vorbissrc/lookup.c",
		"sound/OggVorbis/vorbissrc/bitrate.c",
		"sound/OggVorbis/vorbissrc/vorbisfile.c",
		
		"sys/sys_*.cpp", "sys/sys_*.h",
		
		"tools/**.c", "tools/**.cpp", "tools/**.h",
		"tools/**.CPP", "tools/**.H",
		"ui/*.cpp", "ui/*.h",
	}
	excludes
	{
		"idlib/math/Simd_AltiVec.cpp", "../idlib/math/Simd_AltiVec.h",
		"idlib/bv/Frustum_gcc.cpp",
		
		"renderer/draw_exp*.cpp",
		"renderer/jpeg-6/jmemdos.c",
		"renderer/jpeg-6/jload.c",
		"renderer/jpeg-6/jpegtran.c",
		
		"tools/common/RenderBumpFlatDialog.*",
		"tools/debugger/*",
		"tools/decl/DialogEntityDefEditor.*",
		"tools/edit_stub.cpp",
		"tools/guied/GEWindowWrapper_stub.cpp",
	}
	includedirs
	{
		--"../shared",
		--"../libs/zlib",
		--"../libs/glew/include",
		--"../libs/freetype/include",
		--"../libs/ogg/include",
		"sound/OggVorbis/ogg",
		"sound/OggVorbis/vorbis",
	}
	defines
	{
		"__DOOM__",
		"__DOOM_DLL_",
		--"STANDALONE",
		--"REF_HARD_LINKED",
		--"GLEW_STATIC",
		--"BUILD_FREETYPE",
		--"FT2_BUILD_LIBRARY",
		--"USE_CODEC_VORBIS",
		--"USE_VOIP",
		--"USE_CIN_THEORA",
		--"USE_ALLOCA",
		--"FLOATING_POINT",
		--"USE_CURL", 
		--"USE_MUMBLE",
		--"USE_INTERNAL_GLFW",
		--"USE_INTERNAL_GLEW",
	}
	
	--
	-- Platform Configurations
	-- 	
	configuration "x32"
		files
		{ 
			--"code/qcommon/vm_x86.c",
		}
	
	configuration "x64"
		--targetdir 	"../../bin64"
		files
		{ 
			--"qcommon/vm_x86_64.c",
			--"qcommon/vm_x86_64_assembler.c",
		}
		
	--
	-- Options Configurations
	--		
	
	--configuration { "vs*", "Release" }
	-- newaction {
		-- trigger = "prebuild",
		-- description = "Compile libcurl.lib",
		-- execute = function ()
			-- os.execute("cd ../libs/curl-7.12.2;cd lib;nmake /f Makefile.vc6 CFG=release")
		-- end
	-- }
	
	-- 
	-- Project Configurations
	-- 
	configuration "vs*"
		flags       { "WinMain" }
		files
		{
			"sys/win32/**.cpp", "sys/win32/**.h",
			"sys/win32/rc/doom.rc",
			"sys/win32/res/*",
			
			--"../libs/glew/src/wglew.h",
			
			-- "curl/lib/base64.c",
			-- "curl/lib/connect.c",
			-- "curl/lib/cookie.c",
			-- "curl/lib/dict.c",
			-- "curl/lib/easy.c",
			-- "curl/lib/escape.c",
			-- "curl/lib/file.c",
			-- "curl/lib/formdata.c",
			-- "curl/lib/ftp.c",
			-- "curl/lib/getdate.c",
			-- "curl/lib/getenv.c",
			-- "curl/lib/getinfo.c",
			-- "curl/lib/hash.c",
			-- "curl/lib/hostip.c",
			-- "curl/lib/http.c",
			-- "curl/lib/http_chunks.c",
			-- "curl/lib/if2ip.c",
			-- "curl/lib/krb4.c",
			-- "curl/lib/ldap.c",
			-- "curl/lib/llist.c",
			-- "curl/lib/memdebug.c",
			-- "curl/lib/mprintf.c",
			-- "curl/lib/multi.c",
			-- "curl/lib/netrc.c",
			-- "curl/lib/progress.c",
			-- "curl/lib/security.c",
			-- "curl/lib/sendf.c",
			-- "curl/lib/share.c",
			-- "curl/lib/speedcheck.c",
			-- "curl/lib/ssluse.c",
			-- "curl/lib/strequal.c",
			-- "curl/lib/strtok.c",
			-- "curl/lib/telnet.c",
			-- "curl/lib/timeval.c",
			-- "curl/lib/transfer.c",
			-- "curl/lib/url.c",
			-- "curl/lib/version.c",
		}
		excludes
		{
			"sys/win32/gl_logfuncs.cpp",
			"sys/win32/win_gamma.cpp",
			"sys/win32/win_snd.cpp",
		}
		defines
		{
			--"USE_OPENAL",
		}
		includedirs
		{
			"curl/include",
			--"../libs/sdl/include",
			--"../libs/openal/include",
		}
		libdirs
		{
			--"openal/lib",
			--"../libs/curl-7.12.2/lib"
		}
		links
		{
			"nafxcw",
		}
		buildoptions
		{
			--"/MT"
		}
		linkoptions 
		{
			"/LARGEADDRESSAWARE",
			--"/NODEFAULTLIB:libcmt.lib",
			--"/NODEFAULTLIB:libcmtd.lib"
			--"/NODEFAULTLIB:libc"
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
		
		
	configuration { "vs*", "x32" }
		targetdir 	"../bin/win32"
		libdirs
		{
			--"../libs/sdl/lib",
			--"../libs/openal/libs/win32",
			"openal/lib",
			--"../libs/curl-7.12.2/lib"
			"curl/lib"
		}
		links
		{ 
			"libcurl",
			"openal32",
			"opengl32",
			"glu32",
			"dbghelp",
			"dinput8",
			"dsound",
			"dxguid",
			"DxErr",
			"eaxguid",
		}
		prebuildcommands
		{
		   "cd curl/lib",
		   "nmake /f Makefile.vc6 CFG=release",
		}
		
	configuration { "vs*", "x64" }
		targetdir 	"../bin/win64"
		libdirs
		{
			--"../libs/sdl/lib64",
			--"../libs/openal/libs/win64",
			"openal/lib",
			--"../libs/curl-7.12.2/lib"
		}
		links
		{ 
			--"libcurl",
			"OpenAL32",
		}

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

		
		
include "game"