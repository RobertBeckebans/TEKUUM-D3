cmake_minimum_required(VERSION 2.8.11)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

project(Tekuum)

option(USE_CMDLINE_TOOLS
		"Compile the built-in command line based tools" ON)

option(USE_MFC_TOOLS
		"Compile the built-in MFC based tools" OFF)
		
option(USE_QT_TOOLS
		"Compile the built-in Qt based tools" OFF)
		
option(USE_QT_WINDOWING
		"Run game as Qt app" OFF)
		
option(MONOLITH
		"Embed game logic into main executable" ON)
		
option(STANDALONE
		"Skip Doom 3 base/ folder" ON)
		
option(GLES2
		"Only use OpenGL ES 2.0 functions" OFF)
		
option(ANGLE
		"Only use OpenGL ES 2.0 functions through ANGLE" OFF)

option(MESA
		"Only use OpenGL ES 3.0 functions and turn off GPU skeletal animation" OFF)
		
option(DEBUG_MEMORY
		"Enables memory logging to file" OFF)

#if(USE_QT_TOOLS)
#	set(USE_QT_WINDOWING TRUE)
#endif()

# if(ANGLE OR GLES2)
	# set(USE_CMDLINE_TOOLS FALSE)
# endif()


if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
	add_definitions(-pipe)
    add_definitions(-mmmx -msse -msse2)
	add_definitions(-std=c++11)
	
    # the warnings are used for every profile anyway, so put them in a variable
	set(my_warn_flags "-Wno-pragmas -Wno-unused-variable -Wno-switch -Wno-unused-value -Winvalid-pch")

    if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
        # append clang-specific settings for warnings (the second one make sure clang doesn't complain
        # about unknown -W flags, like -Wno-unused-but-set-variable)
        set(my_warn_flags "${my_warn_flags} -Wno-local-type-template-args -Wno-unknown-warning-option")
    endif()

	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG -O0 -ggdb -fno-strict-aliasing ${my_warn_flags}")
	#set(CMAKE_C_FLAGS_DEBUGALL "${CMAKE_C_FLAGS_DEBUGALL} -g -ggdb -D_DEBUG -fno-strict-aliasing ${my_warn_flags}")
	#set(CMAKE_C_FLAGS_PROFILE "${CMAKE_C_FLAGS_PROFILE} -g -ggdb -D_DEBUG -O1 -fno-omit-frame-pointer -fno-strict-aliasing ${my_warn_flags}")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -ffast-math -fno-unsafe-math-optimizations -fomit-frame-pointer -fno-strict-aliasing ${my_warn_flags}")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g -O3 -ffast-math -fno-unsafe-math-optimizations -fomit-frame-pointer -fno-strict-aliasing ${my_warn_flags}")
	set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -Os -ffast-math -fno-unsafe-math-optimizations -fomit-frame-pointer -fno-strict-aliasing ${my_warn_flags}")

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -std=c++11" )
	#set(CMAKE_CXX_FLAGS_DEBUGALL "${CMAKE_C_FLAGS_DEBUGALL} -std=c++11")
	#set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_C_FLAGS_PROFILE})
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -std=c++11" )
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -std=c++11" )
	set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -std=c++11" )

elseif(MSVC)
	
	# Multiprocessor support for compiling
	# /MP
	
	# Inline Function Expansion: Any Suitable (/Ob2)
	# "/Ob2",
			
	# enable Intrinsic Functions
	# "/Oi",
			
	# Omit Frame Pointers
	# "/Oy",
	
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /D _DEBUG /MP /MDd")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MP /Oi /Oy /MD")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /MP /Oi /MDd")
	set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} /MP /Oi /Oy /MD")
	
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP /MDd")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP /Oi /Oy /MD")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MP /MDd")
	set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MP /Oi /Oy /MD")

	add_definitions(-DWIN32 
					-D_WINDOWS
					-D_CRT_SECURE_NO_DEPRECATE
					-D_CRT_NONSTDC_NO_DEPRECATE
					-D_CRT_SECURE_NO_WARNINGS
					-D_MBCS
					#-DUSE_OPENAL
					-DUSE_EXCEPTIONS)
					
	if(NOT CMAKE_CL_64)
		add_definitions(-D_USE_32BIT_TIME_T)
	endif()
					
	if(USE_MFC_TOOLS)
		add_definitions(-D_AFXDLL)
		
		# 0: Use Standard Windows Libraries
		# 1: Use MFC in a Static Library
		# 2: Use MFC in a Shared DLL 
		set(CMAKE_MFC_FLAG 2)
	endif()
endif()

# CMake only used for desktop builds
add_definitions(-DID_PC)

if(STANDALONE)
	add_definitions(-DSTANDALONE)
endif()

if(DEBUG_MEMORY)
	add_definitions(-DID_DEBUG_MEMORY)
	add_definitions(-DID_REDIRECT_NEWDELETE)
endif()


add_subdirectory(idlib)
add_subdirectory(libs)

file(GLOB AAS_INCLUDES aas/*.h)
file(GLOB AAS_SOURCES aas/*.cpp)

#file(GLOB_RECURSE CM_INCLUDES cm/*.h)
#file(GLOB_RECURSE CM_SOURCES cm/*.cpp)

set(CM_INCLUDES
	cm/CollisionModel.h
	cm/CollisionModel_local.h)
	
set(CM_SOURCES
	cm/CollisionModel_contacts.cpp
	cm/CollisionModel_contents.cpp
	cm/CollisionModel_debug.cpp
	cm/CollisionModel_files.cpp
	cm/CollisionModel_load.cpp
	cm/CollisionModel_rotate.cpp
	cm/CollisionModel_trace.cpp
	cm/CollisionModel_translate.cpp)

file(GLOB FRAMEWORK_INCLUDES framework/*.h)
file(GLOB FRAMEWORK_SOURCES framework/*.cpp)

file(GLOB FRAMEWORK_ASYNC_INCLUDES framework/async/*.h)
file(GLOB FRAMEWORK_ASYNC_SOURCES framework/async/*.cpp)

file(GLOB RENDERER_INCLUDES renderer/*.h)
file(GLOB RENDERER_SOURCES renderer/*.cpp)
file(GLOB RENDERER_COLOR_INCLUDES renderer/Color/*.h)
file(GLOB RENDERER_COLOR_SOURCES renderer/Color/*.cpp)
file(GLOB RENDERER_DXT_INCLUDES renderer/DXT/*.h)
file(GLOB RENDERER_DXT_SOURCES renderer/DXT/*.cpp)
file(GLOB RENDERER_JOBS_INCLUDES renderer/jobs/*.h)
file(GLOB RENDERER_JOBS_SOURCES renderer/jobs/*.cpp)
file(GLOB RENDERER_JOBS_DYNAMICSHADOWVOLUME_INCLUDES renderer/jobs/dynamicshadowvolume/*.h)
file(GLOB RENDERER_JOBS_DYNAMICSHADOWVOLUME_SOURCES renderer/jobs/dynamicshadowvolume/*.cpp)
file(GLOB RENDERER_JOBS_PRELIGHTSHADOWVOLUME_INCLUDES renderer/jobs/prelightshadowvolume/*.h)
file(GLOB RENDERER_JOBS_PRELIGHTSHADOWVOLUME_SOURCES renderer/jobs/prelightshadowvolume/*.cpp)
file(GLOB RENDERER_JOBS_STATICSHADOWVOLUME_INCLUDES renderer/jobs/staticshadowvolume/*.h)
file(GLOB RENDERER_JOBS_STATICSHADOWVOLUME_SOURCES renderer/jobs/staticshadowvolume/*.cpp)
file(GLOB RENDERER_OPENGL_INCLUDES renderer/OpenGL/*.h)
file(GLOB RENDERER_OPENGL_SOURCES renderer/OpenGL/*.cpp)

file(GLOB IRRXML_INCLUDES libs/irrxml/src/*.h)
file(GLOB IRRXML_SOURCES libs/irrxml/src/*.cpp)
	
set(JPEG_INCLUDES
	libs/jpeg-6/jchuff.h
	libs/jpeg-6/jconfig.h
	libs/jpeg-6/jdct.h
	libs/jpeg-6/jdhuff.h
	libs/jpeg-6/jerror.h
	libs/jpeg-6/jinclude.h
	libs/jpeg-6/jmemsys.h
	libs/jpeg-6/jmorecfg.h
	libs/jpeg-6/jpegint.h
	libs/jpeg-6/jpeglib.h
	libs/jpeg-6/jversion.h)
	
set(JPEG_SOURCES
	libs/jpeg-6/jcapimin.c
	libs/jpeg-6/jcapistd.c
	libs/jpeg-6/jccoefct.c
	libs/jpeg-6/jccolor.c
	libs/jpeg-6/jcdctmgr.c
	libs/jpeg-6/jchuff.c
	libs/jpeg-6/jcinit.c
	libs/jpeg-6/jcmainct.c
	libs/jpeg-6/jcmarker.c
	libs/jpeg-6/jcmaster.c
	libs/jpeg-6/jcomapi.c
	libs/jpeg-6/jcparam.c
	libs/jpeg-6/jcphuff.c
	libs/jpeg-6/jcprepct.c
	libs/jpeg-6/jcsample.c
	libs/jpeg-6/jctrans.c
	libs/jpeg-6/jdapimin.c
	libs/jpeg-6/jdapistd.c
	libs/jpeg-6/jdatadst.c
	libs/jpeg-6/jdatasrc.c
	libs/jpeg-6/jdcoefct.c
	libs/jpeg-6/jdcolor.c
	libs/jpeg-6/jddctmgr.c
	libs/jpeg-6/jdhuff.c
	libs/jpeg-6/jdinput.c
	libs/jpeg-6/jdmainct.c
	libs/jpeg-6/jdmarker.c
	libs/jpeg-6/jdmaster.c
	libs/jpeg-6/jdmerge.c
	libs/jpeg-6/jdphuff.c
	libs/jpeg-6/jdpostct.c
	libs/jpeg-6/jdsample.c
	libs/jpeg-6/jdtrans.c
	libs/jpeg-6/jerror.c
	libs/jpeg-6/jfdctflt.c
	libs/jpeg-6/jfdctfst.c
	libs/jpeg-6/jfdctint.c
	libs/jpeg-6/jidctflt.c
	libs/jpeg-6/jidctfst.c
	libs/jpeg-6/jidctint.c
	libs/jpeg-6/jidctred.c
	#libs/jpeg-6/jload.c
	libs/jpeg-6/jmemmgr.c
	libs/jpeg-6/jmemnobs.c
	#libs/jpeg-6/jpegtran.c
	libs/jpeg-6/jquant1.c
	libs/jpeg-6/jquant2.c
	libs/jpeg-6/jutils.c)

file(GLOB PNG_INCLUDES libs/png/*.h)
file(GLOB PNG_SOURCES libs/png/*.c)

file(GLOB ZLIB_INCLUDES libs/zlib/*.h)
file(GLOB ZLIB_SOURCES libs/zlib/*.c)

set(ETC1_INCLUDES libs/etc1/etc1.h)
set(ETC1_SOURCES libs/etc1/etc1.cpp)
	
set(GLEW_INCLUDES
	libs/glew/include/GL/glew.h)
	
if(WIN32)
	set(GLEW_INCLUDES ${GLEW_INCLUDES} libs/glew/include/GL/wglew.h)
else()
	set(GLEW_INCLUDES ${GLEW_INCLUDES} libs/glew/include/GL/glxew.h)
endif()
	
set(GLEW_SOURCES
	libs/glew/src/glew.c)
	
set(FREETYPE_SOURCES
	libs/freetype/src/autofit/autofit.c
	libs/freetype/src/bdf/bdf.c
	libs/freetype/src/cff/cff.c
	libs/freetype/src/base/ftbase.c
	libs/freetype/src/base/ftbitmap.c
	libs/freetype/src/cache/ftcache.c
	libs/freetype/src/base/ftdebug.c
	libs/freetype/src/base/ftgasp.c
	libs/freetype/src/base/ftglyph.c
	libs/freetype/src/gzip/ftgzip.c
	libs/freetype/src/base/ftinit.c
	libs/freetype/src/lzw/ftlzw.c
	libs/freetype/src/base/ftstroke.c
	libs/freetype/src/base/ftsystem.c
	libs/freetype/src/smooth/smooth.c
	libs/freetype/src/base/ftbbox.c
	libs/freetype/src/base/ftmm.c
	libs/freetype/src/base/ftpfr.c
	libs/freetype/src/base/ftsynth.c
	libs/freetype/src/base/fttype1.c
	libs/freetype/src/base/ftwinfnt.c
	libs/freetype/src/pcf/pcf.c
	libs/freetype/src/pfr/pfr.c
	libs/freetype/src/psaux/psaux.c
	libs/freetype/src/pshinter/pshinter.c
	libs/freetype/src/psnames/psmodule.c
	libs/freetype/src/raster/raster.c
	libs/freetype/src/sfnt/sfnt.c
	libs/freetype/src/truetype/truetype.c
	libs/freetype/src/type1/type1.c
	libs/freetype/src/cid/type1cid.c
	libs/freetype/src/type42/type42.c
	libs/freetype/src/winfonts/winfnt.c)
	
	
set(SOUND_INCLUDES
	sound/snd_local.h
	sound/sound.h)
	
set(SOUND_SOURCES
	sound/snd_cache.cpp
	sound/snd_decoder.cpp
	#sound/snd_efxfile.cpp
	sound/snd_emitter.cpp
	sound/snd_shader.cpp
	sound/snd_system.cpp
	sound/snd_wavefile.cpp
	sound/snd_world.cpp)
	
set(OGGVORBIS_INCLUDES
	libs/oggvorbis/ogg/ogg.h
	libs/oggvorbis/ogg/os_types.h)
	
set(OGGVORBIS_SOURCES
	libs/oggvorbis/oggsrc/bitwise.c
	libs/oggvorbis/oggsrc/framing.c
	
	libs/oggvorbis/vorbissrc/mdct.c
	libs/oggvorbis/vorbissrc/smallft.c
	libs/oggvorbis/vorbissrc/block.c
	libs/oggvorbis/vorbissrc/envelope.c
	libs/oggvorbis/vorbissrc/windowvb.c
	libs/oggvorbis/vorbissrc/lsp.c
	libs/oggvorbis/vorbissrc/lpc.c
	libs/oggvorbis/vorbissrc/analysis.c
	libs/oggvorbis/vorbissrc/synthesis.c
	libs/oggvorbis/vorbissrc/psy.c
	libs/oggvorbis/vorbissrc/info.c
	libs/oggvorbis/vorbissrc/floor1.c
	libs/oggvorbis/vorbissrc/floor0.c
	libs/oggvorbis/vorbissrc/res0.c
	libs/oggvorbis/vorbissrc/mapping0.c
	libs/oggvorbis/vorbissrc/registry.c
	libs/oggvorbis/vorbissrc/codebook.c
	libs/oggvorbis/vorbissrc/sharedbook.c
	libs/oggvorbis/vorbissrc/lookup.c
	libs/oggvorbis/vorbissrc/bitrate.c
	libs/oggvorbis/vorbissrc/vorbisfile.c)

file(GLOB SYS_INCLUDES sys/*.h)
file(GLOB SYS_SOURCES sys/*.cpp)

file(GLOB UI_INCLUDES ui/*.h)
file(GLOB UI_SOURCES ui/*.cpp)

file(GLOB UI_LUA_INCLUDES ui-lua/*.h)
file(GLOB UI_LUA_SOURCES ui-lua/*.cpp)

set(LUA_INCLUDES
	libs/lua/src/lapi.h
	libs/lua/src/lauxlib.h
	libs/lua/src/lcode.h
	libs/lua/src/lctype.h
	libs/lua/src/ldebug.h
	libs/lua/src/ldo.h
	libs/lua/src/lfunc.h
	libs/lua/src/lgc.h
	libs/lua/src/llex.h
	libs/lua/src/llimits.h
	libs/lua/src/lmem.h
	libs/lua/src/lobject.h
	libs/lua/src/lopcodes.h
	libs/lua/src/lparser.h
	libs/lua/src/lstate.h
	libs/lua/src/lstring.h
	libs/lua/src/ltable.h
	libs/lua/src/ltm.h
	libs/lua/src/lua.h
	libs/lua/src/luaconf.h
	libs/lua/src/lualib.h
	libs/lua/src/lundump.h
	libs/lua/src/lvm.h
	libs/lua/src/lzio.h
	)
	
set(LUA_SOURCES
	libs/lua/src/lapi.c
	libs/lua/src/lauxlib.c
	libs/lua/src/lbaselib.cpp
	libs/lua/src/lbitlib.c
	libs/lua/src/lcode.c
	libs/lua/src/lcorolib.c
	libs/lua/src/lctype.c
	libs/lua/src/ldblib.c
	libs/lua/src/ldebug.c
	libs/lua/src/ldo.c
	libs/lua/src/ldump.c
	libs/lua/src/lfunc.c
	libs/lua/src/lgc.c
	#linit.c
	libs/lua/src/liolib.c
	libs/lua/src/llex.c
	libs/lua/src/lmathlib.c
	libs/lua/src/lmem.c
	libs/lua/src/loadlib.c
	libs/lua/src/lobject.c
	libs/lua/src/lopcodes.c
	libs/lua/src/loslib.c
	libs/lua/src/lparser.c
	libs/lua/src/lstate.c
	libs/lua/src/lstring.c
	libs/lua/src/lstrlib.c
	libs/lua/src/ltable.c
	libs/lua/src/ltablib.c
	libs/lua/src/ltm.c
	#libs/lua/src/lua.c
	libs/lua/src/lundump.c
	libs/lua/src/lvm.c
	libs/lua/src/lzio.c
	)
	
set(LUAWRAPPER_INCLUDES
	libs/luawrapper/luawrapper.hpp
	libs/luawrapper/luawrapperutil.hpp
	)
	
file(GLOB LUASOCKET_INCLUDES libs/luasocket/src/*.h)

set(LUASOCKET_SOURCES
	libs/luasocket/src/luasocket.c
	libs/luasocket/src/timeout.c
	libs/luasocket/src/buffer.c
	libs/luasocket/src/io.c
	libs/luasocket/src/auxiliar.c
	libs/luasocket/src/options.c
	libs/luasocket/src/inet.c
	libs/luasocket/src/tcp.c
	libs/luasocket/src/udp.c
	libs/luasocket/src/except.c
	libs/luasocket/src/select.c
	# Modules belonging mime-core
	#$(COMPAT)/compat-5.1.o \
	libs/luasocket/src/mime.c

	libs/luasocket/src/buffer.c
	libs/luasocket/src/auxiliar.c
	libs/luasocket/src/options.c
	libs/luasocket/src/timeout.c
	libs/luasocket/src/io.c
	)
	
if(WIN32)
	list(APPEND LUASOCKET_SOURCES libs/luasocket/src/wsocket.c)
else()
	list(APPEND LUASOCKET_SOURCES libs/luasocket/src/unix.c)
	list(APPEND LUASOCKET_SOURCES libs/luasocket/src/usocket.c)
endif()


set(GAME_INCLUDES
	game/AF.h
	game/AFEntity.h
	game/Actor.h
	game/BrittleFracture.h
	game/Camera.h
	#game/EndLevel.h
	game/Entity.h
	game/Fx.h
	game/Game.h
	game/GameEdit.h
	game/Game_local.h
	game/Grabber.h
	game/IK.h
	game/Item.h
	game/Light.h
	game/Misc.h
	game/Moveable.h
	game/Mover.h
	game/MultiplayerGame.h
	game/Player.h
	game/PlayerIcon.h
	game/PlayerView.h
	game/Portal.h
	game/Projectile.h
	game/Pvs.h
	game/SecurityCamera.h
	game/SmokeParticles.h
	game/Sound.h
	game/Target.h
	game/Trigger.h
	game/Weapon.h
	game/WorldSpawn.h)
	
set(GAME_SOURCES
	game/AF.cpp
	game/AFEntity.cpp
	game/Actor.cpp
	game/BrittleFracture.cpp
	game/Camera.cpp
	#game/EndLevel.cpp
	game/Entity.cpp
	game/Fx.cpp
	game/GameEdit.cpp
	game/Game_local.cpp
	game/Game_network.cpp
	game/Grabber.cpp
	game/IK.cpp
	game/Item.cpp
	game/Light.cpp
	game/Misc.cpp
	game/Moveable.cpp
	game/Mover.cpp
	game/MultiplayerGame.cpp
	game/Player.cpp
	game/PlayerIcon.cpp
	game/PlayerView.cpp
	game/Portal.cpp
	game/Projectile.cpp
	game/Pvs.cpp
	game/SecurityCamera.cpp
	game/SmokeParticles.cpp
	game/Sound.cpp
	game/Target.cpp
	game/Trigger.cpp
	game/Weapon.cpp
	game/WorldSpawn.cpp)

file(GLOB GAME_AI_INCLUDES game/ai/*.h)
file(GLOB GAME_AI_SOURCES game/ai/*.cpp)

file(GLOB GAME_ANIM_INCLUDES game/anim/*.h)
file(GLOB GAME_ANIM_SOURCES game/anim/*.cpp)

file(GLOB GAME_GAMESYS_INCLUDES game/gamesys/*.h)
set(GAME_GAMESYS_SOURCES
	#game/gamesys/Callbacks.cpp
	game/gamesys/Class.cpp
	game/gamesys/Event.cpp
	game/gamesys/SaveGame.cpp
	game/gamesys/SysCmds.cpp
	game/gamesys/SysCvar.cpp
	#game/gamesys/TypeInfo.cpp
	)

file(GLOB GAME_PHYSICS_INCLUDES game/physics/*.h)
file(GLOB GAME_PHYSICS_SOURCES game/physics/*.cpp)

file(GLOB GAME_SCRIPT_INCLUDES game/script/*.h)
file(GLOB GAME_SCRIPT_SOURCES game/script/*.cpp)


file(GLOB COMPILER_INCLUDES tools/compilers/*.h)

file(GLOB COMPILER_AAS_INCLUDES tools/compilers/aas/*.h)
file(GLOB COMPILER_AAS_SOURCES tools/compilers/aas/*.cpp)

file(GLOB COMPILER_DMAP_INCLUDES tools/compilers/dmap/*.h)
file(GLOB COMPILER_DMAP_SOURCES tools/compilers/dmap/*.cpp)

file(GLOB COMPILER_ROQVQ_INCLUDES tools/compilers/roqvq/*.h)
file(GLOB COMPILER_ROQVQ_SOURCES tools/compilers/roqvq/*.cpp)

set(TOOLS_INCLUDES tools/edit_public.h)

file(GLOB TOOLS_COMAFX_INCLUDES tools/comafx/*.h)
file(GLOB TOOLS_COMAFX_SOURCES tools/comafx/*.cpp)

set(TOOLS_COMMON_INCLUDES
	tools/common/ColorButton.h
	tools/common/DialogHelpers.h
	tools/common/MaskEdit.h
	tools/common/OpenFileDialog.h
	tools/common/PropertyGrid.h
	tools/common/RegistryOptions.h
	#tools/common/RenderBumpFlatDialog.h
	tools/common/RollupPanel.h
	tools/common/SpinButton.h)

set(TOOLS_COMMON_SOURCES
	tools/common/AlphaPopup.cpp
	tools/common/ColorButton.cpp
	tools/common/MaskEdit.cpp
	tools/common/OpenFileDialog.cpp
	tools/common/PropertyGrid.cpp
	tools/common/RegistryOptions.cpp
	#tools/common/RenderBumpFlatDialog.cpp
	tools/common/RollupPanel.cpp
	tools/common/SpinButton.cpp)

file(GLOB TOOLS_COMMON_PROPTREE_INCLUDES tools/common/PropTree/*.h)
file(GLOB TOOLS_COMMON_PROPTREE_SOURCES tools/common/PropTree/*.cpp)

file(GLOB EDITOR_AF_INCLUDES tools/af/*.h)
file(GLOB EDITOR_AF_SOURCES tools/af/*.cpp)

set(EDITOR_DECL_INCLUDES
	tools/decl/DialogDeclBrowser.h
	tools/decl/DialogDeclEditor.h
	tools/decl/DialogDeclNew.h)
	
set(EDITOR_DECL_SOURCES
	tools/decl/DialogDeclBrowser.cpp
	tools/decl/DialogDeclEditor.cpp
	tools/decl/DialogDeclNew.cpp)
	
file(GLOB EDITOR_DEBUGGER_INCLUDES tools/debugger/*.h)
file(GLOB EDITOR_DEBUGGER_SOURCES tools/debugger/*.cpp)

file(GLOB EDITOR_GUIED_INCLUDES tools/guied/*.h)
set(EDITOR_GUIED_SOURCES
	tools/guied/GEApp.cpp
	tools/guied/GECheckInDlg.cpp
	tools/guied/GEDeleteModifier.cpp
	tools/guied/GEHideModifier.cpp
	tools/guied/GEInsertModifier.cpp
	tools/guied/GEItemPropsDlg.cpp
	tools/guied/GEItemScriptsDlg.cpp
	tools/guied/GEKeyValueModifier.cpp
	tools/guied/GEModifier.cpp
	tools/guied/GEModifierGroup.cpp
	tools/guied/GEModifierStack.cpp
	tools/guied/GEMoveModifier.cpp
	tools/guied/GENavigator.cpp
	tools/guied/GEOptions.cpp
	tools/guied/GEOptionsDlg.cpp
	tools/guied/GEProperties.cpp
	tools/guied/GEPropertyPage.cpp
	tools/guied/GESelectionMgr.cpp
	tools/guied/GESizeModifier.cpp
	tools/guied/GEStateModifier.cpp
	tools/guied/GEStatusBar.cpp
	tools/guied/GETransformer.cpp
	tools/guied/GEViewer.cpp
	tools/guied/GEWindowWrapper.cpp
	#tools/guied/GEWindowWrapper_stub.cpp
	tools/guied/GEWorkspace.cpp
	tools/guied/GEWorkspaceFile.cpp
	tools/guied/GEZOrderModifier.cpp
	tools/guied/guied.cpp)

file(GLOB EDITOR_MATERIALEDITOR_INCLUDES tools/materialeditor/*.h)
file(GLOB EDITOR_MATERIALEDITOR_SOURCES tools/materialeditor/*.cpp)

file(GLOB EDITOR_PARTICLE_INCLUDES tools/particle/*.h)
file(GLOB EDITOR_PARTICLE_SOURCES tools/particle/*.cpp)

file(GLOB EDITOR_PDA_INCLUDES tools/pda/*.h)
file(GLOB EDITOR_PDA_SOURCES tools/pda/*.cpp)

file(GLOB EDITOR_RADIANT_INCLUDES tools/radiant/*.h)
file(GLOB EDITOR_RADIANT_SOURCES tools/radiant/*.cpp)

file(GLOB EDITOR_SCRIPT_INCLUDES tools/script/*.h)
file(GLOB EDITOR_SCRIPT_SOURCES tools/script/*.cpp)

file(GLOB EDITOR_SOUND_INCLUDES tools/sound/*.h)
file(GLOB EDITOR_SOUND_SOURCES tools/sound/*.cpp)
	


	
	
if(USE_QT_WINDOWING)
	
	set(WIN32_INCLUDES
		sys/win32/win_local.h
		sys/win32/rc/doom_resource.h
		)

	set(WIN32_SOURCES
		sys/win32/win_cpu.cpp
		sys/win32/win_main.cpp
		sys/win32/win_net.cpp
		sys/win32/win_shared.cpp
		sys/win32/win_snd.cpp
		#sys/win32/win_syscon.cpp
		sys/win32/win_taskkeyhook.cpp
		)
	
	set(WIN32_RESOURCES sys/win32/rc/res/doom.ico)
	
else()
	file(GLOB_RECURSE WIN32_INCLUDES sys/win32/*.h)
	file(GLOB_RECURSE WIN32_SOURCES sys/win32/*.cpp)
	
	if(ANGLE)
		list(REMOVE_ITEM WIN32_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/sys/win32/win_glimp.cpp)
	else()
		list(REMOVE_ITEM WIN32_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/sys/win32/win_egl.cpp)
	endif()

	set(WIN32_RESOURCES
		sys/win32/rc/res/BEVEL.BMP
		sys/win32/rc/res/BITMAP2.BMP
		sys/win32/rc/res/BMP00001.BMP
		sys/win32/rc/res/BMP0002.BMP
		sys/win32/rc/res/DEFTEX.WAL
		sys/win32/rc/res/ENDCAP.BMP
		sys/win32/rc/res/GetString.htm
		sys/win32/rc/res/IBEVEL.BMP
		sys/win32/rc/res/IENDCAP.BMP
		sys/win32/rc/res/MEFileToolbar.bmp
		sys/win32/rc/res/MEtoolbar.bmp
		sys/win32/rc/res/MaterialEditor.ico
		sys/win32/rc/res/PropTree.rc2
		sys/win32/rc/res/Q.BMP
		sys/win32/rc/res/RADIANT3.GIF
		sys/win32/rc/res/Radiant.ico
		sys/win32/rc/res/RadiantDoc.ico
		sys/win32/rc/res/TOOLBAR1.BMP
		sys/win32/rc/res/TOOLBAR2.BMP
		sys/win32/rc/res/Toolbar.bmp
		sys/win32/rc/res/VIEWDEFA.BMP
		sys/win32/rc/res/VIEWOPPO.BMP
		sys/win32/rc/res/bmp00002.bmp
		sys/win32/rc/res/bmp00003.bmp
		sys/win32/rc/res/bmp00004.bmp
		sys/win32/rc/res/bmp00005.bmp
		sys/win32/rc/res/cchsb.bmp
		sys/win32/rc/res/ccrgb.bmp
		sys/win32/rc/res/dbg_back.bmp
		sys/win32/rc/res/dbg_breakpoint.ico
		sys/win32/rc/res/dbg_current.ico
		sys/win32/rc/res/dbg_currentline.ico
		sys/win32/rc/res/dbg_empty.ico
		sys/win32/rc/res/dbg_open.bmp
		sys/win32/rc/res/dbg_toolbar.bmp
		sys/win32/rc/res/doom.ico
		sys/win32/rc/res/fpoint.cur
		sys/win32/rc/res/fxed_link.ico
		sys/win32/rc/res/fxed_toolbar.bmp
		sys/win32/rc/res/fxeditor.ico
		sys/win32/rc/res/guied.ico
		sys/win32/rc/res/guied_collapse.ico
		sys/win32/rc/res/guied_expand.ico
		sys/win32/rc/res/guied_hand.cur
		sys/win32/rc/res/guied_nav_visible.ico
		sys/win32/rc/res/guied_nav_visibledisabled.ico
		sys/win32/rc/res/guied_scripts.ico
		sys/win32/rc/res/guied_scripts_white.ico
		sys/win32/rc/res/guied_viewer_toolbar.bmp
		sys/win32/rc/res/icon2.ico
		sys/win32/rc/res/logo_sm3dfx.bmp
		sys/win32/rc/res/matedtree.bmp
		sys/win32/rc/res/me_disabled_icon.ico
		sys/win32/rc/res/me_enabled.ico
		sys/win32/rc/res/me_off_icon.ico
		sys/win32/rc/res/me_on_icon.ico
		sys/win32/rc/res/qe3.ico
		sys/win32/rc/res/shaderbar.bmp
		sys/win32/rc/res/shaderdoc.ico
		sys/win32/rc/res/shaderframe.ico
		sys/win32/rc/res/spliter.cur
		)
endif()


if(USE_MFC_TOOLS)
	list(APPEND WIN32_RESOURCES sys/win32/rc/doom.rc)
else()
	list(APPEND WIN32_RESOURCES sys/win32/rc/doom_nomfc.rc)
endif()


set(POSIX_INCLUDES
    sys/posix/posix_public.h
    )

set(POSIX_SOURCES
    sys/posix/posix_net.cpp
    sys/posix/posix_main.cpp
    sys/posix/posix_signal.cpp
    sys/posix/posix_threads.cpp
    )

set(LINUX_INCLUDES
    sys/linux/sound.h
    )

set(LINUX_SOURCES
    sys/linux/linux_main.cpp
    sys/linux/sound.cpp
    sys/linux/sound_pulse.cpp
    sys/linux/stack.cpp
    )

set(SDL_INCLUDES
    sys/sdl/sdl_local.h
    )

set(SDL_SOURCES
    sys/sdl/sdl_events.cpp
    sys/sdl/sdl_glimp.cpp
    )


if(USE_QT_TOOLS)

	#set(QT_USE_QTOPENGL TRUE)
	#set(QT_USE_IMPORTED_TARGETS TRUE)
	
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
	#set(CMAKE_AUTOMOC ON)
	
	if(MSVC)
		#set(CMAKE_PREFIX_PATH "C:\\Qt\\Qt5.2.1-src\\5.2.1\\msvc2010_opengl")
		set(CMAKE_PREFIX_PATH "C:\\Qt\\Qt5.2.1-src\\qtbase")
		
		if(CMAKE_CL_64)
			set(CMAKE_LIBRARY_PATH "C:\\Program Files (x86)\\Windows Kits\\8.0\\Lib\\win8\\um\\x64")
		else()
			#set(CMAKE_LIBRARY_PATH "C:\\Program Files (x86)\\Windows Kits\\8.0\\Lib\\win8\\um\\x86")
			set(CMAKE_LIBRARY_PATH "C:\\Program Files (x86)\\Microsoft SDKs\\Windows\\v7.1A\\Lib")
		endif()
	endif()
	
	find_package(Qt5Core REQUIRED)
	find_package(Qt5Widgets REQUIRED)
	find_package(Qt5OpenGL REQUIRED)
	
	include_directories(${Qt5Core_INCLUDE_DIRS})
	add_definitions(${Qt5Core_DEFINITIONS})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}")
	
	include_directories(${Qt5Widgets_INCLUDE_DIRS})
	add_definitions(${Qt5Widgets_DEFINITIONS})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
	
	include_directories(${Qt5OpenGL_INCLUDE_DIRS})
	add_definitions(${Qt5OpenGL_DEFINITIONS})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5OpenGL_EXECUTABLE_COMPILE_FLAGS}")
	
	set(QT_LIBRARIES
		Qt5::Widgets
		Qt5::OpenGL)
	
	
	set(EDITOR_QTSTRING_INCLUDES
		tools/qtstringeditor/QtStringEditor.h)
		
	set(EDITOR_QTSTRING_SOURCES
		tools/qtstringeditor/main.cpp)
		
	set(EDITOR_QTSTRING_RESOURCES
		tools/qtstringeditor/QtStringEditor.qrc)
		
	set(EDITOR_QTSTRING_FORMS
		tools/qtstringeditor/QtStringEditor.ui)
		
	qt5_wrap_cpp(EDITOR_QTSTRING_INCLUDES_MOC ${EDITOR_QTSTRING_INCLUDES})
	qt5_wrap_ui(EDITOR_QTSTRING_FORMS_INCLUDES ${EDITOR_QTSTRING_FORMS})
	qt5_add_resources(EDITOR_QTSTRING_RESOURCES_RCC ${EDITOR_QTSTRING_RESOURCES})
	
	source_group("tools\\qtstringeditor\\Header Files" FILES ${EDITOR_QTSTRING_INCLUDES})
	source_group("tools\\qtstringeditor\\Source Files" FILES ${EDITOR_QTSTRING_SOURCES})
	source_group("tools\\qtstringeditor\\Resource Files" FILES ${EDITOR_QTSTRING_RESOURCES})
	source_group("tools\\qtstringeditor\\Form Files" FILES ${EDITOR_QTSTRING_FORMS})
	
	source_group("tools\\qtstringeditor\\Generated Files" FILES ${EDITOR_QTSTRING_INCLUDES_MOC})
	source_group("tools\\qtstringeditor\\Generated Files" FILES ${EDITOR_QTSTRING_FORMS_INCLUDES})
	source_group("tools\\qtstringeditor\\Generated Files" FILES ${EDITOR_QTSTRING_RESOURCES_RCC})
	
	
	
	file(GLOB EDITOR_QTRADIANT_INCLUDES	tools/qtradiant/src/*.h)
	file(GLOB EDITOR_QTRADIANT_SOURCES tools/qtradiant/src/*.cpp)
	
	#file(GLOB EDITOR_QTRADIANT_RESOURCES tools/qtradiant/resources/icons/*.png)
	set(EDITOR_QTRADIANT_RESOURCES tools/qtradiant/QtRadiant.qrc)
	file(GLOB EDITOR_QTRADIANT_FORMS tools/qtradiant/src/*.ui)
		
	qt5_wrap_cpp(EDITOR_QTRADIANT_INCLUDES_MOC ${EDITOR_QTRADIANT_INCLUDES})
	qt5_wrap_ui(EDITOR_QTRADIANT_FORMS_INCLUDES ${EDITOR_QTRADIANT_FORMS})
	qt5_add_resources(EDITOR_QTRADIANT_RESOURCES_RCC ${EDITOR_QTRADIANT_RESOURCES})
	
	source_group("tools\\qtradiant\\Header Files" FILES ${EDITOR_QTRADIANT_INCLUDES})
	source_group("tools\\qtradiant\\Source Files" FILES ${EDITOR_QTRADIANT_SOURCES})
	source_group("tools\\qtradiant\\Resource Files" FILES ${EDITOR_QTRADIANT_RESOURCES})
	source_group("tools\\qtradiant\\Form Files" FILES ${EDITOR_QTRADIANT_FORMS})
	
	source_group("tools\\qtradiant\\Generated Files" FILES ${EDITOR_QTRADIANT_INCLUDES_MOC})
	source_group("tools\\qtradiant\\Generated Files" FILES ${EDITOR_QTRADIANT_FORMS_INCLUDES})
	source_group("tools\\qtradiant\\Generated Files" FILES ${EDITOR_QTRADIANT_RESOURCES_RCC})
	
	if(USE_QT_WINDOWING)
		add_definitions(-DUSE_QT_WINDOWING)
	
		file(GLOB QTTEKUUM_INCLUDES sys/qt/*.h)
		file(GLOB QTTEKUUM_SOURCES sys/qt/*.cpp)
		
		set(QTTEKUUM_RESOURCES sys/qt/Tekuum.qrc)
		file(GLOB QTTEKUUM_FORMS sys/qt/*.ui)
			
		qt5_wrap_cpp(QTTEKUUM_INCLUDES_MOC ${QTTEKUUM_INCLUDES})
		qt5_wrap_ui(QTTEKUUM_FORMS_INCLUDES ${QTTEKUUM_FORMS})
		qt5_add_resources(QTTEKUUM_RESOURCES_RCC ${QTTEKUUM_RESOURCES})
		
		source_group("sys\\qt\\Header Files" FILES ${QTTEKUUM_INCLUDES})
		source_group("sys\\qt\\Source Files" FILES ${QTTEKUUM_SOURCES})
		source_group("sys\\qt\\Resource Files" FILES ${QTTEKUUM_RESOURCES})
		source_group("sys\\qt\\Form Files" FILES ${QTTEKUUM_FORMS})
		
		source_group("sys\\qt\\Generated Files" FILES ${QTTEKUUM_INCLUDES_MOC})
		source_group("sys\\qt\\Generated Files" FILES ${QTTEKUUM_FORMS_INCLUDES})
		source_group("sys\\qt\\Generated Files" FILES ${QTTEKUUM_RESOURCES_RCC})
	endif()
	
endif(USE_QT_TOOLS)

source_group("aas" FILES ${AAS_INCLUDES})
source_group("aas" FILES ${AAS_SOURCES})

source_group("cm" FILES ${CM_INCLUDES})
source_group("cm" FILES ${CM_SOURCES})

source_group("framework" FILES ${FRAMEWORK_INCLUDES})
source_group("framework" FILES ${FRAMEWORK_SOURCES})

source_group("framework\\async" FILES ${FRAMEWORK_ASYNC_INCLUDES})
source_group("framework\\async" FILES ${FRAMEWORK_ASYNC_SOURCES})

source_group("renderer" FILES ${RENDERER_INCLUDES})
source_group("renderer" FILES ${RENDERER_SOURCES})

source_group("renderer\\Color" FILES ${RENDERER_COLOR_INCLUDES})
source_group("renderer\\Color" FILES ${RENDERER_COLOR_SOURCES})

source_group("renderer\\DXT" FILES ${RENDERER_DXT_INCLUDES})
source_group("renderer\\DXT" FILES ${RENDERER_DXT_SOURCES})

source_group("renderer\\jobs" FILES ${RENDERER_JOBS_INCLUDES})
source_group("renderer\\jobs" FILES ${RENDERER_JOBS_SOURCES})

source_group("renderer\\jobs\\dynamicshadowvolume" FILES ${RENDERER_JOBS_DYNAMICSHADOWVOLUME_INCLUDES})
source_group("renderer\\jobs\\dynamicshadowvolume" FILES ${RENDERER_JOBS_DYNAMICSHADOWVOLUME_SOURCES})

source_group("renderer\\jobs\\prelightshadowvolume" FILES ${RENDERER_JOBS_PRELIGHTSHADOWVOLUME_INCLUDES})
source_group("renderer\\jobs\\prelightshadowvolume" FILES ${RENDERER_JOBS_PRELIGHTSHADOWVOLUME_SOURCES})

source_group("renderer\\jobs\\staticshadowvolume" FILES ${RENDERER_JOBS_STATICSHADOWVOLUME_INCLUDES})
source_group("renderer\\jobs\\staticshadowvolume" FILES ${RENDERER_JOBS_STATICSHADOWVOLUME_SOURCES})

source_group("renderer\\OpenGL" FILES ${RENDERER_OPENGL_INCLUDES})
source_group("renderer\\OpenGL" FILES ${RENDERER_OPENGL_SOURCES})

source_group("libs\\irrxml" FILES ${IRRXML_INCLUDES})
source_group("libs\\irrxml" FILES ${IRRXML_SOURCES})

source_group("libs\\jpeg-6" FILES ${JPEG_INCLUDES})
source_group("libs\\jpeg-6" FILES ${JPEG_SOURCES})

source_group("libs\\png" FILES ${PNG_INCLUDES})
source_group("libs\\png" FILES ${PNG_SOURCES})

source_group("libs\\etc1" FILES ${ETC1_INCLUDES})
source_group("libs\\etc1" FILES ${ETC1_SOURCES})

source_group("libs\\zlib" FILES ${ZLIB_INCLUDES})
source_group("libs\\zlib" FILES ${ZLIB_SOURCES})

source_group("libs\\glew" FILES ${GLEW_INCLUDES})
source_group("libs\\glew" FILES ${GLEW_SOURCES})

source_group("libs\\freetype" FILES ${FREETYPE_SOURCES})

source_group("sound" FILES ${SOUND_INCLUDES})
source_group("sound" FILES ${SOUND_SOURCES})

source_group("libs\\oggvorbis" FILES ${OGGVORBIS_INCLUDES})
source_group("libs\\oggvorbis" FILES ${OGGVORBIS_SOURCES})

source_group("ui" FILES ${UI_INCLUDES})
source_group("ui" FILES ${UI_SOURCES})

source_group("ui-lua" FILES ${UI_LUA_INCLUDES})
source_group("ui-lua" FILES ${UI_LUA_SOURCES})

source_group("libs\\lua" FILES ${LUA_INCLUDES})
source_group("libs\\lua" FILES ${LUA_SOURCES})

source_group("libs\\luawrapper" FILES ${LUAWRAPPER_INCLUDES})

source_group("libs\\luasocket" FILES ${LUASOCKET_INCLUDES})
source_group("libs\\luasocket" FILES ${LUASOCKET_SOURCES})

source_group("sys" FILES ${SYS_INCLUDES})
source_group("sys" FILES ${SYS_SOURCES})


source_group("game" FILES ${GAME_INCLUDES})
source_group("game" FILES ${GAME_SOURCES})

source_group("game\\ai" FILES ${GAME_AI_INCLUDES})
source_group("game\\ai" FILES ${GAME_AI_SOURCES})

source_group("game\\anim" FILES ${GAME_ANIM_INCLUDES})
source_group("game\\anim" FILES ${GAME_ANIM_SOURCES})

source_group("game\\gamesys" FILES ${GAME_GAMESYS_INCLUDES})
source_group("game\\gamesys" FILES ${GAME_GAMESYS_SOURCES})

source_group("game\\physics" FILES ${GAME_PHYSICS_INCLUDES})
source_group("game\\physics" FILES ${GAME_PHYSICS_SOURCES})

source_group("game\\script" FILES ${GAME_SCRIPT_INCLUDES})
source_group("game\\script" FILES ${GAME_SCRIPT_SOURCES})






#source_group("sys\\win32" FILES ${WIN32_INCLUDES})
#source_group("sys\\win32" FILES ${WIN32_SOURCES})
#source_group("sys\\win32\\Resources" FILES ${WIN32_RESOURCES})

source_group("sys\\posix" FILES ${POSIX_INCLUDES})
source_group("sys\\posix" FILES ${POSIX_SOURCES})

source_group("sys\\common" FILES ${COMMON_INCLUDES})
source_group("sys\\common" FILES ${COMMON_SOURCES})

source_group("sys\\sdl" FILES ${SDL_INCLUDES})
source_group("sys\\sdl" FILES ${SDL_SOURCES})

#if(USE_QT_WINDOWING)
#	source_group("sys\\win32" FILES ${WIN32_INCLUDES})
#	source_group("sys\\qt\\win32" FILES ${WIN32_SOURCES})
#	source_group("sys\\qt\\win32\\Resources" FILES ${WIN32_RESOURCES})
#else()
	source_group("sys\\win32" FILES ${WIN32_INCLUDES})
	source_group("sys\\win32" FILES ${WIN32_SOURCES})
	source_group("sys\\win32\\Resources" FILES ${WIN32_RESOURCES})
#endif()

source_group("tools\\compilers" FILES ${COMPILER_INCLUDES})

source_group("tools\\compilers\\aas" FILES ${COMPILER_AAS_INCLUDES})
source_group("tools\\compilers\\aas" FILES ${COMPILER_AAS_SOURCES})

source_group("tools\\compilers\\dmap" FILES ${COMPILER_DMAP_INCLUDES})
source_group("tools\\compilers\\dmap" FILES ${COMPILER_DMAP_SOURCES})

source_group("tools\\compilers\\roqvq" FILES ${COMPILER_ROQVQ_INCLUDES})
source_group("tools\\compilers\\roqvq" FILES ${COMPILER_ROQVQ_SOURCES})

source_group("tools" FILES ${TOOLS_INCLUDES})

source_group("tools\\comafx" FILES ${TOOLS_COMAFX_INCLUDES})
source_group("tools\\comafx" FILES ${TOOLS_COMAFX_SOURCES})

source_group("tools\\common" FILES ${TOOLS_COMMON_INCLUDES})
source_group("tools\\common" FILES ${TOOLS_COMMON_SOURCES})

source_group("tools\\common\\proptree" FILES ${TOOLS_COMMON_PROPTREE_INCLUDES})
source_group("tools\\common\\proptree" FILES ${TOOLS_COMMON_PROPTREE_SOURCES})

source_group("tools\\af" FILES ${EDITOR_AF_INCLUDES})
source_group("tools\\af" FILES ${EDITOR_AF_SOURCES})

source_group("tools\\debugger" FILES ${EDITOR_DEBUGGER_INCLUDES})
source_group("tools\\debugger" FILES ${EDITOR_DEBUGGER_SOURCES})

source_group("tools\\decl" FILES ${EDITOR_DECL_INCLUDES})
source_group("tools\\decl" FILES ${EDITOR_DECL_SOURCES})

source_group("tools\\guied" FILES ${EDITOR_GUIED_INCLUDES})
source_group("tools\\guied" FILES ${EDITOR_GUIED_SOURCES})

source_group("tools\\materialeditor" FILES ${EDITOR_MATERIALEDITOR_INCLUDES})
source_group("tools\\materialeditor" FILES ${EDITOR_MATERIALEDITOR_SOURCES})

source_group("tools\\particle" FILES ${EDITOR_PARTICLE_INCLUDES})
source_group("tools\\particle" FILES ${EDITOR_PARTICLE_SOURCES})

source_group("tools\\pda" FILES ${EDITOR_PDA_INCLUDES})
source_group("tools\\pda" FILES ${EDITOR_PDA_SOURCES})

source_group("tools\\radiant" FILES ${EDITOR_RADIANT_INCLUDES})
source_group("tools\\radiant" FILES ${EDITOR_RADIANT_SOURCES})

source_group("tools\\script" FILES ${EDITOR_SCRIPT_INCLUDES})
source_group("tools\\script" FILES ${EDITOR_SCRIPT_SOURCES})

source_group("tools\\sound" FILES ${EDITOR_SOUND_INCLUDES})
source_group("tools\\sound" FILES ${EDITOR_SOUND_SOURCES})


if(ANGLE OR GLES2)
	if(ANGLE)
		add_definitions(-DUSE_ANGLE)
		include_directories(libs/angle/include)
	endif()
	
	add_definitions(-DUSE_GLES3)
endif()

set(TEKUUM_INCLUDES
			${AAS_INCLUDES}
			${CM_INCLUDES}
			${FRAMEWORK_INCLUDES}
			${FRAMEWORK_ASYNC_INCLUDES}
			${RENDERER_INCLUDES}
			${RENDERER_COLOR_INCLUDES}
			${RENDERER_DXT_INCLUDES}
			${RENDERER_JOBS_INCLUDES}
			${RENDERER_JOBS_DYNAMICSHADOWVOLUME_INCLUDES}
			${RENDERER_JOBS_PRELIGHTSHADOWVOLUME_INCLUDES}
			${RENDERER_JOBS_STATICSHADOWVOLUME_INCLUDES}
			${RENDERER_OPENGL_INCLUDES}
			${IRRXML_INCLUDES}
			${JPEG_INCLUDES}
			${PNG_INCLUDES}
			${ETC1_INCLUDES}
			${ZLIB_INCLUDES}
			${SOUND_INCLUDES}
			${OGGVORBIS_INCLUDES}
			)
			
set(TEKUUM_SOURCES
			${AAS_SOURCES}
			${CM_SOURCES}
			${FRAMEWORK_SOURCES}
			${FRAMEWORK_ASYNC_SOURCES} 
			${RENDERER_SOURCES}
			${RENDERER_COLOR_SOURCES}
			${RENDERER_DXT_SOURCES}
			${RENDERER_JOBS_SOURCES}
			${RENDERER_JOBS_DYNAMICSHADOWVOLUME_SOURCES}
			${RENDERER_JOBS_PRELIGHTSHADOWVOLUME_SOURCES}
			${RENDERER_JOBS_STATICSHADOWVOLUME_SOURCES}
			${RENDERER_OPENGL_SOURCES}
			${IRRXML_SOURCES}
			${JPEG_SOURCES}
			${PNG_SOURCES}
			${ETC1_SOURCES}
			${ZLIB_SOURCES}
			${FREETYPE_SOURCES}
			${SOUND_SOURCES}
			${OGGVORBIS_SOURCES}
			)
		

if(STANDALONE)
	list(APPEND TEKUUM_INCLUDES
		${UI_LUA_INCLUDES}
		${LUA_INCLUDES}
		${LUAWRAPPER_INCLUDES}
		${LUASOCKET_INCLUDES})

	list(APPEND TEKUUM_SOURCES 
		${UI_LUA_SOURCES}
		${LUA_SOURCES}
		${LUASOCKET_SOURCES})

	add_definitions(-DUSE_LUA)

	include_directories(
			libs/lua/src
			libs/luawrapper)
else()
	list(APPEND TEKUUM_INCLUDES ${UI_INCLUDES})
	list(APPEND TEKUUM_SOURCES ${UI_SOURCES})
endif()
	
if(NOT ANGLE)
	list(APPEND TEKUUM_INCLUDES ${GLEW_INCLUDES})
	list(APPEND TEKUUM_SOURCES ${GLEW_SOURCES})
endif()

add_definitions(-D__DOOM__
				-DGLEW_STATIC
				-DBUILD_FREETYPE
				-DFT2_BUILD_LIBRARY)
				

				
if(MONOLITH)
	list(APPEND TEKUUM_INCLUDES
		${GAME_INCLUDES}
		${GAME_AI_INCLUDES}
		${GAME_ANIM_INCLUDES}
		${GAME_GAMESYS_INCLUDES}
		${GAME_PHYSICS_INCLUDES}
		${GAME_SCRIPT_INCLUDES})
		
	list(APPEND TEKUUM_SOURCES
		${GAME_SOURCES}
		${GAME_AI_SOURCES}
		${GAME_ANIM_SOURCES}
		${GAME_GAMESYS_SOURCES}
		${GAME_PHYSICS_SOURCES}
		${GAME_SCRIPT_SOURCES})
else()
	add_definitions(-D__DOOM_DLL__)
endif()

if(USE_CMDLINE_TOOLS)
	add_definitions(-DUSE_CMDLINE_TOOLS)

	list(APPEND TEKUUM_INCLUDES
		${COMPILER_INCLUDES}
		${COMPILER_AAS_INCLUDES}
		${COMPILER_DMAP_INCLUDES}
		${COMPILER_ROQVQ_INCLUDES})
		
	list(APPEND TEKUUM_SOURCES
		${COMPILER_AAS_SOURCES}
		${COMPILER_DMAP_SOURCES}
		${COMPILER_ROQVQ_SOURCES})
endif()

include_directories(
			idlib
			libs/curl/include
			#libs/openal/include
			libs/glew/include
			libs/freetype/include
			libs/zlib)
			
link_directories(
			${CMAKE_CURRENT_SOURCE_DIR}/libs/curl/lib
			${CMAKE_CURRENT_SOURCE_DIR}/libs/openal/lib)

if(MSVC)

	# if(MSVC_VERSION EQUAL 1700)
		# set(DirectX_LIBRARIES
			# dinput8
			# dsound
			# dxguid
			# #dxerr
			# Xinput
			# )
		# add_definitions(-DUSE_XINPUT)
	# else()
		find_package(DirectX REQUIRED)
		include_directories(${DirectX_INCLUDE_DIR})
		add_definitions(-DUSE_XINPUT)
	# endif()
	
	if(USE_MFC_TOOLS)
		# find_package(MFC REQUIRED)
		# if(MFC_FOUND)
			# include_directories(${MFC_INCLUDE_DIR})
			# link_directories(${MFC_LIBRARY_DIR})
		# endif()
		
		# find_package(ATL REQUIRED)
		# if(ATL_FOUND)
			# include_directories(${ATL_INCLUDE_DIR})
			# link_directories(${ATL_LIBRARY_DIR})
			# add_definitions(-DUSE_MFC6_WITH_ATL7)
		# endif()
		
		include_directories(libs/atlmfc/include)
		if(CMAKE_CL_64)
			link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/atlmfc/lib/amd64)
		else(CMAKE_CL_64)
			link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/atlmfc/lib)
		endif(CMAKE_CL_64)
		
		add_definitions(-DUSE_MFC_TOOLS)
		
		list(APPEND TEKUUM_INCLUDES
			${TOOLS_INCLUDES}
			${TOOLS_COMAFX_INCLUDES}
			${TOOLS_COMMON_INCLUDES}
			${TOOLS_COMMON_PROPTREE_INCLUDES}
			
			${EDITOR_AF_INCLUDES}
			#${EDITOR_DEBUGGER_INCLUDES}
			${EDITOR_DECL_INCLUDES}
			#${EDITOR_GUIED_INCLUDES}
			#${EDITOR_MATERIALEDITOR_INCLUDES}
			${EDITOR_PARTICLE_INCLUDES}
			${EDITOR_PDA_INCLUDES}
			${EDITOR_RADIANT_INCLUDES}
			${EDITOR_SCRIPT_INCLUDES}
			${EDITOR_SOUND_INCLUDES})
			
		list(APPEND TEKUUM_SOURCES
			${TOOLS_COMAFX_SOURCES}
			${TOOLS_COMMON_SOURCES}
			${TOOLS_COMMON_PROPTREE_SOURCES}
			
			${EDITOR_AF_SOURCES}
			#${EDITOR_DEBUGGER_SOURCES}
			${EDITOR_DECL_SOURCES}
			#${EDITOR_GUIED_SOURCES}
			#${EDITOR_MATERIALEDITOR_SOURCES}
			${EDITOR_PARTICLE_SOURCES}
			${EDITOR_PDA_SOURCES}
			${EDITOR_RADIANT_SOURCES}
			${EDITOR_SCRIPT_SOURCES}
			${EDITOR_SOUND_SOURCES})
		
	elseif(USE_QT_TOOLS)
	
		add_definitions(-DUSE_QT_TOOLS) # ${Qt5_DEFINITIONS})
		#include(${QT_USE_FILE})
		
		# As all intermediate files are generated outside the source tree,
		# the header file generated by uic will not be located by the compiler.
		# In order to handle this, the build directory needs to be added to the list of include directories.
		#include_directories(${CMAKE_CURRENT_BINARY_DIR})
	
		list(APPEND TEKUUM_INCLUDES
			${TOOLS_INCLUDES}
			
			${QTTEKUUM_INCLUDES}
			${QTTEKUUM_SOURCES}
			${QTTEKUUM_RESOURCES}
			
			${EDITOR_QTRADIANT_INCLUDES}
			${EDITOR_QTRADIANT_SOURCES}
			${EDITOR_QTRADIANT_RESOURCES}
			${EDITOR_QTRADIANT_FORMS}
			
			${EDITOR_QTRADIANT_INCLUDES_MOC}
			${EDITOR_QTRADIANT_FORMS_INCLUDES}
			${EDITOR_QTRADIANT_RESOURCES_RCC}
			
			${EDITOR_QTSTRING_INCLUDES}
			${EDITOR_QTSTRING_SOURCES}
			${EDITOR_QTSTRING_RESOURCES}
			${EDITOR_QTSTRING_FORMS}
			
			${EDITOR_QTSTRING_INCLUDES_MOC}
			${EDITOR_QTSTRING_FORMS_INCLUDES}
			${EDITOR_QTSTRING_RESOURCES_RCC}
			)
			
		if(USE_QT_WINDOWING)
			list(APPEND TEKUUM_SOURCES
				${QTTEKUUM_INCLUDES}
				${QTTEKUUM_SOURCES}
				${QTTEKUUM_RESOURCES}
				${QTTEKUUM_FORMS}
				
				${QTTEKUUM_INCLUDES_MOC}
				${QTTEKUUM_FORMS_INCLUDES}
				${QTTEKUUM_RESOURCES_RCC})
		endif(USE_QT_WINDOWING)
	endif()
	
	list(APPEND TEKUUM_INCLUDES
		${SYS_INCLUDES}
		${WIN32_INCLUDES})
		
	list(APPEND TEKUUM_SOURCES
		${SYS_SOURCES}
		${WIN32_SOURCES})
		

	list(REMOVE_DUPLICATES TEKUUM_SOURCES)
	
	set(TEKUUM_PRECOMPILED_SOURCES ${TEKUUM_SOURCES})
	list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${JPEG_SOURCES} ${PNG_SOURCES} ${ETC1_SOURCES} ${ZLIB_SOURCES} ${GLEW_SOURCES} ${FREETYPE_SOURCES} ${OGGVORBIS_SOURCES} ${LUA_SOURCES} ${LUASOCKET_SOURCES})
	list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/game/gamesys/TypeInfo.cpp)
	list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ui-lua/Lua_init.cpp)
	list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/DXT/DXTDecoder.cpp)
	list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/DXT/DXTEncoder.cpp)
	list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/DXT/DXTEncoder_SSE2.cpp)
	list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/jobs/dynamicshadowvolume/DynamicShadowVolume.cpp)
	list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/jobs/prelightshadowvolume/PreLightShadowVolume.cpp)
	list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/jobs/staticshadowvolume/StaticShadowVolume.cpp)
	list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/jobs/ShadowShared.cpp)
	list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/RenderLog.cpp)
	
	if(USE_QT_WINDOWING)
		list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${QTTEKUUM_SOURCES})
		list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${QTTEKUUM_INCLUDES_MOC})
		list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${QTTEKUUM_RESOURCES})
		list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${QTTEKUUM_RESOURCES_RCC})
	endif()
	
	#list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${TOOLS_COMAFX_SOURCES})
	# list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${TOOLS_COMMON_SOURCES})
	# list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${TOOLS_COMMON_PROPTREE_SOURCES})
	
	# list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${EDITOR_AF_SOURCES})
	# #list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES #${EDITOR_DEBUGGER_SOURCES})
	# list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${EDITOR_DECL_SOURCES})
	# #list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES #${EDITOR_GUIED_SOURCES})
	# #list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES #${EDITOR_MATERIALEDITOR_SOURCES})
	# list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${EDITOR_PARTICLE_SOURCES})
	# list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${EDITOR_PDA_SOURCES})
	# list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${EDITOR_RADIANT_SOURCES})
	# list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${EDITOR_SCRIPT_SOURCES})
	# list(REMOVE_ITEM TEKUUM_PRECOMPILED_SOURCES ${EDITOR_SOUND_SOURCES})
	
	foreach( src_file ${TEKUUM_PRECOMPILED_SOURCES} )
		#message(STATUS "/Yuprecompiled.h for ${src_file}")
		set_source_files_properties(
			${src_file}
			PROPERTIES
			COMPILE_FLAGS "/Yuprecompiled.h"
			)
	endforeach()
	
	set_source_files_properties(framework/precompiled.cpp
		PROPERTIES
		COMPILE_FLAGS "/Ycprecompiled.h"
		)
	
	if(USE_QT_WINDOWING)
		add_executable(Tekuum ${TEKUUM_INCLUDES} ${TEKUUM_SOURCES} ${QTTEKUUM_INCLUDES} ${QTTEKUUM_SOURCES} ${QTTEKUUM_RESOURCES})
	else()
		add_executable(Tekuum WIN32 ${TEKUUM_INCLUDES} ${TEKUUM_SOURCES} ${WIN32_RESOURCES})
	endif()
		
	# add_custom_command(TARGET Tekuum
                   # PRE_BUILD
				   # COMMAND nmake ARGS /f Makefile.vc6 CFG=release
				   # WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/libs/curl/lib
                   # COMMENT "Compiling libcURL")
		
	if(ANGLE)
		add_dependencies(Tekuum idlib libANGLE)
		target_link_libraries(Tekuum
			idlib
			libANGLE
			${DirectX_LIBRARIES}
			${QT_LIBRARIES}
			#libcurl 
			#openal32 
			#opengl32
			glu32
			dbghelp
			eaxguid
			iphlpapi
			winmm
			ws2_32.lib)
	else()
		add_dependencies(Tekuum idlib)
			target_link_libraries(Tekuum
			idlib
			${DirectX_LIBRARIES}
			${QT_LIBRARIES}
			#libcurl 
			#openal32 
			opengl32
			glu32
			dbghelp
			eaxguid
			iphlpapi
			winmm
			ws2_32.lib)
	endif()

elseif(UNIX)

    if(SDL2)
        find_package(SDL2 REQUIRED)
        include_directories(${SDL2_INCLUDE_DIR})
        set(SDLx_LIBRARY ${SDL2_LIBRARY})
    else()
        find_package(SDL REQUIRED)
        include_directories(${SDL_INCLUDE_DIR})
        set(SDLx_LIBRARY ${SDL_LIBRARY})
    endif()

    add_definitions(-DUSE_SDL)
    add_definitions(-DUSE_SDL_ASYNC)
    add_definitions(-DUSE_SOUND_PULSE)
	add_definitions(-DUSE_EXCEPTIONS)

	if(MESA)
		add_definitions(-DUSE_GLES3)
		add_definitions(-DUSE_MESA)
	endif()

    #find_package(PulseAudio REQUIRED)

    list(REMOVE_DUPLICATES TEKUUM_SOURCES)

    list(APPEND TEKUUM_INCLUDES
        ${SYS_INCLUDES}
        ${POSIX_INCLUDES}
        ${SDL_INCLUDES}
        ${LINUX_INCLUDES})

    list(APPEND TEKUUM_SOURCES
        ${SYS_SOURCES}
        ${POSIX_SOURCES}
        ${SDL_SOURCES}
        ${LINUX_SOURCES})

	add_executable(tekuum ${TEKUUM_INCLUDES} ${TEKUUM_SOURCES})

	target_link_libraries(tekuum
        idlib
        GL
        pthread
        dl
        rt
        pulse
        pulse-simple
        ${SDLx_LIBRARY}
        )

endif()
