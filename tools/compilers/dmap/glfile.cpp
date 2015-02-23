/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
Copyright (C) 2015 Robert Beckebans

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


struct OBJFace
{
	OBJFace()
	{
		material = nullptr;
	}
	
	const idMaterial*			material;
	idList<idDrawVert>			verts;
	idList<triIndex_t>			indexes;
};


int PortalVisibleSides( uPortal_t* p )
{
	int		fcon, bcon;
	
	if( !p->onnode )
		return 0;		// outside
		
	fcon = p->nodes[0]->opaque;
	bcon = p->nodes[1]->opaque;
	
	// same contents never create a face
	if( fcon == bcon )
		return 0;
		
	if( !fcon )
		return 1;
	if( !bcon )
		return 2;
		
	return 0;
}

void OutputWinding( idWinding* w, idList<OBJFace>& faces )
{
	static	int	level = 128;
	float		light;
	int			i;
	
	//glview->WriteFloatString( "%i\n", w->GetNumPoints() );
	level += 28;
	light = ( level & 255 ) / 255.0;
	
	OBJFace& face = faces.Alloc();
	
	//for( i = 0; i < w->GetNumPoints(); i++ )
	
	for( i = w->GetNumPoints() - 1; i >= 0; i-- )
	{
		idDrawVert& dv = face.verts.Alloc();
		
		dv.xyz.x = ( *w )[i][0];
		dv.xyz.y = ( *w )[i][1];
		dv.xyz.z = ( *w )[i][2];
		
		dv.SetColor( level & 255 );
		
		//dv.SetNormal( w->GetPlane() )
		
		/*
		glview->WriteFloatString( "%6.3f %6.3f %6.3f %6.3f %6.3f %6.3f\n",
								  ( *w )[i][0],
								  ( *w )[i][1],
								  ( *w )[i][2],
								  light,
								  light,
								  light );*/
	}
	
	//glview->WriteFloatString( "\n" );
}

/*
=============
OutputPortal
=============
*/
void OutputPortal( uPortal_t* p, idList<OBJFace>& faces )
{
	idWinding*	w;
	int		sides;
	
	sides = PortalVisibleSides( p );
	if( !sides )
	{
		return;
	}
	
	//c_glfaces++;
	
	w = p->winding;
	
	if( sides == 2 )  		// back side
	{
		w = w->Reverse();
	}
	
	OutputWinding( w, faces );
	
	if( sides == 2 )
	{
		delete w;
	}
}

/*
=============
WriteGLView_r
=============
*/
void WriteGLView_r( node_t* node, idList<OBJFace>& faces )
{
	uPortal_t*	p, *nextp;
	
	if( node->planenum != PLANENUM_LEAF )
	{
		WriteGLView_r( node->children[0], faces );
		WriteGLView_r( node->children[1], faces );
		return;
	}
	
	// write brush list
	
	
	// write all the portals
	for( p = node->portals; p; p = nextp )
	{
		if( p->nodes[0] == node )
		{
			OutputPortal( p, faces );
			nextp = p->next[0];
		}
		else
		{
			nextp = p->next[1];
		}
	}
}

/*
=============
WriteGLView
=============
*/
void WriteGLView( tree_t* tree, const char* source )
{
	//c_glfaces = 0;
	//common->Printf( "Writing %s\n", source );
	
	if( dmapGlobals.entityNum != 0 )
	{
		return;
	}
	
	idStrStatic< MAX_OSPATH > path;
	path.Format( "generated/%s_BSP_%s_%i.obj", dmapGlobals.mapFileBase, source, dmapGlobals.entityNum );
	idFileLocal objFile( fileSystem->OpenFileWrite( path, "fs_basepath" ) );
	
	path.SetFileExtension( ".mtl" );
	idFileLocal mtlFile( fileSystem->OpenFileWrite( path, "fs_basepath" ) );
	
	idList<OBJFace> faces;
	WriteGLView_r( tree->headnode, faces );
	
	common->Printf( "%5i c_glfaces\n", faces.Num() );
	
	int numVerts = 0;
	
	for( int i = 0; i < faces.Num(); i++ )
	{
		OBJFace& face = faces[i];
		
		for( int j = 0; j < face.verts.Num(); j++ )
		{
			const idVec3& v = face.verts[j].xyz;
			
			objFile->Printf( "v %1.6f %1.6f %1.6f\n", v.x, v.y, v.z );
		}
		
		/*
		for( int j = 0; j < face.verts.Num(); j++ )
		{
			const idVec2& vST = face.verts[j].GetTexCoord();
		
			objFile->Printf( "vt %1.6f %1.6f\n", vST.x, vST.y );
		}
		
		for( int j = 0; j < face.verts.Num(); j++ )
		{
			const idVec3& n = face.verts[j].GetNormal();
		
			objFile->Printf( "vn %1.6f %1.6f %1.6f\n", n.x, n.y, n.z );
		}
		
		objFile->Printf( "usemtl %s\n", face.material->GetName() );
		*/
		
		//for( int j = 0; j < face.indexes.Num(); j += 3 )
		
		objFile->Printf( "f " );
		for( int j = 0; j < face.verts.Num(); j++ )
		{
			objFile->Printf( "%i// ", numVerts + 1 + j );
		}
		
		numVerts += face.verts.Num();
		
		objFile->Printf( "\n\n" );
	}
}

