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
#include <QApplication>

#include "GameMainWindow.h"

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../../framework/Common.h"
#include "../../renderer/tr_local.h"
#include "../sys_public.h"

//static char		sys_cmdline[MAX_STRING_CHARS];

int main(int argc, char **argv)
{
	QApplication app(argc, argv);

	GameMainWindow gameMainWindow(argc, (const char **) argv);
	gameMainWindow.show();

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

	gameMainWindow.resize(glConfig.vidWidth, glConfig.vidHeight);

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

	//::SetCursor( hcurSave );
	//::SetFocus( win32.hWnd );
	
	return app.exec();
}

bool		GLimp_SetScreenParms( glimpParms_t parms )
{
	// TODO
	return true;
}

void GLimp_WakeBackEnd(void*a) {};
void GLimp_EnableLogging(bool) {};
void GLimp_FrontEndSleep() {};
void GLimp_ActivateContext() {};
void GLimp_DeactivateContext() {};
bool GLimp_SpawnRenderThread(void (*a)()) {return false;};

bool GLimp_Init(glimpParms_t a) {return true;};
void GLimp_SetGamma(unsigned short*a, unsigned short*b, unsigned short*c) {};
void GLimp_Shutdown() {};
void GLimp_SwapBuffers() {};
void *GLimp_BackEndSleep() {return 0;};

void			Sys_ShowConsole( int visLevel, bool quitOnClose )
{
}

void Sys_InitInput( void ) {}

void Sys_ShutdownInput( void ) {}

// event generation
#define	MAX_QUED_EVENTS		256
#define	MASK_QUED_EVENTS	( MAX_QUED_EVENTS - 1 )

static sysEvent_t	eventQue[MAX_QUED_EVENTS];
static int			eventHead = 0;
static int			eventTail = 0;

void Sys_GenerateEvents( void ) {}

void Sys_ClearEvents( void )
{
	eventHead = eventTail = 0;
}

/*
================
Sys_QueEvent

Ptr should either be null, or point to a block of data that can
be freed by the game later.
================
*/
void Sys_QueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr ) {
	sysEvent_t	*ev;

	ev = &eventQue[ eventHead & MASK_QUED_EVENTS ];

	if ( eventHead - eventTail >= MAX_QUED_EVENTS ) {
		common->Printf("Sys_QueEvent: overflow\n");
		// we are discarding an event, but don't leak memory
		if ( ev->evPtr ) {
			Mem_Free( ev->evPtr );
		}
		eventTail++;
	}

	eventHead++;

	ev->evType = type;
	ev->evValue = value;
	ev->evValue2 = value2;
	ev->evPtrLength = ptrLength;
	ev->evPtr = ptr;
}

sysEvent_t Sys_GetEvent( void ) {
	sysEvent_t	ev;

	// return if we have data
	if ( eventHead > eventTail ) {
		eventTail++;
		return eventQue[ ( eventTail - 1 ) & MASK_QUED_EVENTS ];
	}

	// return the empty event 
	memset( &ev, 0, sizeof( ev ) );

	return ev;
}

unsigned char Sys_GetConsoleKey( bool shifted ) {
	return Qt::Key_AsciiCircum;
}

void Sys_InitScanTable( void ) {}

unsigned char Sys_MapCharForKey( int key ) {
	return (unsigned char)key;
}

// keyboard input polling
int				Sys_PollKeyboardInputEvents( void ) { return 0; }
int				Sys_ReturnKeyboardInputEvent( const int n, int &ch, bool &state ) { return 0; }
void			Sys_EndKeyboardInputEvents( void ) {}

// mouse input polling
int				Sys_PollMouseInputEvents( void ) { return 0; }
int				Sys_ReturnMouseInputEvent( const int n, int &action, int &value ) { return 0; }
void			Sys_EndMouseInputEvents( void ) {}


void Sys_GrabMouseCursor( bool grabIt ) {}