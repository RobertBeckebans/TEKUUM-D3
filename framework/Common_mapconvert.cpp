/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2015 Robert Beckebans

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

#include "precompiled.h"
#pragma hdrstop

static int totalVerts = 0;
static int totalIndexes = 0;

static void ConvertBrush( idFile* file, const idMapBrush* mapBrush, int primitiveNum )
{
	file->Printf( "o Primitive.%i\n", primitiveNum );
	
	// fix degenerate planes
	idPlane* planes = ( idPlane* ) _alloca16( mapBrush->GetNumSides() * sizeof( planes[0] ) );
	for( int i = 0; i < mapBrush->GetNumSides(); i++ )
	{
		planes[i] = mapBrush->GetSide( i )->GetPlane();
		planes[i].FixDegeneracies( DEGENERATE_DIST_EPSILON );
	}
	
	//idFixedWinding w;
	idList<idFixedWinding> planeWindings;
	idBounds bounds;
	bounds.Clear();
	
	int numVerts = 0;
	int numIndexes = 0;
	
	bool badBrush = false;
	
	for( int i = 0; i < mapBrush->GetNumSides(); i++ )
	{
		idMapBrushSide* mapSide = mapBrush->GetSide( i );
		
		//material = declManager->FindMaterial( mapSide->GetMaterial() );
		//contents |= ( material->GetContentFlags() & CONTENTS_REMOVE_UTIL );
		
		// chop base plane by other brush sides
		idFixedWinding& w = planeWindings.Alloc();
		w.BaseForPlane( -planes[i] );
		
		if( !w.GetNumPoints() )
		{
			badBrush = true;
		}
		
		for( int j = 0; j < mapBrush->GetNumSides() && w.GetNumPoints(); j++ )
		{
			if( i == j )
			{
				continue;
			}
			
			
			if( !w.ClipInPlace( -planes[j], 0 ) )
			{
				// no intersection
				badBrush = true;
				break;
			}
		}
		
		if( w.GetNumPoints() <= 2 )
		{
			continue;
		}
		
		numVerts += w.GetNumPoints();
		numIndexes += ( w.GetNumPoints() - 2 ) * 3;
		
		
		for( int j = 0; j < w.GetNumPoints(); j++ )
		{
			const idVec3& v = w[j].ToVec3();
			bounds.AddPoint( v );
		}
	}
	
	// allocate the surface
	srfTriangles_t tri;
	
	tri.numVerts = 0;
	tri.numIndexes = 0;
	
	tri.verts = ( idDrawVert* )Mem_Alloc16( numVerts * sizeof( idDrawVert ) );
	tri.indexes = ( triIndex_t* )Mem_Alloc16( numIndexes * sizeof( triIndex_t ) );
	
	// copy the data from the windings
	for( int i = 0; i < mapBrush->GetNumSides(); i++ )
	{
		idMapBrushSide* mapSide = mapBrush->GetSide( i );
		
		idFixedWinding& w = planeWindings[i];
		if( !w.GetNumPoints() )
		{
			continue;
		}
		
		for( int j = 0 ; j < w.GetNumPoints() ; j++ )
		{
			const idVec3& v = w[j].ToVec3();
			
			tri.verts[tri.numVerts + j ].Clear();
			tri.verts[tri.numVerts + j ].xyz = v;
		}
		
		for( int j = 1 ; j < w.GetNumPoints() - 1 ; j++ )
		{
			tri.indexes[ tri.numIndexes + 0 ] = tri.numVerts;
			tri.indexes[ tri.numIndexes + 1 ] = tri.numVerts + j;
			tri.indexes[ tri.numIndexes + 2 ] = tri.numVerts + j + 1;
			
			tri.numIndexes += 3;
		}
		
		tri.numVerts += w.GetNumPoints();
	}
	
	for( int j = 0; j < tri.numVerts; j++ )
	{
		const idVec3& v = tri.verts[j].xyz;
		
		file->Printf( "v %1.6f %1.6f %1.6f\n", v.x, v.y, v.z );
	}
	
	if( tri.numIndexes > 0 )
	{
		for( int j = 0; j < tri.numIndexes; j += 3 )
		{
			file->Printf( "f %i/%i/%i %i/%i/%i %i/%i/%i\n",
						  tri.indexes[j + 2] + 1 + totalVerts,
						  tri.indexes[j + 2] + 1 + totalVerts,
						  tri.indexes[j + 2] + 1 + totalVerts,
						  
						  tri.indexes[j + 1] + 1 + totalVerts,
						  tri.indexes[j + 1] + 1 + totalVerts,
						  tri.indexes[j + 1] + 1 + totalVerts,
						  
						  tri.indexes[j + 0] + 1 + totalVerts,
						  tri.indexes[j + 0] + 1 + totalVerts,
						  tri.indexes[j + 0] + 1 + totalVerts );
		}
	}
	
	totalVerts += tri.numVerts;
	
	Mem_Free16( tri.verts );
	Mem_Free16( tri.indexes );
}

CONSOLE_COMMAND( exportMapToOBJ, "Convert .map file to .obj/.mtl ", idCmdSystem::ArgCompletion_MapName )
{
	common->SetRefreshOnPrint( true );
	
	if( args.Argc() != 2 )
	{
		common->Printf( "Usage: exportMapToOBJ <map>\n" );
		return;
	}
	
	idStr filename = args.Argv( 1 );
	if( !filename.Length() )
	{
		return;
	}
	filename.StripFileExtension();
	
	idStr mapName;
	sprintf( mapName, "maps/%s.map", filename.c_str() );
	
	idMapFile map;
	if( map.Parse( mapName, false, false ) )
	{
		idStrStatic< MAX_OSPATH > canonical = mapName;
		canonical.ToLower();
		
		idStrStatic< MAX_OSPATH > extension;
		canonical.ExtractFileExtension( extension );
		
		idStrStatic< MAX_OSPATH > convertedFileName;
		
		convertedFileName = "converted/";
		convertedFileName.AppendPath( canonical );
		convertedFileName.AppendPath( "_converted" );
		convertedFileName.SetFileExtension( ".obj" );
		
		idFileLocal outputFile( fileSystem->OpenFileWrite( convertedFileName, "fs_basepath" ) );
		
		totalVerts = totalIndexes = 0;
		
		int count = map.GetNumEntities();
		for( int j = 0; j < count; j++ )
		{
			idMapEntity* ent = map.GetEntity( j );
			if( ent )
			{
				idStr classname = ent->epairs.GetString( "classname" );
				
				if( classname == "worldspawn" )
				{
					for( int i = 0; i < ent->GetNumPrimitives(); i++ )
					{
						idMapPrimitive*	mapPrim;
						
						mapPrim = ent->GetPrimitive( i );
						if( mapPrim->GetType() == idMapPrimitive::TYPE_BRUSH )
						{
							ConvertBrush( outputFile, static_cast<idMapBrush*>( mapPrim ), i );
							continue;
						}
					}
				}
				
				//Hack: for info_location
				/*
				bool hasLocation = false;
				
				idStrList* list;
				listHash.Get( classname, &list );
				if( list )
				{
					for( int k = 0; k < list->Num(); k++ )
					{
						idStr val = ent->epairs.GetString( ( *list )[k], "" );
				
						if( val.Length() && classname == "info_location" && ( *list )[k] == "location" )
						{
							hasLocation = true;
						}
				
						if( val.Length() && TestMapVal( val ) )
						{
				
							if( !hasLocation || ( *list )[k] == "location" )
							{
								//Localize it!!!
								strCount++;
								ent->epairs.Set( ( *list )[k], langDict.AddString( val ) );
							}
						}
					}
				}
				
				*/
			}
		}
		
	}
	
	common->SetRefreshOnPrint( false );
}