/*
 * Copyright (C) 2010 Roderick Colenbrander
 * Copyright (C) 2012 Robert Beckebans
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

import java.io.IOException;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;

import android.annotation.TargetApi;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.PixelFormat;
import android.graphics.RectF;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;

@TargetApi( 8 )
class TekuumView extends GLSurfaceView
{
	private TekuumRenderer mTekuumRenderer;
	private TekuumEGLConfigChooser mEGLConfigChooser;
	//private Joystick[] mJoysticks;
	private Joystick	mMoveStick;
	private Joystick	mSecondMotion;
	
	private enum DoomJoystickAxis
	{
		AXIS_LEFT_X,
		AXIS_LEFT_Y,
		AXIS_RIGHT_X,
		AXIS_RIGHT_Y,
		AXIS_LEFT_TRIG,
		AXIS_RIGHT_TRIG,
	};
	
	public TekuumView( Context context )
	{
		super( context );
		
		/* We need the path to the library directory for dlopen in our JNI library */
		String cache_dir, lib_dir;
		try
		{
			cache_dir = context.getCacheDir().getCanonicalPath();
			lib_dir = cache_dir.replace( "cache", "lib" );
		}
		catch( IOException e )
		{
			e.printStackTrace();
			lib_dir = "/data/data/com.robertbeckebans.tekuum/lib";
		}
		TekuumJNI.setLibraryDirectory( lib_dir );
		
		
		//getHolder().setFormat( PixelFormat.TRANSLUCENT );
		
		// enable OpenGL debugging
		//setDebugFlags(GLSurfaceView.DEBUG_CHECK_GL_ERROR | GLSurfaceView.DEBUG_LOG_GL_CALLS);
		
		mEGLConfigChooser = new TekuumEGLConfigChooser( 8, 8, 8, 8, 16, 8 );
		setEGLConfigChooser( mEGLConfigChooser );
		
		
		// Pick an EGLConfig with RGB8 color, 16-bit depth, no stencil,
		// supporting OpenGL ES 2.0 or later backwards-compatible versions.
		//setEGLConfigChooser(8, 8, 8, 8, 16, 8);
		setEGLContextClientVersion( 3 );
		
		mTekuumRenderer = new TekuumRenderer();
		setRenderer( mTekuumRenderer );
		TekuumJNI.setRenderer( mTekuumRenderer );
		
		setRenderMode( GLSurfaceView.RENDERMODE_CONTINUOUSLY );
		
		setFocusable( true );
		setFocusableInTouchMode( true );
		
		
		//mMoveStick = new Joystick[2];
		mMoveStick = new Joystick( this );
		mMoveStick.setBoundary( new RectF( 0, 768 - 400, 400, 768 ) );
		
		mSecondMotion = new Joystick( this );
		mSecondMotion.setBoundary( new RectF( 0, 0, 1200, 768 ) );
		
		setOnTouchListener( mViewListener );
	}
	
	private OnTouchListener mViewListener = new OnTouchListener()
	{
		public boolean onTouch( View v, MotionEvent event )
		{
			switch( event.getActionMasked() )
			{
				case MotionEvent.ACTION_DOWN:
				case MotionEvent.ACTION_POINTER_DOWN:
					//for( Joystick stick : mJoysticks )
				{
					if( !TekuumJNI.isMenuActive() && mMoveStick.onDown( event ) )
					{
						return true;
					}
					
					if( mSecondMotion.onDown( event ) )
					{
						if( TekuumJNI.isConsoleActive() )
						{
							Tekuum game = ( Tekuum ) getContext();
							game.showDialog( Tekuum.DIALOG_CONSOLE_COMMAND );
							
						}
						else
						{
							queueMotionEvent( event.getAction(), mSecondMotion.getEventX(), mSecondMotion.getEventY(), 0 );
						}
						
						return true;
					}
				}
				break;
				
				case MotionEvent.ACTION_MOVE:
					boolean consumed = false;
					//for( Joystick stick : mJoysticks )
					{
						if( !TekuumJNI.isMenuActive() && mMoveStick.onMove( event ) )
						{
							queueJoystickEvent( DoomJoystickAxis.AXIS_LEFT_X.ordinal(), mMoveStick.getXAxis() );
							queueJoystickEvent( DoomJoystickAxis.AXIS_LEFT_Y.ordinal(), mMoveStick.getYAxis() );
							
							consumed = true;
						}
						
						if( mSecondMotion.onMove( event ) )// stick == mJoysticks[1] )
						{
							if( TekuumJNI.isConsoleActive() )
							{
								Tekuum game = ( Tekuum ) getContext();
								game.showDialog( Tekuum.DIALOG_CONSOLE_COMMAND );
								
							}
							else
							{
								queueMotionEvent( event.getAction(), mSecondMotion.getEventX(), mSecondMotion.getEventY(), 0 );
							}
							
							consumed = true;
						}
						
					}
					if( consumed )
					{
						return true;
					}
					break;
					
				case MotionEvent.ACTION_UP:
				case MotionEvent.ACTION_POINTER_UP:
				{
					consumed = false;
					
					if( !TekuumJNI.isMenuActive() && mMoveStick.onUp( event ) )
					{
						queueJoystickEvent( DoomJoystickAxis.AXIS_LEFT_X.ordinal(), 0 );
						queueJoystickEvent( DoomJoystickAxis.AXIS_LEFT_Y.ordinal(), 0 );
						
						consumed = true;
					}
					
					if( mSecondMotion.onUp( event ) )// stick == mJoysticks[1] )
					{
						if( TekuumJNI.isConsoleActive() )
						{
							Tekuum game = ( Tekuum ) getContext();
							game.showDialog( Tekuum.DIALOG_CONSOLE_COMMAND );
						}
						else
						{
							queueMotionEvent( event.getAction(), mSecondMotion.getEventX(), mSecondMotion.getEventY(), 0 );
						}
						
						consumed = true;
					}
					
					if( consumed )
					{
						return true;
					}
				}
				break;
				
				default:
					Log.d( "Tekuum_Java", "onTouch action=" + event.getAction() + " actionIndex=" + event.getActionIndex() + " x=" + event.getX() + " y=" + event.getY() + " pressure=" + event.getPressure() + "size = " + event.getSize() );
					break;
			}
			
			//Log.d("Tekuum_Java", "onTouch action=" + event.getAction() + " actionIndex=" + event.getActionIndex() + " x=" + event.getX() + " y=" + event.getY() + " pressure=" + event.getPressure() + "size = " + event.getSize());
			
			return false;
		}
	};
	
	@Override
	public boolean onKeyDown( int keyCode, KeyEvent event )
	{
		int qKeyCode = androidKeyCodeToQuake( keyCode, event );
		Log.d( "Tekuum_Java", "onKeyDown=" + keyCode + " " + qKeyCode + " " + event.getDisplayLabel() + " " + event.getUnicodeChar() + " " + event.getNumber() );
		return queueKeyEvent( qKeyCode, 1 );
	}
	
	@Override
	public boolean onKeyUp( int keyCode, KeyEvent event )
	{
		int qKeyCode = androidKeyCodeToQuake( keyCode, event );
		//Log.d("Tekuum_Java", "onKeyUp=" + keyCode + " " + qKeyCode + " shift=" + event.isShiftPressed() + " =" + event.getMetaState());
		return queueKeyEvent( qKeyCode, 0 );
	}
	
	@Override
	public boolean onTouchEvent( MotionEvent event )
	{
		Log.d( "Tekuum_Java", "onTouchEvent action=" + event.getAction() + " actionIndex=" + event.getActionIndex() + " x=" + event.getX() + " y=" + event.getY() + " pressure=" + event.getPressure() + "size = " + event.getSize() );
		/* Perhaps we should pass integers down to reduce the number of float computations */
		
		if( TekuumJNI.isConsoleActive() )
		{
			Tekuum game = ( Tekuum ) getContext();
			game.showDialog( Tekuum.DIALOG_CONSOLE_COMMAND );
			return true;
		}
		
		return queueMotionEvent( event.getAction(), event.getX(), event.getY(), event.getPressure() );
	}
	
	@Override
	public boolean onTrackballEvent( MotionEvent event )
	{
		//Log.d("Tekuum_Java", "onTrackballEvent action=" + event.getAction() + " x=" + event.getX() + " y=" + event.getY());
		return queueTrackballEvent( event.getAction(), event.getX(), event.getY() );
	}
	
	public boolean queueKeyEvent( final int qKeyCode, final int state )
	{
		if( qKeyCode == 0 ) return false;
		
		/* Make sure all communication with Quake is done from the Renderer thread */
		queueEvent( new Runnable()
		{
			public void run()
			{
				TekuumJNI.queueKeyEvent( qKeyCode, state );
			}
		} );
		return true;
	}
	
	private boolean queueMotionEvent( final int action, final float x, final float y, final float pressure )
	{
		/* Make sure all communication with Quake is done from the Renderer thread */
		queueEvent( new Runnable()
		{
			public void run()
			{
				TekuumJNI.queueMotionEvent( action, x, y, pressure );
			}
		} );
		return true;
	}
	
	private boolean queueTrackballEvent( final int action, final float x, final float y )
	{
		/* Make sure all communication with Quake is done from the Renderer thread */
		queueEvent( new Runnable()
		{
			public void run()
			{
				TekuumJNI.queueTrackballEvent( action, x, y );
			}
		} );
		return true;
	}
	
	private boolean queueJoystickEvent( final int axis, final float value )
	{
		/* Make sure all communication with Quake is done from the Renderer thread */
		queueEvent( new Runnable()
		{
			public void run()
			{
				TekuumJNI.queueJoystickEvent( axis, value );
			}
		} );
		return true;
	}
	
	private static final int QK_ENTER = 28;
	private static final int QK_ESCAPE = 1;
	private static final int QK_BACKSPACE = 14;
	private static final int QK_LEFT = 203;
	private static final int QK_RIGHT = 205;
	private static final int QK_UP = 200;
	private static final int QK_DOWN = 208;
	private static final int QK_CTRL = 29;
	private static final int QK_SHIFT = 42;
	private static final int QK_CONSOLE = 41;
	
	/*
	private static final int QK_F1 = 145;
	private static final int QK_F2 = 146;
	private static final int QK_F3 = 147;
	private static final int QK_F4 = 148;
	*/
	
	private int androidKeyCodeToQuake( int aKeyCode, KeyEvent event )
	{
		/* Convert non-ASCII keys by hand */
		switch( aKeyCode )
		{
				/* For now map the focus buttons to F1 and let the user remap it in game.
				 * This should allow some basic movement on the Nexus One if people map it to forward.
				 * At least on the Milestone the camera button itself is shared with the Focus one. You have
				 * to press focus first and then you hit camera, this leads to the following event sequence which
				 * I don't handle right now: focus_down -> camera_down -> camera_up -> focus_up.
				 */
				//case KeyEvent.KEYCODE_FOCUS:
				//	return QK_F1;
				//case KeyEvent.KEYCODE_VOLUME_DOWN:
				//	return QK_F2;
				//case KeyEvent.KEYCODE_VOLUME_UP:
				//	return QK_F3;
			case KeyEvent.KEYCODE_DPAD_UP:
				return QK_UP;
				
			case KeyEvent.KEYCODE_DPAD_DOWN:
				return QK_DOWN;
				
			case KeyEvent.KEYCODE_DPAD_LEFT:
				return QK_LEFT;
				
			case KeyEvent.KEYCODE_DPAD_RIGHT:
				return QK_RIGHT;
				
			case KeyEvent.KEYCODE_DPAD_CENTER:
				/* Center is useful for shooting if you only use the keyboard */
				return QK_CTRL;
				
			case KeyEvent.KEYCODE_ENTER:
				return QK_ENTER;
				
			case KeyEvent.KEYCODE_GRAVE:
			case KeyEvent.KEYCODE_SEARCH:
				return QK_CONSOLE;
				
			case KeyEvent.KEYCODE_BACK:
				return QK_ESCAPE;
				
			case KeyEvent.KEYCODE_DEL:
				return QK_BACKSPACE;
				
			case KeyEvent.KEYCODE_ALT_LEFT:
				return QK_CTRL;
				
			case KeyEvent.KEYCODE_SHIFT_LEFT:
				return QK_SHIFT;
		}
		
		/* Let Android do all the character conversion for us. This way we don't have
		 * to care about modifier keys and specific keyboard layouts.
		 * TODO: add some more filtering
		 */
		int uchar = event.getUnicodeChar();
		if( uchar < 127 )
			return uchar;
			
		return 0;
	}
}
