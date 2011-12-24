cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

project(Techyon)

option(USE_MFC_TOOLS
		"Compile the built-in MFC based tools" OFF)
		
option(USE_QT_TOOLS
		"Compile the built-in Qt based tools" OFF)
		
option(USE_QT_WINDOWING
		"Run game as Qt app" OFF)

#if(USE_QT_TOOLS)
#	set(USE_QT_WINDOWING TRUE)
#endif()

if(MSVC)
	#set(CMAKE_USE_RELATIVE_PATHS ON)

	add_definitions(-DWIN32 
					-D_WINDOWS
					-D_CRT_SECURE_NO_DEPRECATE
					-D_CRT_NONSTDC_NO_DEPRECATE
					-D_CRT_SECURE_NO_WARNINGS
					-D_USE_32BIT_TIME_T
					-D_MBCS)
					
	if(USE_MFC_TOOLS)
		add_definitions(-D_AFXDLL)
		
		# 0: Use Standard Windows Libraries
		# 1: Use MFC in a Static Library
		# 2: Use MFC in a Shared DLL 
		set(CMAKE_MFC_FLAG 2)
	endif()
endif()

add_subdirectory(idlib)

file(GLOB_RECURSE CM_INCLUDES cm/*.h)
file(GLOB_RECURSE CM_SOURCES cm/*.cpp)

file(GLOB FRAMEWORK_INCLUDES framework/*.h)
file(GLOB FRAMEWORK_SOURCES framework/*.cpp)

file(GLOB FRAMEWORK_ASYNC_INCLUDES framework/async/*.h)
file(GLOB FRAMEWORK_ASYNC_SOURCES framework/async/*.cpp)

set(RENDERER_INCLUDES
	renderer/Cinematic.h
	renderer/GuiModel.h
	renderer/Image.h
	renderer/Interaction.h
	renderer/Material.h
	renderer/MegaTexture.h
	renderer/Model.h
	renderer/ModelDecal.h
	renderer/ModelManager.h
	renderer/ModelOverlay.h
	renderer/Model_ase.h
	renderer/Model_local.h
	renderer/Model_lwo.h
	renderer/Model_ma.h
	renderer/Model_md3.h
	renderer/RenderSystem.h
	renderer/RenderWorld.h
	renderer/RenderWorld_local.h
	renderer/VertexCache.h
	renderer/cg_explicit.h
	renderer/simplex.h
	renderer/tr_local.h)


set(RENDERER_SOURCES
	renderer/Cinematic.cpp
	renderer/GuiModel.cpp
	renderer/Image_files.cpp
	renderer/Image_init.cpp
	renderer/Image_load.cpp
	renderer/Image_process.cpp
	renderer/Image_program.cpp
	renderer/Interaction.cpp
	renderer/Material.cpp
	renderer/MegaTexture.cpp
	renderer/Model.cpp
	renderer/ModelDecal.cpp
	renderer/ModelManager.cpp
	renderer/ModelOverlay.cpp
	renderer/Model_ase.cpp
	renderer/Model_beam.cpp
	renderer/Model_liquid.cpp
	renderer/Model_lwo.cpp
	renderer/Model_ma.cpp
	renderer/Model_md3.cpp
	renderer/Model_md5.cpp
	renderer/Model_prt.cpp
	renderer/Model_sprite.cpp
	renderer/RenderEntity.cpp
	renderer/RenderSystem.cpp
	renderer/RenderSystem_init.cpp
	renderer/RenderWorld.cpp
	renderer/RenderWorld_demo.cpp
	renderer/RenderWorld_load.cpp
	renderer/RenderWorld_portals.cpp
	renderer/VertexCache.cpp
	renderer/cg_explicit.cpp
	renderer/draw_arb.cpp
	renderer/draw_arb2.cpp
	renderer/draw_common.cpp
	#renderer/draw_exp.cpp
	#renderer/draw_exp_stub.cpp
	renderer/draw_nv10.cpp
	renderer/draw_nv20.cpp
	renderer/draw_r200.cpp
	renderer/tr_backend.cpp
	renderer/tr_deform.cpp
	renderer/tr_font.cpp
	renderer/tr_guisurf.cpp
	renderer/tr_light.cpp
	renderer/tr_lightrun.cpp
	renderer/tr_main.cpp
	renderer/tr_orderIndexes.cpp
	renderer/tr_polytope.cpp
	renderer/tr_render.cpp
	renderer/tr_rendertools.cpp
	renderer/tr_shadowbounds.cpp
	renderer/tr_stencilshadow.cpp
	renderer/tr_subview.cpp
	renderer/tr_trace.cpp
	renderer/tr_trisurf.cpp
	renderer/tr_turboshadow.cpp)

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

set(GLEW_INCLUDES
	libs/glew/include/GL/glew.h)
	
if(WIN32)
	set(GLEW_INCLUDES ${GLEW_INCLUDES} libs/glew/include/GL/wglew.h)
else()
	set(GLEW_INCLUDES ${GLEW_INCLUDES} libs/glew/include/GL/glxew.h)
endif()
	
set(GLEW_SOURCES
	libs/glew/src/glew.c)
	
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

file(GLOB COMPILER_INCLUDES tools/compilers/*.h)

file(GLOB COMPILER_AAS_INCLUDES tools/compilers/aas/*.h)
file(GLOB COMPILER_AAS_SOURCES tools/compilers/aas/*.cpp)

file(GLOB COMPILER_DMAP_INCLUDES tools/compilers/dmap/*.h)
file(GLOB COMPILER_DMAP_SOURCES tools/compilers/dmap/*.cpp)

file(GLOB COMPILER_RENDERBUMP_INCLUDES tools/compilers/renderbump/*.h)
file(GLOB COMPILER_RENDERBUMP_SOURCES tools/compilers/renderbump/*.cpp)

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
	
else(USE_QT_WINDOWING)
	file(GLOB_RECURSE WIN32_INCLUDES sys/win32/*.h)
	file(GLOB_RECURSE WIN32_SOURCES sys/win32/*.cpp)

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
endif(USE_QT_WINDOWING)
	
if(USE_MFC_TOOLS)
	list(APPEND WIN32_RESOURCES sys/win32/rc/doom.rc)
else()
	list(APPEND WIN32_RESOURCES sys/win32/rc/doom_nomfc.rc)
endif()

if(USE_QT_TOOLS)

	set(QT_USE_QTOPENGL TRUE)
	set(QT_USE_IMPORTED_TARGETS TRUE)
	find_package(Qt4 REQUIRED)

	set(EDITOR_QTTEST_INCLUDES
		tools/qttest/qttest.h)
		
	set(EDITOR_QTTEST_SOURCES
		tools/qttest/main.cpp)
		
	set(EDITOR_QTTEST_RESOURCES
		tools/qttest/qttest.qrc)
		
	set(EDITOR_QTTEST_FORMS
		tools/qttest/qttest.ui)
		
	QT4_WRAP_CPP(EDITOR_QTTEST_INCLUDES_MOC ${EDITOR_QTTEST_INCLUDES})
	QT4_WRAP_UI(EDITOR_QTTEST_FORMS_INCLUDES ${EDITOR_QTTEST_FORMS})
	QT4_ADD_RESOURCES(EDITOR_QTTEST_RESOURCES_RCC ${EDITOR_QTTEST_RESOURCES})
	
	source_group("tools\\qttest\\Header Files" FILES ${EDITOR_QTTEST_INCLUDES})
	source_group("tools\\qttest\\Source Files" FILES ${EDITOR_QTTEST_SOURCES})
	source_group("tools\\qttest\\Resource Files" FILES ${EDITOR_QTTEST_RESOURCES})
	source_group("tools\\qttest\\Form Files" FILES ${EDITOR_QTTEST_FORMS})
	
	source_group("tools\\qttest\\Generated Files" FILES ${EDITOR_QTTEST_INCLUDES_MOC})
	source_group("tools\\qttest\\Generated Files" FILES ${EDITOR_QTTEST_FORMS_INCLUDES})
	source_group("tools\\qttest\\Generated Files" FILES ${EDITOR_QTTEST_RESOURCES_RCC})
	
	
	file(GLOB EDITOR_QTRADIANT_INCLUDES	tools/qtradiant/src/*.h)
	file(GLOB EDITOR_QTRADIANT_SOURCES tools/qtradiant/src/*.cpp)
	
	#file(GLOB EDITOR_QTRADIANT_RESOURCES tools/qtradiant/resources/icons/*.png)
	set(EDITOR_QTRADIANT_RESOURCES tools/qtradiant/QtRadiant.qrc)
	file(GLOB EDITOR_QTRADIANT_FORMS tools/qtradiant/src/*.ui)
		
	QT4_WRAP_CPP(EDITOR_QTRADIANT_INCLUDES_MOC ${EDITOR_QTRADIANT_INCLUDES})
	QT4_WRAP_UI(EDITOR_QTRADIANT_FORMS_INCLUDES ${EDITOR_QTRADIANT_FORMS})
	QT4_ADD_RESOURCES(EDITOR_QTRADIANT_RESOURCES_RCC ${EDITOR_QTRADIANT_RESOURCES})
	
	source_group("tools\\qtradiant\\Header Files" FILES ${EDITOR_QTRADIANT_INCLUDES})
	source_group("tools\\qtradiant\\Source Files" FILES ${EDITOR_QTRADIANT_SOURCES})
	source_group("tools\\qtradiant\\Resource Files" FILES ${EDITOR_QTRADIANT_RESOURCES})
	source_group("tools\\qtradiant\\Form Files" FILES ${EDITOR_QTRADIANT_FORMS})
	
	source_group("tools\\qtradiant\\Generated Files" FILES ${EDITOR_QTRADIANT_INCLUDES_MOC})
	source_group("tools\\qtradiant\\Generated Files" FILES ${EDITOR_QTRADIANT_FORMS_INCLUDES})
	source_group("tools\\qtradiant\\Generated Files" FILES ${EDITOR_QTRADIANT_RESOURCES_RCC})
	
	if(USE_QT_WINDOWING)
		add_definitions(-DUSE_QT_WINDOWING)
	
		file(GLOB QTTECHYON_INCLUDES sys/qt/*.h)
		file(GLOB QTTECHYON_SOURCES sys/qt/*.cpp)
		
		set(QTTECHYON_RESOURCES sys/qt/Techyon.qrc)
		file(GLOB QTTECHYON_FORMS sys/qt/*.ui)
			
		QT4_WRAP_CPP(QTTECHYON_INCLUDES_MOC ${QTTECHYON_INCLUDES})
		QT4_WRAP_UI(QTTECHYON_FORMS_INCLUDES ${QTTECHYON_FORMS})
		QT4_ADD_RESOURCES(QTTECHYON_RESOURCES_RCC ${QTTECHYON_RESOURCES})
		
		source_group("sys\\qt\\Header Files" FILES ${QTTECHYON_INCLUDES})
		source_group("sys\\qt\\Source Files" FILES ${QTTECHYON_SOURCES})
		source_group("sys\\qt\\Resource Files" FILES ${QTTECHYON_RESOURCES})
		source_group("sys\\qt\\Form Files" FILES ${QTTECHYON_FORMS})
		
		source_group("sys\\qt\\Generated Files" FILES ${QTTECHYON_INCLUDES_MOC})
		source_group("sys\\qt\\Generated Files" FILES ${QTTECHYON_FORMS_INCLUDES})
		source_group("sys\\qt\\Generated Files" FILES ${QTTECHYON_RESOURCES_RCC})
	endif()
	
endif(USE_QT_TOOLS)

source_group("cm" FILES ${CM_INCLUDES})
source_group("cm" FILES ${CM_SOURCES})

source_group("framework" FILES ${FRAMEWORK_INCLUDES})
source_group("framework" FILES ${FRAMEWORK_SOURCES})

source_group("framework\\async" FILES ${FRAMEWORK_ASYNC_INCLUDES})
source_group("framework\\async" FILES ${FRAMEWORK_ASYNC_SOURCES})

source_group("renderer" FILES ${RENDERER_INCLUDES})
source_group("renderer" FILES ${RENDERER_SOURCES})

source_group("renderer\\jpeg-6" FILES ${JPEG_INCLUDES})
source_group("renderer\\jpeg-6" FILES ${JPEG_SOURCES})

source_group("libs\\glew" FILES ${GLEW_INCLUDES})
source_group("libs\\glew" FILES ${GLEW_SOURCES})

source_group("sound" FILES ${SOUND_INCLUDES})
source_group("sound" FILES ${SOUND_SOURCES})

source_group("sound\\oggvorbis" FILES ${OGGVORBIS_INCLUDES})
source_group("sound\\oggvorbis" FILES ${OGGVORBIS_SOURCES})

source_group("ui" FILES ${UI_INCLUDES})
source_group("ui" FILES ${UI_SOURCES})

source_group("sys" FILES ${SYS_INCLUDES})
source_group("sys" FILES ${SYS_SOURCES})

if(USE_QT_WINDOWING)
	source_group("sys\\qt\\win32" FILES ${WIN32_INCLUDES})
	source_group("sys\\qt\\win32" FILES ${WIN32_SOURCES})
	source_group("sys\\qt\\win32\\Resources" FILES ${WIN32_RESOURCES})
else()
	source_group("sys\\win32" FILES ${WIN32_INCLUDES})
	source_group("sys\\win32" FILES ${WIN32_SOURCES})
	source_group("sys\\win32\\Resources" FILES ${WIN32_RESOURCES})
endif()

source_group("tools\\compilers" FILES ${COMPILER_INCLUDES})

source_group("tools\\compilers\\aas" FILES ${COMPILER_AAS_INCLUDES})
source_group("tools\\compilers\\aas" FILES ${COMPILER_AAS_SOURCES})

source_group("tools\\compilers\\dmap" FILES ${COMPILER_DMAP_INCLUDES})
source_group("tools\\compilers\\dmap" FILES ${COMPILER_DMAP_SOURCES})

source_group("tools\\compilers\\renderbump" FILES ${COMPILER_RENDERBUMP_INCLUDES})
source_group("tools\\compilers\\renderbump" FILES ${COMPILER_RENDERBUMP_SOURCES})

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


set(TECHYON_SOURCES
			${CM_INCLUDES} ${CM_SOURCES}
			${FRAMEWORK_INCLUDES} ${FRAMEWORK_SOURCES}
			${FRAMEWORK_ASYNC_INCLUDES} ${FRAMEWORK_ASYNC_SOURCES} 
			${RENDERER_INCLUDES} ${RENDERER_SOURCES}
			${JPEG_INCLUDES} ${JPEG_SOURCES}
			${GLEW_INCLUDES} ${GLEW_SOURCES}
			${SOUND_INCLUDES} ${SOUND_SOURCES}
			${OGGVORBIS_INCLUDES} ${OGGVORBIS_SOURCES}
			${UI_INCLUDES} ${UI_SOURCES}
			
			${COMPILER_INCLUDES}
			${COMPILER_AAS_INCLUDES} ${COMPILER_AAS_SOURCES}
			${COMPILER_DMAP_INCLUDES} ${COMPILER_DMAP_SOURCES}
			${COMPILER_RENDERBUMP_INCLUDES} ${COMPILER_RENDERBUMP_SOURCES}
			${COMPILER_ROQVQ_INCLUDES} ${COMPILER_ROQVQ_SOURCES})

add_definitions(-D__DOOM__
				-D__DOOM_DLL__
				-DGLEW_STATIC)

include_directories(
			curl/include
			openal/include
			libs/glew/include)
			
link_directories(
			${CMAKE_CURRENT_SOURCE_DIR}/curl/lib
			${CMAKE_CURRENT_SOURCE_DIR}/openal/lib)

if(MSVC)
	find_package(DirectX REQUIRED)
	include_directories(${DirectX_INCLUDE_DIR})
	
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
		
		# include_directories(libs/atlmfc/include)
		# if(CMAKE_CL_64)
			# link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/atlmfc/lib/amd64)
		# else(CMAKE_CL_64)
			# link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/atlmfc/lib)
		# endif(CMAKE_CL_64)
		
		add_definitions(-DUSE_MFC_TOOLS)
		
		set(TECHYON_SOURCES ${TECHYON_SOURCES}
			${TOOLS_INCLUDES}
			${TOOLS_COMAFX_INCLUDES} ${TOOLS_COMAFX_SOURCES}
			${TOOLS_COMMON_INCLUDES} ${TOOLS_COMMON_SOURCES}
			${TOOLS_COMMON_PROPTREE_INCLUDES} ${TOOLS_COMMON_PROPTREE_SOURCES}
			
			${EDITOR_AF_INCLUDES} ${EDITOR_AF_SOURCES}
			${EDITOR_DECL_INCLUDES} ${EDITOR_DECL_SOURCES}
			${EDITOR_GUIED_INCLUDES} ${EDITOR_GUIED_SOURCES}
			${EDITOR_MATERIALEDITOR_INCLUDES} ${EDITOR_MATERIALEDITOR_SOURCES}
			${EDITOR_PARTICLE_INCLUDES} ${EDITOR_PARTICLE_SOURCES}
			${EDITOR_PDA_INCLUDES} ${EDITOR_PDA_SOURCES}
			${EDITOR_RADIANT_INCLUDES} ${EDITOR_RADIANT_SOURCES}
			${EDITOR_SCRIPT_INCLUDES} ${EDITOR_SCRIPT_SOURCES}
			${EDITOR_SOUND_INCLUDES} ${EDITOR_SOUND_SOURCES})
		
	elseif(USE_QT_TOOLS)
	
		add_definitions(-DUSE_QT_TOOLS ${QT_DEFINITIONS})
		include(${QT_USE_FILE})
		
		# As all intermediate files are generated outside the source tree,
		# the header file generated by uic will not be located by the compiler.
		# In order to handle this, the build directory needs to be added to the list of include directories.
		include_directories(${CMAKE_CURRENT_BINARY_DIR})
	
		set(TECHYON_SOURCES ${TECHYON_SOURCES}
			${TOOLS_INCLUDES}
			
			${QTTECHYON_INCLUDES}
			${QTTECHYON_SOURCES}
			${QTTECHYON_RESOURCES}
			
			${EDITOR_QTTEST_INCLUDES}
			${EDITOR_QTTEST_SOURCES}
			${EDITOR_QTTEST_RESOURCES}
			${EDITOR_QTTEST_FORMS}
			
			${EDITOR_QTTEST_INCLUDES_MOC}
			${EDITOR_QTTEST_FORMS_INCLUDES}
			${EDITOR_QTTEST_RESOURCES_RCC}
			
			${EDITOR_QTRADIANT_INCLUDES}
			${EDITOR_QTRADIANT_SOURCES}
			${EDITOR_QTRADIANT_RESOURCES}
			${EDITOR_QTRADIANT_FORMS}
			
			${EDITOR_QTRADIANT_INCLUDES_MOC}
			${EDITOR_QTRADIANT_FORMS_INCLUDES}
			${EDITOR_QTRADIANT_RESOURCES_RCC}
			)
			
		if(USE_QT_WINDOWING)
			set(TECHYON_SOURCES ${TECHYON_SOURCES}
				${QTTECHYON_INCLUDES}
				${QTTECHYON_SOURCES}
				${QTTECHYON_RESOURCES}
				${QTTECHYON_FORMS}
				
				${QTTECHYON_INCLUDES_MOC}
				${QTTECHYON_FORMS_INCLUDES}
				${QTTECHYON_RESOURCES_RCC})
		endif(USE_QT_WINDOWING)
	endif()
	
	set(TECHYON_SOURCES ${TECHYON_SOURCES}
		${SYS_INCLUDES} ${SYS_SOURCES}
		${WIN32_INCLUDES} ${WIN32_SOURCES} ${WIN32_RESOURCES})
		

	list(REMOVE_DUPLICATES TECHYON_SOURCES)
	
	if(USE_QT_WINDOWING)
		add_executable(Techyon ${TECHYON_SOURCES})
	else()
		add_executable(Techyon WIN32 ${TECHYON_SOURCES})
	endif()
		
	add_custom_command(TARGET Techyon
                   PRE_BUILD
				   COMMAND nmake ARGS /f Makefile.vc6 CFG=release
				   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/curl/lib
                   COMMENT "Compiling libcURL")
		
	add_dependencies(Techyon idlib)
	target_link_libraries(Techyon
		idlib
		${DirectX_LIBRARIES}
		${QT_LIBRARIES}
		libcurl 
		openal32 
		opengl32
		glu32
		dbghelp
		eaxguid
		iphlpapi
		winmm
		ws2_32.lib)
endif()
