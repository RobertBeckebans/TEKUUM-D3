/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012-2013 Robert Beckebans

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

#include "../../idlib/precompiled.h"
#include "../posix/posix_public.h"
#include "../../renderer/tr_local.h"
#include "android_local.h"





// toggled by grab calls - decides if we ignore MotionNotify events
static bool mouse_active = false;

// non-DGA pointer-warping mouse input
static int mwx, mwy;
static int mx = 0, my = 0;
static int mdx = 0, mdy = 0;

// time mouse was last reset, we ignore the first 50ms of the mouse to allow settling of events
//static int mouse_reset_time = 0;
//#define MOUSE_RESET_DELAY 50

// backup original values for pointer grab/ungrab
static int mouse_accel_numerator;
static int mouse_accel_denominator;
static int mouse_threshold;

struct motion_poll_t
{
	int action;
	int value;
	int value2;
	int value3;
	int value4;
	
	motion_poll_t()
	{
	}
	
	motion_poll_t( int a, int v, int v2, int v3, int v4 )
	{
		action = a;
		value = v;
		value2 = v2;
		value3 = v3;
		value4 = v4;
	}
};

struct mouse_poll_t
{
	int action;
	int value;
	
	mouse_poll_t()
	{
	}
	
	mouse_poll_t( int a, int v )
	{
		action = a;
		value = v;
	}
};

static idList<motion_poll_t> motion_polls;
static idList<mouse_poll_t> mouse_polls;




void JE_QueueKeyEvent( int key, int state )
{
	if( !common || !common->IsInitialized() )
		return;
		
	common->Printf( "JE_QueueKeyEvent( key = %i, state = %i )\n", key, state );
	
	Posix_QueEvent( SE_KEY, key, state, 0, NULL );
	//Posix_QueEvent( SE_KEY, s_scantokey[key], state, 0, NULL);
	//Posix_QueEvent( SE_KEY, s_scantokey[key], false, 0, NULL);
	
	if( state == 1 && ( key >= ' ' && key <= 127 ) )
	{
		Posix_QueEvent( SE_CHAR, key, 0, 0, NULL );
	}
}

// see http://developer.android.com/reference/android/view/MotionEvent.html
enum MotionEventAction
{
	MOTION_EVENT_ACTION_DOWN = 0,
	MOTION_EVENT_ACTION_UP = 1,
	MOTION_EVENT_ACTION_MOVE = 2,
	
	MOTION_EVENT_ACTION_POINTER_1_DOWN = 5,
	MOTION_EVENT_ACTION_POINTER_1_UP = 6,
	
	MOTION_EVENT_ACTION_POINTER_2_DOWN = 261,
	MOTION_EVENT_ACTION_POINTER_2_UP = 262,
	
	MOTION_EVENT_ACTION_POINTER_3_DOWN = 517,
	MOTION_EVENT_ACTION_POINTER_3_UP = 518,
	
};

void JE_QueueMotionEvent( int action, float x, float y, float pressure )
{
#if 0
	if( !common || !common->IsInitialized() )
		return;
		
	common->Printf( "JE_QueueMotionEvent( action = %i, x = %f, y = %f, pressure = %f )\n", action, x, y, pressure );
#endif
	
#if 1
	int rx = idMath::ClampInt( 0, 999, idMath::Ftoi( ( x / ( float )glConfig.nativeScreenWidth ) * 1000 ) );
	int ry = idMath::ClampInt( 0, 999, idMath::Ftoi( ( y / ( float )glConfig.nativeScreenHeight ) * 1000 ) );
	
	switch( action )
	{
		case MOTION_EVENT_ACTION_DOWN:
		case MOTION_EVENT_ACTION_POINTER_1_DOWN:
		case MOTION_EVENT_ACTION_POINTER_2_DOWN:
			Posix_QueEvent( SE_TOUCH_MOTION_DOWN, rx, ry, 0, NULL );
			//Posix_QueEvent( SE_KEY, K_MOUSE1, true, 0, NULL );
			
			motion_polls.Append( motion_poll_t( TOUCH_MOTION_DOWN, rx, ry, 0, 0 ) );
			break;
			
		case MOTION_EVENT_ACTION_UP:
		case MOTION_EVENT_ACTION_POINTER_1_UP:
		case MOTION_EVENT_ACTION_POINTER_2_UP:
			Posix_QueEvent( SE_TOUCH_MOTION_UP, rx, ry, 0, NULL );
			
			//if( ( idMath::Fabs( mdx - x ) < 3 ) && ( idMath::Fabs( mdy - y ) < 3 ) )
			{
				//Posix_QueEvent( SE_KEY, K_MOUSE1, false, 0, NULL );
			}
			
			motion_polls.Append( motion_poll_t( TOUCH_MOTION_UP, rx, ry , 0, 0 ) );
			break;
			
		case MOTION_EVENT_ACTION_MOVE:
		{
			int dx = ( ( int ) x - mwx );
			int dy = ( ( int ) y - mwy );
			
			Posix_QueEvent( SE_TOUCH_MOTION_MOVE, rx, ry, 0, NULL );
			
			motion_polls.Append( motion_poll_t( TOUCH_MOTION_DELTA_XY, rx, ry, dx, dy ) );
			break;
		}
	}
#else
	
	int b, dx, dy;
	
	switch( action )
	{
		case MOTION_EVENT_ACTION_DOWN:
			mdx = x;
			mdy = y;
	
			dx = ( ( int ) x - mwx );
			dy = ( ( int ) y - mwy );
	
			Posix_QueEvent( SE_MOUSE, dx, dy, 0, NULL );
	
			mx += dx;
			my += dy;
			break;
	
		case MOTION_EVENT_ACTION_UP:
			//if( ( idMath::Fabs( mdx - x ) < 3 ) && ( idMath::Fabs( mdy - y ) < 3 ) )
		{
			Posix_QueEvent( SE_KEY, K_MOUSE1, true, 0, NULL );
			Posix_QueEvent( SE_KEY, K_MOUSE1, false, 0, NULL );
		}

		mx = x;
		my = y;
		break;

		case MOTION_EVENT_ACTION_MOVE:
			// FIXME: we generate mouse delta on wrap return, but that lags us quite a bit from the initial event..
			/*
			if (x == glConfig.vidWidth / 2 && y == glConfig.vidHeight / 2) {
				mwx = glConfig.vidWidth / 2;
				mwy = glConfig.vidHeight / 2;
	
				Posix_QueEvent( SE_MOUSE, mx, my, 0, NULL);
	
				//Posix_AddMousePollEvent( M_DELTAX, mx );
				//if (!Posix_AddMousePollEvent( M_DELTAY, my ))
				//	return;
				mx = my = 0;
				break;
			}
			*/
	
	
			dx = ( ( int ) x - mwx );
			dy = ( ( int ) y - mwy );
	
			Posix_QueEvent( SE_MOUSE, dx, dy, 0, NULL );
	
			mx += dx;
			my += dy;
			break;
	}
#endif
	
	mwx = x;
	mwy = y;
}

void JE_QueueTrackballEvent( int action, float dx, float dy )
{

}

void JE_QueueConsoleEvent( const char* s )
{
	if( s )
	{
		char* b;
		int len;
		
		len = strlen( s ) + 1;
		b = ( char* )Mem_Alloc( len );
		strcpy( b, s );
		Posix_QueEvent( SE_CONSOLE, 0, 0, len, b );
	}
}

int JE_IsConsoleActive()
{
	if( console && console->Active() )
		return 1;
		
	return 0;
}

int JE_IsMenuActive()
{
	if( session && session->IsMenuActive() )
		return 1;
		
	return 0;
}

void Sys_InitInput()
{
	motion_polls.SetGranularity( 64 );
	mouse_polls.SetGranularity( 64 );
}

void Sys_ShutdownInput()
{
	motion_polls.Clear();
	mouse_polls.Clear();
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
#if 0
	if( mouse_polls.Num() > 0 )
	{
		common->Printf( "Sys_PollMouseInputEvents() = %i )\n", mouse_polls.Num() );
	}
#endif
	
	int numEvents =  mouse_polls.Num();
	
	mouse_polls.SetNum( 0 );
	
	return numEvents;
}


// touch screen input polling
int				Sys_PollTouchScreenInputEvents()
{
#if 0
	if( motion_polls.Num() > 0 )
	{
		common->Printf( "Sys_PollTouchScreenInputEvents() = %i )\n", motion_polls.Num() );
	}
#endif
	
	return motion_polls.Num();
}

int				Sys_ReturnTouchScreenInputEvent( const int n, int& action, int& value, int& value2, int& value3, int& value4 )
{
	if( n >= motion_polls.Num() )
	{
		return 0;
	}
	
	action = motion_polls[ n ].action;
	value = motion_polls[ n ].value;
	value2 = motion_polls[ n ].value2;
	value3 = motion_polls[ n ].value3;
	value4 = motion_polls[ n ].value4;
	
	return 1;
}

void			Sys_EndTouchScreenInputEvents()
{
	motion_polls.SetNum( 0 );
}

//=====================================================================================
//	Joystick Input Handling
//=====================================================================================

/*
class idJoystickWin32 : idJoystick
{
public:
	idJoystickWin32();

	virtual bool	Init();
	virtual void	SetRumble( int deviceNum, int rumbleLow, int rumbleHigh );
	virtual int		PollInputEvents( int inputDeviceNum );
	virtual int		ReturnInputEvent( const int n, int& action, int& value );
	virtual void	EndInputEvents() {}

protected:
	friend void		JoystickSamplingThread( void* data );

	void 			PushButton( int inputDeviceNum, int key, bool value );
	void 			PostInputEvent( int inputDeviceNum, int event, int value, int range = 16384 );

	idSysMutex				mutexXis;		// lock this before using currentXis or stickIntegrations
	HANDLE					timer;			// fire every 4 msec

	int						numEvents;

	struct
	{
		int event;
		int value;
	}						events[ MAX_JOY_EVENT ];

	controllerState_t		controllers[ MAX_JOYSTICKS ];

	// should these be per-controller?
	bool					buttonStates[MAX_INPUT_DEVICES][K_LAST_KEY];	// For keeping track of button up/down events
	int						joyAxis[MAX_INPUT_DEVICES][MAX_JOYSTICK_AXIS];	// For keeping track of joystick axises
};
*/





static float		s_joystickAxis[MAX_JOYSTICK_AXIS];	// set by joystick events

void JE_QueueJoystickEvent( int axis, float value )
{
#if 0
	if( !common || !common->IsInitialized() )
		return;
		
	common->Printf( "JE_QueueJoystickEvent( axis = %i, value = %f )\n", axis, value );
#endif
	
	if( axis < 0 || axis >= MAX_JOYSTICK_AXIS )
	{
		return;
	}
	
	s_joystickAxis[axis] = value;
}

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

/*
static void IN_TouchScreenJoystickAxis( int event, float value, float range )
{
	int axis = event - J_AXIS_MIN;
	int percent = ( value * 16 ) / range;
	if( joyAxis[inputDeviceNum][axis] != percent )
	{
		joyAxis[inputDeviceNum][axis] = percent;

		IN_AddGamepadPollEvent( J_AXIS_LEFT_X, s_joystickAxis[AXIS_SIDE] * f * scale, 0 );
		//Sys_QueEvent( SE_JOYSTICK, axis, percent, 0, NULL, inputDeviceNum );
	}
}
*/

void Sys_SetRumble( int device, int low, int hi )
{
	// TODO
	
	//win32.g_Joystick.SetRumble( device, low, hi );
}

int Sys_PollJoystickInputEvents( int deviceNum )
{
	//return win32.g_Joystick.PollInputEvents( deviceNum );
	
	s_pollGamepadEventsCount = 0;
	
	// use left analog stick for strafing
#if 1
	IN_AddGamepadPollEvent( J_AXIS_LEFT_X, ( s_joystickAxis[AXIS_LEFT_X] * 32767 ), 0 );
	IN_AddGamepadPollEvent( J_AXIS_LEFT_Y, ( s_joystickAxis[AXIS_LEFT_Y] * -32767 ), 0 );
#else
	IN_AddGamepadPollEvent( J_AXIS_LEFT_X, ( s_joystickAxis[AXIS_LEFT_X] * 0.5 + 0.5 ) * 32767, 0 );
	IN_AddGamepadPollEvent( J_AXIS_LEFT_Y, ( s_joystickAxis[AXIS_LEFT_Y] * 0.5 + 0.5 ) * 32767, 0 );
#endif
	
	return s_pollGamepadEventsCount;
}


int Sys_ReturnJoystickInputEvent( const int n, int& action, int& value )
{
	// return win32.g_Joystick.ReturnInputEvent( n, action, value );
	
	if( n >= s_pollGamepadEventsCount )
	{
		return 0;
	}
	
	action = s_pollGamepadEvents[ n ].action;
	value = s_pollGamepadEvents[ n ].value;
	
	return 1;
}


void Sys_EndJoystickInputEvents()
{
	s_pollGamepadEventsCount = 0;
}

void Sys_GrabMouseCursor( bool grabIt ) {}
