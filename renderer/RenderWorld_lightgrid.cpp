/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
Copyright (C) 2013 Robert Beckebans

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

#include "precompiled.h"
#pragma hdrstop

#include "tr_local.h"

void idRenderWorldLocal::SetupEntityGridLighting( idRenderEntityLocal* def )
{
	// lighting calculations
#if 0
	if( def->lightgridCalculated )
	{
		return;
	}
	
	def->lightgridCalculated = true;
#endif
	
	if( lightGridPoints.Num() > 0 )
	{
		idVec3          lightOrigin;
		int             pos[3];
		int             i, j;
		int				gridPointIndex;
		lightGridPoint_t*  gridPoint;
		lightGridPoint_t*  gridPoint2;
		float           frac[3];
		int             gridStep[3];
		idVec3          direction;
		idVec3			direction2;
		float			lattitude;
		float			longitude;
		float           totalFactor;
		
#if 0
		if( forcedOrigin )
		{
			VectorCopy( forcedOrigin, lightOrigin );
		}
		else
		{
			if( ent->e.renderfx & RF_LIGHTING_ORIGIN )
			{
				// seperate lightOrigins are needed so an object that is
				// sinking into the ground can still be lit, and so
				// multi-part models can be lit identically
				VectorCopy( ent->e.lightingOrigin, lightOrigin );
			}
			else
			{
				VectorCopy( ent->e.origin, lightOrigin );
			}
		}
#else
		// some models, like empty particles have no volume
#if 1
		lightOrigin = def->parms.origin;
#else
		if( def->referenceBounds.IsCleared() )
		{
			lightOrigin = def->parms.origin;
		}
		else
		{
			lightOrigin = def->volumeMidPoint;
		}
#endif
		
#endif
		
		lightOrigin -= lightGridOrigin;
		for( i = 0; i < 3; i++ )
		{
			float           v;
			
			v = lightOrigin[i] * ( 1.0f / lightGridSize[i] );
			pos[i] = floor( v );
			frac[i] = v - pos[i];
			if( pos[i] < 0 )
			{
				pos[i] = 0;
			}
			else if( pos[i] >= lightGridBounds[i] - 1 )
			{
				pos[i] = lightGridBounds[i] - 1;
			}
		}
		
		def->ambientLight.Zero();
		def->directedLight.Zero();
		direction.Zero();
		
		// trilerp the light value
		gridStep[0] = 1;
		gridStep[1] = lightGridBounds[0];
		gridStep[2] = lightGridBounds[0] * lightGridBounds[1];
		
		gridPointIndex = pos[0] * gridStep[0] + pos[1] * gridStep[1] + pos[2] * gridStep[2];
		gridPoint = &lightGridPoints[ gridPointIndex ];
		
		totalFactor = 0;
		for( i = 0; i < 8; i++ )
		{
			float           factor;
			
			factor = 1.0;
			gridPoint2 = gridPoint;
			for( j = 0; j < 3; j++ )
			{
				if( i & ( 1 << j ) )
				{
					int gridPointIndex2 = gridPointIndex + gridStep[j];
					
					if( gridPointIndex2 < 0 || gridPointIndex2 >= lightGridPoints.Num() )
						continue;
						
					factor *= frac[j];
					
					gridPoint2 = &lightGridPoints[ gridPointIndex + gridStep[j] ];
				}
				else
				{
					factor *= ( 1.0f - frac[j] );
				}
			}
			
			if( !( gridPoint2->ambient[0] + gridPoint2->ambient[1] + gridPoint2->ambient[2] ) )
			{
				continue;			// ignore samples in walls
			}
			
			totalFactor += factor;
			
			def->ambientLight[0] += factor * gridPoint2->ambient[0] * ( 1.0f / 255.0f );
			def->ambientLight[1] += factor * gridPoint2->ambient[1] * ( 1.0f / 255.0f );
			def->ambientLight[2] += factor * gridPoint2->ambient[2] * ( 1.0f / 255.0f );
			
			def->directedLight[0] += factor * gridPoint2->directed[0] * ( 1.0f / 255.0f );
			def->directedLight[1] += factor * gridPoint2->directed[1] * ( 1.0f / 255.0f );
			def->directedLight[2] += factor * gridPoint2->directed[2] * ( 1.0f / 255.0f );
			
			lattitude = DEG2RAD( gridPoint2->latLong[1] * ( 360.0f / 255.0f ) );
			longitude = DEG2RAD( gridPoint2->latLong[0] * ( 360.0f / 255.0f ) );
			
			direction2[0] = idMath::Cos( lattitude ) * idMath::Sin( longitude );
			direction2[1] = idMath::Sin( lattitude ) * idMath::Sin( longitude );
			direction2[2] = idMath::Cos( longitude );
			
			direction += ( direction2 * factor );
			
			//direction += ( gridPoint2->dir * factor );
		}
		
#if 1
		if( totalFactor > 0 && totalFactor < 0.99 )
		{
			totalFactor = 1.0f / totalFactor;
			def->ambientLight *= totalFactor;
			def->directedLight *= totalFactor;
		}
#endif
		
		def->ambientLight[0] = idMath::ClampFloat( 0, 1, def->ambientLight[0] );
		def->ambientLight[1] = idMath::ClampFloat( 0, 1, def->ambientLight[1] );
		def->ambientLight[2] = idMath::ClampFloat( 0, 1, def->ambientLight[2] );
		
		def->directedLight[0] = idMath::ClampFloat( 0, 1, def->directedLight[0] );
		def->directedLight[1] = idMath::ClampFloat( 0, 1, def->directedLight[1] );
		def->directedLight[2] = idMath::ClampFloat( 0, 1, def->directedLight[2] );
		
		def->lightDir = direction;
		def->lightDir.Normalize();
		
#if 0
		if( VectorLength( ent->ambientLight ) < r_forceAmbient->value )
		{
			ent->ambientLight[0] = r_forceAmbient->value;
			ent->ambientLight[1] = r_forceAmbient->value;
			ent->ambientLight[2] = r_forceAmbient->value;
		}
#endif
	}
}
