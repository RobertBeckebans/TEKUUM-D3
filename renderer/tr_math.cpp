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

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "tr_local.h"
#ifdef __ppc__
#include <vecLib/vecLib.h>
#endif
#if defined(MACOS_X) && defined(__i386__)
#include <xmmintrin.h>
#endif






void MatrixIdentity( matrix_t m )
{
	m[ 0] = 1;
	m[ 4] = 0;
	m[ 8] = 0;
	m[12] = 0;
	m[ 1] = 0;
	m[ 5] = 1;
	m[ 9] = 0;
	m[13] = 0;
	m[ 2] = 0;
	m[ 6] = 0;
	m[10] = 1;
	m[14] = 0;
	m[ 3] = 0;
	m[ 7] = 0;
	m[11] = 0;
	m[15] = 1;
}

void MatrixCopy( const matrix_t in, matrix_t out )
{
	out[ 0] = in[ 0];
	out[ 4] = in[ 4];
	out[ 8] = in[ 8];
	out[12] = in[12];
	out[ 1] = in[ 1];
	out[ 5] = in[ 5];
	out[ 9] = in[ 9];
	out[13] = in[13];
	out[ 2] = in[ 2];
	out[ 6] = in[ 6];
	out[10] = in[10];
	out[14] = in[14];
	out[ 3] = in[ 3];
	out[ 7] = in[ 7];
	out[11] = in[11];
	out[15] = in[15];
}

void MatrixTranspose( const matrix_t in, matrix_t out )
{
	out[ 0] = in[ 0];
	out[ 1] = in[ 4];
	out[ 2] = in[ 8];
	out[ 3] = in[12];
	out[ 4] = in[ 1];
	out[ 5] = in[ 5];
	out[ 6] = in[ 9];
	out[ 7] = in[13];
	out[ 8] = in[ 2];
	out[ 9] = in[ 6];
	out[10] = in[10];
	out[11] = in[14];
	out[12] = in[ 3];
	out[13] = in[ 7];
	out[14] = in[11];
	out[15] = in[15];
}

void MatrixAffineInverse( const matrix_t in, matrix_t out )
{
#if 0
	MatrixCopy( in, out );
	MatrixInverse( out );
#else
	out[ 0] = in[ 0];
	out[ 4] = in[ 1];
	out[ 8] = in[ 2];
	out[ 1] = in[ 4];
	out[ 5] = in[ 5];
	out[ 9] = in[ 6];
	out[ 2] = in[ 8];
	out[ 6] = in[ 9];
	out[10] = in[10];
	out[ 3] = 0;
	out[ 7] = 0;
	out[11] = 0;
	out[15] = 1;
	
	out[12] = -( in[12] * out[ 0] + in[13] * out[ 4] + in[14] * out[ 8] );
	out[13] = -( in[12] * out[ 1] + in[13] * out[ 5] + in[14] * out[ 9] );
	out[14] = -( in[12] * out[ 2] + in[13] * out[ 6] + in[14] * out[10] );
#endif
}

void MatrixFullInverse( const matrix_t a, matrix_t r )
{
	idMat4	am;
	
	for( int i = 0 ; i < 4 ; i++ )
	{
		for( int j = 0 ; j < 4 ; j++ )
		{
			am[i][j] = a[j * 4 + i];
		}
	}
	
//	idVec4 test( 100, 100, 100, 1 );
//	idVec4	transformed, inverted;
//	transformed = test * am;

	if( !am.InverseSelf() )
	{
		common->Error( "Invert failed" );
	}
//	inverted = transformed * am;

	for( int i = 0 ; i < 4 ; i++ )
	{
		for( int j = 0 ; j < 4 ; j++ )
		{
			r[j * 4 + i] = am[i][j];
		}
	}
}

void MatrixSetupTranslation( matrix_t m, float x, float y, float z )
{
	m[ 0] = 1;
	m[ 4] = 0;
	m[ 8] = 0;
	m[12] = x;
	m[ 1] = 0;
	m[ 5] = 1;
	m[ 9] = 0;
	m[13] = y;
	m[ 2] = 0;
	m[ 6] = 0;
	m[10] = 1;
	m[14] = z;
	m[ 3] = 0;
	m[ 7] = 0;
	m[11] = 0;
	m[15] = 1;
}

void MatrixSetupScale( matrix_t m, float x, float y, float z )
{
	m[ 0] = x;
	m[ 4] = 0;
	m[ 8] = 0;
	m[12] = 0;
	m[ 1] = 0;
	m[ 5] = y;
	m[ 9] = 0;
	m[13] = 0;
	m[ 2] = 0;
	m[ 6] = 0;
	m[10] = z;
	m[14] = 0;
	m[ 3] = 0;
	m[ 7] = 0;
	m[11] = 0;
	m[15] = 1;
}

void MatrixMultiply( const matrix_t a, const matrix_t b, matrix_t out )
{
#if id386_sse
//#error MatrixMultiply
	int				i;
	__m128			_t0, _t1, _t2, _t3, _t4, _t5, _t6, _t7;
	
	_t4 = _mm_loadu_ps( &a[0] );
	_t5 = _mm_loadu_ps( &a[4] );
	_t6 = _mm_loadu_ps( &a[8] );
	_t7 = _mm_loadu_ps( &a[12] );
	
	for( i = 0; i < 4; i++ )
	{
		_t0 = _mm_load1_ps( &b[i * 4 + 0] );
		_t0 = _mm_mul_ps( _t4, _t0 );
		
		_t1 = _mm_load1_ps( &b[i * 4 + 1] );
		_t1 = _mm_mul_ps( _t5, _t1 );
		
		_t2 = _mm_load1_ps( &b[i * 4 + 2] );
		_t2 = _mm_mul_ps( _t6, _t2 );
		
		_t3 = _mm_load1_ps( &b[i * 4 + 3] );
		_t3 = _mm_mul_ps( _t7, _t3 );
		
		_t1 = _mm_add_ps( _t0, _t1 );
		_t2 = _mm_add_ps( _t1, _t2 );
		_t3 = _mm_add_ps( _t2, _t3 );
		
		_mm_storeu_ps( &out[i * 4], _t3 );
	}
	
#else
	out[ 0] = b[ 0] * a[ 0] + b[ 1] * a[ 4] + b[ 2] * a[ 8] + b[ 3] * a[12];
	out[ 1] = b[ 0] * a[ 1] + b[ 1] * a[ 5] + b[ 2] * a[ 9] + b[ 3] * a[13];
	out[ 2] = b[ 0] * a[ 2] + b[ 1] * a[ 6] + b[ 2] * a[10] + b[ 3] * a[14];
	out[ 3] = b[ 0] * a[ 3] + b[ 1] * a[ 7] + b[ 2] * a[11] + b[ 3] * a[15];
	
	out[ 4] = b[ 4] * a[ 0] + b[ 5] * a[ 4] + b[ 6] * a[ 8] + b[ 7] * a[12];
	out[ 5] = b[ 4] * a[ 1] + b[ 5] * a[ 5] + b[ 6] * a[ 9] + b[ 7] * a[13];
	out[ 6] = b[ 4] * a[ 2] + b[ 5] * a[ 6] + b[ 6] * a[10] + b[ 7] * a[14];
	out[ 7] = b[ 4] * a[ 3] + b[ 5] * a[ 7] + b[ 6] * a[11] + b[ 7] * a[15];
	
	out[ 8] = b[ 8] * a[ 0] + b[ 9] * a[ 4] + b[10] * a[ 8] + b[11] * a[12];
	out[ 9] = b[ 8] * a[ 1] + b[ 9] * a[ 5] + b[10] * a[ 9] + b[11] * a[13];
	out[10] = b[ 8] * a[ 2] + b[ 9] * a[ 6] + b[10] * a[10] + b[11] * a[14];
	out[11] = b[ 8] * a[ 3] + b[ 9] * a[ 7] + b[10] * a[11] + b[11] * a[15];
	
	out[12] = b[12] * a[ 0] + b[13] * a[ 4] + b[14] * a[ 8] + b[15] * a[12];
	out[13] = b[12] * a[ 1] + b[13] * a[ 5] + b[14] * a[ 9] + b[15] * a[13];
	out[14] = b[12] * a[ 2] + b[13] * a[ 6] + b[14] * a[10] + b[15] * a[14];
	out[15] = b[12] * a[ 3] + b[13] * a[ 7] + b[14] * a[11] + b[15] * a[15];
#endif
}

void MatrixMultiply2( matrix_t m, const matrix_t m2 )
{
	matrix_t        tmp;
	
	MatrixCopy( m, tmp );
	MatrixMultiply( tmp, m2, m );
}

void MatrixMultiplyTranslation( matrix_t m, float x, float y, float z )
{
#if 1
	matrix_t        tmp, trans;
	
	MatrixCopy( m, tmp );
	MatrixSetupTranslation( trans, x, y, z );
	MatrixMultiply( tmp, trans, m );
#else
	m[12] += m[ 0] * x + m[ 4] * y + m[ 8] * z;
	m[13] += m[ 1] * x + m[ 5] * y + m[ 9] * z;
	m[14] += m[ 2] * x + m[ 6] * y + m[10] * z;
	m[15] += m[ 3] * x + m[ 7] * y + m[11] * z;
#endif
}

void MatrixMultiplyScale( matrix_t m, float x, float y, float z )
{
#if 1
	matrix_t        tmp, scale;
	
	MatrixCopy( m, tmp );
	MatrixSetupScale( scale, x, y, z );
	MatrixMultiply( tmp, scale, m );
#else
	m[ 0] *= x;
	m[ 4] *= y;
	m[ 8] *= z;
	m[ 1] *= x;
	m[ 5] *= y;
	m[ 9] *= z;
	m[ 2] *= x;
	m[ 6] *= y;
	m[10] *= z;
	m[ 3] *= x;
	m[ 7] *= y;
	m[11] *= z;
#endif
}

void MatrixFromPlanes( matrix_t m, const idPlane frustum[6] )
{
	const idPlane& left = frustum[FRUSTUM_LEFT];
	const idPlane& right = frustum[FRUSTUM_RIGHT];
	const idPlane& bottom = frustum[FRUSTUM_BOTTOM];
	const idPlane& top = frustum[FRUSTUM_TOP];
	const idPlane& zNear = frustum[FRUSTUM_NEAR];
	const idPlane& zFar = frustum[FRUSTUM_FAR];
	
	m[ 0] = ( right[0] - left[0] ) / 2;
	m[ 1] = ( top[0] - bottom[0] ) / 2;
	m[ 2] = ( zFar[0] - zNear[0] ) / 2;
	m[ 3] = right[0] - ( right[0] - left[0] ) / 2;
	
	m[ 4] = ( right[1] - left[1] ) / 2;
	m[ 5] = ( top[1] - bottom[1] ) / 2;
	m[ 6] = ( zFar[1] - zNear[1] ) / 2;
	m[ 7] = right[1] - ( right[1] - left[1] ) / 2;
	
	m[ 8] = ( right[2] - left[2] ) / 2;
	m[ 9] = ( top[2] - bottom[2] ) / 2;
	m[10] = ( zFar[2] - zNear[2] ) / 2;
	m[11] = right[2] - ( right[2] - left[2] ) / 2;
	
#if 1
	m[12] = ( right[3] - left[3] ) / 2;
	m[13] = ( top[3] - bottom[3] ) / 2;
	m[14] = ( zFar[3] - zNear[3] ) / 2;
	m[15] = right[3] - ( right[3] - left[3] ) / 2;
#else
	m[12] = ( -right[3] - -left[3] ) / 2;
	m[13] = ( -top[3] - -bottom[3] ) / 2;
	m[14] = ( -zFar[3] - -zNear[3] ) / 2;
	m[15] = -right[3] - ( -right[3] - -left[3] ) / 2;
#endif
}


void MatrixPerspectiveProjectionFovXYRH( matrix_t m, float fovX, float fovY, float zNear, float zFar )
{
	float width, height;
	
	width = tanf( DEG2RAD( fovX * 0.5f ) );
	height = tanf( DEG2RAD( fovY * 0.5f ) );
	
	m[0] = 1 / width;
	m[4] = 0;
	m[8] = 0;
	m[12] = 0;
	m[1] = 0;
	m[5] = 1 / height;
	m[9] = 0;
	m[13] = 0;
	m[2] = 0;
	m[6] = 0;
	m[10] = zFar / ( zNear - zFar );
	m[14] = ( zNear * zFar ) / ( zNear - zFar );
	m[3] = 0;
	m[7] = 0;
	m[11] = -1;
	m[15] = 0;
}

// RB: far plane at infinity, see RobustShadowVolumes.pdf by Nvidia
void MatrixPerspectiveProjectionFovXYInfiniteRH( matrix_t m, float fovX, float fovY, float zNear )
{
	float width, height;
	
	width = tanf( DEG2RAD( fovX * 0.5f ) );
	height = tanf( DEG2RAD( fovY * 0.5f ) );
	
	m[0] = 1 / width;
	m[4] = 0;
	m[8] = 0;
	m[12] = 0;
	m[1] = 0;
	m[5] = 1 / height;
	m[9] = 0;
	m[13] = 0;
	m[2] = 0;
	m[6] = 0;
	m[10] = -1;
	m[14] = -2 * zNear;
	m[3] = 0;
	m[7] = 0;
	m[11] = -1;
	m[15] = 0;
}