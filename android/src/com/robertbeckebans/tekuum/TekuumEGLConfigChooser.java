/*
 * Copyright (C) 2008-2009 The Android Open Source Project
 * Copyright (C) 2012 Robert Beckebans
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.robertbeckebans.tekuum;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;

import android.opengl.GLSurfaceView.EGLConfigChooser;
import android.util.Log;

class TekuumEGLConfigChooser implements EGLConfigChooser
{
	protected int mRedSize;
	protected int mGreenSize;
	protected int mBlueSize;
	protected int mAlphaSize;
	protected int mDepthSize;
	protected int mStencilSize;
	private int[] mValue = new int[1];
	
	public TekuumEGLConfigChooser( int r, int g, int b, int a, int depth, int stencil )
	{
		mRedSize = r;
		mGreenSize = g;
		mBlueSize = b;
		mAlphaSize = a;
		mDepthSize = depth;
		mStencilSize = stencil;
	}
	
	private static int[] s_configAttribs =
	{
		EGL10.EGL_RED_SIZE, 5,
		EGL10.EGL_GREEN_SIZE, 6,
		EGL10.EGL_BLUE_SIZE, 5,
		EGL10.EGL_ALPHA_SIZE, 5,
		//EGL10.EGL_DEPTH_SIZE, 16,
		//EGL10.EGL_STENCIL_SIZE, 8,
		//EGL10.EGL_NATIVE_RENDERABLE, 1,
		EGL10.EGL_NONE
	};
	
	public EGLConfig chooseConfig( EGL10 egl, EGLDisplay display )
	{
		// Get the number of minimally matching EGL configurations
		int[] num_config = new int[1];
		egl.eglChooseConfig( display, s_configAttribs, null, 0, num_config );
		
		int numConfigs = num_config[0];
		
		if( numConfigs <= 0 )
		{
			throw new IllegalArgumentException( "No configs match configSpec" );
		}
		
		// Allocate then read the array of minimally matching EGL configs
		EGLConfig[] configs = new EGLConfig[numConfigs];
		egl.eglChooseConfig( display, s_configAttribs, configs, numConfigs, num_config );
		
		//if (DEBUG) {
		//printConfigs( egl, display, configs );
		//}
		
		// Now return the "best" one
		EGLConfig best = chooseConfig( egl, display, configs );
		
		
		if( best == null )
		{
			throw new IllegalArgumentException( "Could not find EGL Configuration matching:\nred = " + mRedSize +
												"\ngreen = " + mGreenSize +
												"\nblue = " + mBlueSize +
												"\nalpha = " + mAlphaSize +
												"\ndepth = " + mDepthSize +
												"\nstencil = " + mStencilSize );
		}
		
		
		//if( DEBUG )
		//{
		Log.d( "Tekuum_Java", "Chosen EGL configuration: " );
		printConfig( egl, display, best );
		//}
		
		return best;
	}
	
	public EGLConfig chooseConfig( EGL10 egl, EGLDisplay display, EGLConfig[] configs )
	{
		for( EGLConfig config : configs )
		{
			int d = findConfigAttrib( egl, display, config, EGL10.EGL_DEPTH_SIZE, 0 );
			int s = findConfigAttrib( egl, display, config, EGL10.EGL_STENCIL_SIZE, 0 );
			
			// We need at least mDepthSize and mStencilSize bits
			if( d < mDepthSize || s < mStencilSize )
				continue;
				
			// We want an *exact* match for red/green/blue/alpha
			int r = findConfigAttrib( egl, display, config, EGL10.EGL_RED_SIZE, 0 );
			int g = findConfigAttrib( egl, display, config, EGL10.EGL_GREEN_SIZE, 0 );
			int b = findConfigAttrib( egl, display, config, EGL10.EGL_BLUE_SIZE, 0 );
			int a = findConfigAttrib( egl, display, config, EGL10.EGL_ALPHA_SIZE, 0 );
			
			if( r == mRedSize && g == mGreenSize && b == mBlueSize && a == mAlphaSize )
				return config;
		}
		
		return null;
	}
	
	private int findConfigAttrib( EGL10 egl, EGLDisplay display, EGLConfig config, int attribute, int defaultValue )
	{
		if( egl.eglGetConfigAttrib( display, config, attribute, mValue ) )
		{
			return mValue[0];
		}
		
		return defaultValue;
	}
	
	private void printConfigs( EGL10 egl, EGLDisplay display, EGLConfig[] configs )
	{
		int numConfigs = configs.length;
		
		Log.d( "Tekuum_Java", String.format( "%d configurations", numConfigs ) );
		
		for( int i = 0; i < numConfigs; i++ )
		{
			Log.d( "Tekuum_Java" , String.format( "Configuration %d:\n", i ) );
			printConfig( egl, display, configs[i] );
		}
	}
	
	private void printConfig( EGL10 egl, EGLDisplay display, EGLConfig config )
	{
		int[] attributes =
		{
			EGL10.EGL_BUFFER_SIZE,
			EGL10.EGL_ALPHA_SIZE,
			EGL10.EGL_BLUE_SIZE,
			EGL10.EGL_GREEN_SIZE,
			EGL10.EGL_RED_SIZE,
			EGL10.EGL_DEPTH_SIZE,
			EGL10.EGL_STENCIL_SIZE,
			EGL10.EGL_CONFIG_CAVEAT,
			EGL10.EGL_CONFIG_ID,
			EGL10.EGL_LEVEL,
			EGL10.EGL_MAX_PBUFFER_HEIGHT,
			EGL10.EGL_MAX_PBUFFER_PIXELS,
			EGL10.EGL_MAX_PBUFFER_WIDTH,
			EGL10.EGL_NATIVE_RENDERABLE,
			EGL10.EGL_NATIVE_VISUAL_ID,
			EGL10.EGL_NATIVE_VISUAL_TYPE,
			0x3030, // EGL10.EGL_PRESERVED_RESOURCES,
			EGL10.EGL_SAMPLES,
			EGL10.EGL_SAMPLE_BUFFERS,
			EGL10.EGL_SURFACE_TYPE,
			EGL10.EGL_TRANSPARENT_TYPE,
			EGL10.EGL_TRANSPARENT_RED_VALUE,
			EGL10.EGL_TRANSPARENT_GREEN_VALUE,
			EGL10.EGL_TRANSPARENT_BLUE_VALUE,
			0x3039, // EGL10.EGL_BIND_TO_TEXTURE_RGB,
			0x303A, // EGL10.EGL_BIND_TO_TEXTURE_RGBA,
			0x303B, // EGL10.EGL_MIN_SWAP_INTERVAL,
			0x303C, // EGL10.EGL_MAX_SWAP_INTERVAL,
			EGL10.EGL_LUMINANCE_SIZE,
			EGL10.EGL_ALPHA_MASK_SIZE,
			EGL10.EGL_COLOR_BUFFER_TYPE,
			EGL10.EGL_RENDERABLE_TYPE,
			0x3042 // EGL10.EGL_CONFORMANT
		};
		
		String[] names =
		{
			"EGL_BUFFER_SIZE",
			"EGL_ALPHA_SIZE",
			"EGL_BLUE_SIZE",
			"EGL_GREEN_SIZE",
			"EGL_RED_SIZE",
			"EGL_DEPTH_SIZE",
			"EGL_STENCIL_SIZE",
			"EGL_CONFIG_CAVEAT",
			"EGL_CONFIG_ID",
			"EGL_LEVEL",
			"EGL_MAX_PBUFFER_HEIGHT",
			"EGL_MAX_PBUFFER_PIXELS",
			"EGL_MAX_PBUFFER_WIDTH",
			"EGL_NATIVE_RENDERABLE",
			"EGL_NATIVE_VISUAL_ID",
			"EGL_NATIVE_VISUAL_TYPE",
			"EGL_PRESERVED_RESOURCES",
			"EGL_SAMPLES",
			"EGL_SAMPLE_BUFFERS",
			"EGL_SURFACE_TYPE",
			"EGL_TRANSPARENT_TYPE",
			"EGL_TRANSPARENT_RED_VALUE",
			"EGL_TRANSPARENT_GREEN_VALUE",
			"EGL_TRANSPARENT_BLUE_VALUE",
			"EGL_BIND_TO_TEXTURE_RGB",
			"EGL_BIND_TO_TEXTURE_RGBA",
			"EGL_MIN_SWAP_INTERVAL",
			"EGL_MAX_SWAP_INTERVAL",
			"EGL_LUMINANCE_SIZE",
			"EGL_ALPHA_MASK_SIZE",
			"EGL_COLOR_BUFFER_TYPE",
			"EGL_RENDERABLE_TYPE",
			"EGL_CONFORMANT"
		};
		
		int[] value = new int[1];
		for( int i = 0; i < attributes.length; i++ )
		{
			int attribute = attributes[i];
			String name = names[i];
			if( egl.eglGetConfigAttrib( display, config, attribute, value ) )
			{
				Log.d( "Tekuum_Java", String.format( "  %s: %d\n", name, value[0] ) );
			}
			else
			{
				// Log.d( "Tekuum_Java", String.format("  %s: failed\n", name));
				while( egl.eglGetError() != EGL10.EGL_SUCCESS );
			}
		}
	}
}
