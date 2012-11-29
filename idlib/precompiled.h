/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef __PRECOMPILED_H__
#define __PRECOMPILED_H__

#ifdef __cplusplus

//-----------------------------------------------------

#define ID_TIME_T time_t

#ifdef _WIN32

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// prevent auto literal to string conversion

#ifndef _D3SDK
#ifndef GAME_DLL

#ifndef WINVER
#define WINVER				0x501
#endif

#if 0
// Dedicated server hits unresolved when trying to link this way now. Likely because of the 2010/Win7 transition? - TTimo

#ifdef	ID_DEDICATED
// dedicated sets windows version here
#define	_WIN32_WINNT WINVER
#define	WIN32_LEAN_AND_MEAN
#else
// non-dedicated includes MFC and sets windows version here
#include "../tools/comafx/StdAfx.h"			// this will go away when MFC goes away
#endif

// RB begin
#elif defined(USE_MFC_TOOLS)
// RB end

#include "../tools/comafx/StdAfx.h"

#endif

// RB begin
#if !defined(_WINSOCKAPI_)
#include <winsock2.h>
#endif
// RB end

#include <mmsystem.h>
#include <mmreg.h>

#define DIRECTINPUT_VERSION  0x0800			// was 0x0700 with the old mssdk
#define DIRECTSOUND_VERSION  0x0800

#include <dsound.h>
#include <dinput.h>

#endif /* !GAME_DLL */
#endif /* !_D3SDK */

#pragma warning(disable : 4100)				// unreferenced formal parameter
#pragma warning(disable : 4244)				// conversion to smaller type, possible loss of data
#pragma warning(disable : 4714)				// function marked as __forceinline not inlined
#pragma warning(disable : 4996)				// unsafe string operations

// RB begin
#pragma warning(disable : 4189)				// local variable is initialized but not referenced
// RB end

#include <malloc.h>							// no malloc.h on mac or unix
#include <windows.h>						// for qgl.h
#undef FindText								// stupid namespace poluting Microsoft monkeys

#endif /* _WIN32 */

//-----------------------------------------------------

#if !defined( _DEBUG ) && !defined( NDEBUG )
// don't generate asserts
#define NDEBUG
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <ctype.h>
#include <typeinfo>
#include <errno.h>
#include <math.h>

// RB begin
#include <stdint.h>
// RB end

//-----------------------------------------------------

// non-portable system services
#include "../sys/sys_public.h"

// id lib
#include "../idlib/Lib.h"

// framework
#include "../framework/BuildVersion.h"
#include "../framework/BuildDefines.h"
#include "../framework/Licensee.h"
#include "../framework/CmdSystem.h"
#include "../framework/CVarSystem.h"
#include "../framework/Common.h"
#include "../framework/File.h"
#include "../framework/FileSystem.h"
#include "../framework/UsercmdGen.h"

// decls
#include "../framework/DeclManager.h"
#include "../framework/DeclTable.h"
#include "../framework/DeclSkin.h"
#include "../framework/DeclEntityDef.h"
#include "../framework/DeclFX.h"
#include "../framework/DeclParticle.h"
#include "../framework/DeclAF.h"
#include "../framework/DeclPDA.h"

// We have expression parsing and evaluation code in multiple places:
// materials, sound shaders, and guis. We should unify them.
const int MAX_EXPRESSION_OPS = 4096;
const int MAX_EXPRESSION_REGISTERS = 4096;

// renderer
// RB begin
#if !defined(ID_TYPEINFO) && !defined(__ANDROID__)
#include "../libs/glew/include/GL/glew.h"
//#include "../renderer/qgl.h"
#endif

#if defined(__ANDROID__)

#if defined(USE_GLES2)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#elif defined(USE_GLES1)
#include <GLES/gl.h>
#include <GLES/glext.h>
#endif



#if defined(USE_GLES2)

#define glClearDepth glClearDepthf
#define glDepthRange glDepthRangef

#define glLoadIdentity esLoadIdentity
#define glLoadMatrixf esLoadMatrixf
#define glMatrixMode esMatrixMode
#define glOrtho esOrthof
#define glPushMatrix esPushMatrix
#define glPopMatrix glPopMatrix

#define glEnableClientState esEnableClientState
#define glDisableClientState esDisableClientState

#define glVertexPointer esVertexPointer
#define glNormalPointer esNormalPointer
#define glTexCoordPointer esTexCoordPointer
#define glColorPointer esColorPointer

#define glColor4f esColor4f


#define GL_MODELVIEW 0x1700
#define GL_PROJECTION 0x1701

#define GL_VERTEX_ARRAY 0x8074
#define GL_NORMAL_ARRAY 0x8075
#define GL_COLOR_ARRAY 0x8076
#define GL_TEXTURE_COORD_ARRAY 0x8078


#define GL_TEXTURE_CUBE_MAP_EXT GL_TEXTURE_CUBE_MAP
#define GL_TEXTURE_BINDING_CUBE_MAP_EXT GL_TEXTURE_BINDING_CUBE_MAP
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT GL_TEXTURE_CUBE_MAP_POSITIVE_X
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT GL_TEXTURE_CUBE_MAP_NEGATIVE_X
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT GL_TEXTURE_CUBE_MAP_POSITIVE_Y
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT GL_TEXTURE_CUBE_MAP_POSITIVE_Z
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT GL_TEXTURE_CUBE_MAP_NEGATIVE_Z

#elif defined(USE_GLES1)

#define glClearDepth glClearDepthf
#define glDepthRange glDepthRangef
#define glOrtho glOrthof

#define GL_TEXTURE_CUBE_MAP_EXT GL_TEXTURE_CUBE_MAP_OES
#define GL_TEXTURE_BINDING_CUBE_MAP_EXT GL_TEXTURE_BINDING_CUBE_MAP_OES
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT GL_TEXTURE_CUBE_MAP_POSITIVE_X_OES
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT GL_TEXTURE_CUBE_MAP_NEGATIVE_X_OES
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT GL_TEXTURE_CUBE_MAP_POSITIVE_Y_OES
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_OES
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT GL_TEXTURE_CUBE_MAP_POSITIVE_Z_OES
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_OES

#endif

/* -------------------- GL_EXT_texture_compression_s3tc -------------------- */

#ifndef GL_EXT_texture_compression_s3tc
#define GL_EXT_texture_compression_s3tc 1

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3

#endif /* GL_EXT_texture_compression_s3tc */

#endif // defined(__ANDROID__)

// RB end
#include "../renderer/Cinematic.h"
#include "../renderer/Material.h"
#include "../renderer/Model.h"
#include "../renderer/ModelManager.h"
#include "../renderer/RenderSystem.h"
#include "../renderer/RenderWorld.h"

// sound engine
#include "../sound/sound.h"

// asynchronous networking
#include "../framework/async/NetworkSystem.h"

// user interfaces
#include "../ui/ListGUI.h"
#include "../ui/UserInterface.h"

// collision detection system
#include "../cm/CollisionModel.h"

// AAS files and manager
#include "../tools/compilers/aas/AASFile.h"
#include "../tools/compilers/aas/AASFileManager.h"

// game
#if defined(_D3XP)
#include "../d3xp/Game.h"
#else
#include "../game/Game.h"
#endif

//-----------------------------------------------------

#ifndef _D3SDK

#ifdef GAME_DLL

#if defined(_D3XP)
#include "../d3xp/Game_local.h"
#else
#include "../game/Game_local.h"
#endif

#else

#include "../framework/DemoChecksum.h"

// framework
#include "../framework/Compressor.h"
#include "../framework/EventLoop.h"
#include "../framework/KeyInput.h"
#include "../framework/EditField.h"
#include "../framework/Console.h"
#include "../framework/DemoFile.h"
#include "../framework/Session.h"

// asynchronous networking
#include "../framework/async/AsyncNetwork.h"

// The editor entry points are always declared, but may just be
// stubbed out on non-windows platforms.
#include "../tools/edit_public.h"

// Compilers for map, model, video etc. processing.
#include "../tools/compilers/compiler_public.h"

#endif /* !GAME_DLL */

#endif /* !_D3SDK */

//-----------------------------------------------------

#endif	/* __cplusplus */

#endif /* !__PRECOMPILED_H__ */
