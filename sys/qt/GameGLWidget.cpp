/*
===========================================================================

Techyon GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company. 
Copyright (C) 2011 Robert Beckebans

This file is part of the Techyon GPL Source Code (Doom 3 Source Code).

Techyon Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Techyon Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Techyon Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Techyon Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Techyon Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include <QTimer>

#include "GameGLWidget.h"

#include "../../idlib/precompiled.h"
#include "../../renderer/tr_local.h"
#pragma hdrstop

#if defined(_WIN32)
//#include "../win32/win_local.h"
void Sys_StartAsyncThread( void );

#endif

bool GameGLWidget::initializedDoom3Engine = false;

GameGLWidget::GameGLWidget(int argc, const char **argv):
	QGLWidget(QGLFormat(QGL::SampleBuffers))
{
	this->argc = argc;
	this->argv = argv;
	initializedDoom3Engine = false;

	timer = new QTimer(this);
	// every 10 milliseconds = 100 fps
	timer->setInterval(10);
	timer->setSingleShot(false);
	timer->start();

	connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
}

void GameGLWidget::keyPressEvent(QKeyEvent *event)
{

}

void GameGLWidget::mousePressEvent(QMouseEvent *event)	
{

}

void GameGLWidget::mouseMoveEvent(QMouseEvent *event)
{

}
	
void GameGLWidget::initializeGL()
{
	glConfig.colorBits = format().redBufferSize() + format().greenBufferSize() + format().blueBufferSize();
	glConfig.depthBits = format().depthBufferSize();
	glConfig.stencilBits = format().stencilBufferSize();

	if(!initializedDoom3Engine)
	{
		initializedDoom3Engine = true;

		initDoom3Engine(argc, argv);
	}
}

void GameGLWidget::paintGL()
{
	


#ifdef DEBUG
	Sys_MemFrame();
#endif

	//makeCurrent();

	// run the game
	if (common != NULL && common->IsInitialized()) {
		common->Frame();
	}

	//doneCurrent();
}

void GameGLWidget::resizeGL(int width, int height)
{
	glConfig.vidWidth = width;
	glConfig.vidHeight = height;
}

void GameGLWidget::onTimer()
{
	updateGL();
}

void GameGLWidget::initDoom3Engine(int argc, const char **argv)
{
	//win32.hInstance = hInstance;
	//idStr::Copynz( sys_cmdline, lpCmdLine, sizeof( sys_cmdline ) );

	// done before Com/Sys_Init since we need this for error output
	//Sys_CreateConsole();

	// no abort/retry/fail errors
	//SetErrorMode( SEM_FAILCRITICALERRORS );

	Sys_InitCriticalSections();

	// get the initial time base
	Sys_Milliseconds();

#ifdef DEBUG
	// disable the painfully slow MS heap check every 1024 allocs
	_CrtSetDbgFlag( 0 );
#endif

//	Sys_FPU_EnableExceptions( TEST_FPU_EXCEPTIONS );
	//Sys_FPU_SetPrecision( FPU_PRECISION_DOUBLE_EXTENDED );

	GLenum glewResult = glewInit();
	if(GLEW_OK != glewResult)
	{
		// glewInit failed, something is seriously wrong
		//common->Printf( "^3GLimp_Init() - GLEW could not load OpenGL subsystem: %s", glewGetErrorString(glewResult));
		printf( "^3GLimp_Init() - GLEW could not load OpenGL subsystem: %s", glewGetErrorString(glewResult));
	}
	else
	{
		common->Printf( "Using GLEW %s\n", glewGetString(GLEW_VERSION));
		printf( "Using GLEW %s\n", glewGetString(GLEW_VERSION));
	}

	if ( argc > 1 ) {
		common->Init( argc-1, (const char**) &argv[1], NULL );
	} else {
		common->Init( 0, NULL, NULL );
	}

#if TEST_FPU_EXCEPTIONS != 0
	common->Printf( Sys_FPU_GetState() );
#endif

#ifndef	ID_DEDICATED
	//if ( win32.win_notaskkeys.GetInteger() ) {
	//	DisableTaskKeys( TRUE, FALSE, /*( win32.win_notaskkeys.GetInteger() == 2 )*/ FALSE );
	//}
#endif

#if defined(_WIN32)
	Sys_StartAsyncThread();
#endif

	// hide or show the early console as necessary
	//if ( win32.win_viewlog.GetInteger() || com_skipRenderer.GetBool() || idAsyncNetwork::serverDedicated.GetInteger() ) {
	//	Sys_ShowConsole( 1, true );
	// else {
	//	Sys_ShowConsole( 0, false );
	//}

#ifdef SET_THREAD_AFFINITY 
	// give the main thread an affinity for the first cpu
	SetThreadAffinityMask( GetCurrentThread(), 1 );
#endif
}