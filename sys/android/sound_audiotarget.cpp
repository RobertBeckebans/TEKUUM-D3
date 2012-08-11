
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
#if defined(USE_AUDIOTARGET)

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <malloc.h>
#include <sys/ioctl.h>
#include <sys/mman.h>


#include "../../idlib/precompiled.h"
#include "../../sound/snd_local.h"
#include "../posix/posix_public.h"
#include "android_local.h"


// if you can't write MIXBUFFER_SAMPLES all at once to the audio device, split in MIXBUFFER_CHUNKS
static const int			MIXBUFFER_CHUNKS = 8;

tyAudioHardwareAudioTarget* g_audioHardwareAudioTarget = NULL;

static int s_readChunk = 0;

void JE_RequestAudioData( void )
{
	common->Printf( "JE_RequestAudioData()\n" );
	
#if 1
	if( g_audioHardwareAudioTarget != NULL )// && g_audioHardwareAudioTarget->writeChunks > 0 )
	{
		int len = ( MIXBUFFER_SAMPLES * g_audioHardwareAudioTarget->GetNumberOfSpeakers() * 2 ) / MIXBUFFER_CHUNKS;
		
		int offset = ( s_readChunk % MIXBUFFER_CHUNKS ) * len;
		s_readChunk++;
		
		//int offset = Max( 0, MIXBUFFER_CHUNKS - g_audioHardwareAudioTarget->writeChunks ) * len;
		//int pos = ( intptr_t )g_audioHardwareAudioTarget->GetMixBuffer() + offset;
		
		common->Printf( "JE_RequestAudioData( offset = %i, len = %i )\n", offset, len );
		ji.WriteAudio( offset, len );
		
		g_audioHardwareAudioTarget->writeChunks -= 1;
	}
#endif
}


/*
===============
tyAudioHardwareAudioTarget::Release
===============
*/
void tyAudioHardwareAudioTarget::Release()
{
	if( mixBuffer )
	{
		free( mixBuffer );
		mixBuffer = NULL;
	}
}

/*
=================
tyAudioHardwareAudioTarget::InitFailed
=================
*/
void tyAudioHardwareAudioTarget::InitFailed()
{
	Release();
	cvarSystem->SetCVarBool( "s_noSound", true );
	common->Warning( "sound subsystem disabled\n" );
	common->Printf( "--------------------------------------\n" );
}

/*
=====================
tyAudioHardwareAudioTarget::Initialize
=====================
*/
bool tyAudioHardwareAudioTarget::Initialize( void )
{
	common->Printf( "------ AudioTarget Sound Initialization -----\n" );
	
	// channels
	
	// sanity over number of speakers
	numChannels = 2;
	idSoundSystemLocal::s_numberOfSpeakers.SetInteger( numChannels );
	
	// allocate the final mix buffer
	mixBufferSize = MIXBUFFER_SAMPLES * numChannels * 2;
	mixBuffer = malloc( mixBufferSize );
	common->Printf( "allocated a mix buffer of %d bytes\n", mixBufferSize );
	
	ji.InitAudio( mixBuffer, mixBufferSize );
	
	common->Printf( "--------------------------------------\n" );
	return true;
}

/*
===============
tyAudioHardwareAudioTarget::~tyAudioHardwareAudioTarget
===============
*/
tyAudioHardwareAudioTarget::~tyAudioHardwareAudioTarget()
{
	common->Printf( "----------- OpenSL Sound Shutdown ------------\n" );
	Release();
	common->Printf( "--------------------------------------\n" );
}

/*
=================
tyAudioHardwareAudioTarget::GetMixBufferSize
=================
*/
int tyAudioHardwareAudioTarget::GetMixBufferSize()
{
	return mixBufferSize;
}

/*
=================
tyAudioHardwareAudioTarget::GetMixBuffer
=================
*/
short* tyAudioHardwareAudioTarget::GetMixBuffer()
{
	return ( short* )mixBuffer;
}

/*
===============
tyAudioHardwareAudioTarget::Flush
===============
*/
bool tyAudioHardwareAudioTarget::Flush( void )
{
	int error;
	
	if( writeChunks > 0 )
	{
		Write( true );
	}
	
	return true;
}

/*
===============
tyAudioHardwareAudioTarget::Write
rely on m_freeWriteChunks which has been set in Flush() before engine did the mixing for this MIXBUFFER_SAMPLE
===============
*/
void tyAudioHardwareAudioTarget::Write( bool flushing )
{
	int error;
	
	if( !flushing && writeChunks )
	{
		// if we write after a new mixing loop, we should have m_writeChunk == 0
		// otherwise that last remaining chunk that was never flushed out to the audio device has just been overwritten
		//Sys_Printf( "tyAudioHardwareAudioTarget::Write: %d samples overflowed and dropped\n", remainingFrames );
		
		Sys_Printf( "tyAudioHardwareAudioTarget::Write: %d samples were overflowed and dropped\n", writeChunks * MIXBUFFER_SAMPLES / MIXBUFFER_CHUNKS );
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
	
	// write the max frames you can in one shot - we need to write it all out in Flush() calls before the next Write() happens
	
#if 0
	int len = ( MIXBUFFER_SAMPLES * numChannels * 2 ) / MIXBUFFER_CHUNKS;
	//for( int i = 0; i < MIXBUFFER_CHUNKS; i++)
	{
		//int offset = i * len;
		int offset = 0;
		int pos = ( intptr_t )mixBuffer;// + offset;
		
		SLresult result;
		result = ( *bqPlayerBufferQueue )->Enqueue( bqPlayerBufferQueue, ( byte* )pos, len );
		if( SL_RESULT_SUCCESS != result )
		{
			//common->Printf( "tyAudioHardwareAudioTarget::Write: bqPlayerBufferQueue->Enqueue( data = %p, offset = %i, len = %i ) failed\n", ( byte* )pos, offset, len );
			return;
		}
		
		writeChunks -= 1;
	}
#endif
}

#else
void JE_RequestAudioData( void )
{
	// stub
}

#endif // #if defined(USE_AUDIOTARGET)



