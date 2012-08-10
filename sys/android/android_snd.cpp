
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
/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <malloc.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

// for native audio
extern "C"
{
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
}

#include "../../idlib/precompiled.h"
#include "../../sound/snd_local.h"
#include "../posix/posix_public.h"
#include "android_local.h"


// engine interfaces
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;

// output mix interfaces
static SLObjectItf outputMixObject = NULL;
static SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;

// buffer queue player interfaces
static SLObjectItf bqPlayerObject = NULL;
static SLPlayItf bqPlayerPlay;
static SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
static SLEffectSendItf bqPlayerEffectSend;
static SLMuteSoloItf bqPlayerMuteSolo;
static SLVolumeItf bqPlayerVolume;

// pointer and size of the next player buffer to enqueue, and number of remaining buffers
static short* nextBuffer;
static unsigned nextSize;
static int nextCount;




// this callback handler is called every time a buffer finishes playing
static void bqPlayerCallback( SLAndroidSimpleBufferQueueItf bq, void* context )
{
	assert( bq == bqPlayerBufferQueue );
	assert( NULL == context );
	
	// for streaming playback, replace this test by logic to find and fill the next buffer
	if( --nextCount > 0 && NULL != nextBuffer && 0 != nextSize )
	{
		SLresult result;
		
		// enqueue another buffer
		result = ( *bqPlayerBufferQueue )->Enqueue( bqPlayerBufferQueue, nextBuffer, nextSize );
		
		// the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
		// which for this code example would indicate a programming error
		assert( SL_RESULT_SUCCESS == result );
	}
}


class tyAudioHardwareAudioTrack : public idAudioHardware
{
private:
	// if you can't write MIXBUFFER_SAMPLES all at once to the audio device, split in MIXBUFFER_CHUNKS
	static const int			MIXBUFFER_CHUNKS = 4;
	
	unsigned int				numChannels;
	void*						mixBuffer;
	int							mixBufferSize;
	
	// how many frames remaining to be written to the device
	int							remainingFrames;
	
	
	
	
public:
	tyAudioHardwareAudioTrack()
	{
		//paPlayback		= NULL;
		numChannels		= 0;
		mixBuffer		= NULL;
		mixBufferSize	= 0;
		remainingFrames	= 0;
	}
	virtual				~tyAudioHardwareAudioTrack();
	
	bool				Initialize( void );
	
	
	// Linux driver doesn't support memory map API
	bool				Lock( void** pDSLockedBuffer, ulong* dwDSLockedBufferSize )
	{
		return false;
	}
	bool				Unlock( void* pDSLockedBuffer, dword dwDSLockedBufferSize )
	{
		return false;
	}
	bool				GetCurrentPosition( ulong* pdwCurrentWriteCursor )
	{
		return false;
	}
	
	bool				Flush();
	void				Write( bool flushing );
	
	int					GetNumberOfSpeakers( void )
	{
		return numChannels;
	}
	int					GetMixBufferSize( void );
	short*				GetMixBuffer( void );
	
private:
	void				Release();
	void				InitFailed();
	
	void				CreateEngine();
	void				CreateBufferQueueAudioPlayer();
};

static tyAudioHardwareAudioTrack* s_tyAudioHardwareAudioTrack = NULL;

void JE_RequestAudioData( void )
{
	// TODO remove
}


static idCVar s_driver( "s_driver", "opensles", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_ROM, "sound driver. only OpenSLES is supported in this build" );

idAudioHardware* idAudioHardware::Alloc()
{
	s_tyAudioHardwareAudioTrack = new tyAudioHardwareAudioTrack;
	
	return s_tyAudioHardwareAudioTrack;
}




/*
===============
idAudioHardware::~idAudioHardware
===============
*/
idAudioHardware::~idAudioHardware() { }


/*
===============
tyAudioHardwareAudioTrack::Release
===============
*/
void tyAudioHardwareAudioTrack::Release()
{
	/*
	if( paPlayback )
	{
		pa_simple_free( paPlayback );
	}
	*/
	
	if( mixBuffer )
	{
		free( mixBuffer );
		mixBuffer = NULL;
	}
}

/*
=================
tyAudioHardwareAudioTrack::InitFailed
=================
*/
void tyAudioHardwareAudioTrack::InitFailed()
{
	Release();
	cvarSystem->SetCVarBool( "s_noSound", true );
	common->Warning( "sound subsystem disabled\n" );
	common->Printf( "--------------------------------------\n" );
}

/*
=====================
tyAudioHardwareAudioTrack::Initialize
=====================
*/
bool tyAudioHardwareAudioTrack::Initialize( void )
{
	common->Printf( "------ OpenSL Sound Initialization -----\n" );
	
	CreateEngine();
	CreateBufferQueueAudioPlayer();
	
	// channels
	
	// sanity over number of speakers
	numChannels = 2;
	idSoundSystemLocal::s_numberOfSpeakers.SetInteger( 2 );
	
	// allocate the final mix buffer
	mixBufferSize = MIXBUFFER_SAMPLES * numChannels * 2;
	mixBuffer = malloc( mixBufferSize );
	common->Printf( "allocated a mix buffer of %d bytes\n", mixBufferSize );
	
	common->Printf( "--------------------------------------\n" );
	return true;
}

/*
===============
tyAudioHardwareAudioTrack::~tyAudioHardwareAudioTrack
===============
*/
tyAudioHardwareAudioTrack::~tyAudioHardwareAudioTrack()
{
	common->Printf( "----------- OpenSL Sound Shutdown ------------\n" );
	Release();
	common->Printf( "--------------------------------------\n" );
}

/*
=================
tyAudioHardwareAudioTrack::GetMixBufferSize
=================
*/
int tyAudioHardwareAudioTrack::GetMixBufferSize()
{
	return mixBufferSize;
}

/*
=================
tyAudioHardwareAudioTrack::GetMixBuffer
=================
*/
short* tyAudioHardwareAudioTrack::GetMixBuffer()
{
	return ( short* )mixBuffer;
}

/*
===============
tyAudioHardwareAudioTrack::Flush
===============
*/
bool tyAudioHardwareAudioTrack::Flush( void )
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
tyAudioHardwareAudioTrack::Write
rely on m_freeWriteChunks which has been set in Flush() before engine did the mixing for this MIXBUFFER_SAMPLE
===============
*/
void tyAudioHardwareAudioTrack::Write( bool flushing )
{
	int error;
	
	if( !flushing && remainingFrames )
	{
		// if we write after a new mixing loop, we should have m_writeChunk == 0
		// otherwise that last remaining chunk that was never flushed out to the audio device has just been overwritten
		Sys_Printf( "tyAudioHardwareAudioTrack::Write: %d frames overflowed and dropped\n", remainingFrames );
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
	
	//intptr_t pos = ( intptr_t )mixBuffer + ( MIXBUFFER_SAMPLES - remainingFrames ) * numChannels * 2;
	
	/*
	if( pa_simple_write( paPlayback, ( void* ) mixBuffer, mixBufferSize, &error ) < 0 )
	{
		common->Printf( "pa_simple_write() failed: %s", pa_strerror( error ) );
		return;
	}
	else
	{
		remainingFrames = 0;
	}
	*/
	
	short* oldBuffer = nextBuffer;
	nextBuffer = ( short* ) mixBuffer;
	nextSize = mixBufferSize;
	
	if( nextSize > 0 )
	{
		// here we only enqueue one buffer because it is a long clip,
		// but for streaming playback we would typically enqueue at least 2 buffers to start
		SLresult result;
		result = ( *bqPlayerBufferQueue )->Enqueue( bqPlayerBufferQueue, mixBuffer, mixBufferSize );
		if( SL_RESULT_SUCCESS != result )
		{
			common->Printf( "bqPlayerBufferQueue->Enqueue() failed\n" );
			return;
		}
		else
		{
			remainingFrames = 0;
		}
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
	// Techyon RB: 64 bit fixes, changed int to intptr_t
	intptr_t pos = (intptr_t)mixBuffer + ( MIXBUFFER_SAMPLES - remainingFrames ) * numChannels * 2;
	snd_pcm_sframes_t frames = id_snd_pcm_writei( m_pcm_handle, (void*)pos, m_remainingFrames );
	// Techyon END
	
	if ( frames < 0 ) {
		if ( frames != -EAGAIN ) {
			Sys_Printf( "snd_pcm_writei %d frames failed: %s\n", m_remainingFrames, id_snd_strerror( frames ) );
		}
		return;
	}
	
	remainingFrames -= frames;
	*/
}

// create the engine and output mix objects
void tyAudioHardwareAudioTrack::CreateEngine( void )
{
	SLresult result;
	
	// create engine
	result = slCreateEngine( &engineObject, 0, NULL, 0, NULL, NULL );
	assert( SL_RESULT_SUCCESS == result );
	
	// realize the engine
	result = ( *engineObject )->Realize( engineObject, SL_BOOLEAN_FALSE );
	assert( SL_RESULT_SUCCESS == result );
	
	// get the engine interface, which is needed in order to create other objects
	result = ( *engineObject )->GetInterface( engineObject, SL_IID_ENGINE, &engineEngine );
	assert( SL_RESULT_SUCCESS == result );
	
	// create output mix, with environmental reverb specified as a non-required interface
	const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
	const SLboolean req[1] = {SL_BOOLEAN_FALSE};
	result = ( *engineEngine )->CreateOutputMix( engineEngine, &outputMixObject, 1, ids, req );
	assert( SL_RESULT_SUCCESS == result );
	
	// realize the output mix
	result = ( *outputMixObject )->Realize( outputMixObject, SL_BOOLEAN_FALSE );
	assert( SL_RESULT_SUCCESS == result );
}


// create buffer queue audio player
void tyAudioHardwareAudioTrack::CreateBufferQueueAudioPlayer( void )
{
	SLresult result;
	
	// configure audio source
	SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
	/*
	SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_8,
								   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
								   SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN
								  };
	*/
	SLDataFormat_PCM format_pcm;
	format_pcm.formatType = SL_DATAFORMAT_PCM;
	format_pcm.numChannels = 2;
	format_pcm.samplesPerSec = SL_SAMPLINGRATE_44_1;
	format_pcm.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
	format_pcm.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
	format_pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
	format_pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;
	
	SLDataSource audioSrc = {&loc_bufq, &format_pcm};
	
	// configure audio sink
	SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
	SLDataSink audioSnk = {&loc_outmix, NULL};
	
	// create audio player
	const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND,
								  /*SL_IID_MUTESOLO,*/ SL_IID_VOLUME
								 };
	const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
							  /*SL_BOOLEAN_TRUE,*/ SL_BOOLEAN_TRUE
							 };
	result = ( *engineEngine )->CreateAudioPlayer( engineEngine, &bqPlayerObject, &audioSrc, &audioSnk,
			 3, ids, req );
	assert( SL_RESULT_SUCCESS == result );
	
	// realize the player
	result = ( *bqPlayerObject )->Realize( bqPlayerObject, SL_BOOLEAN_FALSE );
	assert( SL_RESULT_SUCCESS == result );
	
	// get the play interface
	result = ( *bqPlayerObject )->GetInterface( bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay );
	assert( SL_RESULT_SUCCESS == result );
	
	// get the buffer queue interface
	result = ( *bqPlayerObject )->GetInterface( bqPlayerObject, SL_IID_BUFFERQUEUE,
			 &bqPlayerBufferQueue );
	assert( SL_RESULT_SUCCESS == result );
	
	// register callback on the buffer queue
	result = ( *bqPlayerBufferQueue )->RegisterCallback( bqPlayerBufferQueue, bqPlayerCallback, NULL );
	assert( SL_RESULT_SUCCESS == result );
	
	// get the effect send interface
	result = ( *bqPlayerObject )->GetInterface( bqPlayerObject, SL_IID_EFFECTSEND,
			 &bqPlayerEffectSend );
	assert( SL_RESULT_SUCCESS == result );
	
#if 0   // mute/solo is not supported for sources that are known to be mono, as this is
	// get the mute/solo interface
	result = ( *bqPlayerObject )->GetInterface( bqPlayerObject, SL_IID_MUTESOLO, &bqPlayerMuteSolo );
	assert( SL_RESULT_SUCCESS == result );
#endif
	
	// get the volume interface
	result = ( *bqPlayerObject )->GetInterface( bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume );
	assert( SL_RESULT_SUCCESS == result );
	
	// set the player's state to playing
	result = ( *bqPlayerPlay )->SetPlayState( bqPlayerPlay, SL_PLAYSTATE_PLAYING );
	assert( SL_RESULT_SUCCESS == result );
}


