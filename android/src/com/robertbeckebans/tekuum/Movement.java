/*
 * Copyright (C) 2012 Manuel Fasse
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

public class Movement
{
	private float x, y, origin_x, origin_y;
	
	public Movement()
	{
		this( 0, 0 );
	}
	
	public Movement( float x, float y )
	{
		this( x, y, 0, 0 );
	}
	
	public Movement( float x, float y, float origin_x, float origin_y )
	{
		this.x = x;
		this.y = y;
		this.origin_x = origin_x;
		this.origin_y = origin_y;
	}
	
	public float getX()
	{
		return x;
	}
	
	public float getY()
	{
		return y;
	}
	
	public float getOriginX()
	{
		return origin_x;
	}
	
	public float getOriginY()
	{
		return origin_y;
	}
	
	@Override
	public String toString()
	{
		return getClass().getName() + "[" + "x=" + Math.round( x * 100 ) / 100.0 + ", " + "y=" + Math.round( y * 100 ) / 100.0 + "]";
	}
}
