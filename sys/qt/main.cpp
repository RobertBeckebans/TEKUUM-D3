/*
===========================================================================

Tekuum GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
Copyright (C) 2011 Robert Beckebans

This file is part of the Tekuum GPL Source Code (Doom 3 Source Code).

Tekuum Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Tekuum Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Tekuum Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Tekuum Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Tekuum Source Code.  If not, please request a copy in writing from id Software at the address below.

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

#if defined(_WIN32)
#include <XInput.h>
//#include "../win32/win_local.h"
void Sys_StartAsyncThread();

#endif

//static char		sys_cmdline[MAX_STRING_CHARS];

int main( int argc, char** argv )
{
	QApplication app( argc, argv );
	
	//GameMainWindow gameMainWindow( argc, ( const char** ) argv );
	GameMainWindow::createInstance( argc, ( const char** ) argv );
	GameMainWindow* gameMainWindow = GameMainWindow::getInstance();
	gameMainWindow->show();
	
	//win32.hInstance = hInstance;
	//idStr::Copynz( sys_cmdline, lpCmdLine, sizeof( sys_cmdline ) );
	
	// done before Com/Sys_Init since we need this for error output
	//Sys_CreateConsole();
	
	// no abort/retry/fail errors
	//SetErrorMode( SEM_FAILCRITICALERRORS );
	
	//Sys_InitCriticalSections();
	
	// get the initial time base
	Sys_Milliseconds();
	
#ifdef DEBUG
	// disable the painfully slow MS heap check every 1024 allocs
	_CrtSetDbgFlag( 0 );
#endif
	
//	Sys_FPU_EnableExceptions( TEST_FPU_EXCEPTIONS );
	//Sys_FPU_SetPrecision( FPU_PRECISION_DOUBLE_EXTENDED );
	
	/*
	if ( argc > 1 ) {
		common->Init( argc-1, (const char**) &argv[1], NULL );
	} else {
		common->Init( 0, NULL, NULL );
	}
	*/
	
	//gameMainWindow.resize(glConfig.vidWidth, glConfig.vidHeight);
	gameMainWindow->resize( 640, 480 );
	
	/*
	#if TEST_FPU_EXCEPTIONS != 0
	common->Printf( Sys_FPU_GetState() );
	#endif
	*/
	
#ifndef	ID_DEDICATED
	//if ( win32.win_notaskkeys.GetInteger() ) {
	//	DisableTaskKeys( TRUE, FALSE, /*( win32.win_notaskkeys.GetInteger() == 2 )*/ FALSE );
	//}
#endif
	
	/*
	#if defined(_WIN32)
	Sys_StartAsyncThread();
	#endif
	*/
	
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

void GLimp_WakeBackEnd( void* a ) {};
void GLimp_EnableLogging( bool ) {};
void GLimp_FrontEndSleep() {};
void GLimp_ActivateContext() {};
void GLimp_DeactivateContext() {};
bool GLimp_SpawnRenderThread( void ( *a )() )
{
	return false;
};

void GLimp_PreInit() {}

bool GLimp_Init( glimpParms_t a )
{
	return true;
};
void GLimp_SetGamma( unsigned short* a, unsigned short* b, unsigned short* c ) {};
void GLimp_Shutdown() {};
void GLimp_SwapBuffers() {};
void* GLimp_BackEndSleep()
{
	return 0;
};

class idSort_VidMode : public idSort_Quick< vidMode_t, idSort_VidMode >
{
public:
	int Compare( const vidMode_t& a, const vidMode_t& b ) const
	{
		int wd = a.width - b.width;
		int hd = a.height - b.height;
		int fd = a.displayHz - b.displayHz;
		return ( hd != 0 ) ? hd : ( wd != 0 ) ? wd : fd;
	}
};

static void FillStaticVidModes( idList<vidMode_t>& modeList )
{
	modeList.AddUnique( vidMode_t( 320,   240, 60 ) );
	modeList.AddUnique( vidMode_t( 400,   300, 60 ) );
	modeList.AddUnique( vidMode_t( 512,   384, 60 ) );
	modeList.AddUnique( vidMode_t( 640,   480, 60 ) );
	modeList.AddUnique( vidMode_t( 800,   600, 60 ) );
	modeList.AddUnique( vidMode_t( 960,   720, 60 ) );
	modeList.AddUnique( vidMode_t( 1024,  768, 60 ) );
	modeList.AddUnique( vidMode_t( 1152,  864, 60 ) );
	modeList.AddUnique( vidMode_t( 1280,  720, 60 ) );
	modeList.AddUnique( vidMode_t( 1280,  768, 60 ) );
	modeList.AddUnique( vidMode_t( 1280,  800, 60 ) );
	modeList.AddUnique( vidMode_t( 1280, 1024, 60 ) );
	modeList.AddUnique( vidMode_t( 1360,  768, 60 ) );
	modeList.AddUnique( vidMode_t( 1440,  900, 60 ) );
	modeList.AddUnique( vidMode_t( 1680, 1050, 60 ) );
	modeList.AddUnique( vidMode_t( 1600, 1200, 60 ) );
	modeList.AddUnique( vidMode_t( 1920, 1080, 60 ) );
	modeList.AddUnique( vidMode_t( 1920, 1200, 60 ) );
	modeList.AddUnique( vidMode_t( 2048, 1536, 60 ) );
	modeList.AddUnique( vidMode_t( 2560, 1600, 60 ) );
	
	modeList.SortWithTemplate( idSort_VidMode() );
}

bool R_GetModeListForDisplay( const int requestedDisplayNum, idList<vidMode_t>& modeList )
{
	assert( requestedDisplayNum >= 0 );
	
	modeList.Clear();
	
	// TODO
	
	FillStaticVidModes( modeList );
	return true;
}

void			Sys_ShowConsole( int visLevel, bool quitOnClose )
{
}

void Sys_InitInput() {}

void Sys_ShutdownInput() {}

// event generation
#define	MAX_QUED_EVENTS		256
#define	MASK_QUED_EVENTS	( MAX_QUED_EVENTS - 1 )

static sysEvent_t	eventQue[MAX_QUED_EVENTS];
static int			eventHead = 0;
static int			eventTail = 0;

void Sys_GenerateEvents() {}

void Sys_ClearEvents()
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
void Sys_QueEvent( sysEventType_t type, int value, int value2, int ptrLength, void* ptr, int inputDeviceNum )
{
	sysEvent_t*	ev;
	
	ev = &eventQue[ eventHead & MASK_QUED_EVENTS ];
	
	if( eventHead - eventTail >= MAX_QUED_EVENTS )
	{
		common->Printf( "Sys_QueEvent: overflow\n" );
		// we are discarding an event, but don't leak memory
		if( ev->evPtr )
		{
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

sysEvent_t Sys_GetEvent()
{
	sysEvent_t	ev;
	
	// return if we have data
	if( eventHead > eventTail )
	{
		eventTail++;
		return eventQue[( eventTail - 1 ) & MASK_QUED_EVENTS ];
	}
	
	// return the empty event
	memset( &ev, 0, sizeof( ev ) );
	
	return ev;
}

unsigned char Sys_GetConsoleKey( bool shifted )
{
	return Qt::Key_AsciiCircum;
}

unsigned char Sys_MapCharForKey( int key )
{
	return ( unsigned char )key;
}

// keyboard input polling
int				Sys_PollKeyboardInputEvents()
{
	return 0;
}
int				Sys_ReturnKeyboardInputEvent( const int n, int& ch, bool& state )
{
	return 0;
}
void			Sys_EndKeyboardInputEvents() {}

// mouse input polling
int				Sys_PollMouseInputEvents( int mouseEvents[MAX_MOUSE_EVENTS][2] )
{
	return 0;
}
int				Sys_ReturnMouseInputEvent( const int n, int& action, int& value )
{
	return 0;
}
void			Sys_EndMouseInputEvents() {}


void Sys_GrabMouseCursor( bool grabIt ) {}

//=====================================================================================

int Sys_PollJoystickInputEvents( int deviceNum )
{
	return 0;
}

int	Sys_ReturnJoystickInputEvent( const int n, int& action, int& value )
{
	return 0;
}

void Sys_EndJoystickInputEvents() {}

//=====================================================================================

int	Sys_PollTouchScreenInputEvents()
{
	return 0;
}

int Sys_ReturnTouchScreenInputEvent( const int n, int& action, int& value, int& value2, int& value3, int& value4 )
{
	return 0;
}
void Sys_EndTouchScreenInputEvents() {}
