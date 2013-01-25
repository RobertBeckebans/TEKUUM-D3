/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012 dhewg (dhewm3)
Copyright (C) 2012 Robert Beckebans
Copyright (C) 2013 Daniel Gibson

This file is part of the Doom 3 GPL Source Code ("Doom 3 Source Code").

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

#include <SDL.h>
#include <SDL_syswm.h>

#include "precompiled.h"
#pragma hdrstop

#include "../renderer/tr_local.h"
#include "sdl_local.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

idCVar sys_videoRam( "sys_videoRam", "0", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_INTEGER, "Texture memory on the video card (in megabytes) - 0: autodetect", 0, 512 );


idCVar in_nograb( "in_nograb", "0", CVAR_SYSTEM | CVAR_NOCHEAT, "prevents input grabbing" );
idCVar r_waylandcompat( "r_waylandcompat", "0", CVAR_SYSTEM | CVAR_NOCHEAT | CVAR_ARCHIVE, "wayland compatible framebuffer" );

static bool grabbed = false;

#if SDL_VERSION_ATLEAST(2, 0, 0)
static SDL_Window* window = NULL;
static SDL_GLContext context = NULL;
#else
static SDL_Surface* window = NULL;
#define SDL_WINDOW_OPENGL SDL_OPENGL
#define SDL_WINDOW_FULLSCREEN SDL_FULLSCREEN
#define SDL_WINDOW_RESIZABLE SDL_RESIZABLE
#endif

/*
===================
GLimp_Init
===================
*/
bool GLimp_Init( glimpParms_t parms )
{
	common->Printf( "Initializing OpenGL subsystem\n" );
	
	if( !SDL_WasInit( SDL_INIT_VIDEO ) )
	{
		if( SDL_Init( SDL_INIT_VIDEO ) )
		{
			common->Error( "Error while initializing SDL: %s", SDL_GetError() );
			return false;
		}
	}
	
	//assert( SDL_WasInit( SDL_INIT_VIDEO ) );
	
	// DG: make window resizable
	Uint32 flags = SDL_WINDOW_OPENGL;// | SDL_WINDOW_RESIZABLE;
	// DG end
	
	if( parms.fullScreen )
		flags |= SDL_WINDOW_FULLSCREEN;
		
	int colorbits = 24;
	int depthbits = 24;
	int stencilbits = 8;
	
	for( int i = 0; i < 16; i++ )
	{
		// 0 - default
		// 1 - minus colorbits
		// 2 - minus depthbits
		// 3 - minus stencil
		if( ( i % 4 ) == 0 && i )
		{
			// one pass, reduce
			switch( i / 4 )
			{
				case 2 :
					if( colorbits == 24 )
						colorbits = 16;
					break;
				case 1 :
					if( depthbits == 24 )
						depthbits = 16;
					else if( depthbits == 16 )
						depthbits = 8;
				case 3 :
					if( stencilbits == 24 )
						stencilbits = 16;
					else if( stencilbits == 16 )
						stencilbits = 8;
			}
		}
		
		int tcolorbits = colorbits;
		int tdepthbits = depthbits;
		int tstencilbits = stencilbits;
		
		if( ( i % 4 ) == 3 )
		{
			// reduce colorbits
			if( tcolorbits == 24 )
				tcolorbits = 16;
		}
		
		if( ( i % 4 ) == 2 )
		{
			// reduce depthbits
			if( tdepthbits == 24 )
				tdepthbits = 16;
			else if( tdepthbits == 16 )
				tdepthbits = 8;
		}
		
		if( ( i % 4 ) == 1 )
		{
			// reduce stencilbits
			if( tstencilbits == 24 )
				tstencilbits = 16;
			else if( tstencilbits == 16 )
				tstencilbits = 8;
			else
				tstencilbits = 0;
		}
		
		int channelcolorbits = 4;
		if( tcolorbits == 24 )
			channelcolorbits = 8;
			
		SDL_GL_SetAttribute( SDL_GL_RED_SIZE, channelcolorbits );
		SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, channelcolorbits );
		SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, channelcolorbits );
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
		SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, tdepthbits );
		SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, tstencilbits );
		
		if( r_waylandcompat.GetBool() )
			SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 0 );
		else
			SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, channelcolorbits );
			
		SDL_GL_SetAttribute( SDL_GL_STEREO, parms.stereo ? 1 : 0 );
		
		SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, parms.multiSamples ? 1 : 0 );
		SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, parms.multiSamples );
		
#if SDL_VERSION_ATLEAST(2, 0, 0)
		
		// RB begin
		if( r_useOpenGL32.GetInteger() > 0 )
		{
			SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
			SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
			
			if( r_debugContext.GetBool() )
			{
				SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG );
			}
		}
		
		if( r_useOpenGL32.GetInteger() > 1 )
		{
			SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
		}
		// RB end
		
		// DG: set display num for fullscreen
		int windowPos = SDL_WINDOWPOS_UNDEFINED;
		if( parms.fullScreen > 0 )
		{
			if( parms.fullScreen > SDL_GetNumVideoDisplays() )
			{
				common->Warning( "Couldn't set display to num %i because we only have %i displays",
								 parms.fullScreen, SDL_GetNumVideoDisplays() );
			}
			else
			{
				// -1 because SDL starts counting displays at 0, while parms.fullScreen starts at 1
				windowPos = SDL_WINDOWPOS_UNDEFINED_DISPLAY( ( parms.fullScreen - 1 ) );
			}
		}
		// TODO: if parms.fullScreen == -1 there should be a borderless window spanning multiple displays
		/*
		 * NOTE that this implicitly handles parms.fullScreen == -2 (from r_fullscreen -2) meaning
		 * "do fullscreen, but I don't care on what monitor", at least on my box it's the monitor with
		 * the mouse cursor.
		 */
		
		
		window = SDL_CreateWindow( GAME_NAME,
								   windowPos,
								   windowPos,
								   parms.width, parms.height, flags );
		// DG end
		context = SDL_GL_CreateContext( window );
		
		if( !window )
		{
			common->DPrintf( "Couldn't set GL mode %d/%d/%d: %s",
							 channelcolorbits, tdepthbits, tstencilbits, SDL_GetError() );
			continue;
		}
		
		if( SDL_GL_SetSwapInterval( r_swapInterval.GetInteger() ) < 0 )
			common->Warning( "SDL_GL_SWAP_CONTROL not supported" );
			
		SDL_GetWindowSize( window, &glConfig.vidWidth, &glConfig.vidHeight );
		
		glConfig.isFullscreen = ( SDL_GetWindowFlags( window ) & SDL_WINDOW_FULLSCREEN ) == SDL_WINDOW_FULLSCREEN;
#else
		// no SDL 2 -> no OpenGL 3 context
		r_useOpenGL32.SetInteger( 0 );
		
		SDL_WM_SetCaption( GAME_NAME, GAME_NAME );
		
		if( SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, r_swapInterval.GetInteger() ) < 0 )
			common->Warning( "SDL_GL_SWAP_CONTROL not supported" );
		
		window = SDL_SetVideoMode( parms.width, parms.height, colorbits, flags );
		if( !window )
		{
			common->DPrintf( "Couldn't set GL mode %d/%d/%d: %s",
							 channelcolorbits, tdepthbits, tstencilbits, SDL_GetError() );
			continue;
		}
		
		glConfig.vidWidth = window->w;
		glConfig.vidHeight = window->h;
		
		glConfig.isFullscreen = ( window->flags & SDL_FULLSCREEN ) == SDL_FULLSCREEN;
#endif
		
		common->Printf( "Using %d color bits, %d depth, %d stencil display\n",
						channelcolorbits, tdepthbits, tstencilbits );
						
		glConfig.colorBits = tcolorbits;
		glConfig.depthBits = tdepthbits;
		glConfig.stencilBits = tstencilbits;
		
		// RB begin
		glConfig.displayFrequency = 60;
		//glConfig.isStereoPixelFormat = parms.stereo;
		//glConfig.multisamples = parms.multiSamples;
		
		//glConfig.pixelAspect = 1.0f;	// FIXME: some monitor modes may be distorted
		// should side-by-side stereo modes be consider aspect 0.5?
		
		// RB end
		
		break;
	}
	
	if( !window )
	{
		common->Printf( "No usable GL mode found: %s", SDL_GetError() );
		return false;
	}
	
	// RB: use glewExperimental to avoid issues with OpenGL 3.x core profiles
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if( r_useOpenGL32.GetInteger() > 1 )
	{
		glewExperimental = GL_TRUE;
	}
#endif
	
	GLenum glewResult = glewInit();
	if( GLEW_OK != glewResult )
	{
		// glewInit failed, something is seriously wrong
		common->Printf( "^3GLimp_Init() - GLEW could not load OpenGL subsystem: %s", glewGetErrorString( glewResult ) );
	}
	else
	{
		common->Printf( "Using GLEW %s\n", glewGetString( GLEW_VERSION ) );
	}
	
	// DG: disable cursor, we have two cursors in menu (because mouse isn't grabbed in menu)
	SDL_ShowCursor( SDL_DISABLE );
	// DG end
	
	return true;
}
/*
===================
 Helper functions for GLimp_SetScreenParms()
===================
*/

#if SDL_VERSION_ATLEAST(2, 0, 0)
// SDL1 doesn't support multiple displays, so the source is much shorter and doesn't need seperate functions
// makes sure the window will be full-screened on the right display and returns the SDL display index
static int ScreenParmsHandleDisplayIndex( glimpParms_t parms )
{
	int displayIdx;
	if( parms.fullScreen > 0 )
	{
		displayIdx = parms.fullScreen - 1; // first display for SDL is 0, in parms it's 1
	}
	else // -2 == use current display
	{
		displayIdx = SDL_GetWindowDisplay( window );
		if( displayIdx < 0 ) // for some reason the display for the window couldn't be detected
			displayIdx = 0;
	}
	
	if( parms.fullScreen > SDL_GetNumVideoDisplays() )
	{
		common->Warning( "Can't set fullscreen mode to display number %i, because SDL2 only knows about %i displays!",
						 parms.fullScreen, SDL_GetNumVideoDisplays() );
		return -1;
	}
	
	if( parms.fullScreen != glConfig.isFullscreen )
	{
		// we have to switch to another display
		if( glConfig.isFullscreen )
		{
			// if we're already in fullscreen mode but want to switch to another monitor
			// we have to go to windowed mode first to move the window.. SDL-oddity.
			SDL_SetWindowFullscreen( window, SDL_FALSE );
		}
		// select display ; SDL_WINDOWPOS_UNDEFINED_DISPLAY() doesn't work.
		int x = SDL_WINDOWPOS_CENTERED_DISPLAY( displayIdx );
		// move window to the center of selected display
		SDL_SetWindowPosition( window, x, x );
	}
	return displayIdx;
}

static bool SetScreenParmsFullscreen( glimpParms_t parms )
{
	SDL_DisplayMode m = {0};
	int displayIdx = ScreenParmsHandleDisplayIndex( parms );
	if( displayIdx < 0 )
		return false;
		
	// get current mode of display the window should be full-screened on
	SDL_GetCurrentDisplayMode( displayIdx, &m );
	
	// change settings in that display mode according to parms
	// FIXME: check if refreshrate, width and height are supported?
	// m.refresh_rate = parms.displayHz;
	m.w = parms.width;
	m.h = parms.height;
	
	// set that displaymode
	if( SDL_SetWindowDisplayMode( window, &m ) < 0 )
	{
		common->Warning( "Couldn't set window mode for fullscreen, reason: %s", SDL_GetError() );
		return false;
	}
	
	// if we're currently not in fullscreen mode, we need to switch to fullscreen
	if( !( SDL_GetWindowFlags( window ) & SDL_WINDOW_FULLSCREEN ) )
	{
		if( SDL_SetWindowFullscreen( window, SDL_TRUE ) < 0 )
		{
			common->Warning( "Couldn't switch to fullscreen mode, reason: %s!", SDL_GetError() );
			return false;
		}
	}
	return true;
}

static bool SetScreenParmsWindowed( glimpParms_t parms )
{
	SDL_SetWindowSize( window, parms.width, parms.height );
	//SDL_SetWindowPosition( window, parms.x, parms.y );
	
	// if we're currently in fullscreen mode, we need to disable that
	if( SDL_GetWindowFlags( window ) & SDL_WINDOW_FULLSCREEN )
	{
		if( SDL_SetWindowFullscreen( window, SDL_FALSE ) < 0 )
		{
			common->Warning( "Couldn't switch to windowed mode, reason: %s!", SDL_GetError() );
			return false;
		}
	}
	return true;
}
#endif // SDL_VERSION_ATLEAST(2, 0, 0)

/*
===================
GLimp_SetScreenParms
===================
*/
bool GLimp_SetScreenParms( glimpParms_t parms )
{
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if( parms.fullScreen > 0 || parms.fullScreen == -2 )
	{
		if( !SetScreenParmsFullscreen( parms ) )
			return false;
	}
	else if( parms.fullScreen == 0 ) // windowed mode
	{
		if( !SetScreenParmsWindowed( parms ) )
			return false;
	}
	else
	{
		common->Warning( "GLimp_SetScreenParms: fullScreen -1 (borderless window for multiple displays) currently unsupported!" );
		return false;
	}
#else // SDL 1.2 - so much shorter, but doesn't handle multiple displays
	SDL_Surface* s = SDL_GetVideoSurface();
	if( s == NULL )
	{
		common->Warning( "GLimp_SetScreenParms: Couldn't get video information, reason: %s", SDL_GetError() );
		return false;
	}
	
	int bitsperpixel = 24;
	if( s->format )
		bitsperpixel = s->format->BitsPerPixel;
	
	Uint32 flags = s->flags;
	
	if( parms.fullScreen )
		flags |= SDL_FULLSCREEN;
	else
		flags &= ~SDL_FULLSCREEN;
	
	s = SDL_SetVideoMode( parms.width, parms.height, bitsperpixel, flags );
	if( s == NULL )
	{
		common->Warning( "GLimp_SetScreenParms: Couldn't set video information, reason: %s", SDL_GetError() );
		return false;
	}
#endif // SDL_VERSION_ATLEAST(2, 0, 0)
	
	// Note: the following stuff would also work with SDL1.2
	SDL_GL_SetAttribute( SDL_GL_STEREO, parms.stereo ? 1 : 0 );
	
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, parms.multiSamples ? 1 : 0 );
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, parms.multiSamples );
	
	glConfig.isFullscreen = parms.fullScreen;
	//glConfig.isStereoPixelFormat = parms.stereo;
	glConfig.vidWidth = parms.width;
	glConfig.vidHeight = parms.height;
	glConfig.displayFrequency = parms.displayHz;
	//glConfig.multisamples = parms.multiSamples;
	
	return true;
}

/*
===================
GLimp_Shutdown
===================
*/
void GLimp_Shutdown()
{
	common->Printf( "Shutting down OpenGL subsystem\n" );
	
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if( context )
	{
		SDL_GL_DeleteContext( context );
		context = NULL;
	}
	
	if( window )
	{
		SDL_DestroyWindow( window );
		window = NULL;
	}
#endif
}

/*
===================
GLimp_SwapBuffers
===================
*/
void GLimp_SwapBuffers()
{
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_GL_SwapWindow( window );
#else
	SDL_GL_SwapBuffers();
#endif
}

/*
=================
GLimp_SetGamma
=================
*/
void GLimp_SetGamma( unsigned short red[256], unsigned short green[256], unsigned short blue[256] )
{
	if( !window )
	{
		common->Warning( "GLimp_SetGamma called without window" );
		return;
	}
	
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if( SDL_SetWindowGammaRamp( window, red, green, blue ) )
#else
	if( SDL_SetGammaRamp( red, green, blue ) )
#endif
		common->Warning( "Couldn't set gamma ramp: %s", SDL_GetError() );
}

/*
=================
GLimp_ActivateContext
=================
*/
void GLimp_ActivateContext()
{
	common->DPrintf( "TODO: GLimp_ActivateContext\n" );
}

/*
=================
GLimp_DeactivateContext
=================
*/
void GLimp_DeactivateContext()
{
	common->DPrintf( "TODO: GLimp_DeactivateContext\n" );
}

/*
===================
GLimp_ExtensionPointer
===================
*/
/*
GLExtension_t GLimp_ExtensionPointer(const char *name) {
	assert(SDL_WasInit(SDL_INIT_VIDEO));

	return (GLExtension_t)SDL_GL_GetProcAddress(name);
}
*/

void GLimp_GrabInput( int flags )
{
	bool grab = flags & GRAB_ENABLE;
	
	if( grab && ( flags & GRAB_REENABLE ) )
		grab = false;
		
	if( flags & GRAB_SETSTATE )
		grabbed = grab;
		
	if( in_nograb.GetBool() )
		grab = false;
		
	if( !window )
	{
		common->Warning( "GLimp_GrabInput called without window" );
		return;
	}
	
#if SDL_VERSION_ATLEAST(2, 0, 0)
	// DG: disabling the cursor is now done once in GLimp_Init() because it should always be disabled
	
	// DG: check for GRAB_ENABLE instead of GRAB_HIDECURSOR because we always wanna hide it
	SDL_SetRelativeMouseMode( flags & GRAB_ENABLE ? SDL_TRUE : SDL_FALSE );
	SDL_SetWindowGrab( window, grab ? SDL_TRUE : SDL_FALSE );
#else
	// DG end
	SDL_WM_GrabInput( grab ? SDL_GRAB_ON : SDL_GRAB_OFF );
#endif
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
	
	/*
	if( !GLimp_OpenDisplay( ) )
	{
		run_once = 64;
		return run_once;
	}
	l_dpy = dpy;
	l_scrnum = scrnum;
	*/
	
	// try ATI /proc read ( for the lack of a better option )
	int fd;
	if( ( fd = open( "/proc/dri/0/umm", O_RDONLY ) ) != -1 )
	{
		int len;
		char umm_buf[ 1024 ];
		char* line;
		if( ( len = read( fd, umm_buf, 1024 ) ) != -1 )
		{
			// should be way enough to get the full file
			// grab "free  LFB = " line and "free  Inv = " lines
			umm_buf[ len - 1 ] = '\0';
			line = umm_buf;
			line = strtok( umm_buf, "\n" );
			int total = 0;
			while( line )
			{
				if( strlen( line ) >= 13 && strstr( line, "max   LFB =" ) == line )
				{
					total += atoi( line + 12 );
				}
				else if( strlen( line ) >= 13 && strstr( line, "max   Inv =" ) == line )
				{
					total += atoi( line + 12 );
				}
				line = strtok( NULL, "\n" );
			}
			if( total )
			{
				run_once = total / 1048576;
				// round to the lower 16Mb
				run_once &= ~15;
				return run_once;
			}
		}
		else
		{
			common->Printf( "read /proc/dri/0/umm failed: %s\n", strerror( errno ) );
		}
	}
	
	common->Printf( "guess failed, return default mid-range VRAM setting ( 256MB VRAM )\n" );
	run_once = 256;
	
	return run_once;
}


