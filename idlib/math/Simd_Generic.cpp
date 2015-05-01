/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#pragma hdrstop
#include "precompiled.h"
#include "Simd_Generic.h"

//===============================================================
//
//	Generic implementation of idSIMDProcessor
//
//===============================================================

#define UNROLL1(Y) { int _IX; for (_IX=0;_IX<count;_IX++) {Y(_IX);} }
#define UNROLL2(Y) { int _IX, _NM = count&0xfffffffe; for (_IX=0;_IX<_NM;_IX+=2){Y(_IX+0);Y(_IX+1);} if (_IX < count) {Y(_IX);}}
#define UNROLL4(Y) { int _IX, _NM = count&0xfffffffc; for (_IX=0;_IX<_NM;_IX+=4){Y(_IX+0);Y(_IX+1);Y(_IX+2);Y(_IX+3);}for(;_IX<count;_IX++){Y(_IX);}}
#define UNROLL8(Y) { int _IX, _NM = count&0xfffffff8; for (_IX=0;_IX<_NM;_IX+=8){Y(_IX+0);Y(_IX+1);Y(_IX+2);Y(_IX+3);Y(_IX+4);Y(_IX+5);Y(_IX+6);Y(_IX+7);} _NM = count&0xfffffffe; for(;_IX<_NM;_IX+=2){Y(_IX); Y(_IX+1);} if (_IX < count) {Y(_IX);} }


/*
============
idSIMD_Generic::GetName
============
*/
const char* idSIMD_Generic::GetName() const
{
	return "generic code";
}

/*
============
idSIMD_Generic::MinMax
============
*/
void VPCALL idSIMD_Generic::MinMax( float& min, float& max, const float* src, const int count )
{
	min = idMath::INFINITY;
	max = -idMath::INFINITY;
#define OPER(X) if ( src[(X)] < min ) {min = src[(X)];} if ( src[(X)] > max ) {max = src[(X)];}
	UNROLL1( OPER )
#undef OPER
}

/*
============
idSIMD_Generic::MinMax
============
*/
void VPCALL idSIMD_Generic::MinMax( idVec2& min, idVec2& max, const idVec2* src, const int count )
{
	min[0] = min[1] = idMath::INFINITY;
	max[0] = max[1] = -idMath::INFINITY;
#define OPER(X) const idVec2 &v = src[(X)]; if ( v[0] < min[0] ) { min[0] = v[0]; } if ( v[0] > max[0] ) { max[0] = v[0]; } if ( v[1] < min[1] ) { min[1] = v[1]; } if ( v[1] > max[1] ) { max[1] = v[1]; }
	UNROLL1( OPER )
#undef OPER
}

/*
============
idSIMD_Generic::MinMax
============
*/
void VPCALL idSIMD_Generic::MinMax( idVec3& min, idVec3& max, const idVec3* src, const int count )
{
	min[0] = min[1] = min[2] = idMath::INFINITY;
	max[0] = max[1] = max[2] = -idMath::INFINITY;
#define OPER(X) const idVec3 &v = src[(X)]; if ( v[0] < min[0] ) { min[0] = v[0]; } if ( v[0] > max[0] ) { max[0] = v[0]; } if ( v[1] < min[1] ) { min[1] = v[1]; } if ( v[1] > max[1] ) { max[1] = v[1]; } if ( v[2] < min[2] ) { min[2] = v[2]; } if ( v[2] > max[2] ) { max[2] = v[2]; }
	UNROLL1( OPER )
#undef OPER
}

/*
============
idSIMD_Generic::MinMax
============
*/
void VPCALL idSIMD_Generic::MinMax( idVec3& min, idVec3& max, const idDrawVert* src, const int count )
{
	min[0] = min[1] = min[2] = idMath::INFINITY;
	max[0] = max[1] = max[2] = -idMath::INFINITY;
#define OPER(X) const idVec3 &v = src[(X)].xyz; if ( v[0] < min[0] ) { min[0] = v[0]; } if ( v[0] > max[0] ) { max[0] = v[0]; } if ( v[1] < min[1] ) { min[1] = v[1]; } if ( v[1] > max[1] ) { max[1] = v[1]; } if ( v[2] < min[2] ) { min[2] = v[2]; } if ( v[2] > max[2] ) { max[2] = v[2]; }
	UNROLL1( OPER )
#undef OPER
}

/*
============
idSIMD_Generic::MinMax
============
*/
void VPCALL idSIMD_Generic::MinMax( idVec3& min, idVec3& max, const idDrawVert* src, const triIndex_t* indexes, const int count )
{
	min[0] = min[1] = min[2] = idMath::INFINITY;
	max[0] = max[1] = max[2] = -idMath::INFINITY;
#define OPER(X) const idVec3 &v = src[indexes[(X)]].xyz; if ( v[0] < min[0] ) { min[0] = v[0]; } if ( v[0] > max[0] ) { max[0] = v[0]; } if ( v[1] < min[1] ) { min[1] = v[1]; } if ( v[1] > max[1] ) { max[1] = v[1]; } if ( v[2] < min[2] ) { min[2] = v[2]; } if ( v[2] > max[2] ) { max[2] = v[2]; }
	UNROLL1( OPER )
#undef OPER
}

/*
================
idSIMD_Generic::Memcpy
================
*/
void VPCALL idSIMD_Generic::Memcpy( void* dst, const void* src, const int count )
{
	memcpy( dst, src, count );
}

/*
================
idSIMD_Generic::Memset
================
*/
void VPCALL idSIMD_Generic::Memset( void* dst, const int val, const int count )
{
	memset( dst, val, count );
}

/*
============
idSIMD_Generic::BlendJoints
============
*/
void VPCALL idSIMD_Generic::BlendJoints( idJointQuat* joints, const idJointQuat* blendJoints, const float lerp, const int* index, const int numJoints )
{
	for( int i = 0; i < numJoints; i++ )
	{
		int j = index[i];
		joints[j].q.Slerp( joints[j].q, blendJoints[j].q, lerp );
		joints[j].t.Lerp( joints[j].t, blendJoints[j].t, lerp );
		joints[j].w = 0.0f;
	}
}

/*
============
idSIMD_Generic::BlendJointsFast
============
*/
void VPCALL idSIMD_Generic::BlendJointsFast( idJointQuat* joints, const idJointQuat* blendJoints, const float lerp, const int* index, const int numJoints )
{
	for( int i = 0; i < numJoints; i++ )
	{
		int j = index[i];
		joints[j].q.Lerp( joints[j].q, blendJoints[j].q, lerp );
		joints[j].t.Lerp( joints[j].t, blendJoints[j].t, lerp );
		joints[j].w = 0.0f;
	}
}

/*
============
idSIMD_Generic::ConvertJointQuatsToJointMats
============
*/
void VPCALL idSIMD_Generic::ConvertJointQuatsToJointMats( idJointMat* jointMats, const idJointQuat* jointQuats, const int numJoints )
{
	for( int i = 0; i < numJoints; i++ )
	{
		jointMats[i].SetRotation( jointQuats[i].q.ToMat3() );
		jointMats[i].SetTranslation( jointQuats[i].t );
	}
}

/*
============
idSIMD_Generic::ConvertJointMatsToJointQuats
============
*/
void VPCALL idSIMD_Generic::ConvertJointMatsToJointQuats( idJointQuat* jointQuats, const idJointMat* jointMats, const int numJoints )
{
	for( int i = 0; i < numJoints; i++ )
	{
		jointQuats[i] = jointMats[i].ToJointQuat();
	}
}

/*
============
idSIMD_Generic::TransformJoints
============
*/
void VPCALL idSIMD_Generic::TransformJoints( idJointMat* jointMats, const int* parents, const int firstJoint, const int lastJoint )
{
	for( int i = firstJoint; i <= lastJoint; i++ )
	{
		assert( parents[i] < i );
		jointMats[i] *= jointMats[parents[i]];
	}
}

/*
============
idSIMD_Generic::UntransformJoints
============
*/
void VPCALL idSIMD_Generic::UntransformJoints( idJointMat* jointMats, const int* parents, const int firstJoint, const int lastJoint )
{
	for( int i = lastJoint; i >= firstJoint; i-- )
	{
		assert( parents[i] < i );
		jointMats[i] /= jointMats[parents[i]];
	}
}





/*
============
idSIMD_Generic::UpSamplePCMTo44kHz

  Duplicate samples for 44kHz output.
============
*/
void idSIMD_Generic::UpSamplePCMTo44kHz( float* dest, const short* src, const int numSamples, const int kHz, const int numChannels )
{
	if( kHz == 11025 )
	{
		if( numChannels == 1 )
		{
			for( int i = 0; i < numSamples; i++ )
			{
				dest[i * 4 + 0] = dest[i * 4 + 1] = dest[i * 4 + 2] = dest[i * 4 + 3] = ( float ) src[i + 0];
			}
		}
		else
		{
			for( int i = 0; i < numSamples; i += 2 )
			{
				dest[i * 4 + 0] = dest[i * 4 + 2] = dest[i * 4 + 4] = dest[i * 4 + 6] = ( float ) src[i + 0];
				dest[i * 4 + 1] = dest[i * 4 + 3] = dest[i * 4 + 5] = dest[i * 4 + 7] = ( float ) src[i + 1];
			}
		}
	}
	else if( kHz == 22050 )
	{
		if( numChannels == 1 )
		{
			for( int i = 0; i < numSamples; i++ )
			{
				dest[i * 2 + 0] = dest[i * 2 + 1] = ( float ) src[i + 0];
			}
		}
		else
		{
			for( int i = 0; i < numSamples; i += 2 )
			{
				dest[i * 2 + 0] = dest[i * 2 + 2] = ( float ) src[i + 0];
				dest[i * 2 + 1] = dest[i * 2 + 3] = ( float ) src[i + 1];
			}
		}
	}
	else if( kHz == 44100 )
	{
		for( int i = 0; i < numSamples; i++ )
		{
			dest[i] = ( float ) src[i];
		}
	}
	else
	{
		assert( 0 );
	}
}

/*
============
idSIMD_Generic::UpSampleOGGTo44kHz

  Duplicate samples for 44kHz output.
============
*/
void idSIMD_Generic::UpSampleOGGTo44kHz( float* dest, const float* const* ogg, const int numSamples, const int kHz, const int numChannels )
{
	if( kHz == 11025 )
	{
		if( numChannels == 1 )
		{
			for( int i = 0; i < numSamples; i++ )
			{
				dest[i * 4 + 0] = dest[i * 4 + 1] = dest[i * 4 + 2] = dest[i * 4 + 3] = ogg[0][i] * 32768.0f;
			}
		}
		else
		{
			for( int i = 0; i < numSamples >> 1; i++ )
			{
				dest[i * 8 + 0] = dest[i * 8 + 2] = dest[i * 8 + 4] = dest[i * 8 + 6] = ogg[0][i] * 32768.0f;
				dest[i * 8 + 1] = dest[i * 8 + 3] = dest[i * 8 + 5] = dest[i * 8 + 7] = ogg[1][i] * 32768.0f;
			}
		}
	}
	else if( kHz == 22050 )
	{
		if( numChannels == 1 )
		{
			for( int i = 0; i < numSamples; i++ )
			{
				dest[i * 2 + 0] = dest[i * 2 + 1] = ogg[0][i] * 32768.0f;
			}
		}
		else
		{
			for( int i = 0; i < numSamples >> 1; i++ )
			{
				dest[i * 4 + 0] = dest[i * 4 + 2] = ogg[0][i] * 32768.0f;
				dest[i * 4 + 1] = dest[i * 4 + 3] = ogg[1][i] * 32768.0f;
			}
		}
	}
	else if( kHz == 44100 )
	{
		if( numChannels == 1 )
		{
			for( int i = 0; i < numSamples; i++ )
			{
				dest[i * 1 + 0] = ogg[0][i] * 32768.0f;
			}
		}
		else
		{
			for( int i = 0; i < numSamples >> 1; i++ )
			{
				dest[i * 2 + 0] = ogg[0][i] * 32768.0f;
				dest[i * 2 + 1] = ogg[1][i] * 32768.0f;
			}
		}
	}
	else
	{
		assert( 0 );
	}
}

/*
============
idSIMD_Generic::MixSoundTwoSpeakerMono
============
*/
void VPCALL idSIMD_Generic::MixSoundTwoSpeakerMono( float* mixBuffer, const float* samples, const int numSamples, const float lastV[2], const float currentV[2] )
{
	float sL = lastV[0];
	float sR = lastV[1];
	float incL = ( currentV[0] - lastV[0] ) / MIXBUFFER_SAMPLES;
	float incR = ( currentV[1] - lastV[1] ) / MIXBUFFER_SAMPLES;
	
	assert( numSamples == MIXBUFFER_SAMPLES );
	
	for( int j = 0; j < MIXBUFFER_SAMPLES; j++ )
	{
		mixBuffer[j * 2 + 0] += samples[j] * sL;
		mixBuffer[j * 2 + 1] += samples[j] * sR;
		sL += incL;
		sR += incR;
	}
}

/*
============
idSIMD_Generic::MixSoundTwoSpeakerStereo
============
*/
void VPCALL idSIMD_Generic::MixSoundTwoSpeakerStereo( float* mixBuffer, const float* samples, const int numSamples, const float lastV[2], const float currentV[2] )
{
	float sL = lastV[0];
	float sR = lastV[1];
	float incL = ( currentV[0] - lastV[0] ) / MIXBUFFER_SAMPLES;
	float incR = ( currentV[1] - lastV[1] ) / MIXBUFFER_SAMPLES;
	
	assert( numSamples == MIXBUFFER_SAMPLES );
	
	for( int j = 0; j < MIXBUFFER_SAMPLES; j++ )
	{
		mixBuffer[j * 2 + 0] += samples[j * 2 + 0] * sL;
		mixBuffer[j * 2 + 1] += samples[j * 2 + 1] * sR;
		sL += incL;
		sR += incR;
	}
}

/*
============
idSIMD_Generic::MixSoundSixSpeakerMono
============
*/
void VPCALL idSIMD_Generic::MixSoundSixSpeakerMono( float* mixBuffer, const float* samples, const int numSamples, const float lastV[6], const float currentV[6] )
{
	float sL0 = lastV[0];
	float sL1 = lastV[1];
	float sL2 = lastV[2];
	float sL3 = lastV[3];
	float sL4 = lastV[4];
	float sL5 = lastV[5];
	
	float incL0 = ( currentV[0] - lastV[0] ) / MIXBUFFER_SAMPLES;
	float incL1 = ( currentV[1] - lastV[1] ) / MIXBUFFER_SAMPLES;
	float incL2 = ( currentV[2] - lastV[2] ) / MIXBUFFER_SAMPLES;
	float incL3 = ( currentV[3] - lastV[3] ) / MIXBUFFER_SAMPLES;
	float incL4 = ( currentV[4] - lastV[4] ) / MIXBUFFER_SAMPLES;
	float incL5 = ( currentV[5] - lastV[5] ) / MIXBUFFER_SAMPLES;
	
	assert( numSamples == MIXBUFFER_SAMPLES );
	
	for( int i = 0; i < MIXBUFFER_SAMPLES; i++ )
	{
		mixBuffer[i * 6 + 0] += samples[i] * sL0;
		mixBuffer[i * 6 + 1] += samples[i] * sL1;
		mixBuffer[i * 6 + 2] += samples[i] * sL2;
		mixBuffer[i * 6 + 3] += samples[i] * sL3;
		mixBuffer[i * 6 + 4] += samples[i] * sL4;
		mixBuffer[i * 6 + 5] += samples[i] * sL5;
		sL0 += incL0;
		sL1 += incL1;
		sL2 += incL2;
		sL3 += incL3;
		sL4 += incL4;
		sL5 += incL5;
	}
}

/*
============
idSIMD_Generic::MixSoundSixSpeakerStereo
============
*/
void VPCALL idSIMD_Generic::MixSoundSixSpeakerStereo( float* mixBuffer, const float* samples, const int numSamples, const float lastV[6], const float currentV[6] )
{
	float sL0 = lastV[0];
	float sL1 = lastV[1];
	float sL2 = lastV[2];
	float sL3 = lastV[3];
	float sL4 = lastV[4];
	float sL5 = lastV[5];
	
	float incL0 = ( currentV[0] - lastV[0] ) / MIXBUFFER_SAMPLES;
	float incL1 = ( currentV[1] - lastV[1] ) / MIXBUFFER_SAMPLES;
	float incL2 = ( currentV[2] - lastV[2] ) / MIXBUFFER_SAMPLES;
	float incL3 = ( currentV[3] - lastV[3] ) / MIXBUFFER_SAMPLES;
	float incL4 = ( currentV[4] - lastV[4] ) / MIXBUFFER_SAMPLES;
	float incL5 = ( currentV[5] - lastV[5] ) / MIXBUFFER_SAMPLES;
	
	assert( numSamples == MIXBUFFER_SAMPLES );
	
	for( int i = 0; i < MIXBUFFER_SAMPLES; i++ )
	{
		mixBuffer[i * 6 + 0] += samples[i * 2 + 0] * sL0;
		mixBuffer[i * 6 + 1] += samples[i * 2 + 1] * sL1;
		mixBuffer[i * 6 + 2] += samples[i * 2 + 0] * sL2;
		mixBuffer[i * 6 + 3] += samples[i * 2 + 0] * sL3;
		mixBuffer[i * 6 + 4] += samples[i * 2 + 0] * sL4;
		mixBuffer[i * 6 + 5] += samples[i * 2 + 1] * sL5;
		sL0 += incL0;
		sL1 += incL1;
		sL2 += incL2;
		sL3 += incL3;
		sL4 += incL4;
		sL5 += incL5;
	}
}

/*
============
idSIMD_Generic::MixedSoundToSamples
============
*/
void VPCALL idSIMD_Generic::MixedSoundToSamples( short* samples, const float* mixBuffer, const int numSamples )
{

	for( int i = 0; i < numSamples; i++ )
	{
		if( mixBuffer[i] <= -32768.0f )
		{
			samples[i] = -32768;
		}
		else if( mixBuffer[i] >= 32767.0f )
		{
			samples[i] = 32767;
		}
		else
		{
			samples[i] = ( short ) mixBuffer[i];
		}
	}
}
