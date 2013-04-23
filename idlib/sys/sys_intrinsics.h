/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012 monocasa
Copyright (C) 2013 Robert Beckebans

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
#ifndef __SYS_INTRIINSICS_H__
#define __SYS_INTRIINSICS_H__

//#define USE_INTRINSICS_EMU

#if !defined(USE_INTRINSICS_EMU)
#include <emmintrin.h>
#endif

/*
================================================================================================

	SSE intrinsics emulation

================================================================================================
*/

#if defined(USE_INTRINSICS_EMU)

#if 1
typedef union
{
	float	f32[2];
	int32	i32[2];
	
} ALIGNTYPE8 __m64;
#else
typedef float __m64[2] ALIGNTYPE8;
#endif


union __m128
{
//	__int128	i128;
	float		f32[4];
	int32		i32[4];
//	int16		i16[8];
//	uint8		u8[16];

} ALIGNTYPE16;

//typedef __m128s __m128i;

union __m128i
{
#if 0
	__m128i() {}
	__m128i( __m128 i )
	{
		i32[0] = i.i32[0];
		i32[1] = i.i32[1];
		i32[2] = i.i32[2];
		i32[3] = i.i32[3];
	}
#endif
	
	__int128	i128;
	float		f32[4];
	int32		i32[4];
	int16		i16[8];
	uint8		u8[16];
	int8		i8[16];
	
} ALIGNTYPE16;











/*
================================================
	Arithmetic Operations
================================================
*/


ID_FORCE_INLINE_EXTERN __m128 _mm_add_ps( __m128 a, __m128 b )
{
	__m128 ret;
	
	ret.f32[0] = a.f32[0] + b.f32[0];
	ret.f32[1] = a.f32[1] + b.f32[1];
	ret.f32[2] = a.f32[2] + b.f32[2];
	ret.f32[3] = a.f32[3] + b.f32[3];
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128 _mm_sub_ps( __m128 a, __m128 b )
{
	__m128 ret;
	
	ret.f32[0] = a.f32[0] - b.f32[0];
	ret.f32[1] = a.f32[1] - b.f32[1];
	ret.f32[2] = a.f32[2] - b.f32[2];
	ret.f32[3] = a.f32[3] - b.f32[3];
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128 _mm_mul_ps( __m128 a, __m128 b )
{
	__m128 ret;
	
	ret.f32[0] = a.f32[0] * b.f32[0];
	ret.f32[1] = a.f32[1] * b.f32[1];
	ret.f32[2] = a.f32[2] * b.f32[2];
	ret.f32[3] = a.f32[3] * b.f32[3];
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128 _mm_rcp_ps( __m128 a )
{
	__m128 ret;
	
	ret.f32[0] = 1.0f / a.f32[0];
	ret.f32[1] = 1.0f / a.f32[1];
	ret.f32[2] = 1.0f / a.f32[2];
	ret.f32[3] = 1.0f / a.f32[3];
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128 _mm_min_ss( __m128 a, __m128 b )
{
	__m128 ret;
	
	ret.f32[0] = ( a.f32[0] < b.f32[0] ) ? a.f32[0] : b.f32[0];
	ret.f32[1] = a.f32[1];
	ret.f32[2] = a.f32[2];
	ret.f32[3] = a.f32[3];
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128 _mm_min_ps( __m128 a, __m128 b )
{
	__m128 ret;
	
	ret.f32[0] = a.f32[0] < b.f32[0] ? a.f32[0] : b.f32[0];
	ret.f32[1] = a.f32[1] < b.f32[1] ? a.f32[1] : b.f32[1];
	ret.f32[2] = a.f32[2] < b.f32[2] ? a.f32[2] : b.f32[2];
	ret.f32[3] = a.f32[3] < b.f32[3] ? a.f32[3] : b.f32[3];
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128 _mm_max_ss( __m128 a, __m128 b )
{
	__m128 ret;
	
	ret.f32[0] = ( a.f32[0] > b.f32[0] ) ? a.f32[0] : b.f32[0];
	ret.f32[1] = a.f32[1];
	ret.f32[2] = a.f32[2];
	ret.f32[3] = a.f32[3];
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128 _mm_max_ps( __m128 a, __m128 b )
{
	__m128 ret;
	
	ret.f32[0] = a.f32[0] > b.f32[0] ? a.f32[0] : b.f32[0];
	ret.f32[1] = a.f32[1] > b.f32[1] ? a.f32[1] : b.f32[1];
	ret.f32[2] = a.f32[2] > b.f32[2] ? a.f32[2] : b.f32[2];
	ret.f32[3] = a.f32[3] > b.f32[3] ? a.f32[3] : b.f32[3];
	
	return ret;
}

/*
================================================
	Logical Operations
================================================
*/

ID_FORCE_INLINE_EXTERN __m128 _mm_and_ps( __m128 a, __m128 b )
{
	__m128 ret;
	__m128i* reti = ( __m128i* )&ret;
	__m128i* ai = ( __m128i* )&a;
	__m128i* bi = ( __m128i* )&b;
	
	reti->i32[0] = ai->i32[0] & bi->i32[0];
	reti->i32[1] = ai->i32[1] & bi->i32[1];
	reti->i32[2] = ai->i32[2] & bi->i32[2];
	reti->i32[3] = ai->i32[3] & bi->i32[3];
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128 _mm_andnot_ps( __m128 a, __m128 b )
{
	__m128 ret;
	__m128i* reti = ( __m128i* )&ret;
	__m128i* ai = ( __m128i* )&a;
	__m128i* bi = ( __m128i* )&b;
	
	reti->i32[0] = ~ai->i32[0] & bi->i32[0];
	reti->i32[1] = ~ai->i32[1] & bi->i32[1];
	reti->i32[2] = ~ai->i32[2] & bi->i32[2];
	reti->i32[3] = ~ai->i32[3] & bi->i32[3];
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128 _mm_or_ps( __m128 a, __m128 b )
{
	__m128 ret;
	__m128i* reti = ( __m128i* )&ret;
	__m128i* ai = ( __m128i* )&a;
	__m128i* bi = ( __m128i* )&b;
	
	reti->i32[0] = ai->i32[0] | bi->i32[0];
	reti->i32[1] = ai->i32[1] | bi->i32[1];
	reti->i32[2] = ai->i32[2] | bi->i32[2];
	reti->i32[3] = ai->i32[3] | bi->i32[3];
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128 _mm_xor_ps( __m128 a, __m128 b )
{
	__m128 ret;
	__m128i* reti = ( __m128i* )&ret;
	__m128i* ai = ( __m128i* )&a;
	__m128i* bi = ( __m128i* )&b;
	
	reti->i32[0] = ai->i32[0] ^ bi->i32[0];
	reti->i32[1] = ai->i32[1] ^ bi->i32[1];
	reti->i32[2] = ai->i32[2] ^ bi->i32[2];
	reti->i32[3] = ai->i32[3] ^ bi->i32[3];
	
	return ret;
}


/*
================================================
	Comparisons
================================================
*/

ID_FORCE_INLINE_EXTERN __m128 _mm_cmpeq_ps( __m128 a, __m128 b )
{
	__m128 ret;
	
	ret.f32[0] = a.f32[0] == b.f32[0] ? 0xFFFFFFFF : 0;
	ret.f32[1] = a.f32[1] == b.f32[1] ? 0xFFFFFFFF : 0;
	ret.f32[2] = a.f32[2] == b.f32[2] ? 0xFFFFFFFF : 0;
	ret.f32[3] = a.f32[3] == b.f32[3] ? 0xFFFFFFFF : 0;
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128 _mm_cmplt_ps( __m128 a, __m128 b )
{
	__m128 ret;
	
	ret.f32[0] = a.f32[0] < b.f32[0] ? 0xFFFFFFFF : 0;
	ret.f32[1] = a.f32[1] < b.f32[1] ? 0xFFFFFFFF : 0;
	ret.f32[2] = a.f32[2] < b.f32[2] ? 0xFFFFFFFF : 0;
	ret.f32[3] = a.f32[3] < b.f32[3] ? 0xFFFFFFFF : 0;
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128 _mm_cmpgt_ps( __m128 a, __m128 b )
{
	__m128 ret;
	
	ret.f32[0] = a.f32[0] > b.f32[0] ? 0xFFFFFFFF : 0;
	ret.f32[1] = a.f32[1] > b.f32[1] ? 0xFFFFFFFF : 0;
	ret.f32[2] = a.f32[2] > b.f32[2] ? 0xFFFFFFFF : 0;
	ret.f32[3] = a.f32[3] > b.f32[3] ? 0xFFFFFFFF : 0;
	
	return ret;
}


/*
================================================
	Conversions
================================================
*/

ID_FORCE_INLINE_EXTERN int _mm_cvttss_si32( __m128& a )
{
	return ( int )a.f32[0];
}


/*
================================================
	Load Operations
================================================
*/

ID_FORCE_INLINE_EXTERN __m128 _mm_loadh_pi( __m128 a, __m64 const* p )
{
	__m128 ret;
	
	ret.f32[0] = a.f32[0];
	ret.f32[1] = a.f32[1];
	ret.f32[2] = p->f32[0];
	ret.f32[3] = p->f32[1];
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128 _mm_load_ss( const float* p )
{
	__m128 ret;
	
	ret.f32[0] = *p;
	ret.f32[1] = 0;
	ret.f32[2] = 0;
	ret.f32[3] = 0;
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128 _mm_load1_ps( const float* p )
{
	__m128 ret;
	
	ret.f32[0] = *p;
	ret.f32[1] = *p;
	ret.f32[2] = *p;
	ret.f32[3] = *p;
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128 _mm_load_ps( const float* p )
{
	__m128 ret;
	
	ret.f32[0] = p[0];
	ret.f32[1] = p[1];
	ret.f32[2] = p[2];
	ret.f32[3] = p[3];
	
	return ret;
}

#define _mm_loadu_ps _mm_load_ps


/*
================================================
	Set Operations
================================================
*/

ID_FORCE_INLINE_EXTERN __m128 _mm_setzero_ps()
{
	__m128 ret;
	
	ret.f32[0] = 0;
	ret.f32[1] = 0;
	ret.f32[2] = 0;
	ret.f32[3] = 0;
	
	return ret;
}

/*
================================================
	Store Operations
================================================
*/

ID_FORCE_INLINE_EXTERN void _mm_store_ss( float* p, __m128 a )
{
	*p = a.f32[0];
}

ID_FORCE_INLINE_EXTERN void _mm_store_ps( float* p, __m128 a )
{
	p[0] = a.f32[0];
	p[1] = a.f32[1];
	p[2] = a.f32[2];
	p[3] = a.f32[3];
}

#define _mm_storeu_ps _mm_store_ps


/*
================================================
	Cacheability Support
================================================
*/

// Guarantees that every preceding store is globally visible before any subsequent store.
ID_FORCE_INLINE_EXTERN void _mm_sfence()
{
	// DO NOTHING
}


/*
================================================
	Miscellaneous Operations
================================================
*/

ID_FORCE_INLINE_EXTERN __m128 _mm_shuffle_ps( __m128 a, __m128 b, int i )
{
	__m128 ret;
	
	float* af = &a.f32[0];
	float* bf = &b.f32[0];
	
	ret.f32[3] = af[( i >> 6 ) & 3 ];
	ret.f32[2] = af[( i >> 4 ) & 3 ];
	ret.f32[1] = bf[( i >> 2 ) & 3 ];
	ret.f32[0] = bf[( i >> 0 ) & 3 ];
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128 _mm_shuffle_epi32( __m128 a, int i )
{
	__m128 ret;
	float* af = &a.f32[0];
	
	ret.f32[3] = af[( i >> 6 ) & 3 ];
	ret.f32[2] = af[( i >> 4 ) & 3 ];
	ret.f32[1] = af[( i >> 2 ) & 3 ];
	ret.f32[0] = af[( i >> 0 ) & 3 ];
	
	return ret;
}

#define _MM_SHUFFLE(z, y, x, w) ((z<<6) | (y<<4) | (x<<2) | w)

ID_FORCE_INLINE_EXTERN __m128 _mm_unpackhi_ps( __m128 a, __m128 b )
{
	__m128 ret;
	
	ret.f32[0] = a.f32[2];
	ret.f32[1] = b.f32[2];
	ret.f32[2] = a.f32[3];
	ret.f32[3] = b.f32[3];
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128 _mm_unpacklo_ps( __m128 a, __m128 b )
{
	__m128 ret;
	
	ret.f32[0] = a.f32[0];
	ret.f32[1] = b.f32[0];
	ret.f32[2] = a.f32[1];
	ret.f32[3] = b.f32[1];
	
	return ret;
}


/*
================================================================================================

	SSE2 intrinsics emulation

================================================================================================
*/

ID_FORCE_INLINE_EXTERN __m128i _mm_set1_epi32( int i )
{
	__m128i ret;
	
	ret.i32[0] = i;
	ret.i32[1] = i;
	ret.i32[2] = i;
	ret.i32[3] = i;
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128i _mm_set_epi32( int i3, int i2, int i1, int i0 )
{
	__m128i ret;
	
	ret.i32[0] = i0;
	ret.i32[1] = i1;
	ret.i32[2] = i2;
	ret.i32[3] = i3;
	
	return ret;
}

ID_FORCE_INLINE_EXTERN int _mm_movemask_ps( __m128 a )
{
	return ( signbit( a.f32[3] ) << 3 | signbit( a.f32[2] ) << 2 | signbit( a.f32[1] ) << 1 | signbit( a.f32[0] ) );
}

ID_FORCE_INLINE_EXTERN __m128i _mm_add_epi16( __m128i a, __m128i b )
{
	__m128i ret;
	
	ret.i16[0] = a.i16[0] + b.i16[0];
	ret.i16[1] = a.i16[1] + b.i16[1];
	ret.i16[2] = a.i16[2] + b.i16[2];
	ret.i16[3] = a.i16[3] + b.i16[3];
	ret.i16[4] = a.i16[4] + b.i16[4];
	ret.i16[5] = a.i16[5] + b.i16[5];
	ret.i16[6] = a.i16[6] + b.i16[6];
	ret.i16[7] = a.i16[7] + b.i16[7];
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128i _mm_add_epi32( __m128i a, __m128i b )
{
	__m128i ret;
	
	ret.i32[0] = a.i32[0] + b.i32[0];
	ret.i32[1] = a.i32[1] + b.i32[1];
	ret.i32[2] = a.i32[2] + b.i32[2];
	ret.i32[3] = a.i32[3] + b.i32[3];
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128i _mm_setzero_sil128()
{
	__m128i ret;
	
	ret.i32[0] = 0;
	ret.i32[1] = 0;
	ret.i32[2] = 0;
	ret.i32[3] = 0;
	
	return ret;
}

/*
================================================
	Integer Arithmetic Operations
================================================
*/

ID_FORCE_INLINE_EXTERN __m128i _mm_sub_epi32( __m128i a, __m128i b )
{
	__m128i ret;
	
	ret.i32[0] = a.i32[0] - b.i32[0];
	ret.i32[1] = a.i32[1] - b.i32[1];
	ret.i32[2] = a.i32[2] - b.i32[2];
	ret.i32[3] = a.i32[3] - b.i32[3];
	
	return ret;
}

/*
================================================
	Logical Operations
================================================
*/

ID_FORCE_INLINE_EXTERN __m128i _mm_and_si128( __m128i a, __m128i b )
{
	__m128i ret;
	
	ret.i32[0] = a.i32[0] & b.i32[0];
	ret.i32[1] = a.i32[1] & b.i32[1];
	ret.i32[2] = a.i32[2] & b.i32[2];
	ret.i32[3] = a.i32[3] & b.i32[3];
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128i _mm_andnot_si128( __m128i a, __m128i b )
{
	__m128i ret;
	
	ret.i32[0] = ~a.i32[0] & b.i32[0];
	ret.i32[1] = ~a.i32[1] & b.i32[1];
	ret.i32[2] = ~a.i32[2] & b.i32[2];
	ret.i32[3] = ~a.i32[3] & b.i32[3];
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128i _mm_or_si128( __m128i a, __m128i b )
{
	__m128i ret;
	
	ret.i32[0] = a.i32[0] | b.i32[0];
	ret.i32[1] = a.i32[1] | b.i32[1];
	ret.i32[2] = a.i32[2] | b.i32[2];
	ret.i32[3] = a.i32[3] | b.i32[3];
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128i _mm_xor_si128( __m128i a, __m128i b )
{
	__m128i ret;
	
	ret.i32[0] = a.i32[0] ^ b.i32[0];
	ret.i32[1] = a.i32[1] ^ b.i32[1];
	ret.i32[2] = a.i32[2] ^ b.i32[2];
	ret.i32[3] = a.i32[3] ^ b.i32[3];
	
	return ret;
}

/*
================================================
	Integer Shift Operations
================================================
*/

// RB: FIXME
ID_FORCE_INLINE_EXTERN __m128i _mm_slli_si128( __m128i a, int count )
{
	__m128i ret;
	
	ret.i32[0] = a.i32[0] << count;
	ret.i32[1] = a.i32[1] << count;
	ret.i32[2] = a.i32[2] << count;
	ret.i32[3] = a.i32[3] << count;
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128i _mm_slli_epi32( __m128i a, int count )
{
	__m128i ret;
	
	ret.i32[0] = a.i32[0] << count;
	ret.i32[1] = a.i32[1] << count;
	ret.i32[2] = a.i32[2] << count;
	ret.i32[3] = a.i32[3] << count;
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128i _mm_srli_epi32( __m128i a, int count )
{
	__m128i ret;
	
	ret.i32[0] = a.i32[0] >> count;
	ret.i32[1] = a.i32[1] >> count;
	ret.i32[2] = a.i32[2] >> count;
	ret.i32[3] = a.i32[3] >> count;
	
	return ret;
}

#define _mm_srai_epi32 _mm_srli_epi32

// RB: FIXME doublecheck
#define _mm_srli_si128 _mm_srli_epi32

/*
================================================
	Integer Comparisons
================================================
*/

ID_FORCE_INLINE_EXTERN __m128i _mm_cmpeq_epi32( __m128i a, __m128i b )
{
	__m128i ret;
	
	ret.i32[0] = a.i32[0] == b.i32[0] ? 0xFFFFFFFF : 0;
	ret.i32[1] = a.i32[1] == b.i32[1] ? 0xFFFFFFFF : 0;
	ret.i32[2] = a.i32[2] == b.i32[2] ? 0xFFFFFFFF : 0;
	ret.i32[3] = a.i32[3] == b.i32[3] ? 0xFFFFFFFF : 0;
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128i _mm_cmplt_epi32( __m128i a, __m128i b )
{
	__m128i ret;
	
	ret.i32[0] = a.i32[0] < b.i32[0] ? 0xFFFFFFFF : 0;
	ret.i32[1] = a.i32[1] < b.i32[1] ? 0xFFFFFFFF : 0;
	ret.i32[2] = a.i32[2] < b.i32[2] ? 0xFFFFFFFF : 0;
	ret.i32[3] = a.i32[3] < b.i32[3] ? 0xFFFFFFFF : 0;
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128i _mm_cmpgt_epi32( __m128i a, __m128i b )
{
	__m128i ret;
	
	ret.i32[0] = a.i32[0] > b.i32[0] ? 0xFFFFFFFF : 0;
	ret.i32[1] = a.i32[1] > b.i32[1] ? 0xFFFFFFFF : 0;
	ret.i32[2] = a.i32[2] > b.i32[2] ? 0xFFFFFFFF : 0;
	ret.i32[3] = a.i32[3] > b.i32[3] ? 0xFFFFFFFF : 0;
	
	return ret;
}

/*
================================================
	Integer Conversion Operations
================================================
*/

ID_FORCE_INLINE_EXTERN __m128 _mm_cvtepi32_ps( __m128i a )
{
	__m128 ret;
	
	ret.f32[0] = a.f32[0];
	ret.f32[1] = a.f32[1];
	ret.f32[2] = a.f32[2];
	ret.f32[3] = a.f32[3];
	
	return ret;
}

/*
================================================
	Integer Load Operations
================================================
*/

ID_FORCE_INLINE_EXTERN __m128i _mm_load_si128( __m128i const* p )
{
	__m128i ret;
	
	ret.i32[0] = p->i32[0];
	ret.i32[1] = p->i32[1];
	ret.i32[2] = p->i32[2];
	ret.i32[3] = p->i32[3];
	
	return ret;
}

/*
================================================
	Integer Move Operations
================================================
*/

ID_FORCE_INLINE_EXTERN __m128i _mm_cvtps_epi32( __m128 a )
{
	__m128i ret;
	
	ret.i32[0] = ( int ) a.i32[0];
	ret.i32[1] = ( int ) a.i32[1];
	ret.i32[2] = ( int ) a.i32[2];
	ret.i32[3] = ( int ) a.i32[3];
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128i _mm_cvtsi32_si128( int a )
{
	__m128i ret;
	
	ret.i32[0] = a;
	ret.i32[1] = 0;
	ret.i32[2] = 0;
	ret.i32[3] = 0;
	
	return ret;
}

ID_FORCE_INLINE_EXTERN int32 _mm_cvtsi128_si32( __m128i a )
{
	return a.i32[0];
}


/*
================================================
	Integer Set Operations
================================================
*/

ID_FORCE_INLINE_EXTERN __m128i _mm_setzero_si128()
{
	__m128i ret;
	
	ret.i32[0] = 0;
	ret.i32[1] = 0;
	ret.i32[2] = 0;
	ret.i32[3] = 0;
	
	return ret;
}

/*
================================================
	Integer Store Operations
================================================
*/

// Stores the data in a to the address p without polluting the caches. If the cache line
// containing address p is already in the cache, the cache will be updated. Address p
// must be 16 byte aligned.
ID_FORCE_INLINE_EXTERN void _mm_stream_si128( __m128i* p, __m128i a )
{
	p->i32[0] = a.i32[0];
	p->i32[1] = a.i32[1];
	p->i32[2] = a.i32[2];
	p->i32[3] = a.i32[3];
}

#define _mm_store_si128 _mm_stream_si128

// Stores the lower 64 bits of the value pointed to by p.
ID_FORCE_INLINE_EXTERN void _mm_storel_epi64( __m128i* p, __m128i a )
{
	p->i32[0] = a.i32[0];
	p->i32[1] = a.i32[1];
	p->i32[2] = 0;
	p->i32[3] = 0;
}


/*
================================================
	Miscellaneous Operations
================================================
*/

ID_FORCE_INLINE_EXTERN __m128i _mm_shuffle_epi32( __m128i a, int i )
{
	__m128i ret;
	int32* af = &a.i32[0];
	
	ret.i32[3] = af[( i >> 6 ) & 3 ];
	ret.i32[2] = af[( i >> 4 ) & 3 ];
	ret.i32[1] = af[( i >> 2 ) & 3 ];
	ret.i32[0] = af[( i >> 0 ) & 3 ];
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128i _mm_shufflehi_epi16( __m128i a, int i )
{
	__m128i ret;
	int16* af = &a.i16[4];
	
	ret.i32[3 + 4] = af[( i >> 6 ) & 3 ];
	ret.i32[2 + 4] = af[( i >> 4 ) & 3 ];
	ret.i32[1 + 4] = af[( i >> 2 ) & 3 ];
	ret.i32[0 + 4] = af[( i >> 0 ) & 3 ];
	
	return ret;
}


ID_FORCE_INLINE_EXTERN int SignedSaturate8( int32 s )
{
	return s & 0x80;
}

ID_FORCE_INLINE_EXTERN int UnsignedSaturate8( int32 s )
{
	return s & 0xFF;
}

ID_FORCE_INLINE_EXTERN int SignedSaturate16( int32 s )
{
	return s & 0x8000;
}

ID_FORCE_INLINE_EXTERN int UnsignedSaturate16( int32 s )
{
	return s & 0xFFFF;
}

// Packs the 16 signed 16-bit integers from a and b into 8-bit integers and saturates.
ID_FORCE_INLINE_EXTERN __m128i _mm_packs_epi16( __m128i a, __m128i b )
{
	__m128i ret;
	
	ret.i8[ 0] = SignedSaturate8( a.i16[0] );
	ret.i8[ 1] = SignedSaturate8( a.i16[1] );
	ret.i8[ 2] = SignedSaturate8( a.i16[2] );
	ret.i8[ 3] = SignedSaturate8( a.i16[3] );
	ret.i8[ 4] = SignedSaturate8( a.i16[4] );
	ret.i8[ 5] = SignedSaturate8( a.i16[5] );
	ret.i8[ 6] = SignedSaturate8( a.i16[6] );
	ret.i8[ 7] = SignedSaturate8( a.i16[7] );
	
	ret.i8[ 8] = SignedSaturate8( b.i16[0] );
	ret.i8[ 9] = SignedSaturate8( b.i16[1] );
	ret.i8[10] = SignedSaturate8( b.i16[2] );
	ret.i8[11] = SignedSaturate8( b.i16[3] );
	ret.i8[12] = SignedSaturate8( b.i16[4] );
	ret.i8[13] = SignedSaturate8( b.i16[5] );
	ret.i8[14] = SignedSaturate8( b.i16[6] );
	ret.i8[15] = SignedSaturate8( b.i16[7] );
	
	return ret;
}

// Packs the 8 signed 32-bit integers from a and b into signed 16-bit integers and saturates
ID_FORCE_INLINE_EXTERN __m128i _mm_packs_epi32( __m128i a, __m128i b )
{
	__m128i ret;
	
	ret.i16[0] = SignedSaturate16( a.i32[0] );
	ret.i16[1] = SignedSaturate16( a.i32[1] );
	ret.i16[2] = SignedSaturate16( a.i32[2] );
	ret.i16[3] = SignedSaturate16( a.i32[3] );
	
	ret.i16[4] = SignedSaturate16( b.i32[0] );
	ret.i16[5] = SignedSaturate16( b.i32[1] );
	ret.i16[6] = SignedSaturate16( b.i32[2] );
	ret.i16[7] = SignedSaturate16( b.i32[3] );
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128i _mm_packus_epi16( __m128i a, __m128i b )
{
	__m128i ret;
	
	ret.u8[ 0] = UnsignedSaturate8( a.i16[0] );
	ret.u8[ 1] = UnsignedSaturate8( a.i16[1] );
	ret.u8[ 2] = UnsignedSaturate8( a.i16[2] );
	ret.u8[ 3] = UnsignedSaturate8( a.i16[3] );
	ret.u8[ 4] = UnsignedSaturate8( a.i16[4] );
	ret.u8[ 5] = UnsignedSaturate8( a.i16[5] );
	ret.u8[ 6] = UnsignedSaturate8( a.i16[6] );
	ret.u8[ 7] = UnsignedSaturate8( a.i16[7] );
	
	ret.u8[ 8] = UnsignedSaturate8( b.i16[0] );
	ret.u8[ 9] = UnsignedSaturate8( b.i16[1] );
	ret.u8[10] = UnsignedSaturate8( b.i16[2] );
	ret.u8[11] = UnsignedSaturate8( b.i16[3] );
	ret.u8[12] = UnsignedSaturate8( b.i16[4] );
	ret.u8[13] = UnsignedSaturate8( b.i16[5] );
	ret.u8[14] = UnsignedSaturate8( b.i16[6] );
	ret.u8[15] = UnsignedSaturate8( b.i16[7] );
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128i _mm_unpackhi_epi32( __m128i a, __m128i b )
{
	__m128i ret;
	
	ret.i32[0] = a.i32[2];
	ret.i32[1] = b.i32[2];
	ret.i32[2] = a.i32[3];
	ret.i32[3] = b.i32[3];
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128i _mm_unpacklo_epi8( __m128i a, __m128i b )
{
	__m128i ret;
	
	ret.u8[ 0] = a.u8[0];
	ret.u8[ 1] = b.u8[0];
	ret.u8[ 2] = a.u8[1];
	ret.u8[ 3] = b.u8[1];
	ret.u8[ 4] = a.u8[2];
	ret.u8[ 5] = b.u8[2];
	ret.u8[ 6] = a.u8[3];
	ret.u8[ 7] = b.u8[3];
	ret.u8[ 8] = a.u8[4];
	ret.u8[ 9] = b.u8[4];
	ret.u8[10] = a.u8[5];
	ret.u8[11] = b.u8[5];
	ret.u8[12] = a.u8[6];
	ret.u8[13] = b.u8[6];
	ret.u8[14] = a.u8[7];
	ret.u8[15] = b.u8[7];
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128i _mm_unpacklo_epi16( __m128i a, __m128i b )
{
	__m128i ret;
	
	ret.i16[ 0] = a.i16[0];
	ret.i16[ 1] = b.i16[0];
	ret.i16[ 2] = a.i16[1];
	ret.i16[ 3] = b.i16[1];
	ret.i16[ 4] = a.i16[2];
	ret.i16[ 5] = b.i16[2];
	ret.i16[ 6] = a.i16[3];
	ret.i16[ 7] = b.i16[3];
	
	return ret;
}

ID_FORCE_INLINE_EXTERN __m128i _mm_unpacklo_epi32( __m128i a, __m128i b )
{
	__m128i ret;
	
	ret.i32[0] = a.i32[0];
	ret.i32[1] = b.i32[0];
	ret.i32[2] = a.i32[1];
	ret.i32[3] = b.i32[1];
	
	return ret;
}



/*
================================================
	Casting Support
================================================
*/

ID_FORCE_INLINE_EXTERN __m128i _mm_castps_si128( __m128 a )
{
	__m128i ret;
	
	ret.f32[0] = a.f32[0];
	ret.f32[1] = a.f32[1];
	ret.f32[2] = a.f32[2];
	ret.f32[3] = a.f32[3];
	
	return ret;
}


#endif // #if defined(USE_INTRINSICS_EMU)

/*
================================================================================================

	Scalar single precision floating-point intrinsics

================================================================================================
*/

ID_INLINE_EXTERN float __fmuls( float a, float b )
{
	return ( a * b );
}
ID_INLINE_EXTERN float __fmadds( float a, float b, float c )
{
	return ( a * b + c );
}
ID_INLINE_EXTERN float __fnmsubs( float a, float b, float c )
{
	return ( c - a * b );
}
ID_INLINE_EXTERN float __fsels( float a, float b, float c )
{
	return ( a >= 0.0f ) ? b : c;
}
ID_INLINE_EXTERN float __frcps( float x )
{
	return ( 1.0f / x );
}
ID_INLINE_EXTERN float __fdivs( float x, float y )
{
	return ( x / y );
}
ID_INLINE_EXTERN float __frsqrts( float x )
{
	return ( 1.0f / sqrtf( x ) );
}
ID_INLINE_EXTERN float __frcps16( float x )
{
	return ( 1.0f / x );
}
ID_INLINE_EXTERN float __fdivs16( float x, float y )
{
	return ( x / y );
}
ID_INLINE_EXTERN float __frsqrts16( float x )
{
	return ( 1.0f / sqrtf( x ) );
}
ID_INLINE_EXTERN float __frndz( float x )
{
	return ( float )( ( int )( x ) );
}

/*
================================================================================================

	Zero cache line and prefetch intrinsics

================================================================================================
*/

// The code below assumes that a cache line is 64 bytes.
// We specify the cache line size as 128 here to make the code consistent with the consoles.
#define CACHE_LINE_SIZE						128

ID_FORCE_INLINE void Prefetch( const void* ptr, int offset )
{
//	const char * bytePtr = ( (const char *) ptr ) + offset;
//	_mm_prefetch( bytePtr +  0, _MM_HINT_NTA );
//	_mm_prefetch( bytePtr + 64, _MM_HINT_NTA );
}
ID_FORCE_INLINE void ZeroCacheLine( void* ptr, int offset )
{
	assert_128_byte_aligned( ptr );
	char* bytePtr = ( ( char* ) ptr ) + offset;
	
#if !defined(USE_INTRINSICS_EMU)
	__m128i zero = _mm_setzero_si128();
	_mm_store_si128( ( __m128i* )( bytePtr + 0 * 16 ), zero );
	_mm_store_si128( ( __m128i* )( bytePtr + 1 * 16 ), zero );
	_mm_store_si128( ( __m128i* )( bytePtr + 2 * 16 ), zero );
	_mm_store_si128( ( __m128i* )( bytePtr + 3 * 16 ), zero );
	_mm_store_si128( ( __m128i* )( bytePtr + 4 * 16 ), zero );
	_mm_store_si128( ( __m128i* )( bytePtr + 5 * 16 ), zero );
	_mm_store_si128( ( __m128i* )( bytePtr + 6 * 16 ), zero );
	_mm_store_si128( ( __m128i* )( bytePtr + 7 * 16 ), zero );
#endif
}
ID_FORCE_INLINE void FlushCacheLine( const void* ptr, int offset )
{
#if !defined(USE_INTRINSICS_EMU)
	const char* bytePtr = ( ( const char* ) ptr ) + offset;
	_mm_clflush( bytePtr +  0 );
	_mm_clflush( bytePtr + 64 );
#endif
}

/*
================================================
	Block Clear Macros
================================================
*/

// number of additional elements that are potentially cleared when clearing whole cache lines at a time
ID_INLINE_EXTERN int CACHE_LINE_CLEAR_OVERFLOW_COUNT( int size )
{
	if( ( size & ( CACHE_LINE_SIZE - 1 ) ) == 0 )
	{
		return 0;
	}
	if( size > CACHE_LINE_SIZE )
	{
		return 1;
	}
	return ( CACHE_LINE_SIZE / ( size & ( CACHE_LINE_SIZE - 1 ) ) );
}

// if the pointer is not on a cache line boundary this assumes the cache line the pointer starts in was already cleared
// RB: changed UINT_PTR to uintptr_t
#define CACHE_LINE_CLEAR_BLOCK( ptr, size )																		\
	byte * startPtr = (byte *)( ( ( (uintptr_t) ( ptr ) ) + CACHE_LINE_SIZE - 1 ) & ~( CACHE_LINE_SIZE - 1 ) );	\
	byte * endPtr = (byte *)( ( (uintptr_t) ( ptr ) + ( size ) - 1 ) & ~( CACHE_LINE_SIZE - 1 ) );				\
	for ( ; startPtr <= endPtr; startPtr += CACHE_LINE_SIZE ) {													\
		ZeroCacheLine( startPtr, 0 );																			\
	}

#define CACHE_LINE_CLEAR_BLOCK_AND_FLUSH( ptr, size )															\
	byte * startPtr = (byte *)( ( ( (uintptr_t) ( ptr ) ) + CACHE_LINE_SIZE - 1 ) & ~( CACHE_LINE_SIZE - 1 ) );	\
	byte * endPtr = (byte *)( ( (uintptr_t) ( ptr ) + ( size ) - 1 ) & ~( CACHE_LINE_SIZE - 1 ) );				\
	for ( ; startPtr <= endPtr; startPtr += CACHE_LINE_SIZE ) {													\
		ZeroCacheLine( startPtr, 0 );																			\
		FlushCacheLine( startPtr, 0 );																			\
	}
// RB end

/*
================================================================================================

	Vector Intrinsics

================================================================================================
*/

/*
================================================
	PC Windows
================================================
*/

#if !defined( R_SHUFFLE_D )
#define R_SHUFFLE_D( x, y, z, w )	(( (w) & 3 ) << 6 | ( (z) & 3 ) << 4 | ( (y) & 3 ) << 2 | ( (x) & 3 ))
#endif

// DG: _CRT_ALIGN seems to be MSVC specific, so provide implementation..
#ifndef _CRT_ALIGN
#if defined(__GNUC__) // also applies for clang
#define _CRT_ALIGN(x) __attribute__ ((__aligned__ (x)))
#elif defined(_MSC_VER) // also for MSVC, just to be sure
#define _CRT_ALIGN(x) __declspec(align(x))
#endif
#endif
// DG: make sure __declspec(intrin_type) is only used on MSVC (it's not available on GCC etc
#ifdef _MSC_VER
#define DECLSPEC_INTRINTYPE __declspec( intrin_type )
#else
#define DECLSPEC_INTRINTYPE
#endif
// DG end


// make the intrinsics "type unsafe"
typedef union DECLSPEC_INTRINTYPE _CRT_ALIGN( 16 ) __m128c
{
	__m128c() {}
	
#if 0 //defined(USE_INTRINSICS_EMU)
	__m128c( __m128 f, __m128i i )
	{
		m128 = f;
		m128i = i;
	}
#else
	__m128c( __m128 f )
	{
		m128 = f;
	}
	__m128c( __m128i i )
	{
		m128i = i;
	}
#endif
	
	operator	__m128()
	{
		return m128;
	}
	operator	__m128i()
	{
		return m128i;
	}
	__m128		m128;
	__m128i		m128i;
} __m128c;


#if defined(USE_INTRINSICS_EMU)
ID_FORCE_INLINE_EXTERN __m128 _mm_shuffle_epi32( __m128c ac, int i )
{
	__m128 ret;
	
	__m128i a = ac;
	int32* af = &a.i32[0];
	
	ret.i32[3] = af[( i >> 6 ) & 3 ];
	ret.i32[2] = af[( i >> 4 ) & 3 ];
	ret.i32[1] = af[( i >> 2 ) & 3 ];
	ret.i32[0] = af[( i >> 0 ) & 3 ];
	
	return ret;
}
#endif

#define _mm_madd_ps( a, b, c )				_mm_add_ps( _mm_mul_ps( (a), (b) ), (c) )
#define _mm_nmsub_ps( a, b, c )				_mm_sub_ps( (c), _mm_mul_ps( (a), (b) ) )
#define _mm_splat_ps( x, i )				__m128c( _mm_shuffle_epi32( __m128c( x ), _MM_SHUFFLE( i, i, i, i ) ) )
#define _mm_perm_ps( x, perm )				__m128c( _mm_shuffle_epi32( __m128c( x ), perm ) )
#define _mm_sel_ps( a, b, c )  				_mm_or_ps( _mm_andnot_ps( __m128c( c ), a ), _mm_and_ps( __m128c( c ), b ) )
#define _mm_sel_si128( a, b, c )			_mm_or_si128( _mm_andnot_si128( __m128c( c ), a ), _mm_and_si128( __m128c( c ), b ) )
#define _mm_sld_ps( x, y, imm )				__m128c( _mm_or_si128( _mm_srli_si128( __m128c( x ), imm ), _mm_slli_si128( __m128c( y ), 16 - imm ) ) )
#define _mm_sld_si128( x, y, imm )			_mm_or_si128( _mm_srli_si128( x, imm ), _mm_slli_si128( y, 16 - imm ) )

ID_FORCE_INLINE_EXTERN __m128 _mm_msum3_ps( __m128 a, __m128 b )
{
	__m128 c = _mm_mul_ps( a, b );
	return _mm_add_ps( _mm_splat_ps( c, 0 ), _mm_add_ps( _mm_splat_ps( c, 1 ), _mm_splat_ps( c, 2 ) ) );
}

ID_FORCE_INLINE_EXTERN __m128 _mm_msum4_ps( __m128 a, __m128 b )
{
	__m128 c = _mm_mul_ps( a, b );
	c = _mm_add_ps( c, _mm_perm_ps( c, _MM_SHUFFLE( 1, 0, 3, 2 ) ) );
	c = _mm_add_ps( c, _mm_perm_ps( c, _MM_SHUFFLE( 2, 3, 0, 1 ) ) );
	return c;
}

#define _mm_shufmix_epi32( x, y, perm )		__m128c( _mm_shuffle_ps( __m128c( x ), __m128c( y ), perm ) )
#define _mm_loadh_epi64( x, address )		__m128c( _mm_loadh_pi( __m128c( x ), (__m64 *)address ) )
#define _mm_storeh_epi64( address, x )		_mm_storeh_pi( (__m64 *)address, __m128c( x ) )


// floating-point reciprocal with close to full precision
ID_FORCE_INLINE_EXTERN __m128 _mm_rcp32_ps( __m128 x )
{
	__m128 r = _mm_rcp_ps( x );		// _mm_rcp_ps() has 12 bits of precision
	r = _mm_sub_ps( _mm_add_ps( r, r ), _mm_mul_ps( _mm_mul_ps( x, r ), r ) );
	r = _mm_sub_ps( _mm_add_ps( r, r ), _mm_mul_ps( _mm_mul_ps( x, r ), r ) );
	return r;
}
// floating-point reciprocal with at least 16 bits precision
ID_FORCE_INLINE_EXTERN __m128 _mm_rcp16_ps( __m128 x )
{
	__m128 r = _mm_rcp_ps( x );		// _mm_rcp_ps() has 12 bits of precision
	r = _mm_sub_ps( _mm_add_ps( r, r ), _mm_mul_ps( _mm_mul_ps( x, r ), r ) );
	return r;
}
// floating-point divide with close to full precision
ID_FORCE_INLINE_EXTERN __m128 _mm_div32_ps( __m128 x, __m128 y )
{
	return _mm_mul_ps( x, _mm_rcp32_ps( y ) );
}
// floating-point divide with at least 16 bits precision
ID_FORCE_INLINE_EXTERN __m128 _mm_div16_ps( __m128 x, __m128 y )
{
	return _mm_mul_ps( x, _mm_rcp16_ps( y ) );
}
// load idBounds::GetMins()
#define _mm_loadu_bounds_0( bounds )		_mm_perm_ps( _mm_loadh_pi( _mm_load_ss( & bounds[0].x ), (__m64 *) & bounds[0].y ), _MM_SHUFFLE( 1, 3, 2, 0 ) )
// load idBounds::GetMaxs()
#define _mm_loadu_bounds_1( bounds )		_mm_perm_ps( _mm_loadh_pi( _mm_load_ss( & bounds[1].x ), (__m64 *) & bounds[1].y ), _MM_SHUFFLE( 1, 3, 2, 0 ) )

#endif	// !__SYS_INTRIINSICS_H__
