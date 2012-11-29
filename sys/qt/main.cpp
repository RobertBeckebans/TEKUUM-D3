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

#if defined(_WIN32)
#include <XInput.h>
//#include "../win32/win_local.h"
void Sys_StartAsyncThread( void );

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
void Sys_QueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void* ptr )
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

sysEvent_t Sys_GetEvent( void )
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

void Sys_InitScanTable( void ) {}

unsigned char Sys_MapCharForKey( int key )
{
	return ( unsigned char )key;
}

// keyboard input polling
int				Sys_PollKeyboardInputEvents( void )
{
	return 0;
}
int				Sys_ReturnKeyboardInputEvent( const int n, int& ch, bool& state )
{
	return 0;
}
void			Sys_EndKeyboardInputEvents( void ) {}

// mouse input polling
int				Sys_PollMouseInputEvents( void )
{
	return 0;
}
int				Sys_ReturnMouseInputEvent( const int n, int& action, int& value )
{
	return 0;
}
void			Sys_EndMouseInputEvents( void ) {}


void Sys_GrabMouseCursor( bool grabIt ) {}

//=====================================================================================

#if defined(USE_XINPUT)

static idCVar in_xbox360Controller( "in_xbox360Controller", "1", CVAR_SYSTEM | CVAR_BOOL, "enable Xbox 360 controller" );
static idCVar in_xbox360ControllerDebug( "in_xbox360ControllerDebug", "0", CVAR_SYSTEM | CVAR_BOOL, "enable Xbox 360 controller debugging" );
static idCVar in_xbox360ControllerThreshold( "in_xbox360ControllerThreshold", "0.15", CVAR_SYSTEM | CVAR_FLOAT, "enable Xbox 360 controller debugging" );

static bool					g_ControllerAvailable;
static XINPUT_STATE			g_Controller;

// RB begin
typedef struct
{
	int action;
	int value;
	int value2;
} pollGamepadEvent_t;

#define MAX_POLL_EVENTS 50
#define POLL_EVENTS_HEADROOM 2
static pollGamepadEvent_t s_pollGamepadEvents[MAX_POLL_EVENTS + POLL_EVENTS_HEADROOM];
static int s_pollGamepadEventsCount;

static bool IN_AddGamepadPollEvent( int action, int value, int value2 )
{
	if( s_pollGamepadEventsCount >= MAX_POLL_EVENTS + POLL_EVENTS_HEADROOM )
		common->FatalError( "pollGamepadEventsCount exceeded MAX_POLL_EVENT + POLL_EVENTS_HEADROOM\n" );
		
	s_pollGamepadEvents[s_pollGamepadEventsCount].action = action;
	s_pollGamepadEvents[s_pollGamepadEventsCount].value = value;
	s_pollGamepadEvents[s_pollGamepadEventsCount].value2 = value2;
	s_pollGamepadEventsCount++;
	
	if( s_pollGamepadEventsCount >= MAX_POLL_EVENTS )
	{
		common->DPrintf( "WARNING: reached MAX_POLL_EVENT pollGamepadEventsCount\n" );
		return false;
	}
	
	return true;
}

static void IN_XBox360Axis( int action, short thumbAxis, float scale )
{
	float           f = ( ( float )thumbAxis ) / 32767.0f;
	
	float threshold = in_xbox360ControllerThreshold.GetFloat();
	if( f > -threshold && f < threshold )
	{
		IN_AddGamepadPollEvent( action, 0, 0 );
	}
	else
	{
		if( in_xbox360ControllerDebug.GetBool() )
		{
			common->Printf( "xbox axis %i = %f\n", action, f );
		}
		
		IN_AddGamepadPollEvent( action, f * scale, 0 );
	}
}

static void IN_XBox360TriggerToButton( byte triggerAxis, byte oldTriggerAxis, int key, float expectedStartValue, float threshold )
{
	float           f = ( ( float )triggerAxis ) / 255.0f;
	float           fOld = ( ( float )triggerAxis ) / 255.0f;
	
	if( f > ( expectedStartValue + threshold + in_xbox360ControllerThreshold.GetFloat() ) )
	{
		IN_AddGamepadPollEvent( GP_BUTTON, key, 1 );
		Sys_QueEvent( GetTickCount(), SE_KEY, key, 1, 0, NULL );
		
		if( in_xbox360ControllerDebug.GetBool() )
		{
			common->Printf( "xbox trigger to key press = Q:0x%02x(%s), value = %f\n", key, idKeyInput::KeyNumToString( key, false ), f );
		}
	}
	else
	{
		IN_AddGamepadPollEvent( GP_BUTTON, key, 0 );
		Sys_QueEvent( GetTickCount(), SE_KEY, key, 0, 0, NULL );
		
		if( in_xbox360ControllerDebug.GetBool() )
		{
			common->Printf( "xbox trigger to key release = Q:0x%02x(%s), value = %f\n", key, idKeyInput::KeyNumToString( key, false ), f );
		}
	}
}

int Sys_PollXbox360ControllerInputEvents( void )
{
	if( !in_xbox360Controller.GetBool() )
		return 0;
		
	s_pollGamepadEventsCount = 0;
	
	XINPUT_STATE state;
	DWORD dwResult = XInputGetState( 0, &state );
	
	if( dwResult == ERROR_SUCCESS )
	{
		g_ControllerAvailable = true;
		
		// always send the axis
		
		// use left analog stick for strafing
		IN_XBox360Axis( GP_AXIS_SIDE, state.Gamepad.sThumbLX, 127 );
		IN_XBox360Axis( GP_AXIS_FORWARD, state.Gamepad.sThumbLY, 127 );
		
		// use right analog stick for viewing
		IN_XBox360Axis( GP_AXIS_YAW, state.Gamepad.sThumbRX, -127 );
		IN_XBox360Axis( GP_AXIS_PITCH, state.Gamepad.sThumbRY, -127 );
		
		if( state.dwPacketNumber == g_Controller.dwPacketNumber )
		{
			// no changes since last frame so skip the buttons
			return s_pollGamepadEventsCount;
		}
		
		if( state.Gamepad.bLeftTrigger != g_Controller.Gamepad.bLeftTrigger )
		{
			IN_XBox360TriggerToButton( state.Gamepad.bLeftTrigger, g_Controller.Gamepad.bLeftTrigger, K_XINPUT_GAMEPAD_LT, 0, 0 );
		}
		
		if( state.Gamepad.bRightTrigger != g_Controller.Gamepad.bRightTrigger )
		{
			IN_XBox360TriggerToButton( state.Gamepad.bRightTrigger, g_Controller.Gamepad.bRightTrigger, K_XINPUT_GAMEPAD_RT, 0, 0 );
		}
		
		WORD diff = state.Gamepad.wButtons ^ g_Controller.Gamepad.wButtons;
		if( diff )
		{
			if( diff & XINPUT_GAMEPAD_DPAD_UP )
			{
				if( state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP )
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_DPAD_UP, 1 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_DPAD_UP, 1, 0, NULL );
				}
				else
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_DPAD_UP, 0 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_DPAD_UP, 0, 0, NULL );
				}
			}
			
			if( diff & XINPUT_GAMEPAD_DPAD_DOWN )
			{
				if( state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN )
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_DPAD_DOWN, 1 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_DPAD_DOWN, 1, 0, NULL );
				}
				else
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_DPAD_DOWN, 0 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_DPAD_DOWN, 0, 0, NULL );
				}
			}
			
			if( diff & XINPUT_GAMEPAD_DPAD_LEFT )
			{
				if( state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT )
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_DPAD_LEFT, 1 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_DPAD_LEFT, 1, 0, NULL );
				}
				else
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_DPAD_LEFT, 0 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_DPAD_LEFT, 0, 0, NULL );
				}
			}
			
			if( diff & XINPUT_GAMEPAD_DPAD_RIGHT )
			{
				if( state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT )
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_DPAD_RIGHT, 1 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_DPAD_RIGHT, 1, 0, NULL );
				}
				else
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_DPAD_RIGHT, 0 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_DPAD_RIGHT, 0, 0, NULL );
				}
			}
			
			if( diff & XINPUT_GAMEPAD_START )
			{
				if( state.Gamepad.wButtons & XINPUT_GAMEPAD_START )
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_START, 1 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_START, 1, 0, NULL );
				}
				else
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_START, 0 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_START, 0, 0, NULL );
				}
			}
			
			if( diff & XINPUT_GAMEPAD_BACK )
			{
				if( state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK )
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_BACK, 1 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_BACK, 1, 0, NULL );
				}
				else
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_BACK, 0 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_BACK, 0, 0, NULL );
				}
			}
			
			if( diff & XINPUT_GAMEPAD_LEFT_THUMB )
			{
				if( state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB )
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_LS, 1 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_LS, 1, 0, NULL );
				}
				else
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_LS, 0 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_LS, 0, 0, NULL );
				}
			}
			
			if( diff & XINPUT_GAMEPAD_RIGHT_THUMB )
			{
				if( state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB )
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_RS, 1 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_RS, 1, 0, NULL );
				}
				else
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_RS, 0 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_RS, 0, 0, NULL );
				}
			}
			
			if( diff & XINPUT_GAMEPAD_LEFT_SHOULDER )
			{
				if( state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER )
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_LB, 1 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_LB, 1, 0, NULL );
				}
				else
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_LB, 0 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_LB, 0, 0, NULL );
				}
			}
			
			if( diff & XINPUT_GAMEPAD_RIGHT_SHOULDER )
			{
				if( state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER )
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_RB, 1 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_RB, 1, 0, NULL );
				}
				else
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_RB, 0 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_RB, 0, 0, NULL );
				}
			}
			
			if( diff & XINPUT_GAMEPAD_A )
			{
				if( state.Gamepad.wButtons & XINPUT_GAMEPAD_A )
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_A, 1 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_A, 1, 0, NULL );
				}
				else
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_A, 0 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_A, 0, 0, NULL );
				}
			}
			
			if( diff & XINPUT_GAMEPAD_B )
			{
				if( state.Gamepad.wButtons & XINPUT_GAMEPAD_B )
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_B, 1 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_B, 1, 0, NULL );
				}
				else
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_B, 0 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_B, 0, 0, NULL );
				}
			}
			
			if( diff & XINPUT_GAMEPAD_X )
			{
				if( state.Gamepad.wButtons & XINPUT_GAMEPAD_X )
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_X, 1 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_X, 1, 0, NULL );
				}
				else
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_X, 0 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_X, 0, 0, NULL );
				}
			}
			
			if( diff & XINPUT_GAMEPAD_Y )
			{
				if( state.Gamepad.wButtons & XINPUT_GAMEPAD_Y )
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_Y, 1 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_Y, 1, 0, NULL );
				}
				else
				{
					IN_AddGamepadPollEvent( GP_BUTTON, K_XINPUT_GAMEPAD_Y, 0 );
					Sys_QueEvent( GetTickCount(), SE_KEY, K_XINPUT_GAMEPAD_Y, 0, 0, NULL );
				}
			}
		}
		
		g_Controller = state;
		return s_pollGamepadEventsCount;
	}
	
	return 0;
}

int	Sys_ReturnXbox360ControllerInputEvent( const int n, int& action, int& value, int& value2 )
{
	if( n >= s_pollGamepadEventsCount )
	{
		return 0;
	}
	
	action = s_pollGamepadEvents[ n ].action;
	value = s_pollGamepadEvents[ n ].value;
	value2 = s_pollGamepadEvents[ n ].value2;
	
	return 1;
}

void Sys_EndXbox360ControllerInputEvents( void ) { }

#else

int Sys_PollXbox360ControllerInputEvents( void )
{
	return 0;
}
int	Sys_ReturnXbox360ControllerInputEvent( const int n, int& action, int& value, int& value2 )
{
	return 0;
}
void Sys_EndXbox360ControllerInputEvents( void ) { }

#endif // #if defined(_WIN32)