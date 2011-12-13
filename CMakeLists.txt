cmake_minimum_required(VERSION 2.8)

project(Techyon)

if(MSVC)
	#set(CMAKE_USE_RELATIVE_PATHS ON)

	add_definitions(-DWIN32 
					-D_WINDOWS
					-D_CRT_SECURE_NO_DEPRECATE
					-D_CRT_NONSTDC_NO_DEPRECATE
					-D_CRT_SECURE_NO_WARNINGS
					-D_USE_32BIT_TIME_T
					-D_MBCS
					-D_AFXDLL)
endif(MSVC)

add_subdirectory(idlib)

file(GLOB_RECURSE CM_INCLUDES cm/*.h)
file(GLOB_RECURSE CM_SOURCES cm/*.cpp)

file(GLOB FRAMEWORK_INCLUDES framework/*.h)
file(GLOB FRAMEWORK_SOURCES framework/*.cpp)

file(GLOB FRAMEWORK_ASYNC_INCLUDES framework/async/*.h)
file(GLOB FRAMEWORK_ASYNC_SOURCES framework/async/*.cpp)

file(GLOB RENDERER_INCLUDES renderer/*.h)
file(GLOB RENDERER_SOURCES renderer/*.cpp)

set(JPEG_INCLUDES
	renderer/jpeg-6/jchuff.h
	renderer/jpeg-6/jconfig.h
	renderer/jpeg-6/jdct.h
	renderer/jpeg-6/jdhuff.h
	renderer/jpeg-6/jerror.h
	renderer/jpeg-6/jinclude.h
	renderer/jpeg-6/jmemsys.h
	renderer/jpeg-6/jmorecfg.h
	renderer/jpeg-6/jpegint.h
	renderer/jpeg-6/jpeglib.h
	renderer/jpeg-6/jversion.h)
	
set(JPEG_SOURCES
	renderer/jpeg-6/jcapimin.c
	renderer/jpeg-6/jcapistd.c
	renderer/jpeg-6/jccoefct.c
	renderer/jpeg-6/jccolor.c
	renderer/jpeg-6/jcdctmgr.c
	renderer/jpeg-6/jchuff.c
	renderer/jpeg-6/jcinit.c
	renderer/jpeg-6/jcmainct.c
	renderer/jpeg-6/jcmarker.c
	renderer/jpeg-6/jcmaster.c
	renderer/jpeg-6/jcomapi.c
	renderer/jpeg-6/jcparam.c
	renderer/jpeg-6/jcphuff.c
	renderer/jpeg-6/jcprepct.c
	renderer/jpeg-6/jcsample.c
	renderer/jpeg-6/jctrans.c
	renderer/jpeg-6/jdapimin.c
	renderer/jpeg-6/jdapistd.c
	renderer/jpeg-6/jdatadst.c
	renderer/jpeg-6/jdatasrc.c
	renderer/jpeg-6/jdcoefct.c
	renderer/jpeg-6/jdcolor.c
	renderer/jpeg-6/jddctmgr.c
	renderer/jpeg-6/jdhuff.c
	renderer/jpeg-6/jdinput.c
	renderer/jpeg-6/jdmainct.c
	renderer/jpeg-6/jdmarker.c
	renderer/jpeg-6/jdmaster.c
	renderer/jpeg-6/jdmerge.c
	renderer/jpeg-6/jdphuff.c
	renderer/jpeg-6/jdpostct.c
	renderer/jpeg-6/jdsample.c
	renderer/jpeg-6/jdtrans.c
	renderer/jpeg-6/jerror.c
	renderer/jpeg-6/jfdctflt.c
	renderer/jpeg-6/jfdctfst.c
	renderer/jpeg-6/jfdctint.c
	renderer/jpeg-6/jidctflt.c
	renderer/jpeg-6/jidctfst.c
	renderer/jpeg-6/jidctint.c
	renderer/jpeg-6/jidctred.c
	#renderer/jpeg-6/jload.c
	renderer/jpeg-6/jmemmgr.c
	renderer/jpeg-6/jmemnobs.c
	#renderer/jpeg-6/jpegtran.c
	renderer/jpeg-6/jquant1.c
	renderer/jpeg-6/jquant2.c
	renderer/jpeg-6/jutils.c)

set(SOUND_INCLUDES
	sound/snd_local.h
	sound/sound.h)
	
set(SOUND_SOURCES
	sound/snd_cache.cpp
	sound/snd_decoder.cpp
	sound/snd_efxfile.cpp
	sound/snd_emitter.cpp
	sound/snd_shader.cpp
	sound/snd_system.cpp
	sound/snd_wavefile.cpp
	sound/snd_world.cpp)
	
set(OGGVORBIS_INCLUDES
	sound/OggVorbis/ogg/ogg.h
	sound/OggVorbis/ogg/os_types.h)
	
set(OGGVORBIS_SOURCES
	sound/OggVorbis/oggsrc/bitwise.c
	sound/OggVorbis/oggsrc/framing.c
	
	sound/OggVorbis/vorbissrc/mdct.c
	sound/OggVorbis/vorbissrc/smallft.c
	sound/OggVorbis/vorbissrc/block.c
	sound/OggVorbis/vorbissrc/envelope.c
	sound/OggVorbis/vorbissrc/windowvb.c
	sound/OggVorbis/vorbissrc/lsp.c
	sound/OggVorbis/vorbissrc/lpc.c
	sound/OggVorbis/vorbissrc/analysis.c
	sound/OggVorbis/vorbissrc/synthesis.c
	sound/OggVorbis/vorbissrc/psy.c
	sound/OggVorbis/vorbissrc/info.c
	sound/OggVorbis/vorbissrc/floor1.c
	sound/OggVorbis/vorbissrc/floor0.c
	sound/OggVorbis/vorbissrc/res0.c
	sound/OggVorbis/vorbissrc/mapping0.c
	sound/OggVorbis/vorbissrc/registry.c
	sound/OggVorbis/vorbissrc/codebook.c
	sound/OggVorbis/vorbissrc/sharedbook.c
	sound/OggVorbis/vorbissrc/lookup.c
	sound/OggVorbis/vorbissrc/bitrate.c
	sound/OggVorbis/vorbissrc/vorbisfile.c)

file(GLOB SYS_INCLUDES sys/*.h)
file(GLOB SYS_SOURCES sys/*.cpp)

file(GLOB UI_INCLUDES ui/*.h)
file(GLOB UI_SOURCES ui/*.cpp)
	
if(MSVC)
	set(WIN32_INCLUDES
		sys/win32/win_local.h
		sys/win32/rc/doom_resource.h
		)

	set(WIN32_INCLUDES
		sys/win32/rc/doom.rc
		sys/win32/win_cpu.cpp
		sys/win32/win_glimp.cpp
		sys/win32/win_input.cpp
		sys/win32/win_main.cpp
		sys/win32/win_net.cpp
		sys/win32/win_qgl.cpp
		sys/win32/win_shared.cpp
		sys/win32/win_syscon.cpp
		sys/win32/win_taskkeyhook.cpp
		sys/win32/win_wndproc.cpp
		)
endif(MSVC)

SOURCE_GROUP("Include Files\\cm" FILES ${CM_INCLUDES})
SOURCE_GROUP("Source Files\\cm" FILES ${CM_SOURCES})

SOURCE_GROUP("Include Files\\framework" FILES ${FRAMEWORK_INCLUDES})
SOURCE_GROUP("Source Files\\framework" FILES ${FRAMEWORK_SOURCES})

SOURCE_GROUP("Include Files\\framework\\async" FILES ${FRAMEWORK_ASYNC_INCLUDES})
SOURCE_GROUP("Source Files\\framework\\async" FILES ${FRAMEWORK_ASYNC_SOURCES})

SOURCE_GROUP("Include Files\\renderer" FILES ${RENDERER_INCLUDES})
SOURCE_GROUP("Source Files\\renderer" FILES ${RENDERER_SOURCES})

SOURCE_GROUP("Include Files\\renderer\\jpeg-6" FILES ${JPEG_INCLUDES})
SOURCE_GROUP("Source Files\\renderer\\jpeg-6" FILES ${JPEG_SOURCES})

SOURCE_GROUP("Include Files\\sound" FILES ${SOUND_INCLUDES})
SOURCE_GROUP("Source Files\\sound" FILES ${SOUND_SOURCES})

SOURCE_GROUP("Include Files\\sound\\oggvorbis" FILES ${OGGVORBIS_INCLUDES})
SOURCE_GROUP("Source Files\\sound\\oggvorbis" FILES ${OGGVORBIS_SOURCES})

SOURCE_GROUP("Include Files\\sys" FILES ${SYS_INCLUDES})
SOURCE_GROUP("Source Files\\sys" FILES ${SYS_SOURCES})

SOURCE_GROUP("Include Files\\ui" FILES ${SYS_INCLUDES})
SOURCE_GROUP("Source Files\\ui" FILES ${SYS_SOURCES})

if(MSVC)
	add_executable(Techyon 
		${CM_INCLUDES} ${CM_SOURCES}	
		${FRAMEWORK_INCLUDES} ${FRAMEWORK_SOURCES}
		${FRAMEWORK_ASYNC_INCLUDES} ${FRAMEWORK_ASYNC_SOURCES} 
		${RENDERER_INCLUDES} ${RENDERER_SOURCES}
		${JPEG_INCLUDES} ${JPEG_SOURCES}
		${SOUND_INCLUDES} ${SOUND_SOURCES}
		${OGGVORBIS_INCLUDES} ${OGGVORBIS_SOURCES}
		${SYS_INCLUDES} ${SYS_SOURCES}
		${WIN32_INCLUDES} ${WIN32_SOURCES}
		)
		
	add_dependencies(Techyon idlib)
endif(MSVC)
