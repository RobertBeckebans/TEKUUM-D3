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
#ifndef __ANDROID_LOCAL_H__
#define __ANDROID_LOCAL_H__

extern glconfig_t glConfig;

#if defined(USE_NATIVE_ACTIVITY)

#include <EGL/egl.h>

#include <jni.h>
#include <android_native_app_glue.h>
#include <android/sensor.h>

struct AndroidGlobals
{
	struct android_app*	app;
//	struct saved_state	state;

	EGLDisplay			eglDisplay;
	EGLContext			eglContext;
	EGLSurface			eglSurface;
	
	ASensorManager*		sensorManager;
	const ASensor*		accelerometerSensor;
	ASensorEventQueue*	sensorEventQueue;
	
	bool				isRunning;
	bool				isTerminating;
};

extern AndroidGlobals android;

#endif

void JE_SetResolution( int width, int height );

void JE_QueueKeyEvent( int key, int state );
void JE_QueueMotionEvent( int action, float x, float y, float pressure );
void JE_QueueTrackballEvent( int action, float dx, float dy );
void JE_QueueJoystickEvent( int axis, float dy );
void JE_QueueConsoleEvent( const char* cmd );

void JE_RequestAudioData();

int JE_IsConsoleActive();
int JE_IsMenuActive();


#endif
