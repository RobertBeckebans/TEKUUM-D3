/*
===========================================================================

Copyright (C) 2012 Robert Beckebans

===========================================================================
*/

#include "precompiled.h"
#pragma hdrstop

#include "tr_local.h"

idList<Framebuffer*>	Framebuffer::framebuffers;

globalFramebuffers_t globalFramebuffers;

static void R_ListFramebuffers_f( const idCmdArgs& args )
{

	if( !glConfig.framebufferObjectAvailable )
	{
		common->Printf( "GL_EXT_framebuffer_object is not available.\n" );
		return;
	}
	
	
}

Framebuffer::Framebuffer( const char* name, int w, int h )
{
	fboName = name;
	
	frameBuffer = 0;
	
	memset( colorBuffers, 0, sizeof( colorBuffers ) );
	colorFormat = 0;
	
	depthBuffer = 0;
	depthFormat = 0;
	
	stencilBuffer = 0;
	stencilFormat = 0;
	
	width = w;
	height = h;
	
	glGenFramebuffers( 1, &frameBuffer );
	
	framebuffers.Append( this );
}

void Framebuffer::Init()
{
	cmdSystem->AddCommand( "listFramebuffers", R_ListFramebuffers_f, CMD_FL_RENDERER, "lists framebuffers" );
	
	backEnd.glState.framebuffer = NULL;
}

void Framebuffer::Shutdown()
{
	// TODO
}

void Framebuffer::Bind()
{
	if( r_logFile.GetBool() )
	{
		RB_LogComment( "--- Framebuffer::Bind( name = '%s' ) ---\n", fboName.c_str() );
	}
	
	if( backEnd.glState.framebuffer != this )
	{
		glBindFramebuffer( GL_FRAMEBUFFER, frameBuffer );
		backEnd.glState.framebuffer = this;
	}
}

void Framebuffer::BindNull()
{
	//if(backEnd.glState.framebuffer != NULL)
	{
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
		glBindRenderbuffer( GL_RENDERBUFFER, 0 );
		backEnd.glState.framebuffer = NULL;
	}
}

void Framebuffer::AddColorBuffer( int format, int index )
{
	if( index < 0 || index >= glConfig.maxColorAttachments )
	{
		common->Warning( "Framebuffer::AddColorBuffer( %s ): bad index = %i", fboName.c_str(), index );
		return;
	}
	
	colorFormat = format;
	
	bool notCreatedYet = colorBuffers[index] == 0;
	if( notCreatedYet )
	{
		glGenRenderbuffers( 1, &colorBuffers[index] );
	}
	
	glBindRenderbuffer( GL_RENDERBUFFER, colorBuffers[index] );
	glRenderbufferStorage( GL_RENDERBUFFER, format, width, height );
	
	if( notCreatedYet )
	{
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_RENDERBUFFER, colorBuffers[index] );
	}
	
	GL_CheckErrors();
}

void Framebuffer::AddDepthBuffer( int format )
{
	depthFormat = format;
	
	bool notCreatedYet = depthBuffer == 0;
	if( notCreatedYet )
	{
		glGenRenderbuffers( 1, &depthBuffer );
	}
	
	glBindRenderbuffer( GL_RENDERBUFFER, depthBuffer );
	glRenderbufferStorage( GL_RENDERBUFFER, format, width, height );
	
	if( notCreatedYet )
	{
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer );
	}
	
	GL_CheckErrors();
}

void Framebuffer::AttachImage2D( int target, const idImage* image, int index )
{
	if( ( target != GL_TEXTURE_2D ) && ( target < GL_TEXTURE_CUBE_MAP_POSITIVE_X || target > GL_TEXTURE_CUBE_MAP_NEGATIVE_Z ) )
	{
		common->Warning( "Framebuffer::AttachImage2D( %s ): invalid target", fboName.c_str() );
		return;
	}
	
	if( index < 0 || index >= glConfig.maxColorAttachments )
	{
		common->Warning( "Framebuffer::AttachImage2D( %s ): bad index = %i", fboName.c_str(), index );
		return;
	}
	
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, target, image->texnum, 0 );
}

void Framebuffer::AttachImageDepth( const idImage* image )
{
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, image->texnum, 0 );
}


void Framebuffer::Check()
{
	int prev;
	glGetIntegerv( GL_FRAMEBUFFER_BINDING, &prev );
	
	glBindFramebuffer( GL_FRAMEBUFFER, frameBuffer );
	
	int status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	if( status == GL_FRAMEBUFFER_COMPLETE )
	{
		glBindFramebuffer( GL_FRAMEBUFFER, prev );
		return;
	}
	
	// something went wrong
	switch( status )
	{
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			common->Error( "Framebuffer::Check( %s ): Framebuffer incomplete, incomplete attachment", fboName.c_str() );
			break;
			
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			common->Error( "Framebuffer::Check( %s ): Framebuffer incomplete, missing attachment", fboName.c_str() );
			break;
			
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			common->Error( "Framebuffer::Check( %s ): Framebuffer incomplete, missing draw buffer", fboName.c_str() );
			break;
			
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			common->Error( "Framebuffer::Check( %s ): Framebuffer incomplete, missing read buffer", fboName.c_str() );
			break;
			
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			common->Error( "Framebuffer::Check( %s ): Framebuffer incomplete, missing layer targets", fboName.c_str() );
			break;
			
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			common->Error( "Framebuffer::Check( %s ): Framebuffer incomplete, missing multisample", fboName.c_str() );
			break;
			
		case GL_FRAMEBUFFER_UNSUPPORTED:
			common->Error( "Framebuffer::Check( %s ): Unsupported framebuffer format", fboName.c_str() );
			break;
			
		default:
			common->Error( "Framebuffer::Check( %s ): Unknown error 0x%X", fboName.c_str(), status );
			break;
	};
	
	glBindFramebuffer( GL_FRAMEBUFFER, prev );
}