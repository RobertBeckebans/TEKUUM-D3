/*
 * Copyright (C) 2012 Manuel Fasse
 * Copyright (C) 2013 Robert Beckebans
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

import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Paint.Style;
import android.graphics.RectF;
import android.view.MotionEvent;
import android.view.View;

public class Joystick implements MotionDevice
{
	View mView;
	float axisX, axisY, posX, posY, size, eventX, eventY;
	MovementListener listener;
	boolean hidden;
	int actionId;
	RectF mBoundary;
	
	public Joystick( View view )
	{
		mView = view;
		axisX = 0;
		axisY = 0;
		size = 50;
		hidden = true;
		actionId = -1;
		listener = new MovementListener()
		{
			public void onMove( MotionDevice dev, Movement m )
			{
				// do nothing
			}
		};
	}
	
	public RectF getBoundary()
	{
		if( mBoundary != null )
		{
			return mBoundary;
		}
		else
		{
			return new RectF( 0, 0, mView.getWidth(), mView.getHeight() );
		}
	}
	
	public void setBoundary( RectF boundary )
	{
		mBoundary = boundary;
	}
	
	public boolean onDown( MotionEvent event )
	{
		int index = event.getActionIndex();
		float x = event.getX( index );
		float y = event.getY( index );
		if( getBoundary().contains( x, y ) )
		{
			actionId = event.getPointerId( event.getActionIndex() );
			init( x, y );
			return true;
		}
		return false;
	}
	
	public boolean onMove( MotionEvent event )
	{
		int index = event.findPointerIndex( actionId );
		float x, y;
		if( index != -1 )
		{
			x = event.getX( index );
			y = event.getY( index );
			update( x, y );
			return true;
		}
		return false;
	}
	
	public boolean onUp( MotionEvent event )
	{
		int index = event.getActionIndex();
		if( index == event.findPointerIndex( actionId ) )
		{
			actionId = -1;
			hide();
			return true;
		}
		return false;
	}
	
	public void setMovementListener( MovementListener newListener )
	{
		if( newListener != null )
		{
			listener = newListener;
		}
	}
	
	public float getXAxis()
	{
		return axisX;
	}
	
	public float getYAxis()
	{
		return axisY;
	}
	
	public float getEventX()
	{
		return eventX;
	}
	
	public float getEventY()
	{
		return eventY;
	}
	
	public void setAxes( float new_x, float new_y )
	{
		axisX = Math.max( -1, Math.min( 1, new_x ) ); // clamp to [-1, 1]
		axisY = Math.max( -1, Math.min( 1, new_y ) );
		
		listener.onMove( this, new Movement( this.axisX, this.axisY, this.posX, this.posY ) );
	}
	
	public void init( float positionX, float positionY )
	{
		eventX = positionX;
		eventY = positionY;
		
		posX = positionX;
		posY = positionY;
		
		setAxes( 0, 0 );
		hidden = false;
	}
	
	public void hide()
	{
		setAxes( 0, 0 );
		hidden = true;
	}
	
	public void update( float positionX, float positionY )
	{
		eventX = positionX;
		eventY = positionY;
		
		float rawX = ( posX - positionX ) / size * -1;
		float rawY = ( posY - positionY ) / size;
		setAxes( rawX, rawY );
	}
	
	public void draw( Canvas c )
	{
		Paint outer = new Paint( Paint.ANTI_ALIAS_FLAG );
		outer.setStyle( Style.STROKE );
		outer.setStrokeWidth( 4 );
		outer.setColor( 0x99AAAAAA );
		c.drawRect( getBoundary(), outer );
		if( !hidden )
		{
			Paint thumb = new Paint( Paint.ANTI_ALIAS_FLAG );
			thumb.setStyle( Style.FILL );
			thumb.setColor( 0x99CCCCCC );
			
			c.drawCircle( posX, posY, size + 35, outer );
			c.drawCircle( posX + ( axisX * size ), posY + ( axisY * size * -1 ), 50,
						  thumb );
		}
	}
}
