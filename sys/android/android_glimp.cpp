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

idCVar sys_videoRam( "sys_videoRam", "0", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_INTEGER, "Texture memory on the video card (in megabytes) - 0: autodetect", 0, 512 );

// JNI
extern "C"
{

void setResolution(int width, int height)
{
	glConfig.vidWidth = width;
	glConfig.vidHeight = height;
}

}

bool GLimp_Init( glimpParms_t parms )
{
	glConfig.isFullscreen = true;

	// FIXME check these with egl functions
	glConfig.colorBits = 32;
	glConfig.depthBits = 24;
	glConfig.stencilBits = 8;
}

bool GLimp_SetScreenParms( glimpParms_t parms )
{
}

void GLimp_Shutdown( void )
{

}

void GLimp_SwapBuffers( void )
{

}

void GLimp_SetGamma(unsigned short red[256], unsigned short green[256], unsigned short blue[256]) {
	common->DPrintf("GLimp_SetGamma stub\n");
}


void GLimp_WakeBackEnd(void *a) {
	common->DPrintf("GLimp_WakeBackEnd stub\n");
}

void GLimp_FrontEndSleep() {
	common->DPrintf("GLimp_FrontEndSleep stub\n");
}

void *GLimp_BackEndSleep() {
	common->DPrintf("GLimp_BackEndSleep stub\n");
	return 0;
}

bool GLimp_SpawnRenderThread(void (*a) ()) {
	common->DPrintf("GLimp_SpawnRenderThread stub\n");
	return false;
}

void GLimp_ActivateContext() {
	common->DPrintf("GLimp_ActivateContext stub\n");
}

void GLimp_DeactivateContext() {
	common->DPrintf("GLimp_DeactivateContext stub\n");
}

/*
================
Sys_GetVideoRam
returns in megabytes
open your own display connection for the query and close it
using the one shared with GLimp_Init is not stable
================
*/
int Sys_GetVideoRam( void ) {
	static int run_once = 0;
	int major, minor, value;

	if ( run_once ) {
		return run_once;
	}

	if ( sys_videoRam.GetInteger() ) {
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





