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
		
		// MrE: if the light is behind the surface
		if( DotProduct( lightOrigin, normal ) - DotProduct( normal, origin ) < 0 )
			continue;
			
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
		//if( light->type == emit_point )
		{
			dir = lightOrigin - origin;
			dist = dir.Normalize();
			
			// clamp the distance to prevent super hot spots
			if( dist < 16 )
			{
				dist = 16;
			}
			angle = DotProduct( normal, dir );
			
			/*
			if( light->linearLight )
			{
				add = angle * light->photons * linearScale - dist;
				if( add < 0 )
				{
					add = 0;
				}
			}
			else
			*/
			{
				add = light->photons / ( dist * dist ) * angle;
			}
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
		color[0] += add * light->def.parms.shaderParms[ SHADERPARM_RED ] * trace.filter[0];
		color[1] += add * light->def.parms.shaderParms[ SHADERPARM_GREEN ] * trace.filter[1];
		color[2] += add * light->def.parms.shaderParms[ SHADERPARM_BLUE ] * trace.filter[2];
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
		
		LightingAtSample( dv->xyz, dv->normal, sample, testOcclusion, forceSunLight, tw );
		
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
			dv->lightColor[j] = sample[j];
		}
		
		// Don't bother writing alpha since it will already be set to 255,
		// plus we don't want to write over alpha generated by SetTerrainTextures
		//dv->color[3] = 255;
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
	idMapEntity* entity = dmapGlobals.uEntities[entityNum].mapEntity;
	
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

/*
=============
LightWorld
=============
*/
void LightWorld()
{
	float           f;
	
	// determine the number of grid points
	//SetupGrid();
	
	// find the optional world ambient
	//GetVectorForKey(&entities[0], "_color", ambientColor);
	//f = FloatForKey(&entities[0], "ambient");
	//VectorScale(ambientColor, f, ambientColor);
	
	// create lights out of patches and lights
	//Sys_FPrintf(SYS_VRB, "--- CreateLights ---\n");
	//CreateEntityLights();
	//Sys_FPrintf(SYS_VRB, "%i point lights\n", numPointLights);
	//Sys_FPrintf(SYS_VRB, "%i area lights\n", numAreaLights);
	
	//if(!nogridlighting)
	//{
	//	Sys_FPrintf(SYS_VRB, "--- TraceGrid ---\n");
	//	RunThreadsOnIndividual(numGridPoints, qtrue, TraceGrid);
	//	Sys_FPrintf(SYS_VRB, "%i x %i x %i = %i grid\n", gridBounds[0], gridBounds[1], gridBounds[2], numGridPoints);
	//}
	
	//Sys_FPrintf(SYS_VRB, "--- TraceLtm ---\n");
	//RunThreadsOnIndividual(numDrawSurfaces, qtrue, TraceLtm);
	//Sys_FPrintf(SYS_VRB, "%5i visible samples\n", c_visible);
	//Sys_FPrintf(SYS_VRB, "%5i occluded samples\n", c_occluded);
	
	// write the entity models and information, writing entities first
	
	common->Printf( "----- LightWorld -----\n" );
	
	for( int i = dmapGlobals.num_entities - 1 ; i >= 0 ; i-- )
	{
		uEntity_t* entity = &dmapGlobals.uEntities[i];
		
		if( !entity->primitives )
		{
			continue;
		}
		
		LightEntity( i );
	}
}
