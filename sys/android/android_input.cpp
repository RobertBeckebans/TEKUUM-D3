/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012 Robert Beckebans

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


static const unsigned char s_scantokey[256] = {
//  0            1       2          3          4       5            6         7
//  8            9       A          B          C       D            E         F
	0,           27,    '1',       '2',        '3',    '4',         '5',      '6',
	'7',        '8',    '9',       '0',        '-',    '=',          K_BACKSPACE, 9, // 0
	'q',        'w',    'e',       'r',        't',    'y',         'u',      'i',
	'o',        'p',    '[',       ']',        K_ENTER,K_CTRL,      'a',      's',   // 1
	'd',        'f',    'g',       'h',        'j',    'k',         'l',      ';',
	'\'',       '`',    K_SHIFT,   '\\',       'z',    'x',         'c',      'v',   // 2
	'b',        'n',    'm',       ',',        '.',    '/',         K_SHIFT,  K_KP_STAR,
	K_ALT,      ' ',    K_CAPSLOCK,K_F1,       K_F2,   K_F3,        K_F4,     K_F5,  // 3
	K_F6,       K_F7,   K_F8,      K_F9,       K_F10,  K_PAUSE,     K_SCROLL, K_HOME,
	K_UPARROW,  K_PGUP, K_KP_MINUS,K_LEFTARROW,K_KP_5, K_RIGHTARROW,K_KP_PLUS,K_END, // 4
	K_DOWNARROW,K_PGDN, K_INS,     K_DEL,      0,      0,           0,        K_F11,
	K_F12,      0,      0,         K_LWIN,     K_RWIN, K_MENU,      0,        0,     // 5
	0,          0,      0,         0,          0,      0,           0,        0,
	0,          0,      0,         0,          0,      0,           0,        0,     // 6
	0,          0,      0,         0,          0,      0,           0,        0,
	0,          0,      0,         0,          0,      0,           0,        0,      // 7
// shifted
	0,           27,    '!',       '@',        '#',    '$',         '%',      '^',
	'&',        '*',    '(',       ')',        '_',    '+',          K_BACKSPACE, 9, // 0
	'q',        'w',    'e',       'r',        't',    'y',         'u',      'i',
	'o',        'p',    '[',       ']',        K_ENTER,K_CTRL,      'a',      's',   // 1
	'd',        'f',    'g',       'h',        'j',    'k',         'l',      ';',
	'\'',       '~',    K_SHIFT,   '\\',       'z',    'x',         'c',      'v',   // 2
	'b',        'n',    'm',       ',',        '.',    '/',         K_SHIFT,  K_KP_STAR,
	K_ALT,      ' ',    K_CAPSLOCK,K_F1,       K_F2,   K_F3,        K_F4,     K_F5,  // 3
	K_F6,       K_F7,   K_F8,      K_F9,       K_F10,  K_PAUSE,     K_SCROLL, K_HOME,
	K_UPARROW,  K_PGUP, K_KP_MINUS,K_LEFTARROW,K_KP_5, K_RIGHTARROW,K_KP_PLUS,K_END, // 4
	K_DOWNARROW,K_PGDN, K_INS,     K_DEL,      0,      0,           0,        K_F11,
	K_F12,      0,      0,         K_LWIN,     K_RWIN, K_MENU,      0,        0,     // 5
	0,          0,      0,         0,          0,      0,           0,        0,
	0,          0,      0,         0,          0,      0,           0,        0,     // 6
	0,          0,      0,         0,          0,      0,           0,        0,
	0,          0,      0,         0,          0,      0,           0,        0      // 7
};


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



void JE_QueueKeyEvent(int key, int state)
{
	if (!common || !common->IsInitialized())
		return;

	common->Printf( "JE_QueueKeyEvent( key = %i, state = %i )\n", key, state );

	Posix_QueEvent( SE_KEY, key, state, 0, NULL);
	//Posix_QueEvent( SE_KEY, s_scantokey[key], state, 0, NULL);
	//Posix_QueEvent( SE_KEY, s_scantokey[key], false, 0, NULL);

	if (state == 1 && (key >= ' ' && key <= 127))
	{
		Posix_QueEvent( SE_CHAR, key, 0, 0, NULL);
	}
}

// see http://developer.android.com/reference/android/view/MotionEvent.html
enum MotionEventAction
{
	MOTION_EVENT_ACTION_DOWN = 0,
	MOTION_EVENT_ACTION_UP = 1,
	MOTION_EVENT_ACTION_MOVE = 2,
};

void JE_QueueMotionEvent(int action, float x, float y, float pressure)
{
	if (!common || !common->IsInitialized())
		return;

	common->Printf( "JE_QueueKeyEvent( action = %i, x = %f, y = %f, pressure = %f )\n", action, x, y, pressure );

	int b, dx, dy;

	switch (action)
	{
		case MOTION_EVENT_ACTION_DOWN:
			mdx = x;
			mdy = y;

			dx = ((int) x - mwx);
			dy = ((int) y - mwy);

			Posix_QueEvent(SE_MOUSE, dx, dy, 0, NULL);

			mx += dx;
			my += dy;
			break;

		case MOTION_EVENT_ACTION_UP:
			if ((idMath::Fabs(mdx - x) < 3) && (idMath::Fabs(mdy - y) < 3))
			{
				Posix_QueEvent(SE_KEY, K_MOUSE1, true, 0, NULL);
				Posix_QueEvent(SE_KEY, K_MOUSE1, false, 0, NULL);
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


			dx = ((int) x - mwx);
			dy = ((int) y - mwy);

			Posix_QueEvent( SE_MOUSE, dx, dy, 0, NULL);

			mx += dx;
			my += dy;
			break;
	}

	mwx = x;
	mwy = y;
}

void JE_QueueTrackballEvent(int action, float dx, float dy)
{

}

void Sys_InitInput( void ) {}

void Sys_ShutdownInput( void ) {}

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
