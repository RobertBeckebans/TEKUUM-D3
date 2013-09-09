--
-- Tekuum build configuration script
-- 

function FindAndroidNDK()

	local ndkdir = os.getenv("NDK")
	if (ndkdir) then
		print("Found Android Native Development Kit at '" .. ndkdir .. "'")
		return true
	end
	
	return false
end

function newplatform(plf)
    local name = plf.name
    local description = plf.description
 
    -- Register new platform
    premake.platforms[name] = {
        cfgsuffix = "_"..name,
        iscrosscompiler = true
    }
 
    -- Allow use of new platform in --platfroms
    table.insert(premake.option.list["platform"].allowed, { name, description })
    table.insert(premake.fields.platforms.allowed, name)
 
    -- Add compiler support
    -- gcc
    premake.gcc.platforms[name] = plf.gcc
    --other compilers (?)
end

newplatform 
{
    name = "armeabi",
    description = "Android armeabi",
    gcc = {
		cc = "$(NDK)/toolchains/arm-linux-androideabi-4.6/prebuilt/windows/bin/arm-linux-androideabi-gcc",
		cxx = "$(NDK)/toolchains/arm-linux-androideabi-4.6/prebuilt/windows/bin/arm-linux-androideabi-g++",
		ar = "$(NDK)/toolchains/arm-linux-androideabi-4.6/prebuilt/windows/bin/arm-linux-androideabi-ar",
    }
}

newplatform
{
    name = "armeabi-v7a",
    description = "Android armeabi-v7a",
    gcc = {
		cc = "$(NDK)/toolchains/arm-linux-androideabi-4.6/prebuilt/windows/bin/arm-linux-androideabi-gcc",
		cxx = "$(NDK)/toolchains/arm-linux-androideabi-4.6/prebuilt/windows/bin/arm-linux-androideabi-g++",
		ar = "$(NDK)/toolchains/arm-linux-androideabi-4.6/prebuilt/windows/bin/arm-linux-androideabi-ar",
    }
}

newplatform
{
	name = "neon",
	description = "Android neon",
	gcc = {
		cc = "$(NDK)/toolchains/arm-linux-androideabi-4.6/prebuilt/windows/bin/arm-linux-androideabi-gcc",
		cxx = "$(NDK)/toolchains/arm-linux-androideabi-4.6/prebuilt/windows/bin/arm-linux-androideabi-g++",
		ar = "$(NDK)/toolchains/arm-linux-androideabi-4.6/prebuilt/windows/bin/arm-linux-androideabi-ar",
   }
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

newoption
{
	trigger = "cmdline-tools",
	description = "Enable command line tools like dmap"
}

newoption
{
	trigger = "mfc-tools",
	description = "Enable original Doom 3 tools"
}

newoption
{
	trigger = "gtk-tools",
	description = "Enable GTK+ based extra tools"
}

-- newoption
-- {
	-- trigger = "qt-tools",
	-- description = "Enable Qt based extra tools"
-- }

newoption
{
	trigger = "android",
	description = "Cross compile for Android"
}

newoption
{
	trigger = "gles3",
	description = "Only use OpenGL ES 3.0 functions",
}

newoption
{
	trigger = "monolith",
	description = "Embed game logic into main executable"
}

newoption
{
	trigger = "standalone",
	description = "Skip Doom 3 base/ folder"
}

newoption
{
	trigger = "xinput",
	description = "Support the Xbox 360 controller"
}

newoption
{
	trigger = "sdl2",
	description = "Use SDL 2.0 instead of SDL 1.2"
}

--newoption
--{
--	trigger = "lua",
--	description = "Replace DoomScript with Lua"
--}

--
-- Use the embed action to convert all of the Lua scripts into C strings, which 
-- can then be built into the executable. Always embed the scripts before creating
-- a release build.
--
dofile("premake/embed.lua")

newaction {
	trigger     = "embed",
	description = "Embed scripts in scripts.c; required before release builds",
	execute     = doembed
}


solution "Tekuum"
	configurations { "Debug", "Release", "Profile" }
	
	if _OPTIONS["android"] then
		platforms {"armeabi", "armeabi-v7a", "neon", "x32"}
	else
		platforms {"x32", "x64"}
	end
	
	--
	-- Release/Debug Configurations
	--
	configuration "Debug"
		defines     "_DEBUG"
		flags
		{
			"Symbols",
			--"StaticRuntime",
			--"NoRuntimeChecks"
		}
	
	configuration "Release"
		defines     "NDEBUG"
		flags      
		{
			"OptimizeSpeed",
			--"EnableSSE",
			--"StaticRuntime"
		}
	
	-- OptimizeSpeed and Symbols do not work together with Visual Studio
	if not os.is("windows") then
		configuration "Profile"
			defines     "NDEBUG"
			flags
			{
				"OptimizeSpeed",
				-- --"EnableSSE",
				"Symbols",
				-- --"StaticRuntime"
			}
	end
	
	configuration { "vs*" }
		flags
		{
			"NoManifest",
			"NoMinimalRebuild",
		}
		buildoptions
		{
			-- multi processor support
			"/MP",
		}
		defines
		{
			-- the C++ Standard Library forbids macroizing keywords
			--"_ALLOW_KEYWORD_MACROS"
		}
	
	configuration { "vs*", "Debug" }
		buildoptions
		{
			-- turn off Smaller Type Check
			--"/RTCc-",
		
			-- turn off Basic Runtime Checks
			--"/RTC1-",
			
			--"/MTd",
		}
	
	configuration { "vs*", "Release" }
		buildoptions
		{
			-- turn off Whole Program Optimization
			"/GL-",
			
			-- Inline Function Expansion: Any Suitable (/Ob2)
			"/Ob2",
			
			-- enable Intrinsic Functions
			"/Oi",
			
			-- Omit Frame Pointers
			"/Oy",
			
			--"/MT",
		}
			
	configuration { "vs*", "Profile" }
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
			
			--"/MTd",
		}
		linkoptions
		{
			-- turn off Whole Program Optimization
			-- "/LTCG-",
			
			-- create .pdb file
			"/DEBUG",
		}
			
	configuration { "linux" }
		buildoptions
		{
			"-pipe",
			"-Wall",
			"-Wno-unknown-pragmas",
			
			-- only export what we mean to from the game SO
			--"-fvisibility=hidden",
			
			-- FIXME
			--"-Wnowrite-strings",
			--"-Wnounitialized",
			--"-Wno-deprecated",
			
			-- maintain this dangerous optimization off at all times
			"-fno-strict-aliasing",
			
			"-Wno-unused-variable",
			"-Wno-unused-but-set-variable",
			
			-- unhandled enums are fine
			"-Wno-switch"
		}
			
	configuration { "linux", "Release" }
		buildoptions
		{
			-- -finline-functions: implicit at -O3
			-- -fschedule-insns2: implicit at -O2
			
			"-ffast-math",
			
			-- no-unsafe-math-optimizations: that should be on by default really. hit some wonko bugs in physics code because of that
			"-fno-unsafe-math-optimizations",
			
			-- -fomit-frame-pointer: -O also turns on -fomit-frame-pointer on machines where doing so does not interfere with debugging
			"-fomit-frame-pointer",
		}
			
	configuration { "linux", "Profile" }
		buildoptions
		{
			-- -finline-functions: implicit at -O3
			-- -fschedule-insns2: implicit at -O2
			
			"-ffast-math",
			
			-- no-unsafe-math-optimizations: that should be on by default really. hit some wonko bugs in physics code because of that
			"-fno-unsafe-math-optimizations",
			
			-- -fomit-frame-pointer: -O also turns on -fomit-frame-pointer on machines where doing so does not interfere with debugging
			"-fomit-frame-pointer",
		}


--		
-- Platform specific defaults
--

function FindDirectXSDK()
	
	configuration {}
	local dxsdkdir = os.getenv("DXSDK_DIR")
	if (dxsdkdir) then
		includedirs { "$(DXSDK_DIR)/include" }
		configuration "x32"
			libdirs {"$(DXSDK_DIR)/lib/x86"}
		configuration "x64"
			libdirs {"$(DXSDK_DIR)/lib/x64"}
		configuration {}
		print("Found DirectX SDK at '" .. dxsdkdir .. "'")
		return true
	end
	
	return false
end

function FindGtkmmSDK()
	
	configuration {}
	local gtkmmsdkdir = os.getenv("GTKMM_BASEPATH")
	if (gtkmmsdkdir) then
		configuration {}
		print("Found Gtkmm SDK at '" .. gtkmmsdkdir .. "'")
		return true
	end
	
	return false
end

if _ACTION == "vs2010" then
	if _OPTIONS["xinput"] then
		foundDirectSDK = FindDirectXSDK()
	end
	
	if _OPTIONS["gtk-tools"] then
		foundGtkMMSDK = FindGtkmmSDK()
	end
end

if _OPTIONS["android"] then
	foundAndroidNDK = FindAndroidNDK()
end
	
if not _OPTIONS["android"] then
	include "idlib"
end













project "Tekuum"
	targetname  "Tekuum"
	language    "C++"
	if _OPTIONS["android"] then
		kind        "SharedLib"
	else
		kind        "WindowedApp"
	end
	flags       { "ExtraWarnings" }
	--debugargs	{ "+set com_allowConsole 1 +set fs_game basety" }
	files
	{
		"aas/*.cpp", "aas/*.h",
		"cm/*.cpp", "cm/*.h",
		"framework/**.cpp", "framework/**.h",
		"renderer/**.c", "renderer/**.cpp", "renderer/**.h",
		"libs/jpeg-6/*.c", "libs/jpeg-6/*.h",
		"libs/glew/src/glew.c",
		"libs/glew/include/GL/glew.h",
		"libs/png/*.c", "libs/png/*.h",
		"libs/zlib/*.c", "libs/zlib/*.h",
		"libs/etc1/etc1.cpp", "libs/etc1/etc1.h",
		
		"libs/irrxml/src/*.cpp", "libs/irrxml/src/*.h",
		
		"sound/*.cpp", "sound/*.h",
		
		"libs/oggvorbis/ogg/*.h",
		"libs/oggvorbis/oggsrc/bitwise.c",
		"libs/oggvorbis/oggsrc/framing.c",
		
		"libs/oggvorbis/vorbis/*.h",
		"libs/oggvorbis/vorbissrc/mdct.c",
		"libs/oggvorbis/vorbissrc/smallft.c",
		"libs/oggvorbis/vorbissrc/block.c",
		"libs/oggvorbis/vorbissrc/envelope.c",
		"libs/oggvorbis/vorbissrc/windowvb.c",
		"libs/oggvorbis/vorbissrc/lsp.c",
		"libs/oggvorbis/vorbissrc/lpc.c",
		"libs/oggvorbis/vorbissrc/analysis.c",
		"libs/oggvorbis/vorbissrc/synthesis.c",
		"libs/oggvorbis/vorbissrc/psy.c",
		"libs/oggvorbis/vorbissrc/info.c",
		"libs/oggvorbis/vorbissrc/floor1.c",
		"libs/oggvorbis/vorbissrc/floor0.c",
		"libs/oggvorbis/vorbissrc/res0.c",
		"libs/oggvorbis/vorbissrc/mapping0.c",
		"libs/oggvorbis/vorbissrc/registry.c",
		"libs/oggvorbis/vorbissrc/codebook.c",
		"libs/oggvorbis/vorbissrc/sharedbook.c",
		"libs/oggvorbis/vorbissrc/lookup.c",
		"libs/oggvorbis/vorbissrc/bitrate.c",
		"libs/oggvorbis/vorbissrc/vorbisfile.c",
		
		"libs/freetype/src/autofit/autofit.c",
		"libs/freetype/src/bdf/bdf.c",
		"libs/freetype/src/cff/cff.c",
		"libs/freetype/src/base/ftbase.c",
		"libs/freetype/src/base/ftbitmap.c",
		"libs/freetype/src/cache/ftcache.c",
		"libs/freetype/src/base/ftdebug.c",
		"libs/freetype/src/base/ftgasp.c",
		"libs/freetype/src/base/ftglyph.c",
		"libs/freetype/src/gzip/ftgzip.c",
		"libs/freetype/src/base/ftinit.c",
		"libs/freetype/src/lzw/ftlzw.c",
		"libs/freetype/src/base/ftstroke.c",
		"libs/freetype/src/base/ftsystem.c",
		"libs/freetype/src/smooth/smooth.c",
		"libs/freetype/src/base/ftbbox.c",
		"libs/freetype/src/base/ftmm.c",
		"libs/freetype/src/base/ftpfr.c",
		"libs/freetype/src/base/ftsynth.c",
		"libs/freetype/src/base/fttype1.c",
		"libs/freetype/src/base/ftwinfnt.c",
		"libs/freetype/src/pcf/pcf.c",
		"libs/freetype/src/pfr/pfr.c",
		"libs/freetype/src/psaux/psaux.c",
		"libs/freetype/src/pshinter/pshinter.c",
		"libs/freetype/src/psnames/psmodule.c",
		"libs/freetype/src/raster/raster.c",
		"libs/freetype/src/sfnt/sfnt.c",
		"libs/freetype/src/truetype/truetype.c",
		"libs/freetype/src/type1/type1.c",
		"libs/freetype/src/cid/type1cid.c",
		"libs/freetype/src/type42/type42.c",
		"libs/freetype/src/winfonts/winfnt.c",
		
		"sys/sys_*.cpp", "sys/sys_*.h",
		
		"ui/*.cpp", "ui/*.h",
	}
	excludes
	{
		"renderer/draw_exp_stub.cpp",
		"libs/jpeg-6/jmemdos.c",
		"libs/jpeg-6/jload.c",
		"libs/jpeg-6/jpegtran.c",
		
		"tools/edit_stub.cpp",
		--"tools/qttest/*",
		--"tools/gtktest/*",
	}
	includedirs
	{
		"idlib",
		"libs/zlib",
		"libs/glew/include",
		"libs/freetype/include",
		"libs/oggvorbis/ogg",
		"libs/oggvorbis/vorbis",
	}
	defines
	{
		"__DOOM__",
		"GLEW_STATIC",
		"BUILD_FREETYPE",
		"FT2_BUILD_LIBRARY",
	}
	links
	{
		--"idlib",
	}
	
if not _OPTIONS["android"] and not _OPTIONS["monolith"] then
	defines
	{
		"__DOOM_DLL__",
	}
end

if not _OPTIONS["android"] then
	defines
	{
		"ID_PC",
	}
end
	
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
		
	configuration "gles3"
		defines
		{
			"USE_GLES3",
		}
		
	configuration "monolith"
		files
		{
			"game/**.c", "game/**.cpp", "game/**.h",
		}
		excludes
		{
			"game/gamesys/Callbacks.cpp",
			"game/EndLevel.cpp", "game/EndLevel.h",
			"game/gamesys/GameTypeInfo.h",
			"game/gamesys/TypeInfo.cpp",
		}
		
	configuration { "monolith", "debug-memory" }
		files
		{
			"game/gamesys/GameTypeInfo.h",
		}
		
	configuration "lua"
		defines
		{
			"USE_LUA",
		}
		files
		{
			"libs/lua/src/*.c", "libs/lua/src/*.h",
		}
		excludes
		{
			"libs/lua/src/linit.c",
			"libs/lua/src/lua.c",
			"libs/lua/src/luac.c",
		}
		includedirs
		{
			"libs/lua/src",
		}
	
	configuration { "cmdline-tools", "vs*" }
		defines
		{
			"USE_CMDLINE_TOOLS",
		}
		files
		{
			"tools/compilers/**.cpp", "tools/compilers/**.h",
		}
	
	configuration { "mfc-tools", "vs*" }
		flags       { "MFC" }
		defines
		{
			"USE_MFC_TOOLS",
			--"_AFXDLL"
		}
		files
		{
			"tools/*.h",
			"tools/af/*.cpp", "tools/af/*.h",
			"tools/comafx/*.cpp", "tools/comafx/*.h",
			"tools/common/**.cpp", "tools/common/**.h",
			--"tools/debugger/**.cpp", "tools/debugger/**.h",
			"tools/decl/*.cpp", "tools/decl/*.h",
			--"tools/guied/*.cpp", "tools/guied/*.h",
			--"tools/materialeditor/*.cpp", "tools/materialeditor/*.h",
			"tools/particle/*.cpp", "tools/particle/*.h",
			"tools/pda/*.cpp", "tools/pda/*.h",
			"tools/radiant/*.cpp", "tools/radiant/*.h",
			"tools/script/*.cpp", "tools/script/*.h",
			"tools/sound/*.cpp", "tools/sound/*.h",
			
			"sys/win32/rc/doom.rc",
		}
		excludes
		{
			"tools/common/RenderBumpFlatDialog.*",
			"tools/decl/DialogEntityDefEditor.*",
			"tools/guied/GEWindowWrapper_stub.cpp",
		
			"sys/win32/rc/doom_nomfc.rc",
		}
		includedirs
		{
			"libs/atlmfc/include"
		}
		
	configuration { "mfc-tools", "vs*", "x32" }
		libdirs
		{
			"libs/atlmfc/lib"
		}
		
	configuration { "mfc-tools", "vs*", "x64" }
		libdirs
		{
			"libs/atlmfc/lib/amd64"
		}
	
	
	-- configuration "qt-tools"
		-- defines
		-- {
			-- "USE_QT"
		-- }
		-- includedirs
		-- {
			-- --"../libs/qt"
		-- }
		-- files
		-- {
			-- "tools/qttest/*",
		-- }
		
		-- -- Files can be customized at the project scope and below.
		-- customizefile "qttest.h"
			-- filebuilddescription "Generating file using blah blah tool"
			-- filebuildcommands
			-- {
				-- "$(QTDIR)\bin\moc.exe -DUNICODE -DWIN32 -DQT_LARGEFILE_SUPPORT -DQT_CORE_LIB -DQT_GUI_LIB -DQT_OPENGL_LIB  -I. -I.\GeneratedFiles "-I$(QTDIR)\include" "-I.\GeneratedFiles\$(ConfigurationName)\." "-I$(QTDIR)\include\qtmain" "-I$(QTDIR)\include\QtCore" "-I$(QTDIR)\include\QtGui" "-I$(QTDIR)\include\QtOpenGL" "-I." "-I." "-I." "-I." "qttest1.h" -o ".\GeneratedFiles\$(ConfigurationName)\moc_%(Filename).cpp"",
				-- "copy some.file generated.cpp"
			-- }
			-- filebuildadditionaldependencies { "*.h" }
			-- filebuildoutputs { "generated.cpp" }
		
	-- configuration { "qt-tools", "vs*", "x32" }
		-- includedirs
		-- {
			-- "$(QTDIR)/include",
			-- "$(QTDIR)/include/qtmain",
			-- "$(QTDIR)/include/QtCore",
			-- "$(QTDIR)/include/QtGui",
			-- "$(QTDIR)/include/QtOpenGL",
		-- }
		-- libdirs
		-- {
			-- "$(QTDIR)/lib"
		-- }
		
	-- configuration { "qt-tools", "Release" }
		-- links
		-- {
			-- "qtmain",
			-- "QtCore4",
			-- "QtGui4",
			-- "QtOpenGL4",
		-- }
		
	-- configuration { "qt-tools", "Debug" }
		-- links
		-- {
			-- "qtmaind",
			-- "QtCored4",
			-- "QtGuid4",
			-- "QtOpenGLd4",
		-- }
	
	configuration "gtk-tools"
		defines
		{
			"USE_GTK_TOOLS"
		}
		includedirs
		{
			--"../libs/qt"
		}
		files
		{
			"tools/*.h",
			"tools/gtktest/*.cpp", "tools/gtktest/*.h",
		}
		
	configuration { "gtk-tools", "vs*" }
		includedirs
		{
			"$(GTKMM_BASEPATH)/include",
			"$(GTKMM_BASEPATH)/include/gtkmm-2.4",
			"$(GTKMM_BASEPATH)/lib/gtkmm-2.4/include",
			"$(GTKMM_BASEPATH)/include/atkmm-1.6",
			"$(GTKMM_BASEPATH)/include/giomm-2.4",
			"$(GTKMM_BASEPATH)/lib/giomm-2.4/include",
			"$(GTKMM_BASEPATH)/include/pangomm-1.4",
			"$(GTKMM_BASEPATH)/lib/pangomm-1.4/include",
			"$(GTKMM_BASEPATH)/include/gtk-2.0",
			"$(GTKMM_BASEPATH)/include/gdkmm-2.4",
			"$(GTKMM_BASEPATH)/lib/gdkmm-2.4/include",
			"$(GTKMM_BASEPATH)/include/atk-1.0",
			"$(GTKMM_BASEPATH)/include/glibmm-2.4",
			"$(GTKMM_BASEPATH)/lib/glibmm-2.4/include",
			"$(GTKMM_BASEPATH)/include/glib-2.0",
			"$(GTKMM_BASEPATH)/lib/glib-2.0/include",
			"$(GTKMM_BASEPATH)/include/sigc++-2.0",
			"$(GTKMM_BASEPATH)/lib/sigc++-2.0/include",
			"$(GTKMM_BASEPATH)/include/cairomm-1.0",
			"$(GTKMM_BASEPATH)/lib/cairomm-1.0/include",
			"$(GTKMM_BASEPATH)/include/pango-1.0",
			"$(GTKMM_BASEPATH)/include/cairo",
			"$(GTKMM_BASEPATH)/include",
			"$(GTKMM_BASEPATH)/include/freetype2",
			"$(GTKMM_BASEPATH)/include/libpng14",
			"$(GTKMM_BASEPATH)/lib/gtk-2.0/include",
			"$(GTKMM_BASEPATH)/include/gdk-pixbuf-2.0",
		}
		libdirs
		{
			"$(GTKMM_BASEPATH)/lib",
		}
		
	--configuration { "gtk-tools", "vs2010", "Release" }
	configuration { "gtk-tools", "vs2010" }
		links
		{
			"gtkmm-vc100-2_4",
			"atkmm-vc100-1_6",
			"gdkmm-vc100-2_4",
			"giomm-vc100-2_4",
			"pangomm-vc100-1_4",
			"gtk-win32-2.0",
			"glibmm-vc100-2_4",
			"cairomm-vc100-1_0",
			"sigc-vc100-2_0",
			"gdk-win32-2.0",
			"atk-1.0",
			"gio-2.0",
			"pangowin32-1.0",
			"gdi32",
			"pangocairo-1.0",
			"gdk_pixbuf-2.0",
			--"png14",
			"pango-1.0",
			"cairo",
			"gobject-2.0",
			"gmodule-2.0",
			"gthread-2.0",
			"glib-2.0",
			"intl",
		}
		
	-- configuration { "gtk-tools", "vs2010", "Debug" }
		-- links
		-- {
			-- "gtkmm-vc100-d-2_4",
			-- ...
		-- }
	
	-- 
	-- Project Configurations
	-- 
	configuration "vs*"
		flags       { "WinMain" }
		files
		{
			"sys/win32/**.cpp", "sys/win32/**.h",
			--"sys/win32/rc/doom.rc",
			"sys/win32/rc/doom_nomfc.rc",
			--"sys/win32/res/*",
			
			"libs/glew/include/GL/wglew.h",
			
			--"libs/openal/idal.cpp", "libs/openal/idal.h",
			"libs/openal/include/*.h",
		}
		excludes
		{
			"sys/win32/win_gamma.cpp",
			--"sys/win32/win_snd.cpp",
		}
		includedirs
		{
			--"libs/curl/include",
			"libs/openal/include",
			--"libs/sdl/include",
		}
		libdirs
		{
			--"libs/openal/lib",
			--"libs/curl-7.12.2/lib"
		}
		links
		{
			"idlib",
		}
		buildoptions
		{
			--"/MT",
			-- multi processor support
			"/MP",
		}
		linkoptions
		{
			"/LARGEADDRESSAWARE",
			"/DYNAMICBASE",
			"/STACK:16777216",
			--"/NODEFAULTLIB:libcmt.lib",
			--"/NODEFAULTLIB:libcmtd.lib"
			--"/NODEFAULTLIB:libc",
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
			"USE_OPENAL",
			"USE_EXCEPTIONS",
			--"USE_GLES1",
		}
	
	configuration { "vs2010", "xinput" }
		defines
		{
			"USE_XINPUT",
		}
		links
		{
			"Xinput"
		}

	configuration { "vs*", "x32" }
		targetdir 	"../bin/win32"
		libdirs
		{
			--"../libs/sdl/lib",
			--"../libs/openal/libs/win32",
			"libs/openal/lib",
			--"../libs/curl-7.12.2/lib"
			--"libs/curl/lib"
		}
		links
		{ 
			--"libcurl",
			"openal32",
			"opengl32",
			"glu32",
			"dbghelp",
			"dinput8",
			"dsound",
			"dxguid",
			--"DxErr",
			"eaxguid",
			"iphlpapi",
			"winmm",
			"ws2_32",
		}
		--prebuildcommands
		--{
		--   "cd libs/curl/lib",
		--   "nmake /f Makefile.vc6 CFG=release",
		--}
		
	configuration { "vs*", "x64" }
		targetdir 	"../bin/win64"
		libdirs
		{
			--"../libs/sdl/lib64",
			--"../libs/openal/libs/win64",
			"libs/openal/lib",
			--"../libs/curl-7.12.2/lib"
		}
		links
		{ 
			--"libcurl",
			"OpenAL32",
			"PNG_NO_ASSEMBLER_CODE",
		}

if not _OPTIONS["android"] then
	configuration { "linux", "gmake" }
		buildoptions
		{
			--"`pkg-config --cflags x11`",
			--"`pkg-config --cflags xext`",
			--"`pkg-config --cflags xf86dgaproto`",
			--"`pkg-config --cflags xxf86vm`",
			"`pkg-config --cflags libpulse-simple`",
			--"`pkg-config --cflags libcurl`",
		}
		linkoptions
		{
			--"`pkg-config --libs x11`",
			--"`pkg-config --libs xext`",
			--"`pkg-config --libs xf86dgaproto`",
			--"`pkg-config --libs xxf86vm`",
			"`pkg-config --libs libpulse-simple`",
			--"`pkg-config --libs libcurl`",
		}
		links
		{ 
			--"curl",
			--"openal",
		}
		
if _OPTIONS["sdl2"] then
	configuration { "linux", "gmake" }
	  buildoptions
		{
			"`pkg-config --cflags sdl2`",
		}
		linkoptions
		{
			"`pkg-config --libs sdl2`",
		}
else
	configuration { "linux", "gmake" }
		buildoptions
		{
			"`pkg-config --cflags sdl`",
		}
		linkoptions
		{
			"`pkg-config --libs sdl`",
		}
end
	
	configuration "linux"
		targetname  "tekuum"
		files
		{
			"sys/sys_local.cpp",
			"sys/posix/posix_net.cpp",
			"sys/posix/posix_main.cpp",
			"sys/posix/posix_signal.cpp",
			"sys/posix/posix_threads.cpp",
			--"sys/posix/posix_input.cpp",
			"sys/linux/stack.cpp",
			"sys/linux/linux_main.cpp",
			--"sys/linux/glimp.cpp",
			--"sys/linux/input.cpp",
			"sys/sdl/sdl_glimp.cpp",
			"sys/sdl/sdl_events.cpp",
			"sys/linux/sound.cpp",
			--"sys/linux/sound_alsa.cpp",
			"sys/linux/sound_pulse.cpp",
			--"sys/linux/libXNVCtrl/NVCtrl.c",
			--"sys/sdl/sdl_sound.cpp", "sys/sdl/sdl_sound.h",
		}
		excludes
		{
			"sound/snd_efxfile.cpp",
		}
		buildoptions
		{
			"-pthread"
		}
		links
		{
			"GL",
			"dl",
			"idlib",
			"rt",
		}
		linkoptions
		{
			"-pthread"
		}
		defines
		{
			"USE_EXCEPTIONS",
			--"USE_OPENAL",
			"USE_SOUND_PULSE",
			--"USE_SOUND_SDL",
			"PNG_NO_ASSEMBLER_CODE",
			"USE_SDL",
			"USE_SDL_ASYNC",
		}
		
	configuration { "cmdline-tools", "linux" }
		defines
		{
			"USE_CMDLINE_TOOLS",
		}
		files
		{
			"tools/compilers/dmap/optimize_gcc.cpp",
		}
			
	configuration { "linux", "x32" }
		targetdir 	"../bin/linux-x86"
		
	configuration { "linux", "x64" }
		targetdir 	"../bin/linux-x86_64"

end -- if not _OPTIONS["android"]

	configuration "android"
		targetname  "tekuum"
		buildoptions
		{
			-- shut up about: the mangling of 'va_list' has changed in GCC 4.4
			"-Wno-psabi",
			
			-- Android NDK does not support exceptions ...
			--"-fno-exceptions",
		}
		files
		{
			"../android/jni/tekuumjni.c",
			"idlib/**.cpp", "idlib/**.h",
			"game/**.cpp", "game/**.h",
			"sys/android/**.cpp", "android/**.h",
			"sys/posix/posix_net.cpp",
			"sys/posix/posix_main.cpp",
			"sys/posix/posix_signal.cpp",
			"sys/posix/posix_threads.cpp",
			"sys/linux/stack.cpp",
			--"sys/linux/main.cpp",
		}
		excludes
		{
			"idlib/math/Simd_AltiVec.cpp", "idlib/math/Simd_AltiVec.h",
			--"idlib/bv/Frustum_gcc.cpp",
			"game/gamesys/Callbacks.cpp",
			"game/EndLevel.cpp", "game/EndLevel.h",
		}
		excludes
		{
			"renderer/GLShader.cpp",
			"renderer/Framebuffer.cpp",
			--"renderer/draw_arb.cpp",
			"renderer/draw_glsl.cpp",
			"renderer/draw_arb2.cpp",
			"renderer/draw_exp.cpp",
			"renderer/draw_nv10.cpp",
			"renderer/draw_nv20.cpp",
			"renderer/draw_r200.cpp",
			"libs/glew/src/glew.c",
			"libs/glew/include/GL/glew.h",
			"sound/snd_efxfile.cpp",
		}
		defines
		{
			"ANDROID",
			"USE_GLES1",
			"PNG_NO_ASSEMBLER_CODE",
		}
		includedirs
		{
			"$(NDK)/platforms/android-9/arch-arm/usr/include",
			--"$(NDK)/prebuilt/windows/lib/gcc/arm-linux-androideabi/4.4.3/include",
			"$(NDK)/sources/cxx-stl/stlport/stlport",
		}
		libdirs
		{
			"$(NDK)/platforms/android-9/arch-arm/usr/lib",
			--"$(NDK)/sources/cxx-stl/gnu-libstdc++/libs/armeabi",
			--"$(NDK)/sources/cxx-stl/stlport/libs/armeabi",
		}
		links
		{
			"EGL",
			"GLESv1_CM",
			"OpenSLES",
			"m",
			"c",
			"log",
			"gcc",
			"dl",
			"stdc++",
			"stlport_static",
			--"stlport_shared",
			--"supc++",
		}
		linkoptions
		{
			"-nostdlib",
			--"--no-undefined",
		}
		--prebuildcommands
		--{
		--   "cd ../android",
		--   "ndk-build V=1",
		--}
		
	configuration "armeabi"
		targetdir 	"../android/libs/armeabi"
		buildoptions
		{
			"-march=armv5te -mtune=xscale -msoft-float -fpic -mthumb -ffunction-sections -funwind-tables -fstack-protector -fno-short-enums",
			"-fuse-ld=gold",
		}
		defines
		{
			"__ARM_ARCH_5__",
			"__ARM_ARCH_5T__",
			"__ARM_ARCH_5E__",
			"__ARM_ARCH_5TE__"
		}
		libdirs
		{
			"$(NDK)/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi",
			"$(NDK)/sources/cxx-stl/stlport/libs/armeabi",
		}
		linkoptions
		{
			"-fuse-ld=gold",
		}
		prebuildcommands
		{
			"cp $(NDK)/prebuilt/android-arm/gdbserver/gdbserver ../android/libs/armeabi/gdbserver",
			"echo 'set solib-search-path ./obj/local/armeabi' > ../android/libs/armeabi/gdb.setup",
			"echo 'directory $(NDK)/platforms/android-9/arch-arm/usr/include $(NDK)/sources/cxx-stl/stlport jni/../../src/idlib jni/../../src/libs/zlib jni/../../src/libs/freetype/include jni/../../src/libs/oggvorbis/ogg jni/../../src/libs/oggvorbis/vorbis $(NDK)/sources/cxx-stl/stlport/stlport $(NDK)/sources/cxx-stl//gabi++/include' >> ../android/libs/armeabi/gdb.setup",
		}
		
	configuration "armeabi-v7a"
		targetname  "tekuum"
		targetdir 	"../android/libs/armeabi-v7a"
		buildoptions
		{
			"-march=armv7-a -mfloat-abi=softfp -mfpu=vfp -fpic -mthumb -ffunction-sections -funwind-tables -fstack-protector -fno-short-enums"
		}
		defines
		{
			"__ARM_ARCH_5__",
			"__ARM_ARCH_5T__",
			"__ARM_ARCH_5E__",
			"__ARM_ARCH_5TE__"
		}
		libdirs
		{
			"$(NDK)/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi-v7a",
			"$(NDK)/sources/cxx-stl/stlport/libs/armeabi-v7a",
		}
		prebuildcommands
		{
			"cp $(NDK)/prebuilt/android-arm/gdbserver/gdbserver ../android/libs/armeabi-v7a/gdbserver",
			"echo 'set solib-search-path ./obj/local/armeabi-v7a' > ../android/libs/armeabi-v7a/gdb.setup",
			"echo 'directory $(NDK)/platforms/android-9/arch-arm/usr/include $(NDK)/sources/cxx-stl/stlport jni/../../src/idlib jni/../../src/libs/zlib jni/../../src/libs/freetype/include jni/../../src/libs/oggvorbis/ogg jni/../../src/libs/oggvorbis/vorbis $(NDK)/sources/cxx-stl/stlport/stlport $(NDK)/sources/cxx-stl//gabi++/include' >> ../android/libs/armeabi-v7a/gdb.setup",
		}
	
	configuration "neon"
		targetname  "tekuum-neon"
		targetdir 	"../android/libs/armeabi"
		buildoptions
		{
			"-mcpu=cortex-a8 -mfloat-abi=softfp -mfpu=neon -fpic -fno-short-enums -ffunction-sections -funwind-tables -fstack-protector -ftree-vectorize -fsingle-precision-constant"
		}
		defines
		{
			"__MATH_NEON",
		}
		libdirs
		{
			"$(NDK)/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi",
			"$(NDK)/sources/cxx-stl/stlport/libs/armeabi",
		}

-- FIXME
--if os.is("windows") and _OPTIONS["debug-memory"] then
if _OPTIONS["debug-memory"] then
	include "TypeInfo"
end

if not _OPTIONS["android"] and not _OPTIONS["monolith"] then
	include "game"
end
