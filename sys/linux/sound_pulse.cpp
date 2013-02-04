/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012 Robert Beckebans

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
#include "../../sound/snd_local.h"
#include "../posix/posix_public.h"
#include "sound.h"

#include <pulse/error.h>

/*
===============
rbAudioHardwarePulseAudio::Release
===============
*/
void rbAudioHardwarePulseAudio::Release()
{

	if( paPlayback )
	{
		pa_simple_free( paPlayback );
	}
	
	if( mixBuffer )
	{
		free( mixBuffer );
		mixBuffer = NULL;
	}
}

/*
=================
rbAudioHardwarePulseAudio::InitFailed
=================
*/
void rbAudioHardwarePulseAudio::InitFailed()
{
	Release();
	cvarSystem->SetCVarBool( "s_noSound", true );
	common->Warning( "sound subsystem disabled\n" );
	common->Printf( "--------------------------------------\n" );
}

/*
=====================
rbAudioHardwarePulseAudio::Initialize
=====================
*/
bool rbAudioHardwarePulseAudio::Initialize()
{
	int err;
	char driverName[128];
	
	common->Printf( "------ PulseAudio Sound Initialization -----\n" );
	
	// sample format to use
	pa_sample_spec sampleSpec;
	
	sampleSpec.channels = 2;
	sampleSpec.format = PA_SAMPLE_S16LE;
	sampleSpec.rate = 44100;
	
	int error;
	paPlayback = pa_simple_new( NULL, "Tekuum", PA_STREAM_PLAYBACK, NULL, "playback", &sampleSpec, NULL, NULL, &error );
	if( paPlayback == NULL )
	{
		common->Printf( "pa_simple_new() failed: %s\n", pa_strerror( error ) );
		InitFailed();
		return false;
	}
	
	// channels
	
	// sanity over number of speakers
#if 0
	if( idSoundSystemLocal::s_numberOfSpeakers.GetInteger() != 6 && idSoundSystemLocal::s_numberOfSpeakers.GetInteger() != 2 )
	{
		common->Warning( "invalid value for s_numberOfSpeakers. Use either 2 or 6" );
		idSoundSystemLocal::s_numberOfSpeakers.SetInteger( 2 );
	}
	
	m_channels = idSoundSystemLocal::s_numberOfSpeakers.GetInteger();
	if( ( err = id_snd_pcm_hw_params_set_channels( m_pcm_handle, hwparams, m_channels ) ) < 0 )
	{
		common->Printf( "error setting %d channels: %s\n", m_channels, id_snd_strerror( err ) );
		if( idSoundSystemLocal::s_numberOfSpeakers.GetInteger() != 2 )
		{
			// fallback to stereo if that works
			m_channels = 2;
			if( ( err = id_snd_pcm_hw_params_set_channels( m_pcm_handle, hwparams, m_channels ) ) < 0 )
			{
				common->Printf( "fallback to stereo failed: %s\n", id_snd_strerror( err ) );
				InitFailed();
				return false;
			}
			else
			{
				common->Printf( "fallback to stereo\n" );
				idSoundSystemLocal::s_numberOfSpeakers.SetInteger( 2 );
			}
		}
		else
		{
			InitFailed();
			return false;
		}
	}
#else
	numChannels = 2;
	common->Printf( "fallback to stereo\n" );
	idSoundSystemLocal::s_numberOfSpeakers.SetInteger( 2 );
#endif
	
	// TODO: can use swparams to setup the device so it doesn't underrun but rather loops over
	// snd_pcm_sw_params_set_stop_threshold
	// To get alsa to just loop on underruns. set the swparam stop_threshold to equal buffer size. The sound buffer will just loop and never throw an xrun.
	
	// allocate the final mix buffer
	mixBufferSize = MIXBUFFER_SAMPLES * numChannels * 2;
	mixBuffer = malloc( mixBufferSize );
	common->Printf( "allocated a mix buffer of %d bytes\n", mixBufferSize );
	
	common->Printf( "--------------------------------------\n" );
	return true;
}

/*
===============
rbAudioHardwarePulseAudio::~rbAudioHardwarePulseAudio
===============
*/
rbAudioHardwarePulseAudio::~rbAudioHardwarePulseAudio()
{
	common->Printf( "----------- SDL Sound Shutdown ------------\n" );
	Release();
	common->Printf( "--------------------------------------\n" );
}

/*
=================
rbAudioHardwarePulseAudio::GetMixBufferSize
=================
*/
int rbAudioHardwarePulseAudio::GetMixBufferSize()
{
	return mixBufferSize;
}

/*
=================
rbAudioHardwarePulseAudio::GetMixBuffer
=================
*/
short* rbAudioHardwarePulseAudio::GetMixBuffer()
{
	return ( short* )mixBuffer;
}

/*
===============
rbAudioHardwarePulseAudio::Flush
===============
*/
bool rbAudioHardwarePulseAudio::Flush()
{
	int error;
	
#if 0
	if( pa_simple_drain( paPlayback, &error ) < 0 )
	{
		common->Printf( "pa_simple_drain() failed: %s", pa_strerror( error ) );
		cvarSystem->SetCVarBool( "s_noSound", true );
		return false;
	}
#endif
	
	Write( true );
	
	return true;
}

/*
===============
rbAudioHardwarePulseAudio::Write
rely on m_freeWriteChunks which has been set in Flush() before engine did the mixing for this MIXBUFFER_SAMPLE
===============
*/
void rbAudioHardwarePulseAudio::Write( bool flushing )
{
	int error;
	
	if( !flushing && remainingFrames )
	{
		// if we write after a new mixing loop, we should have m_writeChunk == 0
		// otherwise that last remaining chunk that was never flushed out to the audio device has just been overwritten
		Sys_Printf( "rbAudioHardwarePulseAudio::Write: %d frames overflowed and dropped\n", remainingFrames );
	}
	
	if( !flushing )
	{
		// if running after the mix loop, then we have a full buffer to write out
		remainingFrames = MIXBUFFER_SAMPLES;
	}
	
	if( remainingFrames == 0 )
	{
		return;
	}
	// write the max frames you can in one shot - we need to write it all out in Flush() calls before the next Write() happens
	
	//intptr_t pos = (intptr_t)mixBuffer + ( MIXBUFFER_SAMPLES - remainingFrames ) * numChannels * 2;
	if( pa_simple_write( paPlayback, ( void* ) mixBuffer, mixBufferSize, &error ) < 0 )
	{
		common->Printf( "pa_simple_write() failed: %s", pa_strerror( error ) );
		return;
	}
	else
	{
		remainingFrames = 0;
	}
	
	/*
	// what to write and how much
	int pos = (int)m_buffer + ( MIXBUFFER_CHUNKS - m_writeChunks ) * m_channels * 2 * MIXBUFFER_SAMPLES / MIXBUFFER_CHUNKS;
	int len = Min( m_writeChunks, m_freeWriteChunks ) * m_channels * 2 * MIXBUFFER_SAMPLES / MIXBUFFER_CHUNKS;
	assert( len > 0 );
	if ( ( ret = write( m_audio_fd, (void*)pos, len ) ) == -1 ) {
		Sys_Printf( "write to audio fd failed: %s\n", strerror( errno ) );
		return;
	}
	if ( len != ret ) {
		Sys_Printf( "short write to audio fd: wrote %d out of %d\n", ret, m_buffer_size );
		return;
	}
	m_writeChunks -= Min( m_writeChunks, m_freeWriteChunks );
	 */
	
	/*
	// RB: 64 bit fixes, changed int to intptr_t
	intptr_t pos = (intptr_t)mixBuffer + ( MIXBUFFER_SAMPLES - remainingFrames ) * numChannels * 2;
	snd_pcm_sframes_t frames = id_snd_pcm_writei( m_pcm_handle, (void*)pos, m_remainingFrames );
	// RB end
	
	if ( frames < 0 ) {
		if ( frames != -EAGAIN ) {
			Sys_Printf( "snd_pcm_writei %d frames failed: %s\n", m_remainingFrames, id_snd_strerror( frames ) );
		}
		return;
	}
	
	remainingFrames -= frames;
	*/
}



