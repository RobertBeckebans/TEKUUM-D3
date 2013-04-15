/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
Copyright (C) 2013 Robert Beckebans

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
#include "precompiled.h"
#pragma hdrstop

#include "tr_local.h"



#if defined(USE_GLES2)

void esLoadIdentity()
{
	if( backEnd.glState.matrixMode == GL_MODELVIEW )
	{
		MatrixIdentity( backEnd.glState.modelViewMatrix[ backEnd.glState.stackIndex ] );
	}
	else if( backEnd.glState.matrixMode == GL_PROJECTION )
	{
		MatrixIdentity( backEnd.glState.projectionMatrix[ backEnd.glState.stackIndex ] );
	}
	else if( backEnd.glState.matrixMode == GL_TEXTURE )
	{
		//MatrixIdentity( backEnd.glState.textureMatrix[ backEnd.glState.currenttmu ] );
		MatrixIdentity( backEnd.glState.textureMatrix[ backEnd.glState.stackIndex ] );
	}
}

void esLoadMatrixf( const GLfloat* m )
{
	if( backEnd.glState.matrixMode == GL_MODELVIEW )
	{
		MatrixCopy( m, backEnd.glState.modelViewMatrix[ backEnd.glState.stackIndex ] );
	}
	else if( backEnd.glState.matrixMode == GL_PROJECTION )
	{
		MatrixCopy( m, backEnd.glState.projectionMatrix[ backEnd.glState.stackIndex ] );
	}
	else if( backEnd.glState.matrixMode == GL_TEXTURE )
	{
		//MatrixCopy( m, backEnd.glState.textureMatrix[ backEnd.glState.currenttmu ] );
		MatrixCopy( m, backEnd.glState.textureMatrix[ backEnd.glState.stackIndex ] );
	}
	
	if( backEnd.glState.matrixMode == GL_MODELVIEW || backEnd.glState.matrixMode == GL_PROJECTION )
	{
		myGlMultMatrix( backEnd.glState.modelViewMatrix[ backEnd.glState.stackIndex ],
						backEnd.glState.projectionMatrix[ backEnd.glState.stackIndex ],
						backEnd.glState.modelViewProjectionMatrix[ backEnd.glState.stackIndex ] );
	}
}

void esMatrixMode( GLenum mode )
{
	if( mode != GL_MODELVIEW && mode != GL_PROJECTION && mode != GL_TEXTURE )
	{
		common->Error( "esMatrixMode: mode = %i is not GL_MODELVIEW, GL_PROJECTION or GL_TEXTURE", mode );
	}
	
	backEnd.glState.matrixMode = mode;
}

void esOrthof( GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar )
{
	if( backEnd.glState.matrixMode == GL_PROJECTION )
	{
		float* m = backEnd.glState.projectionMatrix[ backEnd.glState.stackIndex ];
		
		MatrixOrthogonalProjection( m, left, right, bottom, top, zNear, zFar );
	}
}

void esPushMatrix()
{
	backEnd.glState.stackIndex++;
	
	if( backEnd.glState.stackIndex >= MAX_GLSTACK )
	{
		backEnd.glState.stackIndex = MAX_GLSTACK - 1;
		common->Error( "esPushMatrix: stack overflow = %i", backEnd.glState.stackIndex );
	}
	
	MatrixCopy( backEnd.glState.modelViewMatrix[ backEnd.glState.stackIndex - 1 ], backEnd.glState.modelViewMatrix[ backEnd.glState.stackIndex ] );
	MatrixCopy( backEnd.glState.projectionMatrix[ backEnd.glState.stackIndex - 1 ], backEnd.glState.projectionMatrix[ backEnd.glState.stackIndex ] );
	MatrixCopy( backEnd.glState.modelViewProjectionMatrix[ backEnd.glState.stackIndex - 1 ], backEnd.glState.modelViewProjectionMatrix[ backEnd.glState.stackIndex ] );
}

void esPopMatrix()
{
	backEnd.glState.stackIndex--;
	
	if( backEnd.glState.stackIndex < 0 )
	{
		backEnd.glState.stackIndex = 0;
		common->Error( "esPopMatrix: stack underflow" );
	}
}

void esEnableClientState( GLenum value )
{
	switch( value )
	{
		case GL_VERTEX_ARRAY:
			glEnableVertexAttribArray( VA_INDEX_POSITION );
			break;
			
		case GL_NORMAL_ARRAY:
			glEnableVertexAttribArray( VA_INDEX_NORMAL );
			break;
			
		case GL_COLOR_ARRAY:
			glEnableVertexAttribArray( VA_INDEX_COLOR );
			break;
			
		case GL_TEXTURE_COORD_ARRAY:
			glEnableVertexAttribArray( VA_INDEX_TEXCOORD0 );
			break;
			
		default:
			assert( 0 );
			common->Error( "esEnableClientState: bad value = %i", value );
			break;
	}
	
}

void esDisableClientState( GLenum value )
{
	switch( value )
	{
		case GL_VERTEX_ARRAY:
			glDisableVertexAttribArray( VA_INDEX_POSITION );
			break;
			
		case GL_NORMAL_ARRAY:
			glDisableVertexAttribArray( VA_INDEX_NORMAL );
			break;
			
		case GL_COLOR_ARRAY:
			glDisableVertexAttribArray( VA_INDEX_COLOR );
			break;
			
		case GL_TEXTURE_COORD_ARRAY:
			glDisableVertexAttribArray( VA_INDEX_TEXCOORD0 );
			break;
			
		default:
			assert( 0 );
			common->Error( "esEnableClientState: bad value = %i", value );
			break;
	}
}

void esVertexPointer( GLint size, GLenum type, GLsizei stride, const GLvoid* pointer )
{
	glVertexAttribPointer( VA_INDEX_POSITION, size, type, false, stride, pointer );
}

void esNormalPointer( GLenum type, GLsizei stride, const GLvoid* pointer )
{
	glVertexAttribPointer( VA_INDEX_NORMAL, 3, type, false, stride, pointer );
}

void esTexCoordPointer( GLint size, GLenum type, GLsizei stride, const GLvoid* pointer )
{
	glVertexAttribPointer( VA_INDEX_TEXCOORD0, size, type, false, stride, pointer );
}

void esColorPointer( GLint size, GLenum type, GLsizei stride, const GLvoid* pointer )
{
	glVertexAttribPointer( VA_INDEX_COLOR, size, type, GL_TRUE, stride, pointer );
}

void esColor4f( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{
	backEnd.glState.color.x = red;
	backEnd.glState.color.y = green;
	backEnd.glState.color.z = blue;
	backEnd.glState.color.w = alpha;
	
	//glVertexAttrib4f( VA_INDEX_COLOR, red, green, blue, alpha );
}

#endif


