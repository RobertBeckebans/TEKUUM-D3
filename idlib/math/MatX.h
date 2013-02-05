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

#ifndef __MATH_MATX_H__
#define __MATH_MATX_H__

/*
===============================================================================

idMatX - arbitrary sized dense real matrix

The matrix lives on 16 byte aligned and 16 byte padded memory.

NOTE: due to the temporary memory pool idMatX cannot be used by multiple threads.

===============================================================================
*/

#define MATX_MAX_TEMP		1024
#define MATX_QUAD( x )		( ( ( ( x ) + 3 ) & ~3 ) * sizeof( float ) )
#define MATX_CLEAREND()		int s = numRows * numColumns; while( s < ( ( s + 3 ) & ~3 ) ) { mat[s++] = 0.0f; }
#define MATX_ALLOCA( n )	( (float *) _alloca16( MATX_QUAD( n ) ) )
#define MATX_SIMD

class idMatX
{
public:
	idMatX();
	explicit idMatX( int rows, int columns );
	explicit idMatX( int rows, int columns, float* src );
	~idMatX();
	
	void			Set( int rows, int columns, const float* src );
	void			Set( const idMat3& m1, const idMat3& m2 );
	void			Set( const idMat3& m1, const idMat3& m2, const idMat3& m3, const idMat3& m4 );
	
	const float* 	operator[]( int index ) const;
	float* 			operator[]( int index );
	idMatX& 		operator=( const idMatX& a );
	idMatX			operator*( const float a ) const;
	idVecX			operator*( const idVecX& vec ) const;
	idMatX			operator*( const idMatX& a ) const;
	idMatX			operator+( const idMatX& a ) const;
	idMatX			operator-( const idMatX& a ) const;
	idMatX& 		operator*=( const float a );
	idMatX& 		operator*=( const idMatX& a );
	idMatX& 		operator+=( const idMatX& a );
	idMatX& 		operator-=( const idMatX& a );
	
	friend idMatX	operator*( const float a, const idMatX& m );
	friend idVecX	operator*( const idVecX& vec, const idMatX& m );
	friend idVecX& 	operator*=( idVecX& vec, const idMatX& m );
	
	bool			Compare( const idMatX& a ) const;								// exact compare, no epsilon
	bool			Compare( const idMatX& a, const float epsilon ) const;			// compare with epsilon
	bool			operator==( const idMatX& a ) const;							// exact compare, no epsilon
	bool			operator!=( const idMatX& a ) const;							// exact compare, no epsilon
	
	void			SetSize( int rows, int columns );								// set the number of rows/columns
	void			ChangeSize( int rows, int columns, bool makeZero = false );		// change the size keeping data intact where possible
	int				GetNumRows() const
	{
		return numRows;    // get the number of rows
	}
	int				GetNumColumns() const
	{
		return numColumns;    // get the number of columns
	}
	void			SetData( int rows, int columns, float* data );					// set float array pointer
	void			Zero();													// clear matrix
	void			Zero( int rows, int columns );									// set size and clear matrix
	void			Identity();												// clear to identity matrix
	void			Identity( int rows, int columns );								// set size and clear to identity matrix
	void			Diag( const idVecX& v );										// create diagonal matrix from vector
	void			Random( int seed, float l = 0.0f, float u = 1.0f );				// fill matrix with random values
	void			Random( int rows, int columns, int seed, float l = 0.0f, float u = 1.0f );
	void			Negate();													// (*this) = - (*this)
	void			Clamp( float min, float max );									// clamp all values
	idMatX& 		SwapRows( int r1, int r2 );										// swap rows
	idMatX& 		SwapColumns( int r1, int r2 );									// swap columns
	idMatX& 		SwapRowsColumns( int r1, int r2 );								// swap rows and columns
	idMatX& 		RemoveRow( int r );												// remove a row
	idMatX& 		RemoveColumn( int r );											// remove a column
	idMatX& 		RemoveRowColumn( int r );										// remove a row and column
	void			ClearUpperTriangle();										// clear the upper triangle
	void			ClearLowerTriangle();										// clear the lower triangle
	void			SquareSubMatrix( const idMatX& m, int size );					// get square sub-matrix from 0,0 to size,size
	float			MaxDifference( const idMatX& m ) const;							// return maximum element difference between this and m
	
	bool			IsSquare() const
	{
		return ( numRows == numColumns );
	}
	bool			IsZero( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsIdentity( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsDiagonal( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsTriDiagonal( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetric( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsOrthogonal( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsOrthonormal( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsPMatrix( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsZMatrix( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsPositiveDefinite( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetricPositiveDefinite( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsPositiveSemiDefinite( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetricPositiveSemiDefinite( const float epsilon = MATRIX_EPSILON ) const;
	
	float			Trace() const;											// returns product of diagonal elements
	float			Determinant() const;										// returns determinant of matrix
	idMatX			Transpose() const;										// returns transpose
	idMatX& 		TransposeSelf();											// transposes the matrix itself
	idMatX			Inverse() const;											// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseSelf();											// returns false if determinant is zero
	idMatX			InverseFast() const;										// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseFastSelf();										// returns false if determinant is zero
	
	bool			LowerTriangularInverse();									// in-place inversion, returns false if determinant is zero
	bool			UpperTriangularInverse();									// in-place inversion, returns false if determinant is zero
	
	idVecX			Multiply( const idVecX& vec ) const;							// (*this) * vec
	idVecX			TransposeMultiply( const idVecX& vec ) const;					// this->Transpose() * vec
	
	idMatX			Multiply( const idMatX& a ) const;								// (*this) * a
	idMatX			TransposeMultiply( const idMatX& a ) const;						// this->Transpose() * a
	
	void			Multiply( idVecX& dst, const idVecX& vec ) const;				// dst = (*this) * vec
	void			MultiplyAdd( idVecX& dst, const idVecX& vec ) const;			// dst += (*this) * vec
	void			MultiplySub( idVecX& dst, const idVecX& vec ) const;			// dst -= (*this) * vec
	void			TransposeMultiply( idVecX& dst, const idVecX& vec ) const;		// dst = this->Transpose() * vec
	void			TransposeMultiplyAdd( idVecX& dst, const idVecX& vec ) const;	// dst += this->Transpose() * vec
	void			TransposeMultiplySub( idVecX& dst, const idVecX& vec ) const;	// dst -= this->Transpose() * vec
	
	void			Multiply( idMatX& dst, const idMatX& a ) const;					// dst = (*this) * a
	void			TransposeMultiply( idMatX& dst, const idMatX& a ) const;		// dst = this->Transpose() * a
	
	int				GetDimension() const;										// returns total number of values in matrix
	
	const idVec6& 	SubVec6( int row ) const;										// interpret beginning of row as a const idVec6
	idVec6& 		SubVec6( int row );												// interpret beginning of row as an idVec6
	const idVecX	SubVecX( int row ) const;										// interpret complete row as a const idVecX
	idVecX			SubVecX( int row );												// interpret complete row as an idVecX
	const float* 	ToFloatPtr() const;										// pointer to const matrix float array
	float* 			ToFloatPtr();												// pointer to matrix float array
	const char* 	ToString( int precision = 2 ) const;
	
	void			Update_RankOne( const idVecX& v, const idVecX& w, float alpha );
	void			Update_RankOneSymmetric( const idVecX& v, float alpha );
	void			Update_RowColumn( const idVecX& v, const idVecX& w, int r );
	void			Update_RowColumnSymmetric( const idVecX& v, int r );
	void			Update_Increment( const idVecX& v, const idVecX& w );
	void			Update_IncrementSymmetric( const idVecX& v );
	void			Update_Decrement( int r );
	
	bool			Inverse_GaussJordan();					// invert in-place with Gauss-Jordan elimination
	bool			Inverse_UpdateRankOne( const idVecX& v, const idVecX& w, float alpha );
	bool			Inverse_UpdateRowColumn( const idVecX& v, const idVecX& w, int r );
	bool			Inverse_UpdateIncrement( const idVecX& v, const idVecX& w );
	bool			Inverse_UpdateDecrement( const idVecX& v, const idVecX& w, int r );
	void			Inverse_Solve( idVecX& x, const idVecX& b ) const;
	
	bool			LU_Factor( int* index, float* det = NULL );		// factor in-place: L * U
	bool			LU_UpdateRankOne( const idVecX& v, const idVecX& w, float alpha, int* index );
	bool			LU_UpdateRowColumn( const idVecX& v, const idVecX& w, int r, int* index );
	bool			LU_UpdateIncrement( const idVecX& v, const idVecX& w, int* index );
	bool			LU_UpdateDecrement( const idVecX& v, const idVecX& w, const idVecX& u, int r, int* index );
	void			LU_Solve( idVecX& x, const idVecX& b, const int* index ) const;
	void			LU_Inverse( idMatX& inv, const int* index ) const;
	void			LU_UnpackFactors( idMatX& L, idMatX& U ) const;
	void			LU_MultiplyFactors( idMatX& m, const int* index ) const;
	
	bool			QR_Factor( idVecX& c, idVecX& d );				// factor in-place: Q * R
	bool			QR_UpdateRankOne( idMatX& R, const idVecX& v, const idVecX& w, float alpha );
	bool			QR_UpdateRowColumn( idMatX& R, const idVecX& v, const idVecX& w, int r );
	bool			QR_UpdateIncrement( idMatX& R, const idVecX& v, const idVecX& w );
	bool			QR_UpdateDecrement( idMatX& R, const idVecX& v, const idVecX& w, int r );
	void			QR_Solve( idVecX& x, const idVecX& b, const idVecX& c, const idVecX& d ) const;
	void			QR_Solve( idVecX& x, const idVecX& b, const idMatX& R ) const;
	void			QR_Inverse( idMatX& inv, const idVecX& c, const idVecX& d ) const;
	void			QR_UnpackFactors( idMatX& Q, idMatX& R, const idVecX& c, const idVecX& d ) const;
	void			QR_MultiplyFactors( idMatX& m, const idVecX& c, const idVecX& d ) const;
	
	bool			SVD_Factor( idVecX& w, idMatX& V );				// factor in-place: U * Diag(w) * V.Transpose()
	void			SVD_Solve( idVecX& x, const idVecX& b, const idVecX& w, const idMatX& V ) const;
	void			SVD_Inverse( idMatX& inv, const idVecX& w, const idMatX& V ) const;
	void			SVD_MultiplyFactors( idMatX& m, const idVecX& w, const idMatX& V ) const;
	
	bool			Cholesky_Factor();						// factor in-place: L * L.Transpose()
	bool			Cholesky_UpdateRankOne( const idVecX& v, float alpha, int offset = 0 );
	bool			Cholesky_UpdateRowColumn( const idVecX& v, int r );
	bool			Cholesky_UpdateIncrement( const idVecX& v );
	bool			Cholesky_UpdateDecrement( const idVecX& v, int r );
	void			Cholesky_Solve( idVecX& x, const idVecX& b ) const;
	void			Cholesky_Inverse( idMatX& inv ) const;
	void			Cholesky_MultiplyFactors( idMatX& m ) const;
	
	bool			LDLT_Factor();							// factor in-place: L * D * L.Transpose()
	bool			LDLT_UpdateRankOne( const idVecX& v, float alpha, int offset = 0 );
	bool			LDLT_UpdateRowColumn( const idVecX& v, int r );
	bool			LDLT_UpdateIncrement( const idVecX& v );
	bool			LDLT_UpdateDecrement( const idVecX& v, int r );
	void			LDLT_Solve( idVecX& x, const idVecX& b ) const;
	void			LDLT_Inverse( idMatX& inv ) const;
	void			LDLT_UnpackFactors( idMatX& L, idMatX& D ) const;
	void			LDLT_MultiplyFactors( idMatX& m ) const;
	
	void			TriDiagonal_ClearTriangles();
	bool			TriDiagonal_Solve( idVecX& x, const idVecX& b ) const;
	void			TriDiagonal_Inverse( idMatX& inv ) const;
	
	bool			Eigen_SolveSymmetricTriDiagonal( idVecX& eigenValues );
	bool			Eigen_SolveSymmetric( idVecX& eigenValues );
	bool			Eigen_Solve( idVecX& realEigenValues, idVecX& imaginaryEigenValues );
	void			Eigen_SortIncreasing( idVecX& eigenValues );
	void			Eigen_SortDecreasing( idVecX& eigenValues );
	
	static void		Test();
	
private:
	int				numRows;				// number of rows
	int				numColumns;				// number of columns
	int				alloced;				// floats allocated, if -1 then mat points to data set with SetData
	float* 			mat;					// memory the matrix is stored
	
	static float	temp[MATX_MAX_TEMP + 4];	// used to store intermediate results
	static float* 	tempPtr;				// pointer to 16 byte aligned temporary memory
	static int		tempIndex;				// index into memory pool, wraps around
	
private:
	void			SetTempSize( int rows, int columns );
	float			DeterminantGeneric() const;
	bool			InverseSelfGeneric();
	void			QR_Rotate( idMatX& R, int i, float a, float b );
	float			Pythag( float a, float b ) const;
	void			SVD_BiDiag( idVecX& w, idVecX& rv1, float& anorm );
	void			SVD_InitialWV( idVecX& w, idMatX& V, idVecX& rv1 );
	void			HouseholderReduction( idVecX& diag, idVecX& subd );
	bool			QL( idVecX& diag, idVecX& subd );
	void			HessenbergReduction( idMatX& H );
	void			ComplexDivision( float xr, float xi, float yr, float yi, float& cdivr, float& cdivi );
	bool			HessenbergToRealSchur( idMatX& H, idVecX& realEigenValues, idVecX& imaginaryEigenValues );
};

ID_INLINE idMatX::idMatX()
{
	numRows = numColumns = alloced = 0;
	mat = NULL;
}

ID_INLINE idMatX::~idMatX()
{
	// if not temp memory
	if( mat != NULL && ( mat < idMatX::tempPtr || mat > idMatX::tempPtr + MATX_MAX_TEMP ) && alloced != -1 )
	{
		Mem_Free16( mat );
	}
}

ID_INLINE idMatX::idMatX( int rows, int columns )
{
	numRows = numColumns = alloced = 0;
	mat = NULL;
	SetSize( rows, columns );
}

ID_INLINE idMatX::idMatX( int rows, int columns, float* src )
{
	numRows = numColumns = alloced = 0;
	mat = NULL;
	SetData( rows, columns, src );
}

ID_INLINE void idMatX::Set( int rows, int columns, const float* src )
{
	SetSize( rows, columns );
	memcpy( this->mat, src, rows * columns * sizeof( float ) );
}

ID_INLINE void idMatX::Set( const idMat3& m1, const idMat3& m2 )
{
	int i, j;
	
	SetSize( 3, 6 );
	for( i = 0; i < 3; i++ )
	{
		for( j = 0; j < 3; j++ )
		{
			mat[( i + 0 ) * numColumns + ( j + 0 )] = m1[i][j];
			mat[( i + 0 ) * numColumns + ( j + 3 )] = m2[i][j];
		}
	}
}

ID_INLINE void idMatX::Set( const idMat3& m1, const idMat3& m2, const idMat3& m3, const idMat3& m4 )
{
	int i, j;
	
	SetSize( 6, 6 );
	for( i = 0; i < 3; i++ )
	{
		for( j = 0; j < 3; j++ )
		{
			mat[( i + 0 ) * numColumns + ( j + 0 )] = m1[i][j];
			mat[( i + 0 ) * numColumns + ( j + 3 )] = m2[i][j];
			mat[( i + 3 ) * numColumns + ( j + 0 )] = m3[i][j];
			mat[( i + 3 ) * numColumns + ( j + 3 )] = m4[i][j];
		}
	}
}

ID_INLINE const float* idMatX::operator[]( int index ) const
{
	assert( ( index >= 0 ) && ( index < numRows ) );
	return mat + index * numColumns;
}

ID_INLINE float* idMatX::operator[]( int index )
{
	assert( ( index >= 0 ) && ( index < numRows ) );
	return mat + index * numColumns;
}

ID_INLINE idMatX& idMatX::operator=( const idMatX& a )
{
	SetSize( a.numRows, a.numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->Copy16( mat, a.mat, a.numRows * a.numColumns );
#else
	memcpy( mat, a.mat, a.numRows * a.numColumns * sizeof( float ) );
#endif
	idMatX::tempIndex = 0;
	return *this;
}

ID_INLINE idMatX idMatX::operator*( const float a ) const
{
	idMatX m;
	
	m.SetTempSize( numRows, numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->Mul16( m.mat, mat, a, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for( i = 0; i < s; i++ )
	{
		m.mat[i] = mat[i] * a;
	}
#endif
	return m;
}

ID_INLINE idVecX idMatX::operator*( const idVecX& vec ) const
{
	idVecX dst;
	
	assert( numColumns == vec.GetSize() );
	
	dst.SetTempSize( numRows );
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyVecX( dst, *this, vec );
#else
	Multiply( dst, vec );
#endif
	return dst;
}

ID_INLINE idMatX idMatX::operator*( const idMatX& a ) const
{
	idMatX dst;
	
	assert( numColumns == a.numRows );
	
	dst.SetTempSize( numRows, a.numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyMatX( dst, *this, a );
#else
	Multiply( dst, a );
#endif
	return dst;
}

ID_INLINE idMatX idMatX::operator+( const idMatX& a ) const
{
	idMatX m;
	
	assert( numRows == a.numRows && numColumns == a.numColumns );
	m.SetTempSize( numRows, numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->Add16( m.mat, mat, a.mat, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for( i = 0; i < s; i++ )
	{
		m.mat[i] = mat[i] + a.mat[i];
	}
#endif
	return m;
}

ID_INLINE idMatX idMatX::operator-( const idMatX& a ) const
{
	idMatX m;
	
	assert( numRows == a.numRows && numColumns == a.numColumns );
	m.SetTempSize( numRows, numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->Sub16( m.mat, mat, a.mat, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for( i = 0; i < s; i++ )
	{
		m.mat[i] = mat[i] - a.mat[i];
	}
#endif
	return m;
}

ID_INLINE idMatX& idMatX::operator*=( const float a )
{
#ifdef MATX_SIMD
	SIMDProcessor->MulAssign16( mat, a, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for( i = 0; i < s; i++ )
	{
		mat[i] *= a;
	}
#endif
	idMatX::tempIndex = 0;
	return *this;
}

ID_INLINE idMatX& idMatX::operator*=( const idMatX& a )
{
	*this = *this * a;
	idMatX::tempIndex = 0;
	return *this;
}

ID_INLINE idMatX& idMatX::operator+=( const idMatX& a )
{
	assert( numRows == a.numRows && numColumns == a.numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->AddAssign16( mat, a.mat, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for( i = 0; i < s; i++ )
	{
		mat[i] += a.mat[i];
	}
#endif
	idMatX::tempIndex = 0;
	return *this;
}

ID_INLINE idMatX& idMatX::operator-=( const idMatX& a )
{
	assert( numRows == a.numRows && numColumns == a.numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->SubAssign16( mat, a.mat, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for( i = 0; i < s; i++ )
	{
		mat[i] -= a.mat[i];
	}
#endif
	idMatX::tempIndex = 0;
	return *this;
}

ID_INLINE idMatX operator*( const float a, idMatX const& m )
{
	return m * a;
}

ID_INLINE idVecX operator*( const idVecX& vec, const idMatX& m )
{
	return m * vec;
}

ID_INLINE idVecX& operator*=( idVecX& vec, const idMatX& m )
{
	vec = m * vec;
	return vec;
}

ID_INLINE bool idMatX::Compare( const idMatX& a ) const
{
	int i, s;
	
	assert( numRows == a.numRows && numColumns == a.numColumns );
	
	s = numRows * numColumns;
	for( i = 0; i < s; i++ )
	{
		if( mat[i] != a.mat[i] )
		{
			return false;
		}
	}
	return true;
}

ID_INLINE bool idMatX::Compare( const idMatX& a, const float epsilon ) const
{
	int i, s;
	
	assert( numRows == a.numRows && numColumns == a.numColumns );
	
	s = numRows * numColumns;
	for( i = 0; i < s; i++ )
	{
		if( idMath::Fabs( mat[i] - a.mat[i] ) > epsilon )
		{
			return false;
		}
	}
	return true;
}

ID_INLINE bool idMatX::operator==( const idMatX& a ) const
{
	return Compare( a );
}

ID_INLINE bool idMatX::operator!=( const idMatX& a ) const
{
	return !Compare( a );
}

ID_INLINE void idMatX::SetSize( int rows, int columns )
{
	assert( mat < idMatX::tempPtr || mat > idMatX::tempPtr + MATX_MAX_TEMP );
	int alloc = ( rows * columns + 3 ) & ~3;
	if( alloc > alloced && alloced != -1 )
	{
		if( mat != NULL )
		{
			Mem_Free16( mat );
		}
		mat = ( float* ) Mem_Alloc16( alloc * sizeof( float ) );
		alloced = alloc;
	}
	numRows = rows;
	numColumns = columns;
	MATX_CLEAREND();
}

ID_INLINE void idMatX::SetTempSize( int rows, int columns )
{
	int newSize;
	
	newSize = ( rows * columns + 3 ) & ~3;
	assert( newSize < MATX_MAX_TEMP );
	if( idMatX::tempIndex + newSize > MATX_MAX_TEMP )
	{
		idMatX::tempIndex = 0;
	}
	mat = idMatX::tempPtr + idMatX::tempIndex;
	idMatX::tempIndex += newSize;
	alloced = newSize;
	numRows = rows;
	numColumns = columns;
	MATX_CLEAREND();
}

ID_INLINE void idMatX::SetData( int rows, int columns, float* data )
{
	assert( mat < idMatX::tempPtr || mat > idMatX::tempPtr + MATX_MAX_TEMP );
	if( mat != NULL && alloced != -1 )
	{
		Mem_Free16( mat );
	}
	// RB: 64 bit fix, changed int to intptr_t
	assert( ( ( ( intptr_t ) data ) & 15 ) == 0 ); // data must be 16 byte aligned
	// RB end
	
	mat = data;
	alloced = -1;
	numRows = rows;
	numColumns = columns;
	MATX_CLEAREND();
}

ID_INLINE void idMatX::Zero()
{
#ifdef MATX_SIMD
	SIMDProcessor->Zero16( mat, numRows * numColumns );
#else
	memset( mat, 0, numRows * numColumns * sizeof( float ) );
#endif
}

ID_INLINE void idMatX::Zero( int rows, int columns )
{
	SetSize( rows, columns );
#ifdef MATX_SIMD
	SIMDProcessor->Zero16( mat, numRows * numColumns );
#else
	memset( mat, 0, rows * columns * sizeof( float ) );
#endif
}

ID_INLINE void idMatX::Identity()
{
	assert( numRows == numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->Zero16( mat, numRows * numColumns );
#else
	memset( mat, 0, numRows * numColumns * sizeof( float ) );
#endif
	for( int i = 0; i < numRows; i++ )
	{
		mat[i * numColumns + i] = 1.0f;
	}
}

ID_INLINE void idMatX::Identity( int rows, int columns )
{
	assert( rows == columns );
	SetSize( rows, columns );
	idMatX::Identity();
}

ID_INLINE void idMatX::Diag( const idVecX& v )
{
	Zero( v.GetSize(), v.GetSize() );
	for( int i = 0; i < v.GetSize(); i++ )
	{
		mat[i * numColumns + i] = v[i];
	}
}

ID_INLINE void idMatX::Random( int seed, float l, float u )
{
	int i, s;
	float c;
	idRandom rnd( seed );
	
	c = u - l;
	s = numRows * numColumns;
	for( i = 0; i < s; i++ )
	{
		mat[i] = l + rnd.RandomFloat() * c;
	}
}

ID_INLINE void idMatX::Random( int rows, int columns, int seed, float l, float u )
{
	int i, s;
	float c;
	idRandom rnd( seed );
	
	SetSize( rows, columns );
	c = u - l;
	s = numRows * numColumns;
	for( i = 0; i < s; i++ )
	{
		mat[i] = l + rnd.RandomFloat() * c;
	}
}

ID_INLINE void idMatX::Negate()
{
#ifdef MATX_SIMD
	SIMDProcessor->Negate16( mat, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for( i = 0; i < s; i++ )
	{
		mat[i] = -mat[i];
	}
#endif
}

ID_INLINE void idMatX::Clamp( float min, float max )
{
	int i, s;
	s = numRows * numColumns;
	for( i = 0; i < s; i++ )
	{
		if( mat[i] < min )
		{
			mat[i] = min;
		}
		else if( mat[i] > max )
		{
			mat[i] = max;
		}
	}
}

ID_INLINE idMatX& idMatX::SwapRows( int r1, int r2 )
{
	float* ptr;
	
	ptr = ( float* ) _alloca16( numColumns * sizeof( float ) );
	memcpy( ptr, mat + r1 * numColumns, numColumns * sizeof( float ) );
	memcpy( mat + r1 * numColumns, mat + r2 * numColumns, numColumns * sizeof( float ) );
	memcpy( mat + r2 * numColumns, ptr, numColumns * sizeof( float ) );
	
	return *this;
}

ID_INLINE idMatX& idMatX::SwapColumns( int r1, int r2 )
{
	int i;
	float tmp, *ptr;
	
	for( i = 0; i < numRows; i++ )
	{
		ptr = mat + i * numColumns;
		tmp = ptr[r1];
		ptr[r1] = ptr[r2];
		ptr[r2] = tmp;
	}
	
	return *this;
}

ID_INLINE idMatX& idMatX::SwapRowsColumns( int r1, int r2 )
{

	SwapRows( r1, r2 );
	SwapColumns( r1, r2 );
	return *this;
}

ID_INLINE void idMatX::ClearUpperTriangle()
{
	assert( numRows == numColumns );
	for( int i = numRows - 2; i >= 0; i-- )
	{
		memset( mat + i * numColumns + i + 1, 0, ( numColumns - 1 - i ) * sizeof( float ) );
	}
}

ID_INLINE void idMatX::ClearLowerTriangle()
{
	assert( numRows == numColumns );
	for( int i = 1; i < numRows; i++ )
	{
		memset( mat + i * numColumns, 0, i * sizeof( float ) );
	}
}

ID_INLINE void idMatX::SquareSubMatrix( const idMatX& m, int size )
{
	int i;
	assert( size <= m.numRows && size <= m.numColumns );
	SetSize( size, size );
	for( i = 0; i < size; i++ )
	{
		memcpy( mat + i * numColumns, m.mat + i * m.numColumns, size * sizeof( float ) );
	}
}

ID_INLINE float idMatX::MaxDifference( const idMatX& m ) const
{
	int i, j;
	float diff, maxDiff;
	
	assert( numRows == m.numRows && numColumns == m.numColumns );
	
	maxDiff = -1.0f;
	for( i = 0; i < numRows; i++ )
	{
		for( j = 0; j < numColumns; j++ )
		{
			diff = idMath::Fabs( mat[ i * numColumns + j ] - m[i][j] );
			if( maxDiff < 0.0f || diff > maxDiff )
			{
				maxDiff = diff;
			}
		}
	}
	return maxDiff;
}

ID_INLINE bool idMatX::IsZero( const float epsilon ) const
{
	// returns true if (*this) == Zero
	for( int i = 0; i < numRows; i++ )
	{
		for( int j = 0; j < numColumns; j++ )
		{
			if( idMath::Fabs( mat[i * numColumns + j] ) > epsilon )
			{
				return false;
			}
		}
	}
	return true;
}

ID_INLINE bool idMatX::IsIdentity( const float epsilon ) const
{
	// returns true if (*this) == Identity
	assert( numRows == numColumns );
	for( int i = 0; i < numRows; i++ )
	{
		for( int j = 0; j < numColumns; j++ )
		{
			if( idMath::Fabs( mat[i * numColumns + j] - ( float )( i == j ) ) > epsilon )
			{
				return false;
			}
		}
	}
	return true;
}

ID_INLINE bool idMatX::IsDiagonal( const float epsilon ) const
{
	// returns true if all elements are zero except for the elements on the diagonal
	assert( numRows == numColumns );
	for( int i = 0; i < numRows; i++ )
	{
		for( int j = 0; j < numColumns; j++ )
		{
			if( i != j && idMath::Fabs( mat[i * numColumns + j] ) > epsilon )
			{
				return false;
			}
		}
	}
	return true;
}

ID_INLINE bool idMatX::IsTriDiagonal( const float epsilon ) const
{
	// returns true if all elements are zero except for the elements on the diagonal plus or minus one column
	
	if( numRows != numColumns )
	{
		return false;
	}
	for( int i = 0; i < numRows - 2; i++ )
	{
		for( int j = i + 2; j < numColumns; j++ )
		{
			if( idMath::Fabs( ( *this )[i][j] ) > epsilon )
			{
				return false;
			}
			if( idMath::Fabs( ( *this )[j][i] ) > epsilon )
			{
				return false;
			}
		}
	}
	return true;
}

ID_INLINE bool idMatX::IsSymmetric( const float epsilon ) const
{
	// (*this)[i][j] == (*this)[j][i]
	if( numRows != numColumns )
	{
		return false;
	}
	for( int i = 0; i < numRows; i++ )
	{
		for( int j = 0; j < numColumns; j++ )
		{
			if( idMath::Fabs( mat[ i * numColumns + j ] - mat[ j * numColumns + i ] ) > epsilon )
			{
				return false;
			}
		}
	}
	return true;
}

ID_INLINE float idMatX::Trace() const
{
	float trace = 0.0f;
	
	assert( numRows == numColumns );
	
	// sum of elements on the diagonal
	for( int i = 0; i < numRows; i++ )
	{
		trace += mat[i * numRows + i];
	}
	return trace;
}

ID_INLINE float idMatX::Determinant() const
{

	assert( numRows == numColumns );
	
	switch( numRows )
	{
		case 1:
			return mat[0];
		case 2:
			return reinterpret_cast<const idMat2*>( mat )->Determinant();
		case 3:
			return reinterpret_cast<const idMat3*>( mat )->Determinant();
		case 4:
			return reinterpret_cast<const idMat4*>( mat )->Determinant();
		case 5:
			return reinterpret_cast<const idMat5*>( mat )->Determinant();
		case 6:
			return reinterpret_cast<const idMat6*>( mat )->Determinant();
		default:
			return DeterminantGeneric();
	}
	return 0.0f;
}

ID_INLINE idMatX idMatX::Transpose() const
{
	idMatX transpose;
	int i, j;
	
	transpose.SetTempSize( numColumns, numRows );
	
	for( i = 0; i < numRows; i++ )
	{
		for( j = 0; j < numColumns; j++ )
		{
			transpose.mat[j * transpose.numColumns + i] = mat[i * numColumns + j];
		}
	}
	
	return transpose;
}

ID_INLINE idMatX& idMatX::TransposeSelf()
{
	*this = Transpose();
	return *this;
}

ID_INLINE idMatX idMatX::Inverse() const
{
	idMatX invMat;
	
	invMat.SetTempSize( numRows, numColumns );
	memcpy( invMat.mat, mat, numRows * numColumns * sizeof( float ) );
	verify( invMat.InverseSelf() );
	
	return invMat;
}

ID_INLINE bool idMatX::InverseSelf()
{

	assert( numRows == numColumns );
	
	switch( numRows )
	{
		case 1:
			if( idMath::Fabs( mat[0] ) < MATRIX_INVERSE_EPSILON )
			{
				return false;
			}
			mat[0] = 1.0f / mat[0];
			return true;
		case 2:
			return reinterpret_cast<idMat2*>( mat )->InverseSelf();
		case 3:
			return reinterpret_cast<idMat3*>( mat )->InverseSelf();
		case 4:
			return reinterpret_cast<idMat4*>( mat )->InverseSelf();
		case 5:
			return reinterpret_cast<idMat5*>( mat )->InverseSelf();
		case 6:
			return reinterpret_cast<idMat6*>( mat )->InverseSelf();
		default:
			return InverseSelfGeneric();
	}
}

ID_INLINE idMatX idMatX::InverseFast() const
{
	idMatX invMat;
	
	invMat.SetTempSize( numRows, numColumns );
	memcpy( invMat.mat, mat, numRows * numColumns * sizeof( float ) );
	verify( invMat.InverseFastSelf() );
	
	return invMat;
}

ID_INLINE bool idMatX::InverseFastSelf()
{

	assert( numRows == numColumns );
	
	switch( numRows )
	{
		case 1:
			if( idMath::Fabs( mat[0] ) < MATRIX_INVERSE_EPSILON )
			{
				return false;
			}
			mat[0] = 1.0f / mat[0];
			return true;
		case 2:
			return reinterpret_cast<idMat2*>( mat )->InverseFastSelf();
		case 3:
			return reinterpret_cast<idMat3*>( mat )->InverseFastSelf();
		case 4:
			return reinterpret_cast<idMat4*>( mat )->InverseFastSelf();
		case 5:
			return reinterpret_cast<idMat5*>( mat )->InverseFastSelf();
		case 6:
			return reinterpret_cast<idMat6*>( mat )->InverseFastSelf();
		default:
			return InverseSelfGeneric();
	}
	return false;
}

ID_INLINE idVecX idMatX::Multiply( const idVecX& vec ) const
{
	idVecX dst;
	
	assert( numColumns == vec.GetSize() );
	
	dst.SetTempSize( numRows );
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyVecX( dst, *this, vec );
#else
	Multiply( dst, vec );
#endif
	return dst;
}

ID_INLINE idMatX idMatX::Multiply( const idMatX& a ) const
{
	idMatX dst;
	
	assert( numColumns == a.numRows );
	
	dst.SetTempSize( numRows, a.numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyMatX( dst, *this, a );
#else
	Multiply( dst, a );
#endif
	return dst;
}

ID_INLINE idVecX idMatX::TransposeMultiply( const idVecX& vec ) const
{
	idVecX dst;
	
	assert( numRows == vec.GetSize() );
	
	dst.SetTempSize( numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->MatX_TransposeMultiplyVecX( dst, *this, vec );
#else
	TransposeMultiply( dst, vec );
#endif
	return dst;
}

ID_INLINE idMatX idMatX::TransposeMultiply( const idMatX& a ) const
{
	idMatX dst;
	
	assert( numRows == a.numRows );
	
	dst.SetTempSize( numColumns, a.numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->MatX_TransposeMultiplyMatX( dst, *this, a );
#else
	TransposeMultiply( dst, a );
#endif
	return dst;
}

ID_INLINE void idMatX::Multiply( idVecX& dst, const idVecX& vec ) const
{
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyVecX( dst, *this, vec );
#else
	int i, j;
	const float* mPtr, *vPtr;
	float* dstPtr;
	
	mPtr = mat;
	vPtr = vec.ToFloatPtr();
	dstPtr = dst.ToFloatPtr();
	for( i = 0; i < numRows; i++ )
	{
		float sum = mPtr[0] * vPtr[0];
		for( j = 1; j < numColumns; j++ )
		{
			sum += mPtr[j] * vPtr[j];
		}
		dstPtr[i] = sum;
		mPtr += numColumns;
	}
#endif
}

ID_INLINE void idMatX::MultiplyAdd( idVecX& dst, const idVecX& vec ) const
{
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyAddVecX( dst, *this, vec );
#else
	int i, j;
	const float* mPtr, *vPtr;
	float* dstPtr;
	
	mPtr = mat;
	vPtr = vec.ToFloatPtr();
	dstPtr = dst.ToFloatPtr();
	for( i = 0; i < numRows; i++ )
	{
		float sum = mPtr[0] * vPtr[0];
		for( j = 1; j < numColumns; j++ )
		{
			sum += mPtr[j] * vPtr[j];
		}
		dstPtr[i] += sum;
		mPtr += numColumns;
	}
#endif
}

ID_INLINE void idMatX::MultiplySub( idVecX& dst, const idVecX& vec ) const
{
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplySubVecX( dst, *this, vec );
#else
	int i, j;
	const float* mPtr, *vPtr;
	float* dstPtr;
	
	mPtr = mat;
	vPtr = vec.ToFloatPtr();
	dstPtr = dst.ToFloatPtr();
	for( i = 0; i < numRows; i++ )
	{
		float sum = mPtr[0] * vPtr[0];
		for( j = 1; j < numColumns; j++ )
		{
			sum += mPtr[j] * vPtr[j];
		}
		dstPtr[i] -= sum;
		mPtr += numColumns;
	}
#endif
}

ID_INLINE void idMatX::TransposeMultiply( idVecX& dst, const idVecX& vec ) const
{
#ifdef MATX_SIMD
	SIMDProcessor->MatX_TransposeMultiplyVecX( dst, *this, vec );
#else
	int i, j;
	const float* mPtr, *vPtr;
	float* dstPtr;
	
	vPtr = vec.ToFloatPtr();
	dstPtr = dst.ToFloatPtr();
	for( i = 0; i < numColumns; i++ )
	{
		mPtr = mat + i;
		float sum = mPtr[0] * vPtr[0];
		for( j = 1; j < numRows; j++ )
		{
			mPtr += numColumns;
			sum += mPtr[0] * vPtr[j];
		}
		dstPtr[i] = sum;
	}
#endif
}

ID_INLINE void idMatX::TransposeMultiplyAdd( idVecX& dst, const idVecX& vec ) const
{
#ifdef MATX_SIMD
	SIMDProcessor->MatX_TransposeMultiplyAddVecX( dst, *this, vec );
#else
	int i, j;
	const float* mPtr, *vPtr;
	float* dstPtr;
	
	vPtr = vec.ToFloatPtr();
	dstPtr = dst.ToFloatPtr();
	for( i = 0; i < numColumns; i++ )
	{
		mPtr = mat + i;
		float sum = mPtr[0] * vPtr[0];
		for( j = 1; j < numRows; j++ )
		{
			mPtr += numColumns;
			sum += mPtr[0] * vPtr[j];
		}
		dstPtr[i] += sum;
	}
#endif
}

ID_INLINE void idMatX::TransposeMultiplySub( idVecX& dst, const idVecX& vec ) const
{
#ifdef MATX_SIMD
	SIMDProcessor->MatX_TransposeMultiplySubVecX( dst, *this, vec );
#else
	int i, j;
	const float* mPtr, *vPtr;
	float* dstPtr;
	
	vPtr = vec.ToFloatPtr();
	dstPtr = dst.ToFloatPtr();
	for( i = 0; i < numColumns; i++ )
	{
		mPtr = mat + i;
		float sum = mPtr[0] * vPtr[0];
		for( j = 1; j < numRows; j++ )
		{
			mPtr += numColumns;
			sum += mPtr[0] * vPtr[j];
		}
		dstPtr[i] -= sum;
	}
#endif
}

ID_INLINE void idMatX::Multiply( idMatX& dst, const idMatX& a ) const
{
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyMatX( dst, *this, a );
#else
	int i, j, k, l, n;
	float* dstPtr;
	const float* m1Ptr, *m2Ptr;
	double sum;
	
	assert( numColumns == a.numRows );
	
	dstPtr = dst.ToFloatPtr();
	m1Ptr = ToFloatPtr();
	m2Ptr = a.ToFloatPtr();
	k = numRows;
	l = a.GetNumColumns();
	
	for( i = 0; i < k; i++ )
	{
		for( j = 0; j < l; j++ )
		{
			m2Ptr = a.ToFloatPtr() + j;
			sum = m1Ptr[0] * m2Ptr[0];
			for( n = 1; n < numColumns; n++ )
			{
				m2Ptr += l;
				sum += m1Ptr[n] * m2Ptr[0];
			}
			*dstPtr++ = sum;
		}
		m1Ptr += numColumns;
	}
#endif
}

ID_INLINE void idMatX::TransposeMultiply( idMatX& dst, const idMatX& a ) const
{
#ifdef MATX_SIMD
	SIMDProcessor->MatX_TransposeMultiplyMatX( dst, *this, a );
#else
	int i, j, k, l, n;
	float* dstPtr;
	const float* m1Ptr, *m2Ptr;
	double sum;
	
	assert( numRows == a.numRows );
	
	dstPtr = dst.ToFloatPtr();
	m1Ptr = ToFloatPtr();
	k = numColumns;
	l = a.numColumns;
	
	for( i = 0; i < k; i++ )
	{
		for( j = 0; j < l; j++ )
		{
			m1Ptr = ToFloatPtr() + i;
			m2Ptr = a.ToFloatPtr() + j;
			sum = m1Ptr[0] * m2Ptr[0];
			for( n = 1; n < numRows; n++ )
			{
				m1Ptr += numColumns;
				m2Ptr += a.numColumns;
				sum += m1Ptr[0] * m2Ptr[0];
			}
			*dstPtr++ = sum;
		}
	}
#endif
}

ID_INLINE int idMatX::GetDimension() const
{
	return numRows * numColumns;
}

ID_INLINE const idVec6& idMatX::SubVec6( int row ) const
{
	assert( numColumns >= 6 && row >= 0 && row < numRows );
	return *reinterpret_cast<const idVec6*>( mat + row * numColumns );
}

ID_INLINE idVec6& idMatX::SubVec6( int row )
{
	assert( numColumns >= 6 && row >= 0 && row < numRows );
	return *reinterpret_cast<idVec6*>( mat + row * numColumns );
}

ID_INLINE const idVecX idMatX::SubVecX( int row ) const
{
	idVecX v;
	assert( row >= 0 && row < numRows );
	v.SetData( numColumns, mat + row * numColumns );
	return v;
}

ID_INLINE idVecX idMatX::SubVecX( int row )
{
	idVecX v;
	assert( row >= 0 && row < numRows );
	v.SetData( numColumns, mat + row * numColumns );
	return v;
}

ID_INLINE const float* idMatX::ToFloatPtr() const
{
	return mat;
}

ID_INLINE float* idMatX::ToFloatPtr()
{
	return mat;
}

#endif // !__MATH_MATRIXX_H__
