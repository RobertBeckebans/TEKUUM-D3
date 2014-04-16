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

#include "dmap.h"


#define	MAX_MAP_LIGHTGRID_POINTS		0x100000

struct traceWork_t
{
	idVec3          start, end;
	int             numOpenLeafs;
	//int             openLeafNumbers[MAX_MAP_LEAFS];
	//trace_t        *trace;
	int             patchshadows;
};


struct lightTrace_t
{
	idVec3			filter;		// starts out 1.0, 1.0, 1.0, may be reduced if
	// transparent surfaces are crossed
	
	idVec3          hit;		// the impact point of a completely opaque surface
	float           hitFraction;	// 0 = at start, 1.0 = at end
	bool	        passSolid;
};


static void ColorToBytes( const idVec3& color, byte colorBytes[3] )
{
	float           max;
	idVec3          sample;
	
	VectorCopy( color, sample );
	
	// clamp with color normalization
	max = sample[0];
	
	if( sample[1] > max )
	{
		max = sample[1];
	}
	
	if( sample[2] > max )
	{
		max = sample[2];
	}
	
	if( max > 255 )
	{
		VectorScale( sample, 255 / max, sample );
	}
	
	colorBytes[0] = sample[0];
	colorBytes[1] = sample[1];
	colorBytes[2] = sample[2];
}

static /*
** NormalToLatLong
**
** We use two byte encoded normals in some space critical applications.
** Lat = 0 at (1,0,0) to 360 (-1,0,0), encoded in 8-bit sine table format
** Lng = 0 at (0,0,1) to 180 (0,0,-1), encoded in 8-bit sine table format
**
*/
void NormalToLatLong( const idVec3& normal, byte bytes[2] )
{
	// check for singularities
	if( normal[0] == 0 && normal[1] == 0 )
	{
		if( normal[2] > 0 )
		{
			bytes[0] = 0;
			bytes[1] = 0;		// lat = 0, long = 0
		}
		else
		{
			bytes[0] = 128;
			bytes[1] = 0;		// lat = 0, long = 128
		}
	}
	else
	{
		int             a, b;
		
		a = RAD2DEG( atan2( normal[1], normal[0] ) ) * ( 255.0f / 360.0f );
		a &= 0xff;
		
		b = RAD2DEG( acos( normal[2] ) ) * ( 255.0f / 360.0f );
		b &= 0xff;
		
		bytes[0] = b;			// longitude
		bytes[1] = a;			// lattitude
	}
}

/*
================
LightingAtSample
================
*/
void LightingAtSample( const idVec3& origin, const idVec3& normal, idVec3& color, bool testOcclusion, bool forceSunLight, traceWork_t* tw )
{
#if 1
	lightTrace_t	trace;
	float           angle;
	float           add = 0;
	float           dist;
	idVec3          dir;
	
	//VectorCopy( ambientColor, color );
	color = vec3_zero;
	
	// trace to all the lights
	for( int i = 0 ; i < dmapGlobals.mapLights.Num() ; i++ )
	{
		mapLight_t*	light = dmapGlobals.mapLights[i];
		
		idVec3 lightOrigin = light->def.globalLightOrigin;
		
		
		
		
		
		// testing exact PTPFF
#if 0
		if( exactPointToPolygon && light->type == emit_area )
		{
			float           factor;
			float           d;
			vec3_t          pushedOrigin;
			
			// see if the point is behind the light
			d = DotProduct( origin, light->normal ) - light->dist;
			if( !light->twosided )
			{
				if( d < -1 )
				{
					continue;	// point is behind light
				}
			}
			
			// test occlusion and find light filters
			// clip the line, tracing from the surface towards the light
			if( !notrace && testOcclusion )
			{
				TraceLine( origin, light->origin, &trace, qfalse, tw );
				
				// other light rays must not hit anything
				if( trace.passSolid )
				{
					continue;
				}
			}
			else
			{
				trace.filter[0] = 1.0;
				trace.filter[1] = 1.0;
				trace.filter[2] = 1.0;
			}
			
			// nudge the point so that it is clearly forward of the light
			// so that surfaces meeting a light emiter don't get black edges
			if( d > -8 && d < 8 )
			{
				VectorMA( origin, ( 8 - d ), light->normal, pushedOrigin );
			}
			else
			{
				VectorCopy( origin, pushedOrigin );
			}
			
			// calculate the contribution
			factor = PointToPolygonFormFactor( pushedOrigin, normal, light->w );
			if( factor <= 0 )
			{
				if( light->twosided )
				{
					factor = -factor;
				}
				else
				{
					continue;
				}
			}
			color[0] += factor * light->emitColor[0] * trace.filter[0];
			color[1] += factor * light->emitColor[1] * trace.filter[1];
			color[2] += factor * light->emitColor[2] * trace.filter[2];
			
			continue;
		}
#endif
		
		// calculate the amount of light at this sample
		if( light->def.lightShader->IsAmbientLight() )
		{
			add = 255;
		}
		else if( light->def.parms.parallel )
		{
			dir = lightOrigin - origin;
			dist = dir.Normalize();
			
			// clamp the distance to prevent super hot spots
			if( dist < 16 )
			{
				dist = 16;
			}
			angle = DotProduct( normal, dir );
			
			add = 255 * angle;
		}
		else if( light->def.parms.pointLight )
		{
			// MrE: if the light is behind the surface
			if( DotProduct( lightOrigin, normal ) - DotProduct( normal, origin ) < 0 )
				continue;
				
			dir = lightOrigin - origin;
			dist = dir.Normalize();
			
			// clamp the distance to prevent super hot spots
			if( dist < 16 )
			{
				dist = 16;
			}
			angle = DotProduct( normal, dir );
			
			const float pointScale = 7500.0f;
			
			//if( light->linearLight )
#if 0
			{
				float linearScale = 1.0f;
				add = angle * ( light->photons * pointScale ) * linearScale - dist;
				if( add < 0 )
				{
					add = 0;
				}
			}
			//else
#else
			{
			
				add = ( light->photons * pointScale ) / ( dist * dist ) * angle;
			}
#endif
		}
		else
		{
			// TODO spot light
		}
		
		/*
		else if(light->type == emit_spotlight)
		{
			float           distByNormal;
			vec3_t          pointAtDist;
			float           radiusAtDist;
			float           sampleRadius;
			vec3_t          distToSample;
			float           coneScale;
		
			VectorSubtract(light->origin, origin, dir);
		
			distByNormal = -DotProduct(dir, light->normal);
			if(distByNormal < 0)
			{
				continue;
			}
			VectorMA(light->origin, distByNormal, light->normal, pointAtDist);
			radiusAtDist = light->radiusByDist * distByNormal;
		
			VectorSubtract(origin, pointAtDist, distToSample);
			sampleRadius = VectorLength(distToSample);
		
			if(sampleRadius >= radiusAtDist)
			{
				continue;		// outside the cone
			}
			if(sampleRadius <= radiusAtDist - 32)
			{
				coneScale = 1.0;	// fully inside
			}
			else
			{
				coneScale = (radiusAtDist - sampleRadius) / 32.0;
			}
		
			dist = VectorNormalize(dir);
			// clamp the distance to prevent super hot spots
			if(dist < 16)
			{
				dist = 16;
			}
			angle = DotProduct(normal, dir);
			add = light->photons / (dist * dist) * angle * coneScale;
		
		}
		else if(light->type == emit_area)
		{
			VectorSubtract(light->origin, origin, dir);
			dist = VectorNormalize(dir);
			// clamp the distance to prevent super hot spots
			if(dist < 16)
			{
				dist = 16;
			}
			angle = DotProduct(normal, dir);
			if(angle <= 0)
			{
				continue;
			}
			angle *= -DotProduct(light->normal, dir);
			if(angle <= 0)
			{
				continue;
			}
		
			if(light->linearLight)
			{
				add = angle * light->photons * linearScale - dist;
				if(add < 0)
				{
					add = 0;
				}
			}
			else
			{
				add = light->photons / (dist * dist) * angle;
			}
		}
		*/
		
		if( add <= 1.0 )
		{
			continue;
		}
		
#if 0
		// clip the line, tracing from the surface towards the light
		if( !notrace && testOcclusion )
		{
			TraceLine( origin, light->origin, &trace, qfalse, tw );
			
			// other light rays must not hit anything
			if( trace.passSolid )
			{
				continue;
			}
		}
		else
#endif
		{
			trace.filter[0] = 1;
			trace.filter[1] = 1;
			trace.filter[2] = 1;
		}
		
		// add the result
		color[0] += add * light->def.parms.shaderParms[ SHADERPARM_RED ]   * trace.filter[0];
		color[1] += add * light->def.parms.shaderParms[ SHADERPARM_GREEN ] * trace.filter[1];
		color[2] += add * light->def.parms.shaderParms[ SHADERPARM_BLUE ]  * trace.filter[2];
	}
	
	// trace directly to the sun
	/*
	if(testOcclusion || forceSunLight)
	{
		SunToPlane(origin, normal, color, tw);
	}
	*/
#endif
}

/*
=============
VertexLighting

Vertex lighting will completely ignore occlusion, because
shadows would not be resolvable anyway.
=============
*/
void VertexLighting( mapTri_t* tri, bool testOcclusion, bool forceSunLight, float scale, traceWork_t* tw )
{
	int             i, j;
	idDrawVert*     dv;
	idVec3			sample, normal;
	float           max;
	
	// generate vertex lighting
	for( i = 0; i < 3; i++ )
	{
		dv = &tri->v[i];
		
		LightingAtSample( dv->xyz, dv->GetNormal(), sample, testOcclusion, forceSunLight, tw );
		
		if( scale >= 0 )
			VectorScale( sample, scale, sample );
			
		// clamp with color normalization
		max = sample[0];
		if( sample[1] > max )
		{
			max = sample[1];
		}
		if( sample[2] > max )
		{
			max = sample[2];
		}
		if( max > 255 )
		{
			VectorScale( sample, 255 / max, sample );
		}
		
		// save the sample
		for( j = 0; j < 3; j++ )
		{
			if( sample[j] > 255 )
			{
				sample[j] = 255;
			}
			
			dv->color[j] = ( byte )( sample[j] );
		}
		
		// Don't bother writing alpha since it will already be set to 255,
		// plus we don't want to write over alpha generated by SetTerrainTextures
		dv->color[3] = 255;
	}
}


/*
====================
VertexLightingSurfaces
====================
*/
static void VertexLightingSurfaces( int entityNum, int areaNum )
{
	traceWork_t tw;
	
	uArea_t* area = &dmapGlobals.uEntities[entityNum].areas[areaNum];
	//idMapEntity* entity = dmapGlobals.uEntities[entityNum].mapEntity;
	
	for( optimizeGroup_t* group = area->groups ; group ; group = group->nextGroup )
	{
		for( mapTri_t* tri = group->triList ; tri ; tri = tri->next )
		{
			VertexLighting( tri, true, false, 1.0f, &tw );
		}
	}
}

/*
====================
LightEntity
====================
*/
static void LightEntity( int entityNum )
{
	uEntity_t* e = &dmapGlobals.uEntities[entityNum];
	
	if( entityNum != 0 )
	{
		// entities may have enclosed, empty areas that we don't need to write out
		if( e->numAreas > 1 )
		{
			e->numAreas = 1;
		}
	}
	
	for( int i = 0 ; i < e->numAreas ; i++ )
	{
		VertexLightingSurfaces( entityNum, i );
	}
}


//=============================================================================

/*
=============
PointInSolid
=============
*/
static bool PointInSolid( const idVec3& start )
{
	uEntity_t* e = &dmapGlobals.uEntities[0];
	
	node_t* node = NodeForPoint( e->tree->headnode, start );
	
	// RB: area -1 is the void
	return ( node->area == -1 );
}

//=============================================================================

/*
========================
LightContributionToPoint
========================
*/
bool LightContributionToPoint( const mapLight_t* light, const idVec3& origin, idVec3& color, traceWork_t* tw )
{
	trace_t         trace;
	float           add;
	
	add = 0;
	
	color.Zero();
	
	// testing exact PTPFF
	/*
	if(exactPointToPolygon && light->type == emit_area)
	{
		float           factor;
		float           d;
		vec3_t          normal;
	
		// see if the point is behind the light
		d = DotProduct(origin, light->normal) - light->dist;
		if(!light->twosided)
		{
			if(d < 1)
			{
				return qfalse;	// point is behind light
			}
		}
	
		// test occlusion
		// clip the line, tracing from the surface towards the light
		TraceLine(origin, light->origin, &trace, qfalse, tw);
		if(trace.passSolid)
		{
			return qfalse;
		}
	
		// calculate the contribution
		VectorSubtract(light->origin, origin, normal);
		if(VectorNormalize(normal) == 0)
		{
			return qfalse;
		}
		factor = PointToPolygonFormFactor(origin, normal, light->w);
		if(factor <= 0)
		{
			if(light->twosided)
			{
				factor = -factor;
			}
			else
			{
				return qfalse;
			}
		}
		VectorScale(light->emitColor, factor, color);
		return qtrue;
	}
	*/
	
	// calculate the amount of light at this sample
	if( light->def.lightShader->IsAmbientLight() )
	{
		add = 255;
	}
	else if( light->def.parms.parallel )
	{
		add = 255;
	}
	else if( light->def.parms.pointLight )
	{
		idVec3 dir = light->def.globalLightOrigin - origin;
		float dist = dir.Normalize();
		
		// clamp the distance to prevent super hot spots
		if( dist < 16 )
		{
			dist = 16;
		}
		
		const float pointScale = 7500.0f;
		
#if 0
		if( light->linearLight )
		{
			add = light->photons * linearScale - dist;
			if( add < 0 )
			{
				add = 0;
			}
		}
		else
#endif
		{
			add = ( light->photons * pointScale ) / ( dist * dist );
		}
	}
	//else
	//{
	//	return false;
	//}
	
	if( add <= 1.0 )
	{
		return false;
	}
	
#if 0
	// clip the line, tracing from the surface towards the light
	TraceLine( origin, light->def.globalLightOrigin, &trace, false, tw );
	
	// other light rays must not hit anything
	if( trace.passSolid )
	{
		return false;
	}
#endif
	
	// add the result
	color[0] = add * light->def.parms.shaderParms[ SHADERPARM_RED ];
	color[1] = add * light->def.parms.shaderParms[ SHADERPARM_GREEN ];
	color[2] = add * light->def.parms.shaderParms[ SHADERPARM_BLUE ];
	
	return true;
}

struct contribution_t
{
	idVec3          dir;
	idVec3          color;
};

/*
=============
TraceGrid

Grid samples are foe quickly determining the lighting
of dynamically placed entities in the world
=============
*/
#define	MAX_CONTRIBUTIONS	1024
void TraceGrid( int num )
{
#if 1
	int             x, y, z;
	idVec3          origin;
	mapLight_t*     light;
	idVec3          color;
	int             mod;
	idVec3          directedColor;
	idVec3          summedDir;
	contribution_t  contributions[MAX_CONTRIBUTIONS];
	int             numCon;
	int             i;
	traceWork_t     tw;
	float           addSize;
	lightGridPoint_t* gridPoint;
	
	gridPoint = &dmapGlobals.lightGridPoints[ num ];
	
	mod = num;
	z = mod / ( dmapGlobals.lightGridBounds[0] * dmapGlobals.lightGridBounds[1] );
	mod -= z * ( dmapGlobals.lightGridBounds[0] * dmapGlobals.lightGridBounds[1] );
	
	y = mod / dmapGlobals.lightGridBounds[0];
	mod -= y * dmapGlobals.lightGridBounds[0];
	
	x = mod;
	
	origin[0] = dmapGlobals.lightGridMins[0] + x * dmapGlobals.lightGridSize[0];
	origin[1] = dmapGlobals.lightGridMins[1] + y * dmapGlobals.lightGridSize[1];
	origin[2] = dmapGlobals.lightGridMins[2] + z * dmapGlobals.lightGridSize[2];
	
	if( PointInSolid( origin ) )
	{
		idVec3          baseOrigin;
		int             step;
		
		VectorCopy( origin, baseOrigin );
		
		// try to nudge the origin around to find a valid point
		for( step = 9; step <= 18; step += 9 )
		{
			for( i = 0; i < 8; i++ )
			{
				VectorCopy( baseOrigin, origin );
				if( i & 1 )
				{
					origin[0] += step;
				}
				else
				{
					origin[0] -= step;
				}
				if( i & 2 )
				{
					origin[1] += step;
				}
				else
				{
					origin[1] -= step;
				}
				if( i & 4 )
				{
					origin[2] += step;
				}
				else
				{
					origin[2] -= step;
				}
				
				if( !PointInSolid( origin ) )
				{
					break;
				}
			}
			
			if( i != 8 )
			{
				break;
			}
		}
		
		if( step > 18 )
		{
			// can't find a valid point at all
			for( i = 0; i < 3; i++ )
			{
				gridPoint->ambient[i] = 0;
				gridPoint->directed[i] = 0;
			}
			gridPoint->latLong[0] = 0;
			gridPoint->latLong[1] = 0;
			return;
		}
	}
	
	summedDir.Zero();
	
	// trace to all the lights
	
	// find the major light direction, and divide the
	// total light between that along the direction and
	// the remaining in the ambient
	numCon = 0;
	for( i = 0; i < dmapGlobals.mapLights.Num() && i < MAX_CONTRIBUTIONS; i++ )
	{
		light = dmapGlobals.mapLights[i];
		
		idVec3          add;
		idVec3          dir;
		float           addSize;
		
		if( !LightContributionToPoint( light, origin, add, &tw ) )
		{
			continue;
		}
		
		dir = light->def.globalLightOrigin - origin;
		dir.Normalize();
		
		VectorCopy( add, contributions[numCon].color );
		VectorCopy( dir, contributions[numCon].dir );
		numCon++;
		
		addSize = add.Length();
		VectorMA( summedDir, addSize, dir, summedDir );
		
		if( numCon == MAX_CONTRIBUTIONS - 1 )
		{
			break;
		}
	}
	
#if 0
	// trace directly to the sun
	SunToPoint( origin, &tw, color );
	addSize = VectorLength( color );
	if( addSize > 0 )
	{
		VectorCopy( color, contributions[numCon].color );
		VectorCopy( sunDirection, contributions[numCon].dir );
		VectorMA( summedDir, addSize, sunDirection, summedDir );
		numCon++;
	}
#endif
	
	// now that we have identified the primary light direction,
	// go back and seperate all the light into directed and ambient
	summedDir.Normalize();
	
	// RB: FIXME add ambientColor
	//VectorCopy(ambientColor, color);
	color.Zero();
	
	directedColor.Zero();
	
	for( i = 0; i < numCon; i++ )
	{
		float           d;
		
		d = DotProduct( contributions[i].dir, summedDir );
		if( d < 0 )
		{
			d = 0;
		}
		
		VectorMA( directedColor, d, contributions[i].color, directedColor );
		
		// the ambient light will be at 1/4 the value of directed light
		d = 0.25 * ( 1.0 - d );
		VectorMA( color, d, contributions[i].color, color );
	}
	
	// now do some fudging to keep the ambient from being too low
	VectorMA( color, 0.25, directedColor, color );
	
	// save the resulting value out
	ColorToBytes( color, gridPoint->ambient );
	ColorToBytes( directedColor, gridPoint->directed );
	
	//gridPoint->ambient = color;
	//gridPoint->directed = directedColor;
	
	summedDir.Normalize();
	
	//gridPoint->dir = summedDir;
	NormalToLatLong( summedDir, gridPoint->latLong );
#endif
}

static void CalculateLightGridBounds( idBounds& bounds )
{
	uEntity_t* e = &dmapGlobals.uEntities[0];
	
	bounds.Clear();
	for( int i = 0; i < e->numAreas; i++ )
	{
		uArea_t* area = &dmapGlobals.uEntities[0].areas[i];
		
		for( optimizeGroup_t* group = area->groups ; group ; group = group->nextGroup )
		{
#if 0
			bounds.AddBounds( group->bounds );
#else
			for( mapTri_t* tri = group->triList ; tri ; tri = tri->next )
			{
				bounds.AddPoint( tri->v[0].xyz );
				bounds.AddPoint( tri->v[1].xyz );
				bounds.AddPoint( tri->v[2].xyz );
			}
#endif
		}
	}
}

static void SetupLightGrid()
{
	idLib::Printf( "----- SetupLightGrid -----\n" );
	
	idBounds bounds;
	CalculateLightGridBounds( bounds );
	
	idVec3 maxs;
	int j = 0;
	int numGridPoints = MAX_MAP_LIGHTGRID_POINTS + 1;
	while( numGridPoints > MAX_MAP_LIGHTGRID_POINTS )
	{
		for( int i = 0; i < 3; i++ )
		{
			dmapGlobals.lightGridMins[i] = dmapGlobals.lightGridSize[i] * ceil( bounds[0][i] / dmapGlobals.lightGridSize[i] );
			maxs[i] = dmapGlobals.lightGridSize[i] * floor( bounds[1][i] / dmapGlobals.lightGridSize[i] );
			dmapGlobals.lightGridBounds[i] = ( maxs[i] - dmapGlobals.lightGridMins[i] ) / dmapGlobals.lightGridSize[i] + 1;
		}
		
		numGridPoints = dmapGlobals.lightGridBounds[0] * dmapGlobals.lightGridBounds[1] * dmapGlobals.lightGridBounds[2];
		
		if( numGridPoints > MAX_MAP_LIGHTGRID_POINTS )
		{
			dmapGlobals.lightGridSize[ j++ % 3 ] += 16.0f;
		}
	}
	
	idLib::Printf( "grid size (%i %i %i)\n", ( int )dmapGlobals.lightGridSize[0], ( int )dmapGlobals.lightGridSize[1], dmapGlobals.lightGridSize[2] );
	idLib::Printf( "grid bounds (%i %i %i)\n", ( int )dmapGlobals.lightGridBounds[0], ( int )dmapGlobals.lightGridBounds[1], ( int )dmapGlobals.lightGridBounds[2] );
	
	
	idLib::Printf( "%i x %i x %i = %i grid points \n", dmapGlobals.lightGridBounds[0], dmapGlobals.lightGridBounds[1], dmapGlobals.lightGridBounds[2], numGridPoints );
	
	dmapGlobals.lightGridPoints.SetNum( numGridPoints );
	
	idLib::Printf( "%9u x %u B = lightGridSize = (%.2fMB)\n", numGridPoints, sizeof( lightGridPoint_t ), ( float )( dmapGlobals.lightGridPoints.MemoryUsed() ) / ( 1024.0f * 1024.0f ) );
}

/*
=============
LightWorld
=============
*/
void LightWorld()
{
	common->Printf( "----- LightWorld -----\n" );
	
	common->Printf( "----- VertexLighting -----\n" );
	for( int i = dmapGlobals.num_entities - 1 ; i >= 0 ; i-- )
	{
		uEntity_t* entity = &dmapGlobals.uEntities[i];
		
		if( !entity->primitives )
		{
			continue;
		}
		
		LightEntity( i );
	}
	
	SetupLightGrid();
	
	// TODO parallel jobs
	for( int i = 0; i < dmapGlobals.lightGridPoints.Num(); i++ )
	{
		TraceGrid( i );
	}
}
