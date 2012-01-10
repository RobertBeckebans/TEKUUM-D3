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

//#include <QTimer>
//#include <QHBoxLayout>
#include <QGridLayout>

#include "GameMainWindow.h"

#include "../../idlib/precompiled.h"
#include "../../renderer/tr_local.h"
#pragma hdrstop

GameMainWindow::GameMainWindow(int argc, const char **argv)
	//:QGLWidget(QGLFormat(QGL::SampleBuffers))
{
	glWidget = new GameGLWidget();
	
	QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(glWidget);
	mainLayout->setMargin(0);
    setLayout(mainLayout);

	//initDoom3Engine(argc, argv);
}

void GameMainWindow::initDoom3Engine(int argc, const char **argv)
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

	common->Init( argc, argv, NULL );

#if TEST_FPU_EXCEPTIONS != 0
	common->Printf( Sys_FPU_GetState() );
#endif

#ifndef	ID_DEDICATED
	//if ( win32.win_notaskkeys.GetInteger() ) {
	//	DisableTaskKeys( TRUE, FALSE, /*( win32.win_notaskkeys.GetInteger() == 2 )*/ FALSE );
	//}
#endif

	//Sys_StartAsyncThread();

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

void GameMainWindow::keyPressEvent(QKeyEvent *event)
{

}

void GameMainWindow::mousePressEvent(QMouseEvent *event)	
{

}

void GameMainWindow::mouseMoveEvent(QMouseEvent *event)
{

}

/*
void GameMainWindow::initializeGL()
{

}

void GameMainWindow::paintGL()
{
#ifdef DEBUG
	Sys_MemFrame();
#endif

	// run the game
	common->Frame();
}

void GameMainWindow::resizeGL(int width, int height)
{
	glConfig.vidWidth = width;
	glConfig.vidHeight = height;
}

void GameMainWindow::onTimer()
{
	updateGL();
}
*/