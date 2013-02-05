/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2013 Robert Beckebans

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#pragma hdrstop
#include "precompiled.h"

#include "tr_local.h"


/*
================
idRenderWorldLocal::FreeWorld
================
*/
void idRenderWorldLocal::FreeWorld()
{
	// this will free all the lightDefs and entityDefs
	FreeDefs();
	
	// free all the portals and check light/model references
	for( int i = 0; i < numPortalAreas; i++ )
	{
		portalArea_t*	area;
		portal_t*		portal, *nextPortal;
		
		area = &portalAreas[i];
		for( portal = area->portals; portal; portal = nextPortal )
		{
			nextPortal = portal->next;
			delete portal->w;
			R_StaticFree( portal );
		}
		
		// there shouldn't be any remaining lightRefs or entityRefs
		if( area->lightRefs.areaNext != &area->lightRefs )
		{
			common->Error( "FreeWorld: unexpected remaining lightRefs" );
		}
		if( area->entityRefs.areaNext != &area->entityRefs )
		{
			common->Error( "FreeWorld: unexpected remaining entityRefs" );
		}
	}
	
	if( portalAreas )
	{
		R_StaticFree( portalAreas );
		portalAreas = NULL;
		numPortalAreas = 0;
		R_StaticFree( areaScreenRect );
		areaScreenRect = NULL;
	}
	
	if( doublePortals )
	{
		R_StaticFree( doublePortals );
		doublePortals = NULL;
		numInterAreaPortals = 0;
	}
	
	// RB begin
	lightGridPoints.Clear();
	// RB end
	
	if( areaNodes )
	{
		R_StaticFree( areaNodes );
		areaNodes = NULL;
	}
	
	// free all the inline idRenderModels
	for( int i = 0; i < localModels.Num(); i++ )
	{
		renderModelManager->RemoveModel( localModels[i] );
		delete localModels[i];
	}
	localModels.Clear();
	
	areaReferenceAllocator.Shutdown();
	interactionAllocator.Shutdown();
	areaNumRefAllocator.Shutdown();
	
	mapName = "<FREED>";
}

/*
================
idRenderWorldLocal::TouchWorldModels
================
*/
void idRenderWorldLocal::TouchWorldModels()
{
	for( int i = 0; i < localModels.Num(); i++ )
	{
		renderModelManager->CheckModel( localModels[i]->Name() );
	}
}

/*
================
idRenderWorldLocal::ReadBinaryShadowModel
================
*/
idRenderModel* idRenderWorldLocal::ReadBinaryModel( idFile* fileIn )
{
	idStrStatic< MAX_OSPATH > name;
	fileIn->ReadString( name );
	idRenderModel* model = renderModelManager->AllocModel();
	model->InitEmpty( name );
	if( model->LoadBinaryModel( fileIn, mapTimeStamp ) )
	{
		return model;
	}
	return NULL;
}

/*
================
idRenderWorldLocal::ParseModel
================
*/
// RB added procVersion
idRenderModel* idRenderWorldLocal::ParseModel( idLexer* src, const char* mapName, ID_TIME_T mapTimeStamp, idFile* fileOut, int procVersion )
{
	idToken token;
	
	src->ExpectTokenString( "{" );
	
	// parse the name
	src->ExpectAnyToken( &token );
	
	idRenderModel* model = renderModelManager->AllocModel();
	model->InitEmpty( token );
	
	if( fileOut != NULL )
	{
		// write out the type so the binary reader knows what to instantiate
		fileOut->WriteString( "shadowmodel" );
		fileOut->WriteString( token );
	}
	
	int numSurfaces = src->ParseInt();
	if( numSurfaces < 0 )
	{
		src->Error( "R_ParseModel: bad numSurfaces" );
	}
	
	for( int i = 0; i < numSurfaces; i++ )
	{
		src->ExpectTokenString( "{" );
		
		src->ExpectAnyToken( &token );
		
		modelSurface_t surf;
		surf.shader = declManager->FindMaterial( token );
		
		( ( idMaterial* )surf.shader )->AddReference();
		
		srfTriangles_t* tri = R_AllocStaticTriSurf();
		surf.geometry = tri;
		
		tri->numVerts = src->ParseInt();
		tri->numIndexes = src->ParseInt();
		
		R_AllocStaticTriSurfVerts( tri, tri->numVerts );
		for( int j = 0 ; j < tri->numVerts ; j++ )
		{
			// RB: changed for new .proc format
			if( procVersion == PROC_EXT_VERSION )
			{
				float	vec[12];
				
				src->Parse1DMatrix( 12, vec );
				
				tri->verts[j].xyz[0] = vec[0];
				tri->verts[j].xyz[1] = vec[1];
				tri->verts[j].xyz[2] = vec[2];
				tri->verts[j].st[0] = vec[3];
				tri->verts[j].st[1] = vec[4];
				tri->verts[j].normal[0] = vec[5];
				tri->verts[j].normal[1] = vec[6];
				tri->verts[j].normal[2] = vec[7];
				
				tri->verts[j].color[0] = ( byte )( vec[8] * 255.0f );
				tri->verts[j].color[1] = ( byte )( vec[9] * 255.0f );
				tri->verts[j].color[2] = ( byte )( vec[10] * 255.0f );
				tri->verts[j].color[3] = ( byte )( vec[11] * 255.0f );
			}
			else
			{
				float	vec[8];
				
				src->Parse1DMatrix( 8, vec );
				
				tri->verts[j].xyz[0] = vec[0];
				tri->verts[j].xyz[1] = vec[1];
				tri->verts[j].xyz[2] = vec[2];
				tri->verts[j].st[0] = vec[3];
				tri->verts[j].st[1] = vec[4];
				tri->verts[j].normal[0] = vec[5];
				tri->verts[j].normal[1] = vec[6];
				tri->verts[j].normal[2] = vec[7];
			}
			// RB end
		}
		
		R_AllocStaticTriSurfIndexes( tri, tri->numIndexes );
		for( int j = 0; j < tri->numIndexes; j++ )
		{
			tri->indexes[j] = src->ParseInt();
		}
		src->ExpectTokenString( "}" );
		
		// add the completed surface to the model
		model->AddSurface( surf );
	}
	
	src->ExpectTokenString( "}" );
	
	model->FinishSurfaces();
	
	if( fileOut != NULL && model->SupportsBinaryModel() && r_binaryLoadRenderModels.GetBool() )
	{
		model->WriteBinaryModel( fileOut, &mapTimeStamp );
	}
	
	return model;
}

/*
================
idRenderWorldLocal::ReadBinaryShadowModel
================
*/
idRenderModel* idRenderWorldLocal::ReadBinaryShadowModel( idFile* fileIn )
{
	idStrStatic< MAX_OSPATH > name;
	fileIn->ReadString( name );
	idRenderModel* model = renderModelManager->AllocModel();
	model->InitEmpty( name );
	if( model->LoadBinaryModel( fileIn, mapTimeStamp ) )
	{
		return model;
	}
	return NULL;
}
/*
================
idRenderWorldLocal::ParseShadowModel
================
*/
idRenderModel* idRenderWorldLocal::ParseShadowModel( idLexer* src, idFile* fileOut )
{
	idToken token;
	
	src->ExpectTokenString( "{" );
	
	// parse the name
	src->ExpectAnyToken( &token );
	
	idRenderModel* model = renderModelManager->AllocModel();
	model->InitEmpty( token );
	
	if( fileOut != NULL )
	{
		// write out the type so the binary reader knows what to instantiate
		fileOut->WriteString( "shadowmodel" );
		fileOut->WriteString( token );
	}
	
	srfTriangles_t* tri = R_AllocStaticTriSurf();
	
	tri->numVerts = src->ParseInt();
	tri->numShadowIndexesNoCaps = src->ParseInt();
	tri->numShadowIndexesNoFrontCaps = src->ParseInt();
	tri->numIndexes = src->ParseInt();
	tri->shadowCapPlaneBits = src->ParseInt();
	
	assert( ( tri->numVerts & 1 ) == 0 );
	
	R_AllocStaticTriSurfShadowVerts( tri, tri->numVerts );
	tri->bounds.Clear();
	for( int j = 0; j < tri->numVerts; j++ )
	{
		float	vec[8];
		
		src->Parse1DMatrix( 3, vec );
		tri->shadowVertexes[j].xyz[0] = vec[0];
		tri->shadowVertexes[j].xyz[1] = vec[1];
		tri->shadowVertexes[j].xyz[2] = vec[2];
		tri->shadowVertexes[j].xyz[3] = 1;		// no homogenous value
		
		tri->bounds.AddPoint( tri->shadowVertexes[j].xyz.ToVec3() );
	}
	
	R_AllocStaticTriSurfIndexes( tri, tri->numIndexes );
	for( int j = 0; j < tri->numIndexes; j++ )
	{
		tri->indexes[j] = src->ParseInt();
	}
	
	// add the completed surface to the model
	modelSurface_t surf;
	surf.id = 0;
	surf.shader = tr.defaultMaterial;
	surf.geometry = tri;
	
	model->AddSurface( surf );
	
	src->ExpectTokenString( "}" );
	
	// NOTE: we do NOT do a model->FinishSurfaceces, because we don't need sil edges, planes, tangents, etc.
	
	if( fileOut != NULL && model->SupportsBinaryModel() && r_binaryLoadRenderModels.GetBool() )
	{
		model->WriteBinaryModel( fileOut, &mapTimeStamp );
	}
	
	return model;
}

/*
================
idRenderWorldLocal::SetupAreaRefs
================
*/
void idRenderWorldLocal::SetupAreaRefs()
{
	int		i;
	
	connectedAreaNum = 0;
	for( i = 0 ; i < numPortalAreas ; i++ )
	{
		portalAreas[i].areaNum = i;
		portalAreas[i].lightRefs.areaNext =
			portalAreas[i].lightRefs.areaPrev =
				&portalAreas[i].lightRefs;
		portalAreas[i].entityRefs.areaNext =
			portalAreas[i].entityRefs.areaPrev =
				&portalAreas[i].entityRefs;
	}
}

/*
================
idRenderWorldLocal::ParseInterAreaPortals
================
*/
void idRenderWorldLocal::ParseInterAreaPortals( idLexer* src, idFile* fileOut )
{
	src->ExpectTokenString( "{" );
	
	numPortalAreas = src->ParseInt();
	if( numPortalAreas < 0 )
	{
		src->Error( "R_ParseInterAreaPortals: bad numPortalAreas" );
		return;
	}
	
	if( fileOut != NULL )
	{
		// write out the type so the binary reader knows what to instantiate
		fileOut->WriteString( "interAreaPortals" );
	}
	
	
	portalAreas = ( portalArea_t* )R_ClearedStaticAlloc( numPortalAreas * sizeof( portalAreas[0] ) );
	areaScreenRect = ( idScreenRect* ) R_ClearedStaticAlloc( numPortalAreas * sizeof( idScreenRect ) );
	
	// set the doubly linked lists
	SetupAreaRefs();
	
	numInterAreaPortals = src->ParseInt();
	if( numInterAreaPortals < 0 )
	{
		src->Error( "R_ParseInterAreaPortals: bad numInterAreaPortals" );
		return;
	}
	
	if( fileOut != NULL )
	{
		fileOut->WriteBig( numPortalAreas );
		fileOut->WriteBig( numInterAreaPortals );
	}
	
	doublePortals = ( doublePortal_t* )R_ClearedStaticAlloc( numInterAreaPortals *
					sizeof( doublePortals [0] ) );
					
	for( int i = 0; i < numInterAreaPortals; i++ )
	{
		int		numPoints, a1, a2;
		idWinding*	w;
		portal_t*	p;
		
		numPoints = src->ParseInt();
		a1 = src->ParseInt();
		a2 = src->ParseInt();
		
		if( fileOut != NULL )
		{
			fileOut->WriteBig( numPoints );
			fileOut->WriteBig( a1 );
			fileOut->WriteBig( a2 );
		}
		
		w = new idWinding( numPoints );
		w->SetNumPoints( numPoints );
		for( int j = 0; j < numPoints; j++ )
		{
			src->Parse1DMatrix( 3, ( *w )[j].ToFloatPtr() );
			
			if( fileOut != NULL )
			{
				fileOut->WriteBig( ( *w )[j].x );
				fileOut->WriteBig( ( *w )[j].y );
				fileOut->WriteBig( ( *w )[j].z );
			}
			// no texture coordinates
			( *w )[j][3] = 0;
			( *w )[j][4] = 0;
		}
		
		// add the portal to a1
		p = ( portal_t* )R_ClearedStaticAlloc( sizeof( *p ) );
		p->intoArea = a2;
		p->doublePortal = &doublePortals[i];
		p->w = w;
		p->w->GetPlane( p->plane );
		
		p->next = portalAreas[a1].portals;
		portalAreas[a1].portals = p;
		
		doublePortals[i].portals[0] = p;
		
		// reverse it for a2
		p = ( portal_t* )R_ClearedStaticAlloc( sizeof( *p ) );
		p->intoArea = a1;
		p->doublePortal = &doublePortals[i];
		p->w = w->Reverse();
		p->w->GetPlane( p->plane );
		
		p->next = portalAreas[a2].portals;
		portalAreas[a2].portals = p;
		
		doublePortals[i].portals[1] = p;
	}
	
	src->ExpectTokenString( "}" );
}

/*
================
idRenderWorldLocal::ReadBinaryAreaPortals
================
*/
void idRenderWorldLocal::ReadBinaryAreaPortals( idFile* file )
{

	file->ReadBig( numPortalAreas );
	file->ReadBig( numInterAreaPortals );
	
	portalAreas = ( portalArea_t* )R_ClearedStaticAlloc( numPortalAreas * sizeof( portalAreas[0] ) );
	areaScreenRect = ( idScreenRect* ) R_ClearedStaticAlloc( numPortalAreas * sizeof( idScreenRect ) );
	
	// set the doubly linked lists
	SetupAreaRefs();
	
	doublePortals = ( doublePortal_t* )R_ClearedStaticAlloc( numInterAreaPortals * sizeof( doublePortals [0] ) );
	
	for( int i = 0; i < numInterAreaPortals; i++ )
	{
		int		numPoints, a1, a2;
		idWinding*	w;
		portal_t*	p;
		
		file->ReadBig( numPoints );
		file->ReadBig( a1 );
		file->ReadBig( a2 );
		w = new idWinding( numPoints );
		w->SetNumPoints( numPoints );
		for( int j = 0; j < numPoints; j++ )
		{
			file->ReadBig( ( *w )[ j ][ 0 ] );
			file->ReadBig( ( *w )[ j ][ 1 ] );
			file->ReadBig( ( *w )[ j ][ 2 ] );
			// no texture coordinates
			( *w )[ j ][ 3 ] = 0;
			( *w )[ j ][ 4 ] = 0;
		}
		
		// add the portal to a1
		p = ( portal_t* )R_ClearedStaticAlloc( sizeof( *p ) );
		p->intoArea = a2;
		p->doublePortal = &doublePortals[i];
		p->w = w;
		p->w->GetPlane( p->plane );
		
		p->next = portalAreas[a1].portals;
		portalAreas[a1].portals = p;
		
		doublePortals[i].portals[0] = p;
		
		// reverse it for a2
		p = ( portal_t* )R_ClearedStaticAlloc( sizeof( *p ) );
		p->intoArea = a1;
		p->doublePortal = &doublePortals[i];
		p->w = w->Reverse();
		p->w->GetPlane( p->plane );
		
		p->next = portalAreas[a2].portals;
		portalAreas[a2].portals = p;
		
		doublePortals[i].portals[1] = p;
	}
}

/*
================
idRenderWorldLocal::ParseLightGridPoints
================
*/
// RB begin
void idRenderWorldLocal::ParseLightGridPoints( idLexer* src, idFile* fileOut )
{
	src->ExpectTokenString( "{" );
	
	int numLightGridPoints = src->ParseInt();
	if( numLightGridPoints < 0 )
	{
		src->Error( "ParseLightGridPoints: bad numLightGridPoints" );
		return;
	}
	
	if( fileOut != NULL )
	{
		// write out the type so the binary reader knows what to instantiate
		fileOut->WriteString( "lightGridPoints" );
	}
	
	// gridMins
	src->Parse1DMatrix( 3, lightGridOrigin.ToFloatPtr() );
	src->Parse1DMatrix( 3, lightGridSize.ToFloatPtr() );
	for( int i = 0; i < 3; i++ )
	{
		lightGridBounds[i] = src->ParseInt();
	}
	
	lightGridPoints.SetNum( numLightGridPoints );
	
	idLib::Printf( "light grid size (%i %i %i)\n", ( int )lightGridSize[0], ( int )lightGridSize[1], ( int )lightGridSize[2] );
	idLib::Printf( "light grid bounds (%i %i %i)\n", ( int )lightGridBounds[0], ( int )lightGridBounds[1], ( int )lightGridBounds[2] );
	idLib::Printf( "%9u x %u B = lightGridSize = (%.2fMB)\n", numLightGridPoints, sizeof( lightGridPoint_t ), ( float )( lightGridPoints.MemoryUsed() ) / ( 1024.0f * 1024.0f ) );
	
	if( fileOut != NULL )
	{
		fileOut->WriteBig( numLightGridPoints );
		fileOut->WriteBig( lightGridOrigin );
		fileOut->WriteBig( lightGridSize );
		fileOut->WriteBigArray( lightGridBounds, 3 );
	}
	
	for( int i = 0; i < numLightGridPoints; i++ )
	{
		lightGridPoint_t* gridPoint = &lightGridPoints[i];
		
		byte values[8];
		
#if 1
		for( int j = 0; j < 8; j++ )
		{
			int intVal = src->ParseInt();
			intVal = idMath::ClampInt( 0, 255, intVal );
			
			values[j] = ( byte ) intVal;
		}
		
		gridPoint->ambient[0] = values[0];
		gridPoint->ambient[1] = values[1];
		gridPoint->ambient[2] = values[2];
		
		gridPoint->directed[0] = values[3];
		gridPoint->directed[1] = values[4];
		gridPoint->directed[2] = values[5];
		
		gridPoint->latLong[0] = values[6];
		gridPoint->latLong[1] = values[7];
#else
		src->Parse1DMatrix( 3, gridPoint->ambient.ToFloatPtr() );
		src->Parse1DMatrix( 3, gridPoint->directed.ToFloatPtr() );
		src->Parse1DMatrix( 3, gridPoint->dir.ToFloatPtr() );
#endif
		
		if( fileOut != NULL )
		{
			fileOut->WriteBigArray( gridPoint->ambient, 3 );
			fileOut->WriteBigArray( gridPoint->directed, 3 );
			fileOut->WriteBigArray( gridPoint->latLong, 2 );
		}
	}
	
	CalculateLightGridPointPositions();
	
	src->ExpectTokenString( "}" );
}

void idRenderWorldLocal::CalculateLightGridPointPositions()
{
#if !defined(USE_GLES1)
	int             gridStep[3];
	int             pos[3];
	idVec3          posFloat;
	
	// calculate grid point positions
	gridStep[0] = 1;
	gridStep[1] = lightGridBounds[0];
	gridStep[2] = lightGridBounds[0] * lightGridBounds[1];
	
	for( int i = 0; i < lightGridBounds[0]; i += 1 )
	{
		for( int j = 0; j < lightGridBounds[1]; j += 1 )
		{
			for( int k = 0; k < lightGridBounds[2]; k += 1 )
			{
				pos[0] = i;
				pos[1] = j;
				pos[2] = k;
				
				posFloat[0] = i * lightGridSize[0];
				posFloat[1] = j * lightGridSize[1];
				posFloat[2] = k * lightGridSize[2];
				
				lightGridPoint_t* gridPoint = &lightGridPoints[ pos[0] * gridStep[0] + pos[1] * gridStep[1] + pos[2] * gridStep[2] ];
				
				gridPoint->origin = lightGridOrigin + posFloat;
			}
		}
	}
#endif
}

// RB end

/*
================
idRenderWorldLocal::ReadBinaryLightGridPoints
================
*/
// RB begin
void idRenderWorldLocal::ReadBinaryLightGridPoints( idFile* file )
{
	int numLightGridPoints = 0;
	file->ReadBig( numLightGridPoints );
	if( numLightGridPoints < 0 )
	{
		idLib::Error( "ReadBinaryLightGridPoints: bad numLightGridPoints" );
		return;
	}
	
	// gridMins
	file->ReadBig( lightGridOrigin );
	file->ReadBig( lightGridSize );
	file->ReadBigArray( lightGridBounds, 3 );
	
	lightGridPoints.SetNum( numLightGridPoints );
	
	idLib::Printf( "light grid size (%i %i %i)\n", ( int )lightGridSize[0], ( int )lightGridSize[1], ( int )lightGridSize[2] );
	idLib::Printf( "light grid bounds (%i %i %i)\n", ( int )lightGridBounds[0], ( int )lightGridBounds[1], ( int )lightGridBounds[2] );
	idLib::Printf( "%9u x %u B = lightGridSize = (%.2fMB)\n", numLightGridPoints, sizeof( lightGridPoint_t ), ( float )( lightGridPoints.MemoryUsed() ) / ( 1024.0f * 1024.0f ) );
	
	for( int i = 0; i < numLightGridPoints; i++ )
	{
		lightGridPoint_t* gridPoint = &lightGridPoints[i];
		
		file->ReadBigArray( gridPoint->ambient, 3 );
		file->ReadBigArray( gridPoint->directed, 3 );
		file->ReadBigArray( gridPoint->latLong, 2 );
	}
	
	CalculateLightGridPointPositions();
}
// RB end


/*
================
idRenderWorldLocal::ParseNodes
================
*/
void idRenderWorldLocal::ParseNodes( idLexer* src, idFile* fileOut )
{
	src->ExpectTokenString( "{" );
	
	numAreaNodes = src->ParseInt();
	if( numAreaNodes < 0 )
	{
		src->Error( "R_ParseNodes: bad numAreaNodes" );
	}
	areaNodes = ( areaNode_t* )R_ClearedStaticAlloc( numAreaNodes * sizeof( areaNodes[0] ) );
	
	if( fileOut != NULL )
	{
		// write out the type so the binary reader knows what to instantiate
		fileOut->WriteString( "nodes" );
	}
	
	if( fileOut != NULL )
	{
		fileOut->WriteBig( numAreaNodes );
	}
	
	for( int i = 0; i < numAreaNodes; i++ )
	{
		areaNode_t*	node;
		
		node = &areaNodes[i];
		
		src->Parse1DMatrix( 4, node->plane.ToFloatPtr() );
		
		node->children[0] = src->ParseInt();
		node->children[1] = src->ParseInt();
		
		if( fileOut != NULL )
		{
			fileOut->WriteBig( node->plane[ 0 ] );
			fileOut->WriteBig( node->plane[ 1 ] );
			fileOut->WriteBig( node->plane[ 2 ] );
			fileOut->WriteBig( node->plane[ 3 ] );
			fileOut->WriteBig( node->children[ 0 ] );
			fileOut->WriteBig( node->children[ 1 ] );
		}
		
	}
	
	src->ExpectTokenString( "}" );
}

/*
================
idRenderWorldLocal::ReadBinaryNodes
================
*/
void idRenderWorldLocal::ReadBinaryNodes( idFile* file )
{
	file->ReadBig( numAreaNodes );
	areaNodes = ( areaNode_t* )R_ClearedStaticAlloc( numAreaNodes * sizeof( areaNodes[0] ) );
	for( int i = 0; i < numAreaNodes; i++ )
	{
		areaNode_t* node = &areaNodes[ i ];
		file->ReadBig( node->plane[ 0 ] );
		file->ReadBig( node->plane[ 1 ] );
		file->ReadBig( node->plane[ 2 ] );
		file->ReadBig( node->plane[ 3 ] );
		file->ReadBig( node->children[ 0 ] );
		file->ReadBig( node->children[ 1 ] );
	}
}

/*
================
idRenderWorldLocal::CommonChildrenArea_r
================
*/
int idRenderWorldLocal::CommonChildrenArea_r( areaNode_t* node )
{
	int	nums[2];
	
	for( int i = 0; i < 2; i++ )
	{
		if( node->children[i] <= 0 )
		{
			nums[i] = -1 - node->children[i];
		}
		else
		{
			nums[i] = CommonChildrenArea_r( &areaNodes[ node->children[i] ] );
		}
	}
	
	// solid nodes will match any area
	if( nums[0] == AREANUM_SOLID )
	{
		nums[0] = nums[1];
	}
	if( nums[1] == AREANUM_SOLID )
	{
		nums[1] = nums[0];
	}
	
	int	common;
	if( nums[0] == nums[1] )
	{
		common = nums[0];
	}
	else
	{
		common = CHILDREN_HAVE_MULTIPLE_AREAS;
	}
	
	node->commonChildrenArea = common;
	
	return common;
}

/*
=================
idRenderWorldLocal::ClearWorld

Sets up for a single area world
=================
*/
void idRenderWorldLocal::ClearWorld()
{
	numPortalAreas = 1;
	portalAreas = ( portalArea_t* )R_ClearedStaticAlloc( sizeof( portalAreas[0] ) );
	areaScreenRect = ( idScreenRect* ) R_ClearedStaticAlloc( sizeof( idScreenRect ) );
	
	SetupAreaRefs();
	
	// even though we only have a single area, create a node
	// that has both children pointing at it so we don't need to
	//
	areaNodes = ( areaNode_t* )R_ClearedStaticAlloc( sizeof( areaNodes[0] ) );
	areaNodes[0].plane[3] = 1;
	areaNodes[0].children[0] = -1;
	areaNodes[0].children[1] = -1;
}

/*
=================
idRenderWorldLocal::FreeDefs

dump all the interactions
=================
*/
void idRenderWorldLocal::FreeDefs()
{
	generateAllInteractionsCalled = false;
	
	if( interactionTable )
	{
		R_StaticFree( interactionTable );
		interactionTable = NULL;
	}
	
	// free all lightDefs
	for( int i = 0; i < lightDefs.Num(); i++ )
	{
		idRenderLightLocal* light = lightDefs[i];
		if( light != NULL && light->world == this )
		{
			FreeLightDef( i );
			lightDefs[i] = NULL;
		}
	}
	
	// free all entityDefs
	for( int i = 0; i < entityDefs.Num(); i++ )
	{
		idRenderEntityLocal*	 mod = entityDefs[i];
		if( mod != NULL && mod->world == this )
		{
			FreeEntityDef( i );
			entityDefs[i] = NULL;
		}
	}
}

/*
=================
idRenderWorldLocal::InitFromMap

A NULL or empty name will make a world without a map model, which
is still useful for displaying a bare model
=================
*/
bool idRenderWorldLocal::InitFromMap( const char* name )
{
	idLexer* 		src;
	idToken			token;
	idRenderModel* 	lastModel;
	
	// if this is an empty world, initialize manually
	if( !name || !name[0] )
	{
		FreeWorld();
		mapName.Clear();
		ClearWorld();
		return true;
	}
	
	// load it
	idStrStatic< MAX_OSPATH > filename = name;
	filename.SetFileExtension( PROC_FILE_EXT );
	
	// check for generated ASCII file
	idStrStatic< MAX_OSPATH > generatedASCIIFileName = filename;
	generatedASCIIFileName.Insert( "generated/", 0 );
	
	// check for generated binary file
	idStrStatic< MAX_OSPATH > generatedFileName = filename;
	generatedFileName.Insert( "generated/", 0 );
	generatedFileName.SetFileExtension( "bproc" );
	
	// if we are reloading the same map, check the timestamp
	// and try to skip all the work
	
	// RB: look for generated/maps/*.proc first
	ID_TIME_T sourceTimeStamp = fileSystem->GetTimestamp( generatedASCIIFileName );
	if( sourceTimeStamp != FILE_NOT_FOUND_TIMESTAMP )
	{
		filename = generatedASCIIFileName;
	}
	else
	{
		// try maps/*.proc without the generated/
		sourceTimeStamp = fileSystem->GetTimestamp( filename );
	}
	
	if( name == mapName )
	{
		if( sourceTimeStamp != FILE_NOT_FOUND_TIMESTAMP && sourceTimeStamp == mapTimeStamp )
		{
			common->Printf( "idRenderWorldLocal::InitFromMap: retaining existing map\n" );
			FreeDefs();
			TouchWorldModels();
			AddWorldModelEntities();
			ClearPortalStates();
			return true;
		}
		common->Printf( "idRenderWorldLocal::InitFromMap: timestamp has changed, reloading.\n" );
	}
	
	FreeWorld();
	
	
	// see if we have a generated version of this
	static const byte BPROC_VERSION = 5;
	static const unsigned int BPROC_MAGIC = ( 'P' << 24 ) | ( 'R' << 16 ) | ( 'O' << 8 ) | BPROC_VERSION;
	bool loaded = false;
	
#if 1
	
	// RB: don't waste memory on low memory systems
#if defined(__ANDROID__)
	idFileLocal file( fileSystem->OpenFileRead( generatedFileName ) );
#else
	idFileLocal file( fileSystem->OpenFileReadMemory( generatedFileName ) );
#endif
	
	if( file != NULL )
	{
		int numEntries = 0;
		int magic = 0;
		
		file->ReadBig( magic );
		file->ReadBig( mapTimeStamp );
		
		// RB: added extra time stamp check
		if( magic == BPROC_MAGIC && sourceTimeStamp == mapTimeStamp )
		{
			common->Printf( "idRenderWorldLocal::InitFromMap: loading binary file '%s'\n", generatedFileName.c_str() );
			
			file->ReadBig( numEntries );
			file->ReadString( mapName );
			
			// if we are writing a demo, archive the load command
			if( session->writeDemo )
			{
				WriteLoadMap();
			}
			
			loaded = true;
			for( int i = 0; i < numEntries; i++ )
			{
				idStrStatic< MAX_OSPATH > type;
				file->ReadString( type );
				type.ToLower();
				if( type == "model" )
				{
					idRenderModel* lastModel = ReadBinaryModel( file );
					if( lastModel == NULL )
					{
						loaded = false;
						break;
					}
					renderModelManager->AddModel( lastModel );
					localModels.Append( lastModel );
				}
				else if( type == "shadowmodel" )
				{
					idRenderModel* lastModel = ReadBinaryModel( file );
					if( lastModel == NULL )
					{
						loaded = false;
						break;
					}
					renderModelManager->AddModel( lastModel );
					localModels.Append( lastModel );
				}
				else if( type == "interareaportals" )
				{
					ReadBinaryAreaPortals( file );
				}
				else if( type == "nodes" )
				{
					ReadBinaryNodes( file );
				}
				// RB begin
				else if( type == "lightgridpoints" )
				{
					ReadBinaryLightGridPoints( file );
				}
				// RB end
				else
				{
					idLib::Error( "Binary proc file failed, unexpected type %s\n", type.c_str() );
				}
			}
		}
	}
#endif
	
	if( !loaded )
	{
		src = new idLexer( filename, LEXFL_NOSTRINGCONCAT | LEXFL_NODOLLARPRECOMPILE );
		if( !src->IsLoaded() )
		{
			common->Printf( "idRenderWorldLocal::InitFromMap: %s not found\n", filename.c_str() );
			ClearWorld();
			return false;
		}
		
		mapName = name;
		mapTimeStamp = sourceTimeStamp;
		
		// if we are writing a demo, archive the load command
		if( session->writeDemo )
		{
			WriteLoadMap();
		}
		
		// RB: added PROC_FILE_ID2
		if( !src->ReadToken( &token ) || ( token.Icmp( PROC_FILE_ID ) && token.Icmp( PROC_FILE_ID2 ) ) )
			// RB end
		{
			common->Printf( "idRenderWorldLocal::InitFromMap: bad id '%s' instead of '%s'\n", token.c_str(), PROC_FILE_ID );
			delete src;
			return false;
		}
		
		// RB begin
		int procVersion = 3;
		
		if( !token.Icmp( PROC_FILE_ID2 ) )
		{
			procVersion = PROC_EXT_VERSION;
		}
		// RB end
		
		int numEntries = 0;
		idFileLocal outputFile( fileSystem->OpenFileWrite( generatedFileName, "fs_basepath" ) );
		if( outputFile != NULL )
		{
			int magic = BPROC_MAGIC;
			outputFile->WriteBig( magic );
			outputFile->WriteBig( mapTimeStamp );
			outputFile->WriteBig( numEntries );
			outputFile->WriteString( mapName );
		}
		
		// parse the file
		while( 1 )
		{
			if( !src->ReadToken( &token ) )
			{
				break;
			}
			
			session->PacifierUpdate();
			
			if( token == "model" )
			{
				lastModel = ParseModel( src, name, sourceTimeStamp, outputFile, procVersion );
				
				// add it to the model manager list
				renderModelManager->AddModel( lastModel );
				
				// save it in the list to free when clearing this map
				localModels.Append( lastModel );
				
				numEntries++;
				
				continue;
			}
			
			if( token == "shadowModel" )
			{
				lastModel = ParseShadowModel( src, outputFile );
				
				// add it to the model manager list
				renderModelManager->AddModel( lastModel );
				
				// save it in the list to free when clearing this map
				localModels.Append( lastModel );
				
				numEntries++;
				continue;
			}
			
			if( token == "interAreaPortals" )
			{
				ParseInterAreaPortals( src, outputFile );
				
				numEntries++;
				continue;
			}
			
			// RB begin
			if( token == "lightGridPoints" )
			{
				ParseLightGridPoints( src, outputFile );
				
				numEntries++;
				continue;
			}
			// RB end
			
			if( token == "nodes" )
			{
				ParseNodes( src, outputFile );
				
				numEntries++;
				continue;
			}
			
			src->Error( "idRenderWorldLocal::InitFromMap: bad token \"%s\"", token.c_str() );
		}
		
		delete src;
		
		if( outputFile != NULL )
		{
			outputFile->Seek( 0, FS_SEEK_SET );
			int magic = BPROC_MAGIC;
			outputFile->WriteBig( magic );
			outputFile->WriteBig( mapTimeStamp );
			outputFile->WriteBig( numEntries );
		}
	}
	
	// if it was a trivial map without any areas, create a single area
	if( !numPortalAreas )
	{
		ClearWorld();
	}
	
	// find the points where we can early-our of reference pushing into the BSP tree
	CommonChildrenArea_r( &areaNodes[0] );
	
	AddWorldModelEntities();
	ClearPortalStates();
	
	// done!
	return true;
}

/*
=====================
idRenderWorldLocal::ClearPortalStates
=====================
*/
void idRenderWorldLocal::ClearPortalStates()
{
	// all portals start off open
	for( int i = 0; i < numInterAreaPortals; i++ )
	{
		doublePortals[i].blockingBits = PS_BLOCK_NONE;
	}
	
	// flood fill all area connections
	for( int i = 0; i < numPortalAreas; i++ )
	{
		for( int j = 0; j < NUM_PORTAL_ATTRIBUTES; j++ )
		{
			connectedAreaNum++;
			FloodConnectedAreas( &portalAreas[i], j );
		}
	}
}

/*
=====================
idRenderWorldLocal::AddWorldModelEntities
=====================
*/
void idRenderWorldLocal::AddWorldModelEntities()
{
	// add the world model for each portal area
	// we can't just call AddEntityDef, because that would place the references
	// based on the bounding box, rather than explicitly into the correct area
	for( int i = 0; i < numPortalAreas; i++ )
	{
		session->PacifierUpdate();
		
		
		idRenderEntityLocal* def = new idRenderEntityLocal;
		
		// try and reuse a free spot
		int index = entityDefs.FindNull();
		if( index == -1 )
		{
			index = entityDefs.Append( def );
		}
		else
		{
			entityDefs[index] = def;
		}
		
		def->index = index;
		def->world = this;
		
		def->parms.hModel = renderModelManager->FindModel( va( "_area%i", i ) );
		if( def->parms.hModel->IsDefaultModel() || !def->parms.hModel->IsStaticWorldModel() )
		{
			common->Error( "idRenderWorldLocal::InitFromMap: bad area model lookup" );
		}
		
		idRenderModel* hModel = def->parms.hModel;
		
		for( int j = 0; j < hModel->NumSurfaces(); j++ )
		{
			const modelSurface_t* surf = hModel->Surface( j );
			
			if( surf->shader->GetName() == idStr( "textures/smf/portal_sky" ) )
			{
				def->needsPortalSky = true;
			}
		}
		
		def->referenceBounds = def->parms.hModel->Bounds();
		
		def->parms.axis[0][0] = 1;
		def->parms.axis[1][1] = 1;
		def->parms.axis[2][2] = 1;
		
		R_AxisToModelMatrix( def->parms.axis, def->parms.origin, def->modelMatrix );
		
		// in case an explicit shader is used on the world, we don't
		// want it to have a 0 alpha or color
		def->parms.shaderParms[0] =
			def->parms.shaderParms[1] =
				def->parms.shaderParms[2] =
					def->parms.shaderParms[3] = 1;
					
		AddEntityRefToArea( def, &portalAreas[i] );
	}
}

/*
=====================
CheckAreaForPortalSky
=====================
*/
bool idRenderWorldLocal::CheckAreaForPortalSky( int areaNum )
{
	assert( areaNum >= 0 && areaNum < numPortalAreas );
	
	for( areaReference_t* ref = portalAreas[areaNum].entityRefs.areaNext; ref->entity; ref = ref->areaNext )
	{
		assert( ref->area == &portalAreas[areaNum] );
		
		if( ref->entity && ref->entity->needsPortalSky )
		{
			return true;
		}
	}
	
	return false;
}
