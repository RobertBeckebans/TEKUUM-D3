/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

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


static void            (*s_setMenuState) (int shown);

extern "C"
{

void JNI_SetInputCallbacks(void (*set_menu_state) (int shown))
{
	s_setMenuState = set_menu_state;
}

void JNI_QueueKeyEvent(int key, int state)
{

}

void JNI_QueueMotionEvent(int action, float x, float y, float pressure)
{

}

void JNI_QueueTrackballEvent(int action, float dx, float dy)
{

}

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
