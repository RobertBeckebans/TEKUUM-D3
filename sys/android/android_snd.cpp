
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
//static SLEffectSendItf bqPlayerEffectSend;
//static SLMuteSoloItf bqPlayerMuteSolo;
static SLVolumeItf bqPlayerVolume;




class rbAudioHardwareSLES : public idAudioHardware
{
private:
	// if you can't write MIXBUFFER_SAMPLES all at once to the audio device, split in MIXBUFFER_CHUNKS
	static const int			MIXBUFFER_CHUNKS = 4;
	
	unsigned int				numChannels;
	void*						mixBuffer;
	int							mixBufferSize;
	
	// how many frames remaining to be written to the device
	//int						remainingFrames;
	
	// how many chunks we have left to write in cases where we need to split
	int							writeChunks;
	
	// how many chunks we can write to the audio device without blocking
	//int						freeWriteChunks;
	
	static rbAudioHardwareSLES* instance;
	
	
public:
	rbAudioHardwareSLES()
	{
		instance = this;
		
		numChannels = 0;
		mixBuffer = NULL;
		mixBufferSize = 0;
		//remainingFrames	= 0;
		writeChunks = 0;
	}
	virtual				~rbAudioHardwareSLES();
	
	bool				Initialize();
	
	
	// Linux driver doesn't support memory map API
	bool				Lock( void** pDSLockedBuffer, uint32* dwDSLockedBufferSize )
	{
		return false;
	}
	bool				Unlock( void* pDSLockedBuffer, dword dwDSLockedBufferSize )
	{
		return false;
	}
	bool				GetCurrentPosition( uint32* pdwCurrentWriteCursor )
	{
		return false;
	}
	
	bool				Flush();
	void				Write( bool flushing );
	
	int					GetNumberOfSpeakers()
	{
		return numChannels;
	}
	int					GetMixBufferSize();
	short*				GetMixBuffer();
	
private:
	void				Release();
	void				InitFailed();
	
	void				CreateEngine();
	void				CreateBufferQueueAudioPlayer();
	
	static void			bqPlayerCallback( SLAndroidSimpleBufferQueueItf bq, void* context );
};

rbAudioHardwareSLES* rbAudioHardwareSLES::instance = NULL;


#if 0
static idCVar s_driver( "s_driver", "audiotarget", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_ROM, "sound driver. only AudioTarget is supported in this build" );

idAudioHardware* idAudioHardware::Alloc()
{
	g_audioHardwareAudioTarget = new rbAudioHardwareAudioTarget;
	return g_audioHardwareAudioTarget;
}

#else
static idCVar s_driver( "s_driver", "opensles", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_ROM, "sound driver. only OpenSLES is supported in this build" );

idAudioHardware* idAudioHardware::Alloc()
{
	return new rbAudioHardwareSLES;
}
#endif



/*
===============
idAudioHardware::~idAudioHardware
===============
*/
idAudioHardware::~idAudioHardware() { }


/*
===============
rbAudioHardwareSLES::Release
===============
*/
void rbAudioHardwareSLES::Release()
{
	// destroy buffer queue audio player object, and invalidate all associated interfaces
	if( bqPlayerObject != NULL )
	{
		( *bqPlayerObject )->Destroy( bqPlayerObject );
		bqPlayerObject = NULL;
		bqPlayerPlay = NULL;
		bqPlayerBufferQueue = NULL;
		//bqPlayerEffectSend = NULL;
		//bqPlayerMuteSolo = NULL;
		bqPlayerVolume = NULL;
	}
	
	// destroy output mix object, and invalidate all associated interfaces
	if( outputMixObject != NULL )
	{
		( *outputMixObject )->Destroy( outputMixObject );
		outputMixObject = NULL;
		outputMixEnvironmentalReverb = NULL;
	}
	
	// destroy engine object, and invalidate all associated interfaces
	if( engineObject != NULL )
	{
		( *engineObject )->Destroy( engineObject );
		engineObject = NULL;
		engineEngine = NULL;
	}
	
	if( mixBuffer )
	{
		free( mixBuffer );
		mixBuffer = NULL;
	}
}

/*
=================
rbAudioHardwareSLES::InitFailed
=================
*/
void rbAudioHardwareSLES::InitFailed()
{
	Release();
	cvarSystem->SetCVarBool( "s_noSound", true );
	common->Warning( "sound subsystem disabled\n" );
	common->Printf( "--------------------------------------\n" );
}

/*
=====================
rbAudioHardwareSLES::Initialize
=====================
*/
bool rbAudioHardwareSLES::Initialize()
{
	common->Printf( "------ OpenSL Sound Initialization -----\n" );
	
	// channels
	
	// sanity over number of speakers
	numChannels = 2;
	idSoundSystemLocal::s_numberOfSpeakers.SetInteger( numChannels );
	
	// allocate the final mix buffer
	mixBufferSize = MIXBUFFER_SAMPLES * numChannels * 2;
	mixBuffer = malloc( mixBufferSize );
	common->Printf( "allocated a mix buffer of %d bytes\n", mixBufferSize );
	
	CreateEngine();
	CreateBufferQueueAudioPlayer();
	
	common->Printf( "--------------------------------------\n" );
	return true;
}

/*
===============
rbAudioHardwareSLES::~rbAudioHardwareSLES
===============
*/
rbAudioHardwareSLES::~rbAudioHardwareSLES()
{
	common->Printf( "----------- OpenSL Sound Shutdown ------------\n" );
	Release();
	common->Printf( "--------------------------------------\n" );
}

/*
=================
rbAudioHardwareSLES::GetMixBufferSize
=================
*/
int rbAudioHardwareSLES::GetMixBufferSize()
{
	return mixBufferSize;
}

/*
=================
rbAudioHardwareSLES::GetMixBuffer
=================
*/
short* rbAudioHardwareSLES::GetMixBuffer()
{
	return ( short* )mixBuffer;
}

/*
===============
rbAudioHardwareSLES::Flush
===============
*/
bool rbAudioHardwareSLES::Flush()
{
	int error;
	
	if( writeChunks > 0 )
	{
		//Write( true );
	}
	
	return true;
}

// this callback handler is called every time a buffer finishes playing
void rbAudioHardwareSLES::bqPlayerCallback( SLAndroidSimpleBufferQueueItf bq, void* context )
{
	assert( bq == bqPlayerBufferQueue );
	assert( instance == context );
	
	// OpenSL thread can be different from sound engine thread
	Sys_EnterCriticalSection( CRITICAL_SECTION_ONE );
	
	// finished a chunk
	instance->writeChunks -= 1;
	
#if 0
	if( instance->writeChunks > 0 )
	{
		int len = ( MIXBUFFER_SAMPLES * instance->numChannels * 2 ) / MIXBUFFER_CHUNKS;
		
		int offset = ( MIXBUFFER_CHUNKS - instance->writeChunks ) * len;
		int pos = ( intptr_t )instance->mixBuffer + offset;
		
		SLresult result;
		result = ( *bqPlayerBufferQueue )->Enqueue( bqPlayerBufferQueue, ( byte* )pos, len );
		if( SL_RESULT_SUCCESS != result )
		{
			common->Printf( "rbAudioHardwareSLES::bqPlayerCallback: bqPlayerBufferQueue->Enqueue( data = %p, offset = %i, len = %i ) failed\n", ( byte* )pos, offset, len );
			//return;
		}
		
		//common->Printf( "rbAudioHardwareSLES::bqPlayerCallback: bqPlayerBufferQueue->Enqueue( data = %p, offset = %i, len = %i ) succeeded\n", ( byte* )pos, offset, len );
	}
#endif
	
	Sys_LeaveCriticalSection( CRITICAL_SECTION_ONE );
}

/*
===============
rbAudioHardwareSLES::Write
rely on m_freeWriteChunks which has been set in Flush() before engine did the mixing for this MIXBUFFER_SAMPLE
===============
*/
void rbAudioHardwareSLES::Write( bool flushing )
{
	int error;
	SLresult result;
	
	if( !flushing && writeChunks > 0 )
	{
		// if we write after a new mixing loop, we should have writeChunks == 0
		// otherwise that last remaining chunk that was never flushed out to the audio device has just been overwritten
		
		Sys_Printf( "rbAudioHardwareSLES::Write: %d samples were overflowed and dropped\n", writeChunks * MIXBUFFER_SAMPLES / MIXBUFFER_CHUNKS );
		
		result = ( *bqPlayerBufferQueue )->Clear( bqPlayerBufferQueue );
		if( SL_RESULT_SUCCESS != result )
		{
			common->Printf( "rbAudioHardwareSLES::Write: bqPlayerBufferQueue->Clear() failed\n" );
			return;
		}
	}
	
	if( !flushing )
	{
		// if running after the mix loop, then we have a full buffer to write out
		//remainingFrames = MIXBUFFER_SAMPLES * numChannels * 2;
		
		writeChunks = MIXBUFFER_CHUNKS;
	}
	
	if( writeChunks == 0 )
	{
		return;
	}
	
	if( flushing && writeChunks > 0 )
	{
		return;
	}
	
	// write the max frames you can in one shot - we need to write it all out in Flush() calls before the next Write() happens
	
#if 0
	result = ( *bqPlayerBufferQueue )->Enqueue( bqPlayerBufferQueue, mixBuffer, mixBufferSize );
	if( SL_RESULT_SUCCESS != result )
	{
		common->Printf( "rbAudioHardwareSLES::Write: bqPlayerBufferQueue->Enqueue( mixBuffer = %p, mixBufferSize = %i ) failed\n", mixBuffer, mixBufferSize );
		return;
	}
	else
	{
		writeChunks = 0;
	}
	
#else
	
	int len = mixBufferSize / MIXBUFFER_CHUNKS;
	
#if 1
	for( int i = 0; i < MIXBUFFER_CHUNKS; i++ )
#else
	int i = 0;
#endif
	{
		int offset = i * len;
		int pos = ( intptr_t )mixBuffer + offset;
	
		result = ( *bqPlayerBufferQueue )->Enqueue( bqPlayerBufferQueue, ( byte* )pos, len );
		if( SL_RESULT_SUCCESS != result )
		{
			common->Printf( "rbAudioHardwareSLES::Write: bqPlayerBufferQueue->Enqueue( data = %p, offset = %i, len = %i ) failed\n", ( byte* )pos, offset, len );
			return;
		}
	
		//writeChunks -= 1;
	}
#endif
}

// create the engine and output mix objects
void rbAudioHardwareSLES::CreateEngine()
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
	
	// create output mix
	const SLInterfaceID ids[1] = {SL_IID_VOLUME};
	const SLboolean req[1] = {SL_BOOLEAN_FALSE};
	result = ( *engineEngine )->CreateOutputMix( engineEngine, &outputMixObject, 1, ids, req );
	assert( SL_RESULT_SUCCESS == result );
	
	// realize the output mix
	result = ( *outputMixObject )->Realize( outputMixObject, SL_BOOLEAN_FALSE );
	assert( SL_RESULT_SUCCESS == result );
}


// create buffer queue audio player
void rbAudioHardwareSLES::CreateBufferQueueAudioPlayer()
{
	SLresult result;
	
	// configure audio source
	SLDataLocator_AndroidSimpleBufferQueue loc_bufq;
	loc_bufq.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
	loc_bufq.numBuffers = 4;
	/*
	SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_8,
								   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
								   SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN
								  };
	*/
	SLDataFormat_PCM format_pcm;
	format_pcm.formatType = SL_DATAFORMAT_PCM;
	format_pcm.numChannels = numChannels;
	format_pcm.samplesPerSec = SL_SAMPLINGRATE_44_1;
	format_pcm.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
	format_pcm.containerSize = 16;
	if( numChannels == 2 )
	{
		format_pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
	}
	else
	{
		format_pcm.channelMask = SL_SPEAKER_FRONT_CENTER;
	}
	format_pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;
	
	SLDataSource audioSrc = { &loc_bufq, &format_pcm };
	
	// configure audio sink
	SLDataLocator_OutputMix loc_outmix = { SL_DATALOCATOR_OUTPUTMIX, outputMixObject };
	SLDataSink audioSnk = {&loc_outmix, NULL};
	
	// create audio player
	const SLInterfaceID ids[2] = { SL_IID_BUFFERQUEUE, /*SL_IID_EFFECTSEND,*/ /*SL_IID_MUTESOLO,*/ SL_IID_VOLUME };
	const SLboolean req[2] = { SL_BOOLEAN_TRUE, /*SL_BOOLEAN_TRUE,*/ /*SL_BOOLEAN_TRUE,*/ SL_BOOLEAN_TRUE };
	result = ( *engineEngine )->CreateAudioPlayer( engineEngine, &bqPlayerObject, &audioSrc, &audioSnk, 2, ids, req );
	assert( SL_RESULT_SUCCESS == result );
	
	// realize the player
	result = ( *bqPlayerObject )->Realize( bqPlayerObject, SL_BOOLEAN_FALSE );
	assert( SL_RESULT_SUCCESS == result );
	
	// get the play interface
	result = ( *bqPlayerObject )->GetInterface( bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay );
	assert( SL_RESULT_SUCCESS == result );
	
	// get the buffer queue interface
	result = ( *bqPlayerObject )->GetInterface( bqPlayerObject, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueue );
	assert( SL_RESULT_SUCCESS == result );
	
	// register callback on the buffer queue
	result = ( *bqPlayerBufferQueue )->RegisterCallback( bqPlayerBufferQueue, bqPlayerCallback, this );
	assert( SL_RESULT_SUCCESS == result );
	
#if 0
	// get the effect send interface
	result = ( *bqPlayerObject )->GetInterface( bqPlayerObject, SL_IID_EFFECTSEND, &bqPlayerEffectSend );
	assert( SL_RESULT_SUCCESS == result );
#endif
	
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


