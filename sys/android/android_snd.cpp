
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


class idAudioHardwareOSS : public idAudioHardware
{
	// if you can't write MIXBUFFER_SAMPLES all at once to the audio device, split in MIXBUFFER_CHUNKS
	static const int MIXBUFFER_CHUNKS = 4;
	
	int				m_audio_fd;
	int				m_sample_format;
	unsigned int	m_channels;
	unsigned int	m_speed;
	void*			m_buffer;
	int				m_buffer_size;
	
	// counting the loops through the dma buffer
	int				m_loops;
	
	// how many chunks we have left to write in cases where we need to split
	int				m_writeChunks;
	// how many chunks we can write to the audio device without blocking
	int				m_freeWriteChunks;
	
public:
	idAudioHardwareOSS()
	{
		m_audio_fd = 0;
		m_sample_format = 0;
		m_channels = 0;
		m_speed = 0;
		m_buffer = NULL;
		m_buffer_size = 0;
		m_loops = 0;
		m_writeChunks		= 0;
		m_freeWriteChunks	= 0;
	}
	virtual		~idAudioHardwareOSS();
	
	bool		Initialize( void );
	
	// Linux driver doesn't support memory map API
	bool		Lock( void** pDSLockedBuffer, ulong* dwDSLockedBufferSize )
	{
		return false;
	}
	bool		Unlock( void* pDSLockedBuffer, dword dwDSLockedBufferSize )
	{
		return false;
	}
	bool		GetCurrentPosition( ulong* pdwCurrentWriteCursor )
	{
		return false;
	}
	
	bool		Flush();
	void		Write( bool flushing );
	
	int			GetNumberOfSpeakers()
	{
		return m_channels;
	}
	int			GetMixBufferSize();
	short*		GetMixBuffer();
	
private:
	void		Release( bool bSilent = false );
	void		InitFailed();
	void		ExtractOSSVersion( int version, idStr& str ) const;
};

void JE_RequestAudioData( void )
{
	//int             offset = (dmapos * bytes_per_sample) & (buf_size - 1);
	
	//writeAudio(offset, chunkSizeBytes);
	//dmapos += dma.submission_chunk;
}

// Techyon BEGIN
#if defined(USE_SOUND_OSS)
const char*	s_driverArgs[]	= { "best", "oss", "alsa", NULL };
#else
const char*	s_driverArgs[]	= { "best", "alsa", NULL };
#endif
// Techyon END

#ifndef NO_ALSA
static idCVar s_driver( "s_driver", s_driverArgs[0], CVAR_SYSTEM | CVAR_ARCHIVE, "sound driver. 'best' will attempt to use alsa and fallback to OSS if not available", s_driverArgs, idCmdSystem::ArgCompletion_String<s_driverArgs> );
#else
static idCVar s_driver( "s_driver", "oss", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_ROM, "sound driver. only OSS is supported in this build" );
#endif

idAudioHardware* idAudioHardware::Alloc()
{

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
