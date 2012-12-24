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

#ifndef __MATH_VECX_H__
#define __MATH_VECX_H__

/*
===============================================================================

idVecX - arbitrary sized vector

The vector lives on 16 byte aligned and 16 byte padded memory.

NOTE: due to the temporary memory pool idVecX cannot be used by multiple threads

===============================================================================
*/

#define VECX_MAX_TEMP		1024
#define VECX_QUAD( x )		( ( ( ( x ) + 3 ) & ~3 ) * sizeof( float ) )
#define VECX_CLEAREND()		int s = size; while( s < ( ( s + 3) & ~3 ) ) { p[s++] = 0.0f; }
#define VECX_ALLOCA( n )	( (float *) _alloca16( VECX_QUAD( n ) ) )
#define VECX_SIMD

class idVecX
{
	friend class idMatX;
	
public:
	idVecX();
	explicit idVecX( int length );
	explicit idVecX( int length, float* data );
	~idVecX();
	
	float			operator[]( const int index ) const;
	float& 			operator[]( const int index );
	idVecX			operator-() const;
	idVecX& 		operator=( const idVecX& a );
	idVecX			operator*( const float a ) const;
	idVecX			operator/( const float a ) const;
	float			operator*( const idVecX& a ) const;
	idVecX			operator-( const idVecX& a ) const;
	idVecX			operator+( const idVecX& a ) const;
	idVecX& 		operator*=( const float a );
	idVecX& 		operator/=( const float a );
	idVecX& 		operator+=( const idVecX& a );
	idVecX& 		operator-=( const idVecX& a );
	
	friend idVecX	operator*( const float a, const idVecX b );
	
	bool			Compare( const idVecX& a ) const;							// exact compare, no epsilon
	bool			Compare( const idVecX& a, const float epsilon ) const;		// compare with epsilon
	bool			operator==(	const idVecX& a ) const;						// exact compare, no epsilon
	bool			operator!=(	const idVecX& a ) const;						// exact compare, no epsilon
	
	void			SetSize( int size );
	void			ChangeSize( int size, bool makeZero = false );
	int				GetSize() const
	{
		return size;
	}
	void			SetData( int length, float* data );
	void			Zero();
	void			Zero( int length );
	void			Random( int seed, float l = 0.0f, float u = 1.0f );
	void			Random( int length, int seed, float l = 0.0f, float u = 1.0f );
	void			Negate();
	void			Clamp( float min, float max );
	idVecX& 		SwapElements( int e1, int e2 );
	
	float			Length() const;
	float			LengthSqr() const;
	idVecX			Normalize() const;
	float			NormalizeSelf();
	
	int				GetDimension() const;
	
	const idVec3& 	SubVec3( int index ) const;
	idVec3& 		SubVec3( int index );
	const idVec6& 	SubVec6( int index ) const;
	idVec6& 		SubVec6( int index );
	const float* 	ToFloatPtr() const;
	float* 			ToFloatPtr();
	const char* 	ToString( int precision = 2 ) const;
	
private:
	int				size;					// size of the vector
	int				alloced;				// if -1 p points to data set with SetData
	float* 			p;						// memory the vector is stored
	
	static float	temp[VECX_MAX_TEMP + 4];	// used to store intermediate results
	static float* 	tempPtr;				// pointer to 16 byte aligned temporary memory
	static int		tempIndex;				// index into memory pool, wraps around
	
private:
	void			SetTempSize( int size );
};


ID_INLINE idVecX::idVecX()
{
	size = alloced = 0;
	p = NULL;
}

ID_INLINE idVecX::idVecX( int length )
{
	size = alloced = 0;
	p = NULL;
	SetSize( length );
}

ID_INLINE idVecX::idVecX( int length, float* data )
{
	size = alloced = 0;
	p = NULL;
	SetData( length, data );
}

ID_INLINE idVecX::~idVecX()
{
	// if not temp memory
	if( p && ( p < idVecX::tempPtr || p >= idVecX::tempPtr + VECX_MAX_TEMP ) && alloced != -1 )
	{
		Mem_Free16( p );
	}
}

ID_INLINE float idVecX::operator[]( const int index ) const
{
	assert( index >= 0 && index < size );
	return p[index];
}

ID_INLINE float& idVecX::operator[]( const int index )
{
	assert( index >= 0 && index < size );
	return p[index];
}

ID_INLINE idVecX idVecX::operator-() const
{
	int i;
	idVecX m;
	
	m.SetTempSize( size );
	for( i = 0; i < size; i++ )
	{
		m.p[i] = -p[i];
	}
	return m;
}

ID_INLINE idVecX& idVecX::operator=( const idVecX& a )
{
	SetSize( a.size );
#ifdef VECX_SIMD
	SIMDProcessor->Copy16( p, a.p, a.size );
#else
	memcpy( p, a.p, a.size * sizeof( float ) );
#endif
	idVecX::tempIndex = 0;
	return *this;
}

ID_INLINE idVecX idVecX::operator+( const idVecX& a ) const
{
	idVecX m;
	
	assert( size == a.size );
	m.SetTempSize( size );
#ifdef VECX_SIMD
	SIMDProcessor->Add16( m.p, p, a.p, size );
#else
	int i;
	for( i = 0; i < size; i++ )
	{
		m.p[i] = p[i] + a.p[i];
	}
#endif
	return m;
}

ID_INLINE idVecX idVecX::operator-( const idVecX& a ) const
{
	idVecX m;
	
	assert( size == a.size );
	m.SetTempSize( size );
#ifdef VECX_SIMD
	SIMDProcessor->Sub16( m.p, p, a.p, size );
#else
	int i;
	for( i = 0; i < size; i++ )
	{
		m.p[i] = p[i] - a.p[i];
	}
#endif
	return m;
}

ID_INLINE idVecX& idVecX::operator+=( const idVecX& a )
{
	assert( size == a.size );
#ifdef VECX_SIMD
	SIMDProcessor->AddAssign16( p, a.p, size );
#else
	int i;
	for( i = 0; i < size; i++ )
	{
		p[i] += a.p[i];
	}
#endif
	idVecX::tempIndex = 0;
	return *this;
}

ID_INLINE idVecX& idVecX::operator-=( const idVecX& a )
{
	assert( size == a.size );
#ifdef VECX_SIMD
	SIMDProcessor->SubAssign16( p, a.p, size );
#else
	int i;
	for( i = 0; i < size; i++ )
	{
		p[i] -= a.p[i];
	}
#endif
	idVecX::tempIndex = 0;
	return *this;
}

ID_INLINE idVecX idVecX::operator*( const float a ) const
{
	idVecX m;
	
	m.SetTempSize( size );
#ifdef VECX_SIMD
	SIMDProcessor->Mul16( m.p, p, a, size );
#else
	int i;
	for( i = 0; i < size; i++ )
	{
		m.p[i] = p[i] * a;
	}
#endif
	return m;
}

ID_INLINE idVecX& idVecX::operator*=( const float a )
{
#ifdef VECX_SIMD
	SIMDProcessor->MulAssign16( p, a, size );
#else
	int i;
	for( i = 0; i < size; i++ )
	{
		p[i] *= a;
	}
#endif
	return *this;
}

ID_INLINE idVecX idVecX::operator/( const float a ) const
{
	assert( a != 0.0f );
	return ( *this ) * ( 1.0f / a );
}

ID_INLINE idVecX& idVecX::operator/=( const float a )
{
	assert( a != 0.0f );
	( *this ) *= ( 1.0f / a );
	return *this;
}

ID_INLINE idVecX operator*( const float a, const idVecX b )
{
	return b * a;
}

ID_INLINE float idVecX::operator*( const idVecX& a ) const
{
	int i;
	float sum = 0.0f;
	
	assert( size == a.size );
	for( i = 0; i < size; i++ )
	{
		sum += p[i] * a.p[i];
	}
	return sum;
}

ID_INLINE bool idVecX::Compare( const idVecX& a ) const
{
	int i;
	
	assert( size == a.size );
	for( i = 0; i < size; i++ )
	{
		if( p[i] != a.p[i] )
		{
			return false;
		}
	}
	return true;
}

ID_INLINE bool idVecX::Compare( const idVecX& a, const float epsilon ) const
{
	int i;
	
	assert( size == a.size );
	for( i = 0; i < size; i++ )
	{
		if( idMath::Fabs( p[i] - a.p[i] ) > epsilon )
		{
			return false;
		}
	}
	return true;
}

ID_INLINE bool idVecX::operator==( const idVecX& a ) const
{
	return Compare( a );
}

ID_INLINE bool idVecX::operator!=( const idVecX& a ) const
{
	return !Compare( a );
}

ID_INLINE void idVecX::SetSize( int newSize )
{
	int alloc = ( newSize + 3 ) & ~3;
	if( alloc > alloced && alloced != -1 )
	{
		if( p )
		{
			Mem_Free16( p );
		}
		p = ( float* ) Mem_Alloc16( alloc * sizeof( float ) );
		alloced = alloc;
	}
	size = newSize;
	VECX_CLEAREND();
}

ID_INLINE void idVecX::ChangeSize( int newSize, bool makeZero )
{
	int alloc = ( newSize + 3 ) & ~3;
	if( alloc > alloced && alloced != -1 )
	{
		float* oldVec = p;
		p = ( float* ) Mem_Alloc16( alloc * sizeof( float ) );
		alloced = alloc;
		if( oldVec )
		{
			for( int i = 0; i < size; i++ )
			{
				p[i] = oldVec[i];
			}
			Mem_Free16( oldVec );
		}
		if( makeZero )
		{
			// zero any new elements
			for( int i = size; i < newSize; i++ )
			{
				p[i] = 0.0f;
			}
		}
	}
	size = newSize;
	VECX_CLEAREND();
}

ID_INLINE void idVecX::SetTempSize( int newSize )
{

	size = newSize;
	alloced = ( newSize + 3 ) & ~3;
	assert( alloced < VECX_MAX_TEMP );
	if( idVecX::tempIndex + alloced > VECX_MAX_TEMP )
	{
		idVecX::tempIndex = 0;
	}
	p = idVecX::tempPtr + idVecX::tempIndex;
	idVecX::tempIndex += alloced;
	VECX_CLEAREND();
}

ID_INLINE void idVecX::SetData( int length, float* data )
{
	if( p && ( p < idVecX::tempPtr || p >= idVecX::tempPtr + VECX_MAX_TEMP ) && alloced != -1 )
	{
		Mem_Free16( p );
	}
	
	// RB: 64 bit fix, changed int to uintptr_t
	assert( ( ( ( uintptr_t ) data ) & 15 ) == 0 ); // data must be 16 byte aligned
	// RB end
	
	p = data;
	size = length;
	alloced = -1;
	VECX_CLEAREND();
}

ID_INLINE void idVecX::Zero()
{
#ifdef VECX_SIMD
	SIMDProcessor->Zero16( p, size );
#else
	memset( p, 0, size * sizeof( float ) );
#endif
}

ID_INLINE void idVecX::Zero( int length )
{
	SetSize( length );
#ifdef VECX_SIMD
	SIMDProcessor->Zero16( p, length );
#else
	memset( p, 0, size * sizeof( float ) );
#endif
}

ID_INLINE void idVecX::Random( int seed, float l, float u )
{
	int i;
	float c;
	idRandom rnd( seed );
	
	c = u - l;
	for( i = 0; i < size; i++ )
	{
		p[i] = l + rnd.RandomFloat() * c;
	}
}

ID_INLINE void idVecX::Random( int length, int seed, float l, float u )
{
	int i;
	float c;
	idRandom rnd( seed );
	
	SetSize( length );
	c = u - l;
	for( i = 0; i < size; i++ )
	{
		p[i] = l + rnd.RandomFloat() * c;
	}
}

ID_INLINE void idVecX::Negate()
{
#ifdef VECX_SIMD
	SIMDProcessor->Negate16( p, size );
#else
	int i;
	for( i = 0; i < size; i++ )
	{
		p[i] = -p[i];
	}
#endif
}

ID_INLINE void idVecX::Clamp( float min, float max )
{
	int i;
	for( i = 0; i < size; i++ )
	{
		if( p[i] < min )
		{
			p[i] = min;
		}
		else if( p[i] > max )
		{
			p[i] = max;
		}
	}
}

ID_INLINE idVecX& idVecX::SwapElements( int e1, int e2 )
{
	float tmp;
	tmp = p[e1];
	p[e1] = p[e2];
	p[e2] = tmp;
	return *this;
}

ID_INLINE float idVecX::Length() const
{
	int i;
	float sum = 0.0f;
	
	for( i = 0; i < size; i++ )
	{
		sum += p[i] * p[i];
	}
	return idMath::Sqrt( sum );
}

ID_INLINE float idVecX::LengthSqr() const
{
	int i;
	float sum = 0.0f;
	
	for( i = 0; i < size; i++ )
	{
		sum += p[i] * p[i];
	}
	return sum;
}

ID_INLINE idVecX idVecX::Normalize() const
{
	int i;
	idVecX m;
	float invSqrt, sum = 0.0f;
	
	m.SetTempSize( size );
	for( i = 0; i < size; i++ )
	{
		sum += p[i] * p[i];
	}
	invSqrt = idMath::InvSqrt( sum );
	for( i = 0; i < size; i++ )
	{
		m.p[i] = p[i] * invSqrt;
	}
	return m;
}

ID_INLINE float idVecX::NormalizeSelf()
{
	float invSqrt, sum = 0.0f;
	int i;
	for( i = 0; i < size; i++ )
	{
		sum += p[i] * p[i];
	}
	invSqrt = idMath::InvSqrt( sum );
	for( i = 0; i < size; i++ )
	{
		p[i] *= invSqrt;
	}
	return invSqrt * sum;
}

ID_INLINE int idVecX::GetDimension() const
{
	return size;
}

ID_INLINE idVec3& idVecX::SubVec3( int index )
{
	assert( index >= 0 && index * 3 + 3 <= size );
	return *reinterpret_cast<idVec3*>( p + index * 3 );
}

ID_INLINE const idVec3& idVecX::SubVec3( int index ) const
{
	assert( index >= 0 && index * 3 + 3 <= size );
	return *reinterpret_cast<const idVec3*>( p + index * 3 );
}

ID_INLINE idVec6& idVecX::SubVec6( int index )
{
	assert( index >= 0 && index * 6 + 6 <= size );
	return *reinterpret_cast<idVec6*>( p + index * 6 );
}

ID_INLINE const idVec6& idVecX::SubVec6( int index ) const
{
	assert( index >= 0 && index * 6 + 6 <= size );
	return *reinterpret_cast<const idVec6*>( p + index * 6 );
}

ID_INLINE const float* idVecX::ToFloatPtr() const
{
	return p;
}

ID_INLINE float* idVecX::ToFloatPtr()
{
	return p;
}


#endif // !__MATH_VECTORX_H__
