# Copyright (C) 2013 Robert Beckebans

# to enable Android NDK profiling, outcomment the sections between PROFILING begin / end

# PROFILING begin
#TARGET_thumb_release_CFLAGS := $(filter-out -ffunction-sections,$(TARGET_thumb_release_CFLAGS))
#TARGET_thumb_release_CFLAGS := $(filter-out -fomit-frame-pointer,$(TARGET_thumb_release_CFLAGS))
#TARGET_arm_release_CFLAGS := $(filter-out -ffunction-sections,$(TARGET_arm_release_CFLAGS))
#TARGET_arm_release_CFLAGS := $(filter-out -fomit-frame-pointer,$(TARGET_arm_release_CFLAGS))
#TARGET_CFLAGS := $(filter-out -ffunction-sections,$(TARGET_CFLAGS))
# PROFILING end

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_LDLIBS    := -lEGL -lGLESv3 -lOpenSLES -lm -llog -ldl #-pg #-lstdc++ -lgcc -lc
LOCAL_MODULE    := tekuum
LOCAL_SRC_FILES := tekuumjni.c

# PROFILING begin
#LOCAL_CFLAGS	:= -DNDEBUG -D__DOOM__ -DUSE_GLES3 -DPNG_NO_ASSEMBLER_CODE -DUSE_EXCEPTIONS -DUSE_ANDROID_NDK_PROFILER #-DBUILD_FREETYPE -DFT2_BUILD_LIBRARY
#LOCAL_STATIC_LIBRARIES := android-ndk-profiler
# else 
LOCAL_CFLAGS	:= -DNDEBUG -D__DOOM__ -DUSE_GLES3 -DPNG_NO_ASSEMBLER_CODE -DUSE_EXCEPTIONS #-DBUILD_FREETYPE -DFT2_BUILD_LIBRARY
# PROFILING end

LOCAL_DISABLE_FORMAT_STRING_CHECKS=true

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../neo/idlib
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../neo/libs/zlib
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../neo/libs/freetype/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../neo/libs/oggvorbis/ogg
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../neo/libs/oggvorbis/vorbis

#LOCAL_CPP_FEATURES += rtti
LOCAL_CPP_FEATURES += exceptions

TEKUUM_SRC :=	../../neo/cm/CollisionModel_load.cpp \
	../../neo/cm/CollisionModel_files.cpp \
	../../neo/cm/CollisionModel_rotate.cpp \
	../../neo/cm/CollisionModel_debug.cpp \
	../../neo/cm/CollisionModel_trace.cpp \
	../../neo/cm/CollisionModel_translate.cpp \
	../../neo/cm/CollisionModel_contents.cpp \
	../../neo/cm/CollisionModel_contacts.cpp \
	../../neo/framework/Session.cpp \
	../../neo/framework/DeclEntityDef.cpp \
	../../neo/framework/DeclFX.cpp \
	../../neo/framework/DeclSkin.cpp \
	../../neo/framework/Compressor.cpp \
	../../neo/framework/CmdSystem.cpp \
	../../neo/framework/precompiled.cpp \
	../../neo/framework/DeclAF.cpp \
	../../neo/framework/DeclPDA.cpp \
	../../neo/framework/UsercmdGen.cpp \
	../../neo/framework/DemoFile.cpp \
	../../neo/framework/CVarSystem.cpp \
	../../neo/framework/DeclTable.cpp \
	../../neo/framework/Console.cpp \
	../../neo/framework/ConsoleHistory.cpp \
	../../neo/framework/Session_menu.cpp \
	../../neo/framework/DebugGraph.cpp \
	../../neo/framework/DeclManager.cpp \
	../../neo/framework/DeclParticle.cpp \
	../../neo/framework/EventLoop.cpp \
	../../neo/framework/File.cpp \
	../../neo/framework/Common.cpp \
	../../neo/framework/Unzip.cpp \
	../../neo/framework/FileSystem.cpp \
	../../neo/framework/KeyInput.cpp \
	../../neo/framework/EditField.cpp \
	../../neo/framework/TokenParser.cpp \
	../../neo/framework/async/AsyncServer.cpp \
	../../neo/framework/async/AsyncClient.cpp \
	../../neo/framework/async/AsyncNetwork.cpp \
	../../neo/framework/async/ServerScan.cpp \
	../../neo/framework/async/NetworkSystem.cpp \
	../../neo/framework/async/MsgChannel.cpp \
	../../neo/renderer/BinaryImage.cpp \
	../../neo/renderer/BoundsTrack.cpp \
	../../neo/renderer/BufferObject.cpp \
	../../neo/renderer/Cinematic.cpp \
	../../neo/renderer/Font.cpp \
	../../neo/renderer/GLMatrix.cpp \
	../../neo/renderer/GuiModel.cpp \
	../../neo/renderer/Image_files.cpp \
	../../neo/renderer/Image_intrinsic.cpp \
	../../neo/renderer/Image_load.cpp \
	../../neo/renderer/ImageManager.cpp \
	../../neo/renderer/Image_process.cpp \
	../../neo/renderer/Image_program.cpp \
	../../neo/renderer/Interaction.cpp \
	../../neo/renderer/Material.cpp \
	../../neo/renderer/Model_ase.cpp \
	../../neo/renderer/Model_beam.cpp \
	../../neo/renderer/Model_ColladaParser.cpp \
	../../neo/renderer/Model.cpp \
	../../neo/renderer/ModelDecal.cpp \
	../../neo/renderer/Model_liquid.cpp \
	../../neo/renderer/Model_lwo.cpp \
	../../neo/renderer/Model_ma.cpp \
	../../neo/renderer/ModelManager.cpp \
	../../neo/renderer/Model_md3.cpp \
	../../neo/renderer/Model_md5.cpp \
	../../neo/renderer/ModelOverlay.cpp \
	../../neo/renderer/Model_prt.cpp \
	../../neo/renderer/Model_sprite.cpp \
	../../neo/renderer/RenderEntity.cpp \
	../../neo/renderer/RenderLog.cpp \
	../../neo/renderer/RenderProgs.cpp \
	../../neo/renderer/RenderProgs_GLSL.cpp \
	../../neo/renderer/RenderSystem.cpp \
	../../neo/renderer/RenderSystem_init.cpp \
	../../neo/renderer/RenderWorld.cpp \
	../../neo/renderer/RenderWorld_defs.cpp \
	../../neo/renderer/RenderWorld_demo.cpp \
	../../neo/renderer/RenderWorld_lightgrid.cpp \
	../../neo/renderer/RenderWorld_load.cpp \
	../../neo/renderer/RenderWorld_portals.cpp \
	../../neo/renderer/ResolutionScale.cpp \
	../../neo/renderer/ScreenRect.cpp \
	../../neo/renderer/tr_backend_draw.cpp \
	../../neo/renderer/tr_backend_rendertools.cpp \
	../../neo/renderer/tr_font.cpp \
	../../neo/renderer/tr_frontend_addlights.cpp \
	../../neo/renderer/tr_frontend_addmodels.cpp \
	../../neo/renderer/tr_frontend_deform.cpp \
	../../neo/renderer/tr_frontend_guisurf.cpp \
	../../neo/renderer/tr_frontend_main.cpp \
	../../neo/renderer/tr_frontend_subview.cpp \
	../../neo/renderer/tr_trace.cpp \
	../../neo/renderer/tr_trisurf.cpp \
	../../neo/renderer/VertexCache.cpp \
	../../neo/renderer/DXT/DXTDecoder.cpp \
	../../neo/renderer/DXT/DXTEncoder.cpp \
	../../neo/renderer/DXT/DXTEncoder_SSE2.cpp \
	../../neo/renderer/Color/ColorSpace.cpp \
	../../neo/renderer/OpenGL/gl_backend.cpp \
	../../neo/renderer/OpenGL/gl_GraphicsAPIWrapper.cpp \
	../../neo/renderer/OpenGL/gl_Image.cpp \
	../../neo/renderer/jobs/ShadowShared.cpp \
	../../neo/renderer/jobs/dynamicshadowvolume/DynamicShadowVolume.cpp \
	../../neo/renderer/jobs/prelightshadowvolume/PreLightShadowVolume.cpp \
	../../neo/renderer/jobs/staticshadowvolume/StaticShadowVolume.cpp \
	../../neo/libs/jpeg-6/jcapistd.c \
	../../neo/libs/jpeg-6/jdmarker.c \
	../../neo/libs/jpeg-6/jutils.c \
	../../neo/libs/jpeg-6/jidctint.c \
	../../neo/libs/jpeg-6/jquant2.c \
	../../neo/libs/jpeg-6/jdpostct.c \
	../../neo/libs/jpeg-6/jidctfst.c \
	../../neo/libs/jpeg-6/jccolor.c \
	../../neo/libs/jpeg-6/jcphuff.c \
	../../neo/libs/jpeg-6/jidctred.c \
	../../neo/libs/jpeg-6/jdmainct.c \
	../../neo/libs/jpeg-6/jcapimin.c \
	../../neo/libs/jpeg-6/jcparam.c \
	../../neo/libs/jpeg-6/jcsample.c \
	../../neo/libs/jpeg-6/jdphuff.c \
	../../neo/libs/jpeg-6/jdtrans.c \
	../../neo/libs/jpeg-6/jcmarker.c \
	../../neo/libs/jpeg-6/jdapistd.c \
	../../neo/libs/jpeg-6/jdsample.c \
	../../neo/libs/jpeg-6/jdatadst.c \
	../../neo/libs/jpeg-6/jmemnobs.c \
	../../neo/libs/jpeg-6/jdcolor.c \
	../../neo/libs/jpeg-6/jddctmgr.c \
	../../neo/libs/jpeg-6/jerror.c \
	../../neo/libs/jpeg-6/jfdctflt.c \
	../../neo/libs/jpeg-6/jcinit.c \
	../../neo/libs/jpeg-6/jmemmgr.c \
	../../neo/libs/jpeg-6/jdmerge.c \
	../../neo/libs/jpeg-6/jctrans.c \
	../../neo/libs/jpeg-6/jcomapi.c \
	../../neo/libs/jpeg-6/jdatasrc.c \
	../../neo/libs/jpeg-6/jdinput.c \
	../../neo/libs/jpeg-6/jcmainct.c \
	../../neo/libs/jpeg-6/jcdctmgr.c \
	../../neo/libs/jpeg-6/jidctflt.c \
	../../neo/libs/jpeg-6/jfdctint.c \
	../../neo/libs/jpeg-6/jchuff.c \
	../../neo/libs/jpeg-6/jdcoefct.c \
	../../neo/libs/jpeg-6/jcprepct.c \
	../../neo/libs/jpeg-6/jdmaster.c \
	../../neo/libs/jpeg-6/jdapimin.c \
	../../neo/libs/jpeg-6/jquant1.c \
	../../neo/libs/jpeg-6/jccoefct.c \
	../../neo/libs/jpeg-6/jcmaster.c \
	../../neo/libs/jpeg-6/jfdctfst.c \
	../../neo/libs/jpeg-6/jdhuff.c \
	../../neo/libs/png/pngpread.c \
	../../neo/libs/png/pngwrite.c \
	../../neo/libs/png/pngget.c \
	../../neo/libs/png/pngerror.c \
	../../neo/libs/png/png.c \
	../../neo/libs/png/pngmem.c \
	../../neo/libs/png/pngwio.c \
	../../neo/libs/png/pngtrans.c \
	../../neo/libs/png/pnggccrd.c \
	../../neo/libs/png/pngwutil.c \
	../../neo/libs/png/pngrutil.c \
	../../neo/libs/png/pngset.c \
	../../neo/libs/png/pngwtran.c \
	../../neo/libs/png/pngrtran.c \
	../../neo/libs/png/pngrio.c \
	../../neo/libs/png/pngvcrd.c \
	../../neo/libs/png/example.c \
	../../neo/libs/png/pngread.c \
	../../neo/libs/zlib/infback.c \
	../../neo/libs/zlib/zlib_crc32.c \
	../../neo/libs/zlib/trees.c \
	../../neo/libs/zlib/gzlib.c \
	../../neo/libs/zlib/gzwrite.c \
	../../neo/libs/zlib/gzclose.c \
	../../neo/libs/zlib/zutil.c \
	../../neo/libs/zlib/inftrees.c \
	../../neo/libs/zlib/compress.c \
	../../neo/libs/zlib/inflate.c \
	../../neo/libs/zlib/inffast.c \
	../../neo/libs/zlib/deflate.c \
	../../neo/libs/zlib/gzread.c \
	../../neo/libs/zlib/uncompr.c \
	../../neo/libs/zlib/adler32.c \
	../../neo/libs/etc1/etc1.cpp \
	../../neo/libs/irrxml/src/irrXML.cpp \
	../../neo/sound/snd_cache.cpp \
	../../neo/sound/snd_world.cpp \
	../../neo/sound/snd_wavefile.cpp \
	../../neo/sound/snd_system.cpp \
	../../neo/sound/snd_emitter.cpp \
	../../neo/sound/snd_shader.cpp \
	../../neo/sound/snd_decoder.cpp \
	../../neo/libs/oggvorbis/oggsrc/bitwise.c \
	../../neo/libs/oggvorbis/oggsrc/framing.c \
	../../neo/libs/oggvorbis/vorbissrc/mdct.c \
	../../neo/libs/oggvorbis/vorbissrc/smallft.c \
	../../neo/libs/oggvorbis/vorbissrc/block.c \
	../../neo/libs/oggvorbis/vorbissrc/envelope.c \
	../../neo/libs/oggvorbis/vorbissrc/windowvb.c \
	../../neo/libs/oggvorbis/vorbissrc/lsp.c \
	../../neo/libs/oggvorbis/vorbissrc/lpc.c \
	../../neo/libs/oggvorbis/vorbissrc/analysis.c \
	../../neo/libs/oggvorbis/vorbissrc/synthesis.c \
	../../neo/libs/oggvorbis/vorbissrc/psy.c \
	../../neo/libs/oggvorbis/vorbissrc/info.c \
	../../neo/libs/oggvorbis/vorbissrc/floor1.c \
	../../neo/libs/oggvorbis/vorbissrc/floor0.c \
	../../neo/libs/oggvorbis/vorbissrc/res0.c \
	../../neo/libs/oggvorbis/vorbissrc/mapping0.c \
	../../neo/libs/oggvorbis/vorbissrc/registry.c \
	../../neo/libs/oggvorbis/vorbissrc/codebook.c \
	../../neo/libs/oggvorbis/vorbissrc/sharedbook.c \
	../../neo/libs/oggvorbis/vorbissrc/lookup.c \
	../../neo/libs/oggvorbis/vorbissrc/bitrate.c \
	../../neo/libs/oggvorbis/vorbissrc/vorbisfile.c \
	../../neo/sys/sys_local.cpp \
	../../neo/aas/AASFile_optimize.cpp \
	../../neo/aas/AASFile_sample.cpp \
	../../neo/aas/AASFileManager.cpp \
	../../neo/aas/AASFile.cpp \
	../../neo/ui/BindWindow.cpp \
	../../neo/ui/RegExp.cpp \
	../../neo/ui/SliderWindow.cpp \
	../../neo/ui/RenderWindow.cpp \
	../../neo/ui/ChoiceWindow.cpp \
	../../neo/ui/MarkerWindow.cpp \
	../../neo/ui/ListWindow.cpp \
	../../neo/ui/GuiScript.cpp \
	../../neo/ui/SimpleWindow.cpp \
	../../neo/ui/ListGUI.cpp \
	../../neo/ui/Winvar.cpp \
	../../neo/ui/UserInterface.cpp \
	../../neo/ui/FieldWindow.cpp \
	../../neo/ui/EditWindow.cpp \
	../../neo/ui/DeviceContext.cpp \
	../../neo/ui/Window.cpp \
	../../neo/idlib/MapFile.cpp \
	../../neo/idlib/Timer.cpp \
	../../neo/idlib/precompiled.cpp \
	../../neo/idlib/CmdArgs.cpp \
	../../neo/idlib/CommandLink.cpp \
	../../neo/idlib/Lexer.cpp \
	../../neo/idlib/Base64.cpp \
	../../neo/idlib/Dict.cpp \
	../../neo/idlib/LangDict.cpp \
	../../neo/idlib/BitMsg.cpp \
	../../neo/idlib/Parser.cpp \
	../../neo/idlib/Token.cpp \
	../../neo/idlib/Lib.cpp \
	../../neo/idlib/Str.cpp \
	../../neo/idlib/Heap.cpp \
	../../neo/idlib/ParallelJobList.cpp \
	../../neo/idlib/RectAllocator.cpp \
	../../neo/idlib/SoftwareCache.cpp \
	../../neo/idlib/Thread.cpp \
	../../neo/idlib/math/VecX.cpp \
	../../neo/idlib/math/Simd_Generic.cpp \
	../../neo/idlib/math/Angles.cpp \
	../../neo/idlib/math/Complex.cpp \
	../../neo/idlib/math/Simd_SSE.cpp \
	../../neo/idlib/math/Vector.cpp \
	../../neo/idlib/math/Math.cpp \
	../../neo/idlib/math/MatX.cpp \
	../../neo/idlib/math/Polynomial.cpp \
	../../neo/idlib/math/Lcp.cpp \
	../../neo/idlib/math/Rotation.cpp \
	../../neo/idlib/math/Ode.cpp \
	../../neo/idlib/math/Plane.cpp \
	../../neo/idlib/math/Pluecker.cpp \
	../../neo/idlib/math/Quat.cpp \
	../../neo/idlib/math/Simd.cpp \
	../../neo/idlib/math/Matrix.cpp \
	../../neo/idlib/geometry/Winding.cpp \
	../../neo/idlib/geometry/Surface_Polytope.cpp \
	../../neo/idlib/geometry/Surface.cpp \
	../../neo/idlib/geometry/JointTransform.cpp \
	../../neo/idlib/geometry/Surface_SweptSpline.cpp \
	../../neo/idlib/geometry/DrawVert.cpp \
	../../neo/idlib/geometry/TraceModel.cpp \
	../../neo/idlib/geometry/Winding2D.cpp \
	../../neo/idlib/geometry/Surface_Patch.cpp \
	../../neo/idlib/geometry/RenderMatrix.cpp \
	../../neo/idlib/bv/Bounds.cpp \
	../../neo/idlib/bv/Sphere.cpp \
	../../neo/idlib/bv/Box.cpp \
	../../neo/idlib/sys/sys_assert.cpp \
	../../neo/idlib/sys/posix/posix_thread.cpp \
	../../neo/idlib/containers/HashIndex.cpp \
	../../neo/idlib/hashing/CRC32.cpp \
	../../neo/idlib/hashing/MD4.cpp \
	../../neo/idlib/hashing/MD5.cpp \
	../../neo/game/Portal.cpp \
	../../neo/game/Sound.cpp \
	../../neo/game/MultiplayerGame.cpp \
	../../neo/game/Projectile.cpp \
	../../neo/game/Player.cpp \
	../../neo/game/Light.cpp \
	../../neo/game/Trigger.cpp \
	../../neo/game/BrittleFracture.cpp \
	../../neo/game/Fx.cpp \
	../../neo/game/Camera.cpp \
	../../neo/game/Item.cpp \
	../../neo/game/Misc.cpp \
	../../neo/game/PlayerView.cpp \
	../../neo/game/AFEntity.cpp \
	../../neo/game/WorldSpawn.cpp \
	../../neo/game/AF.cpp \
	../../neo/game/SmokeParticles.cpp \
	../../neo/game/SecurityCamera.cpp \
	../../neo/game/Pvs.cpp \
	../../neo/game/Game_local.cpp \
	../../neo/game/PlayerIcon.cpp \
	../../neo/game/Game_network.cpp \
	../../neo/game/Weapon.cpp \
	../../neo/game/IK.cpp \
	../../neo/game/Entity.cpp \
	../../neo/game/Moveable.cpp \
	../../neo/game/Target.cpp \
	../../neo/game/GameEdit.cpp \
	../../neo/game/Mover.cpp \
	../../neo/game/Actor.cpp \
	../../neo/game/anim/Anim.cpp \
	../../neo/game/anim/Anim_Testmodel.cpp \
	../../neo/game/anim/Anim_Blend.cpp \
	../../neo/game/ai/AAS.cpp \
	../../neo/game/ai/AAS_pathing.cpp \
	../../neo/game/ai/AI_events.cpp \
	../../neo/game/ai/AI_pathing.cpp \
	../../neo/game/ai/AI.cpp \
	../../neo/game/ai/AI_Vagary.cpp \
	../../neo/game/ai/AAS_routing.cpp \
	../../neo/game/ai/AAS_debug.cpp \
	../../neo/game/script/Script_Interpreter.cpp \
	../../neo/game/script/Script_Program.cpp \
	../../neo/game/script/Script_Compiler.cpp \
	../../neo/game/script/Script_Thread.cpp \
	../../neo/game/physics/Clip.cpp \
	../../neo/game/physics/Force_Constant.cpp \
	../../neo/game/physics/Force_Drag.cpp \
	../../neo/game/physics/Physics_Actor.cpp \
	../../neo/game/physics/Physics.cpp \
	../../neo/game/physics/Physics_Parametric.cpp \
	../../neo/game/physics/Physics_AF.cpp \
	../../neo/game/physics/Physics_Player.cpp \
	../../neo/game/physics/Physics_StaticMulti.cpp \
	../../neo/game/physics/Physics_Static.cpp \
	../../neo/game/physics/Force_Spring.cpp \
	../../neo/game/physics/Physics_Base.cpp \
	../../neo/game/physics/Physics_Monster.cpp \
	../../neo/game/physics/Push.cpp \
	../../neo/game/physics/Force.cpp \
	../../neo/game/physics/Force_Field.cpp \
	../../neo/game/physics/Physics_RigidBody.cpp \
	../../neo/game/gamesys/SysCmds.cpp \
	../../neo/game/gamesys/SaveGame.cpp \
	../../neo/game/gamesys/Event.cpp \
	../../neo/game/gamesys/Class.cpp \
	../../neo/game/gamesys/SysCvar.cpp \
	../../neo/sys/android/android_snd.cpp \
	../../neo/sys/android/android_input.cpp \
	../../neo/sys/android/android_glimp.cpp \
	../../neo/sys/android/android_main.cpp \
	../../neo/sys/posix/posix_net.cpp \
	../../neo/sys/posix/posix_main.cpp \
	../../neo/sys/posix/posix_signal.cpp \
	../../neo/sys/posix/posix_threads.cpp \
	../../neo/sys/linux/stack.cpp #\
	../../neo/libs/android-ndk-profiler/jni/prof.c \
	../../neo/libs/android-ndk-profiler/jni/read_maps.c \
	../../neo/libs/android-ndk-profiler/jni/gnu_mcount.S \
	
#	../../neo/libs/freetype/src/autofit/autofit.c \
#	../../neo/libs/freetype/src/bdf/bdf.c \
#	../../neo/libs/freetype/src/cff/cff.c \
#	../../neo/libs/freetype/src/base/ftbase.c \
#	../../neo/libs/freetype/src/base/ftbitmap.c \
#	../../neo/libs/freetype/src/cache/ftcache.c \
#	../../neo/libs/freetype/src/base/ftdebug.c \
#	../../neo/libs/freetype/src/base/ftgasp.c \
#	../../neo/libs/freetype/src/base/ftglyph.c \
#	../../neo/libs/freetype/src/gzip/ftgzip.c \
#	../../neo/libs/freetype/src/base/ftinit.c \
#	../../neo/libs/freetype/src/lzw/ftlzw.c \
#	../../neo/libs/freetype/src/base/ftstroke.c \
#	../../neo/libs/freetype/src/base/ftsystem.c \
#	../../neo/libs/freetype/src/smooth/smooth.c \
#	../../neo/libs/freetype/src/base/ftbbox.c \
#	../../neo/libs/freetype/src/base/ftmm.c \
#	../../neo/libs/freetype/src/base/ftpfr.c \
#	../../neo/libs/freetype/src/base/ftsynth.c \
#	../../neo/libs/freetype/src/base/fttype1.c \
#	../../neo/libs/freetype/src/base/ftwinfnt.c \
#	../../neo/libs/freetype/src/pcf/pcf.c \
#	../../neo/libs/freetype/src/pfr/pfr.c \
#	../../neo/libs/freetype/src/psaux/psaux.c \
#	../../neo/libs/freetype/src/pshinter/pshinter.c \
#	../../neo/libs/freetype/src/psnames/psmodule.c \
#	../../neo/libs/freetype/src/raster/raster.c \
#	../../neo/libs/freetype/src/sfnt/sfnt.c \
#	../../neo/libs/freetype/src/truetype/truetype.c \
#	../../neo/libs/freetype/src/type1/type1.c \
#	../../neo/libs/freetype/src/cid/type1cid.c \
#	../../neo/libs/freetype/src/type42/type42.c \
#	../../neo/libs/freetype/src/winfonts/winfnt.c \
				
LOCAL_SRC_FILES += $(TEKUUM_SRC)

include $(BUILD_SHARED_LIBRARY)

# PROFILING begin
#$(call import-module,android-ndk-profiler)
# PROFILING end


