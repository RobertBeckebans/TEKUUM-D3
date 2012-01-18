
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
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <malloc.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
// OSS sound interface
// http://www.opensound.com/
//#include <sys/soundcard.h>

#include "../../idlib/precompiled.h"
#include "../../sound/snd_local.h"
#include "../posix/posix_public.h"
#include "android_local.h"


static int             (*s_getPos) (void);
static void            (*s_initAudio) (void *buffer, int size);
static void            (*s_writeAudio) (int offset, int length);

extern "C"
{

void JNI_SetAudioCallbacks(int (*get_pos) (void), void (*write_audio) (int, int), void (*init_audio) (void *, int))
{
	s_getPos = get_pos;
	s_writeAudio = write_audio;
	s_initAudio = init_audio;
}

void JNI_RequestAudioData(void)
{
	//int             offset = (dmapos * bytes_per_sample) & (buf_size - 1);

	//writeAudio(offset, chunkSizeBytes);
	//dmapos += dma.submission_chunk;
}

}

// Techyon BEGIN
#if defined(USE_SOUND_OSS)
const char	*s_driverArgs[]	= { "best", "oss", "alsa", NULL };
#else
const char	*s_driverArgs[]	= { "best", "alsa", NULL };
#endif
// Techyon END

#ifndef NO_ALSA
static idCVar s_driver( "s_driver", s_driverArgs[0], CVAR_SYSTEM | CVAR_ARCHIVE, "sound driver. 'best' will attempt to use alsa and fallback to OSS if not available", s_driverArgs, idCmdSystem::ArgCompletion_String<s_driverArgs> );
#else
static idCVar s_driver( "s_driver", "oss", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_ROM, "sound driver. only OSS is supported in this build" );
#endif

idAudioHardware *idAudioHardware::Alloc() {

#if defined(USE_SOUND_OSS)
	return new idAudioHardwareOSS;
#else
	return NULL;
#endif
}




// OSS sound ----------------------------------------------------

/*
===============
idAudioHardware::~idAudioHardware
===============
*/
idAudioHardware::~idAudioHardware() { }
