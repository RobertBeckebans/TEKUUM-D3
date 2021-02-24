/*
 * Copyright (C) 2010 Roderick Colenbrander
 * Copyright (C) 2012-2013 Robert Beckebans
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

package com.robertbeckebans.tekuum;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLSurfaceView;
import android.util.Log;

public class TekuumRenderer implements GLSurfaceView.Renderer
{
	private boolean mInit = false;
	
	public TekuumRenderer()
	{
		super();
	}
	
	public void onDrawFrame( GL10 gl )
	{
		//Log.d("Tekuum_Java", "onDrawFrame");
		// Compute a new frame. After this call completes, Android will perform a eglSwapBuffers
		TekuumJNI.drawFrame();
		//Log.d("Tekuum_Java", "onDrawFrame end");
	}
	
	public void onSurfaceChanged( GL10 gl, int width, int height )
	{
		Log.d( "Tekuum_Java", "onSurfaceChanged( width = " + width + ", height = " + height );
		
		/* When the screen is rotated or the keyboard slides in and in other cases this is called even
		 * when the activity is not recreated or so. Prevent that we reinitialize the game engine.
		 */
		if( mInit == false )
		{
			TekuumJNI.initGame( width, height );
			mInit = true;
		}
	}
	
	public void onSurfaceCreated( GL10 gl, EGLConfig config )
	{
		Log.d( "Tekuum_Java", "onSurfaceCreated" );
	}
	
	// RB: was missing in kwaak3 svn
	public void setMenuState( int state )
	{
		Log.d( "Tekuum_Java", "setMenuState(" + state + ")" );
	}
}

