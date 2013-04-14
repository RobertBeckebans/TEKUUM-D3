/*
===========================================================================

Copyright (C) 2012 Robert Beckebans

===========================================================================
*/
#include "precompiled.h"
#pragma hdrstop

#include "tr_local.h"



#if defined(USE_GLES2)

void esLoadIdentity()
{
	MatrixIdentity( backEnd.glState.modelViewMatrix[ backEnd.glState.stackIndex ] );
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
		MatrixCopy( m, backEnd.glState.textureMatrix[ backEnd.glState.currenttmu ] );
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
	// TODO
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

void esEnableClientState( GLenum array )
{
	// TODO
}

void esDisableClientState( GLenum array )
{
	// TODO
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
	// TODO
}

void esColorPointer( GLint size, GLenum type, GLsizei stride, const GLvoid* pointer )
{
	// TODO
}

void esColor4f( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{
	glVertexAttrib4f( VA_INDEX_COLOR, red, green, blue, alpha );
}

#endif


