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



#include "../../idlib/precompiled.h"
#include "../../renderer/tr_local.h"
#include "../../sound/snd_local.h"
#include "android_local.h"

idCVar sys_videoRam( "sys_videoRam", "0", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_INTEGER, "Texture memory on the video card (in megabytes) - 0: autodetect", 0, 512 );


void GLimp_PreInit()
{
}

/*
========================
CreateEGLContextOnWindow
========================
*/
static bool CreateEGLContextOnWindow()
{
	//int useOpenGL32 = r_useOpenGL32.GetInteger();
	
	const EGLint configAttribList[] =
	{
		EGL_RED_SIZE,			8,
		EGL_GREEN_SIZE,			8,
		EGL_BLUE_SIZE,			8,
		EGL_ALPHA_SIZE,			8,
		EGL_DEPTH_SIZE,			24,
		EGL_STENCIL_SIZE,		8,
		EGL_SAMPLE_BUFFERS,		0,
		EGL_NONE
	};
	
	const EGLint surfaceAttribList[] =
	{
		//EGL_POST_SUB_BUFFER_SUPPORTED_NV, flags & (ES_WINDOW_POST_SUB_BUFFER_SUPPORTED) ? EGL_TRUE : EGL_FALSE,
		EGL_NONE, EGL_NONE
	};
	
	android.eglDisplay = eglGetDisplay( EGL_DEFAULT_DISPLAY );
	if( android.eglDisplay == EGL_NO_DISPLAY )
	{
		idLib::Printf( "eglGetDisplay failed\n" );
		return false;
	}
	
	EGLint majorVersion, minorVersion;
	if( !eglInitialize( android.eglDisplay, &majorVersion, &minorVersion ) )
	{
		idLib::Printf( "eglInitialize failed\n" );
		return false;
	}
	
	EGLint numConfigs;
	if( !eglGetConfigs( android.eglDisplay, NULL, 0, &numConfigs ) )
	{
		idLib::Printf( "eglGetConfigs failed\n" );
		return false;
	}
	
	EGLConfig config;
	if( !eglChooseConfig( android.eglDisplay, configAttribList, &config, 1, &numConfigs ) )
	{
		idLib::Printf( "eglChooseConfig failed\n" );
		return false;
	}
	
	EGLint redSize, greenSize, blueSize, alphaSize, depthSize, stencilSize;
	
	eglGetConfigAttrib( android.eglDisplay, config, EGL_RED_SIZE, &redSize );
	eglGetConfigAttrib( android.eglDisplay, config, EGL_GREEN_SIZE, &greenSize );
	eglGetConfigAttrib( android.eglDisplay, config, EGL_BLUE_SIZE, &blueSize );
	eglGetConfigAttrib( android.eglDisplay, config, EGL_ALPHA_SIZE, &alphaSize );
	eglGetConfigAttrib( android.eglDisplay, config, EGL_DEPTH_SIZE, &depthSize );
	eglGetConfigAttrib( android.eglDisplay, config, EGL_STENCIL_SIZE, &stencilSize );
	
	glConfig.colorBits = redSize + greenSize + blueSize + alphaSize;
	glConfig.depthBits = depthSize;
	glConfig.stencilBits = stencilSize;
	
	/* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
	     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
	     * As soon as we picked a EGLConfig, we can safely reconfigure the
	     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
	
	EGLint format;
	eglGetConfigAttrib( android.eglDisplay, config, EGL_NATIVE_VISUAL_ID, &format );
	ANativeWindow_setBuffersGeometry( android.app->window, 0, 0, format );
	
	android.eglSurface = eglCreateWindowSurface( android.eglDisplay, config, ( EGLNativeWindowType ) android.app->window, surfaceAttribList );
	if( android.eglSurface == EGL_NO_SURFACE )
	{
		idLib::Printf( "eglCreateWindowSurface failed\n" );
		return false;
	}
	
	const EGLint contextAttribs[] =
	{
		EGL_CONTEXT_CLIENT_VERSION,		3,
		EGL_NONE,
		EGL_NONE,
	};
	android.eglContext = eglCreateContext( android.eglDisplay, config, EGL_NO_CONTEXT, contextAttribs );
	if( android.eglContext == EGL_NO_CONTEXT )
	{
		idLib::Printf( "eglCreateContext failed\n" );
		return false;
	}
	
	glConfig.driverType = GLDRV_OPENGL_ES3;
	
	return true;
}

bool GLimp_Init( glimpParms_t parms )
{
	common->Printf( "----- GLimp_Init -----\n" );
	
	glConfig.isFullscreen = true;
	
	CreateEGLContextOnWindow();
	
	glConfig.isFullscreen = true;
	
	glConfig.displayFrequency = 60;
	glConfig.isStereoPixelFormat = false;
	glConfig.multisamples = false;
	
	glConfig.pixelAspect = 1.0f;	// FIXME: some monitor modes may be distorted
	
	eglQuerySurface( android.eglDisplay, android.eglSurface, EGL_WIDTH, &glConfig.nativeScreenWidth );
	eglQuerySurface( android.eglDisplay, android.eglSurface, EGL_HEIGHT, &glConfig.nativeScreenHeight );
	
	common->Printf( "...making context current: " );
	if( !eglMakeCurrent( android.eglDisplay, android.eglSurface, android.eglSurface, android.eglContext ) )
	{
		common->Printf( "^3failed^0\n" );
		return false;
	}
	common->Printf( "succeeded\n" );
	
	return true;
}


bool GLimp_SetScreenParms( glimpParms_t parms )
{
	common->Printf( "----- GLimp_SetScreenParms -----\n" );
	
	return true;
}

void GLimp_Shutdown()
{
	const char* success[] = { "failed", "success" };
	int retVal;
	
	common->Printf( "Shutting down OpenGL subsystem\n" );
	
	// set current context to NULL
	//if( wglMakeCurrent )
	{
		// FIXME
		//retVal = eglMakeCurrent(EGL_NO_DISPLAY, EGL_NO_CONTEXT, EGL_NO_SURFACE, EGL_NO_SURFACE) == EGL_TRUE;
		//common->Printf( "...wglMakeCurrent( NULL, NULL ): %s\n", success[retVal] );
	}
	
	if( android.eglContext )
	{
	
		retVal = eglDestroyContext( android.eglDisplay, android.eglContext ) == EGL_TRUE;
		common->Printf( "...deleting EGL context: %s\n", success[retVal] );
		android.eglContext = EGL_NO_CONTEXT;
	}
	
	if( android.eglSurface )
	{
		retVal = eglDestroySurface( android.eglDisplay, android.eglSurface ) == EGL_TRUE;
		common->Printf( "...deleting EGL surface: %s\n", success[retVal] );
		android.eglSurface = EGL_NO_SURFACE;
	}
	
	if( android.eglDisplay )
	{
		retVal = eglTerminate( android.eglDisplay ) == EGL_TRUE;
		common->Printf( "...deleting EGL display: %s\n", success[retVal] );
		android.eglDisplay = EGL_NO_DISPLAY;
	}
	
	// restore gamma
	//GLimp_RestoreGamma();
}

void GLimp_SwapBuffers()
{
	//common->Printf( "GLimp_SwapBuffers()\n" );
	
	eglSwapBuffers( android.eglDisplay, android.eglSurface );
}

void GLimp_SetGamma( unsigned short red[256], unsigned short green[256], unsigned short blue[256] )
{
	common->DPrintf( "GLimp_SetGamma stub\n" );
}

/*
================
Sys_GetVideoRam
returns in megabytes
open your own display connection for the query and close it
using the one shared with GLimp_Init is not stable
================
*/
int Sys_GetVideoRam()
{
	static int run_once = 0;
	int major, minor, value;
	
	if( run_once )
	{
		return run_once;
	}
	
	if( sys_videoRam.GetInteger() )
	{
		run_once = sys_videoRam.GetInteger();
		return sys_videoRam.GetInteger();
	}
	
	// try a few strategies to guess the amount of video ram
	common->Printf( "guessing video ram ( use +set sys_videoRam to force ) ..\n" );
	
	// TODO
	
	// try ATI /proc read ( for the lack of a better option )
	/*
	int fd;
	if ( ( fd = open( "/proc/dri/0/umm", O_RDONLY ) ) != -1 ) {
		int len;
		char umm_buf[ 1024 ];
		char *line;
		if ( ( len = read( fd, umm_buf, 1024 ) ) != -1 ) {
			// should be way enough to get the full file
			// grab "free  LFB = " line and "free  Inv = " lines
			umm_buf[ len-1 ] = '\0';
			line = umm_buf;
			line = strtok( umm_buf, "\n" );
			int total = 0;
			while ( line ) {
				if ( strlen( line ) >= 13 && strstr( line, "max   LFB =" ) == line ) {
					total += atoi( line + 12 );
				} else if ( strlen( line ) >= 13 && strstr( line, "max   Inv =" ) == line ) {
					total += atoi( line + 12 );
				}
				line = strtok( NULL, "\n" );
			}
			if ( total ) {
				run_once = total / 1048576;
				// round to the lower 16Mb
				run_once &= ~15;
				return run_once;
			}
		} else {
			common->Printf( "read /proc/dri/0/umm failed: %s\n", strerror( errno ) );
		}
	}
	*/
	
	common->Printf( "guess failed, return default low-end VRAM setting ( 64MB VRAM )\n" );
	run_once = 64;
	return run_once;
}


class idSort_VidMode : public idSort_Quick< vidMode_t, idSort_VidMode >
{
public:
	int Compare( const vidMode_t& a, const vidMode_t& b ) const
	{
		int wd = a.width - b.width;
		int hd = a.height - b.height;
		int fd = a.displayHz - b.displayHz;
		return ( hd != 0 ) ? hd : ( wd != 0 ) ? wd : fd;
	}
};

static void FillStaticVidModes( idList<vidMode_t>& modeList )
{
	modeList.AddUnique( vidMode_t( 320,   240, 60 ) );
	modeList.AddUnique( vidMode_t( 400,   300, 60 ) );
	modeList.AddUnique( vidMode_t( 512,   384, 60 ) );
	modeList.AddUnique( vidMode_t( 640,   480, 60 ) );
	modeList.AddUnique( vidMode_t( 800,   600, 60 ) );
	modeList.AddUnique( vidMode_t( 960,   720, 60 ) );
	modeList.AddUnique( vidMode_t( 1024,  768, 60 ) );
	modeList.AddUnique( vidMode_t( 1152,  864, 60 ) );
	modeList.AddUnique( vidMode_t( 1280,  720, 60 ) );
	modeList.AddUnique( vidMode_t( 1280,  768, 60 ) );
	modeList.AddUnique( vidMode_t( 1280,  800, 60 ) );
	modeList.AddUnique( vidMode_t( 1280, 1024, 60 ) );
	modeList.AddUnique( vidMode_t( 1360,  768, 60 ) );
	modeList.AddUnique( vidMode_t( 1440,  900, 60 ) );
	modeList.AddUnique( vidMode_t( 1680, 1050, 60 ) );
	modeList.AddUnique( vidMode_t( 1600, 1200, 60 ) );
	modeList.AddUnique( vidMode_t( 1920, 1080, 60 ) );
	modeList.AddUnique( vidMode_t( 1920, 1200, 60 ) );
	modeList.AddUnique( vidMode_t( 2048, 1536, 60 ) );
	modeList.AddUnique( vidMode_t( 2560, 1600, 60 ) );
	
	modeList.SortWithTemplate( idSort_VidMode() );
}

/*
====================
R_GetModeListForDisplay
====================
*/
bool R_GetModeListForDisplay( const int requestedDisplayNum, idList<vidMode_t>& modeList )
{
	assert( requestedDisplayNum >= 0 );
	
	modeList.Clear();
	
	FillStaticVidModes( modeList );
	
	return true;
}



