/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

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
#include "Model_local.h"

static const char* MD5_SnapshotName = "_MD5_Snapshot_";

static const byte MD5B_VERSION = 107;
static const unsigned int MD5B_MAGIC = ( '5' << 24 ) | ( 'D' << 16 ) | ( 'M' << 8 ) | MD5B_VERSION;


/***********************************************************************

	idMD5Mesh

***********************************************************************/

static int c_numVerts = 0;
static int c_numWeights = 0;
static int c_numWeightJoints = 0;

struct vertexWeight_t
{
	int							vert;
	int							joint;
	idVec3						offset;
	float						jointWeight;
};

/*
====================
idMD5Mesh::idMD5Mesh
====================
*/
idMD5Mesh::idMD5Mesh()
{
	scaledWeights	= NULL;
	weightIndex		= NULL;
	shader			= NULL;
	numTris			= 0;
	deformInfo		= NULL;
	surfaceNum		= 0;
}

/*
====================
idMD5Mesh::~idMD5Mesh
====================
*/
idMD5Mesh::~idMD5Mesh()
{
	Mem_Free16( scaledWeights );
	Mem_Free16( weightIndex );
	if( deformInfo != NULL )
	{
		R_FreeDeformInfo( deformInfo );
		deformInfo = NULL;
	}
}

/*
====================
idMD5Mesh::ParseMesh
====================
*/
void idMD5Mesh::ParseMesh( idLexer& parser, int numJoints, const idJointMat* joints )
{
	idToken		token;
	idToken		name;
	
	parser.ExpectTokenString( "{" );
	
	//
	// parse name
	//
	if( parser.CheckTokenString( "name" ) )
	{
		parser.ReadToken( &name );
	}
	
	//
	// parse shader
	//
	parser.ExpectTokenString( "shader" );
	
	parser.ReadToken( &token );
	idStr shaderName = token;
	
	shader = declManager->FindMaterial( shaderName );
	
	//
	// parse texture coordinates
	//
	parser.ExpectTokenString( "numverts" );
	int count = parser.ParseInt();
	if( count < 0 )
	{
		parser.Error( "Invalid size: %s", token.c_str() );
	}
	
	//this->numVerts = count;
	
	//idList<idVec2> texCoords;
	idList<int> firstWeightForVertex;
	idList<int> numWeightsForVertex;
	
	texCoords.SetNum( count );
	firstWeightForVertex.SetNum( count );
	numWeightsForVertex.SetNum( count );
	
	numWeights = 0;
	int maxweight = 0;
	for( int i = 0; i < texCoords.Num(); i++ )
	{
		parser.ExpectTokenString( "vert" );
		parser.ParseInt();
		
		parser.Parse1DMatrix( 2, texCoords[ i ].ToFloatPtr() );
		
		firstWeightForVertex[ i ]	= parser.ParseInt();
		numWeightsForVertex[ i ]	= parser.ParseInt();
		
		if( !numWeightsForVertex[ i ] )
		{
			parser.Error( "Vertex without any joint weights." );
		}
		
		numWeights += numWeightsForVertex[ i ];
		if( numWeightsForVertex[ i ] + firstWeightForVertex[ i ] > maxweight )
		{
			maxweight = numWeightsForVertex[ i ] + firstWeightForVertex[ i ];
		}
	}
	
	//
	// parse tris
	//
	parser.ExpectTokenString( "numtris" );
	count = parser.ParseInt();
	if( count < 0 )
	{
		parser.Error( "Invalid size: %d", count );
	}
	
	idList<int> tris;
	tris.SetNum( count * 3 );
	numTris = count;
	for( int i = 0; i < count; i++ )
	{
		parser.ExpectTokenString( "tri" );
		parser.ParseInt();
		
		tris[ i * 3 + 0 ] = parser.ParseInt();
		tris[ i * 3 + 1 ] = parser.ParseInt();
		tris[ i * 3 + 2 ] = parser.ParseInt();
	}
	
	//
	// parse weights
	//
	parser.ExpectTokenString( "numweights" );
	count = parser.ParseInt();
	if( count < 0 )
	{
		parser.Error( "Invalid size: %d", count );
	}
	
	if( maxweight > count )
	{
		parser.Warning( "Vertices reference out of range weights in model (%d of %d weights).", maxweight, count );
	}
	
	idList<vertexWeight_t> tempWeights;
	tempWeights.SetNum( count );
	assert( numJoints < 256 );		// so we can pack into bytes
	
	for( int i = 0; i < count; i++ )
	{
		parser.ExpectTokenString( "weight" );
		parser.ParseInt();
		
		int jointnum = parser.ParseInt();
		if( ( jointnum < 0 ) || ( jointnum >= numJoints ) )
		{
			parser.Error( "Joint Index out of range(%d): %d", numJoints, jointnum );
		}
		
		tempWeights[ i ].joint			= jointnum;
		tempWeights[ i ].jointWeight	= parser.ParseFloat();
		
		parser.Parse1DMatrix( 3, tempWeights[ i ].offset.ToFloatPtr() );
	}
	
	// create pre-scaled weights and an index for the vertex/joint lookup
	scaledWeights = ( idVec4* ) Mem_Alloc16( numWeights * sizeof( scaledWeights[0] ) );
	weightIndex = ( int* ) Mem_Alloc16( numWeights * 2 * sizeof( weightIndex[0] ) );
	memset( weightIndex, 0, numWeights * 2 * sizeof( weightIndex[0] ) );
	
	count = 0;
	for( int i = 0; i < texCoords.Num(); i++ )
	{
		int num = firstWeightForVertex[i];
		for( int j = 0; j < numWeightsForVertex[i]; j++, num++, count++ )
		{
			scaledWeights[count].ToVec3() = tempWeights[num].offset * tempWeights[num].jointWeight;
			scaledWeights[count].w = tempWeights[num].jointWeight;
			weightIndex[count * 2 + 0] = tempWeights[num].joint * sizeof( idJointMat );
		}
		weightIndex[count * 2 - 1] = 1;
	}
	
	parser.ExpectTokenString( "}" );
	
	// update counters
	c_numVerts += texCoords.Num();
	c_numWeights += numWeights;
	c_numWeightJoints++;
	for( int i = 0; i < numWeights; i++ )
	{
		c_numWeightJoints += weightIndex[i * 2 + 1];
	}
	
	//
	// build the information that will be common to all animations of this mesh:
	// silhouette edge connectivity and normal / tangent generation information
	//
	idDrawVert* verts = ( idDrawVert* ) _alloca16( texCoords.Num() * sizeof( idDrawVert ) );
	for( int i = 0; i < texCoords.Num(); i++ )
	{
		verts[i].Clear();
		verts[i].st = texCoords[i];
	}
	TransformVerts( verts, joints );
	deformInfo = R_BuildDeformInfo( texCoords.Num(), verts, tris.Num(), tris.Ptr(), shader->UseUnsmoothedTangents() );
}

/*
====================
idMD5Mesh::TransformVerts
====================
*/
void idMD5Mesh::TransformVerts( idDrawVert* verts, const idJointMat* entJoints )
{
	SIMDProcessor->TransformVerts( verts, texCoords.Num(), entJoints, scaledWeights, weightIndex, numWeights );
}

/*
====================
idMD5Mesh::TransformScaledVerts

Special transform to make the mesh seem fat or skinny.  May be used for zombie deaths
====================
*/
void idMD5Mesh::TransformScaledVerts( idDrawVert* verts, const idJointMat* entJoints, float scale )
{
	idVec4* scaledWeights = ( idVec4* ) _alloca16( numWeights * sizeof( scaledWeights[0] ) );
	SIMDProcessor->Mul( scaledWeights[0].ToFloatPtr(), scale, scaledWeights[0].ToFloatPtr(), numWeights * 4 );
	SIMDProcessor->TransformVerts( verts, texCoords.Num(), entJoints, scaledWeights, weightIndex, numWeights );
}

/*
====================
idMD5Mesh::UpdateSurface
====================
*/
void idMD5Mesh::UpdateSurface( const struct renderEntity_s* ent, const idJointMat* entJoints, modelSurface_t* surf )
{
	int i, base;
	srfTriangles_t* tri;
	
	tr.pc.c_deformedSurfaces++;
	tr.pc.c_deformedVerts += deformInfo->numOutputVerts;
	tr.pc.c_deformedIndexes += deformInfo->numIndexes;
	
	surf->shader = shader;
	
	if( surf->geometry )
	{
		// if the number of verts and indexes are the same we can re-use the triangle surface
		// the number of indexes must be the same to assure the correct amount of memory is allocated for the facePlanes
		if( surf->geometry->numVerts == deformInfo->numOutputVerts && surf->geometry->numIndexes == deformInfo->numIndexes )
		{
			R_FreeStaticTriSurfVertexCaches( surf->geometry );
		}
		else
		{
			R_FreeStaticTriSurf( surf->geometry );
			surf->geometry = R_AllocStaticTriSurf();
		}
	}
	else
	{
		surf->geometry = R_AllocStaticTriSurf();
	}
	
	tri = surf->geometry;
	
	// note that some of the data is references, and should not be freed
	tri->deformedSurface = true;
	tri->tangentsCalculated = false;
	tri->facePlanesCalculated = false;
	
	tri->numIndexes = deformInfo->numIndexes;
	tri->indexes = deformInfo->indexes;
	tri->silIndexes = deformInfo->silIndexes;
	tri->numMirroredVerts = deformInfo->numMirroredVerts;
	tri->mirroredVerts = deformInfo->mirroredVerts;
	tri->numDupVerts = deformInfo->numDupVerts;
	tri->dupVerts = deformInfo->dupVerts;
	tri->numSilEdges = deformInfo->numSilEdges;
	tri->silEdges = deformInfo->silEdges;
	tri->dominantTris = deformInfo->dominantTris;
	tri->numVerts = deformInfo->numOutputVerts;
	
	if( tri->verts == NULL )
	{
		R_AllocStaticTriSurfVerts( tri, tri->numVerts );
		for( i = 0; i < deformInfo->numSourceVerts; i++ )
		{
			tri->verts[i].Clear();
			tri->verts[i].st = texCoords[i];
		}
	}
	
	if( ent->shaderParms[ SHADERPARM_MD5_SKINSCALE ] != 0.0f )
	{
		TransformScaledVerts( tri->verts, entJoints, ent->shaderParms[ SHADERPARM_MD5_SKINSCALE ] );
	}
	else
	{
		TransformVerts( tri->verts, entJoints );
	}
	
	// replicate the mirror seam vertexes
	base = deformInfo->numOutputVerts - deformInfo->numMirroredVerts;
	for( i = 0; i < deformInfo->numMirroredVerts; i++ )
	{
		tri->verts[base + i] = tri->verts[deformInfo->mirroredVerts[i]];
	}
	
	R_BoundTriSurf( tri );
	
	// If a surface is going to be have a lighting interaction generated, it will also have to call
	// R_DeriveTangents() to get normals, tangents, and face planes.  If it only
	// needs shadows generated, it will only have to generate face planes.  If it only
	// has ambient drawing, or is culled, no additional work will be necessary
	if( !r_useDeferredTangents.GetBool() )
	{
		// set face planes, vertex normals, tangents
		R_DeriveTangents( tri );
	}
}

/*
====================
idMD5Mesh::CalcBounds
====================
*/
idBounds idMD5Mesh::CalcBounds( const idJointMat* entJoints )
{
	idBounds	bounds;
	idDrawVert* verts = ( idDrawVert* ) _alloca16( texCoords.Num() * sizeof( idDrawVert ) );
	
	TransformVerts( verts, entJoints );
	
	SIMDProcessor->MinMax( bounds[0], bounds[1], verts, texCoords.Num() );
	
	return bounds;
}

/*
====================
idMD5Mesh::NearestJoint
====================
*/
int idMD5Mesh::NearestJoint( int a, int b, int c ) const
{
	int i, bestJoint, vertNum, weightVertNum;
	float bestWeight;
	
	// duplicated vertices might not have weights
	if( a >= 0 && a < texCoords.Num() )
	{
		vertNum = a;
	}
	else if( b >= 0 && b < texCoords.Num() )
	{
		vertNum = b;
	}
	else if( c >= 0 && c < texCoords.Num() )
	{
		vertNum = c;
	}
	else
	{
		// all vertices are duplicates which shouldn't happen
		return 0;
	}
	
	// find the first weight for this vertex
	weightVertNum = 0;
	for( i = 0; weightVertNum < vertNum; i++ )
	{
		weightVertNum += weightIndex[i * 2 + 1];
	}
	
	// get the joint for the largest weight
	bestWeight = scaledWeights[i].w;
	bestJoint = weightIndex[i * 2 + 0] / sizeof( idJointMat );
	for( ; weightIndex[i * 2 + 1] == 0; i++ )
	{
		if( scaledWeights[i].w > bestWeight )
		{
			bestWeight = scaledWeights[i].w;
			bestJoint = weightIndex[i * 2 + 0] / sizeof( idJointMat );
		}
	}
	return bestJoint;
}

/*
====================
idMD5Mesh::NumVerts
====================
*/
int idMD5Mesh::NumVerts() const
{
	return texCoords.Num();
}

/*
====================
idMD5Mesh::NumTris
====================
*/
int	idMD5Mesh::NumTris() const
{
	return numTris;
}

/*
====================
idMD5Mesh::NumWeights
====================
*/
int	idMD5Mesh::NumWeights() const
{
	return numWeights;
}

/***********************************************************************

	idRenderModelMD5

***********************************************************************/

/*
====================
idRenderModelMD5::ParseJoint
====================
*/
void idRenderModelMD5::ParseJoint( idLexer& parser, idMD5Joint* joint, idJointQuat* defaultPose )
{
	//
	// parse name
	//
	idToken	token;
	parser.ReadToken( &token );
	joint->name = token;
	
	//
	// parse parent
	//
	int num = parser.ParseInt();
	if( num < 0 )
	{
		joint->parent = NULL;
	}
	else
	{
		if( num >= joints.Num() - 1 )
		{
			parser.Error( "Invalid parent for joint '%s'", joint->name.c_str() );
		}
		joint->parent = &joints[ num ];
	}
	
	//
	// parse default pose
	//
	parser.Parse1DMatrix( 3, defaultPose->t.ToFloatPtr() );
	parser.Parse1DMatrix( 3, defaultPose->q.ToFloatPtr() );
	defaultPose->q.w = defaultPose->q.CalcW();
}

/*
====================
idRenderModelMD5::InitFromFile
====================
*/
void idRenderModelMD5::InitFromFile( const char* fileName )
{
	name = fileName;
	LoadModel();
}

/*
========================
idRenderModelMD5::LoadBinaryModel
========================
*/
bool idRenderModelMD5::LoadBinaryModel( idFile* file, const ID_TIME_T sourceTimeStamp )
{

	if( !idRenderModelStatic::LoadBinaryModel( file, sourceTimeStamp ) )
	{
		return false;
	}
	
	unsigned int magic = 0;
	file->ReadBig( magic );
	if( magic != MD5B_MAGIC )
	{
		return false;
	}
	
	int tempNum;
	file->ReadBig( tempNum );
	joints.SetNum( tempNum );
	for( int i = 0; i < joints.Num(); i++ )
	{
		file->ReadString( joints[i].name );
		int offset;
		file->ReadBig( offset );
		if( offset >= 0 )
		{
			joints[i].parent = joints.Ptr() + offset;
		}
		else
		{
			joints[i].parent = NULL;
		}
	}
	
	file->ReadBig( tempNum );
	defaultPose.SetNum( tempNum );
	for( int i = 0; i < defaultPose.Num(); i++ )
	{
		file->ReadBig( defaultPose[i].q.x );
		file->ReadBig( defaultPose[i].q.y );
		file->ReadBig( defaultPose[i].q.z );
		file->ReadBig( defaultPose[i].q.w );
		file->ReadVec3( defaultPose[i].t );
	}
	
#if defined(USE_GPU_SKINNING)
	file->ReadBig( tempNum );
	invertedDefaultPose.SetNum( tempNum );
	for( int i = 0; i < invertedDefaultPose.Num(); i++ )
	{
		file->ReadBigArray( invertedDefaultPose[ i ].ToFloatPtr(), JOINTMAT_TYPESIZE );
	}
	SIMD_INIT_LAST_JOINT( invertedDefaultPose.Ptr(), joints.Num() );
#endif
	
	file->ReadBig( tempNum );
	meshes.SetNum( tempNum );
	for( int i = 0; i < meshes.Num(); i++ )
	{
	
		idStr materialName;
		file->ReadString( materialName );
		if( materialName.IsEmpty() )
		{
			meshes[i].shader = NULL;
		}
		else
		{
			meshes[i].shader = declManager->FindMaterial( materialName );
		}
		
		// RB begin
		file->ReadBig( tempNum );
		meshes[i].texCoords.SetNum( tempNum );
		
		for( int j = 0; j < meshes[i].texCoords.Num(); j++ )
		{
			file->ReadBig( meshes[i].texCoords[j] );
		}
		
		file->ReadBig( meshes[i].numWeights );
		
		if( meshes[i].numWeights > 0 )
		{
			meshes[i].scaledWeights = ( idVec4* ) Mem_Alloc16( meshes[i].numWeights * sizeof( meshes[i].scaledWeights[0] ) );
			meshes[i].weightIndex = ( int* ) Mem_Alloc16( meshes[i].numWeights * 2 * sizeof( meshes[i].weightIndex[0] ) );
			
			file->ReadBigArray( meshes[i].scaledWeights, meshes[i].numWeights );
			file->ReadBigArray( meshes[i].weightIndex, meshes[i].numWeights * 2 );
		}
		// RB end
		
#if defined(USE_GPU_SKINNING)
		file->ReadBig( meshes[i].numVerts );
#endif
		file->ReadBig( meshes[i].numTris );
		
		/*
		file->ReadBig( meshes[i].numMeshJoints );
		meshes[i].meshJoints = ( byte* ) Mem_Alloc( meshes[i].numMeshJoints * sizeof( meshes[i].meshJoints[0] ) );
		file->ReadBigArray( meshes[i].meshJoints, meshes[i].numMeshJoints );
		file->ReadBig( meshes[i].maxJointVertDist );
		*/
		
		meshes[i].deformInfo = ( deformInfo_t* )R_ClearedStaticAlloc( sizeof( deformInfo_t ) );
		deformInfo_t& deform = *meshes[i].deformInfo;
		
		file->ReadBig( deform.numSourceVerts );
		file->ReadBig( deform.numOutputVerts );
		file->ReadBig( deform.numIndexes );
		file->ReadBig( deform.numMirroredVerts );
		file->ReadBig( deform.numDupVerts );
		file->ReadBig( deform.numSilEdges );
		
		srfTriangles_t	tri;
		memset( &tri, 0, sizeof( srfTriangles_t ) );
		
		if( deform.numOutputVerts > 0 )
		{
			R_AllocStaticTriSurfVerts( &tri, deform.numOutputVerts );
			//deform.verts = tri.verts;
			//file->ReadBigArray( deform.verts, deform.numOutputVerts );
		}
		
		if( deform.numIndexes > 0 )
		{
			R_AllocStaticTriSurfIndexes( &tri, deform.numIndexes );
			R_AllocStaticTriSurfSilIndexes( &tri, deform.numIndexes );
			deform.indexes = tri.indexes;
			deform.silIndexes = tri.silIndexes;
			file->ReadBigArray( deform.indexes, deform.numIndexes );
			file->ReadBigArray( deform.silIndexes, deform.numIndexes );
		}
		
		if( deform.numMirroredVerts > 0 )
		{
			R_AllocStaticTriSurfMirroredVerts( &tri, deform.numMirroredVerts );
			deform.mirroredVerts = tri.mirroredVerts;
			file->ReadBigArray( deform.mirroredVerts, deform.numMirroredVerts );
		}
		
		if( deform.numDupVerts > 0 )
		{
			R_AllocStaticTriSurfDupVerts( &tri, deform.numDupVerts );
			deform.dupVerts = tri.dupVerts;
			file->ReadBigArray( deform.dupVerts, deform.numDupVerts * 2 );
		}
		
		if( deform.numSilEdges > 0 )
		{
			R_AllocStaticTriSurfSilEdges( &tri, deform.numSilEdges );
			deform.silEdges = tri.silEdges;
			assert( deform.silEdges != NULL );
			for( int j = 0; j < deform.numSilEdges; j++ )
			{
				file->ReadBig( deform.silEdges[j].p1 );
				file->ReadBig( deform.silEdges[j].p2 );
				file->ReadBig( deform.silEdges[j].v1 );
				file->ReadBig( deform.silEdges[j].v2 );
			}
		}
		
		bool temp;
		file->ReadBig( temp );
		tri.dominantTris = NULL;
		if( temp )
		{
			R_AllocStaticTriSurfDominantTris( &tri, deform.numSourceVerts );
			assert( tri.dominantTris != NULL );
			for( int j = 0; j < deform.numSourceVerts; j++ )
			{
				file->ReadBig( tri.dominantTris[j].v2 );
				file->ReadBig( tri.dominantTris[j].v3 );
				file->ReadFloat( tri.dominantTris[j].normalizationScale[0] );
				file->ReadFloat( tri.dominantTris[j].normalizationScale[1] );
				file->ReadFloat( tri.dominantTris[j].normalizationScale[2] );
			}
		}
		
		/*
		idShadowVertSkinned* shadowVerts = ( idShadowVertSkinned* ) Mem_Alloc( ALIGN( deform.numOutputVerts * 2 * sizeof( idShadowVertSkinned ), 16 ), TAG_MODEL );
		idShadowVertSkinned::CreateShadowCache( shadowVerts, deform.verts, deform.numOutputVerts );
		
		deform.staticAmbientCache = vertexCache.AllocStaticVertex( deform.verts, ALIGN( deform.numOutputVerts * sizeof( idDrawVert ), VERTEX_CACHE_ALIGN ) );
		deform.staticIndexCache = vertexCache.AllocStaticIndex( deform.indexes, ALIGN( deform.numIndexes * sizeof( triIndex_t ), INDEX_CACHE_ALIGN ) );
		deform.staticShadowCache = vertexCache.AllocStaticVertex( shadowVerts, ALIGN( deform.numOutputVerts * 2 * sizeof( idShadowVertSkinned ), VERTEX_CACHE_ALIGN ) );
		
		Mem_Free( shadowVerts );
		*/
		
		file->ReadBig( meshes[i].surfaceNum );
	}
	
	return true;
}

/*
========================
idRenderModelMD5::WriteBinaryModel
========================
*/
void idRenderModelMD5::WriteBinaryModel( idFile* file, ID_TIME_T* _timeStamp ) const
{

	idRenderModelStatic::WriteBinaryModel( file );
	
	if( file == NULL )
	{
		return;
	}
	
	file->WriteBig( MD5B_MAGIC );
	
	file->WriteBig( joints.Num() );
	for( int i = 0; i < joints.Num(); i++ )
	{
		file->WriteString( joints[i].name );
		int offset = -1;
		if( joints[i].parent != NULL )
		{
			offset = joints[i].parent - joints.Ptr();
		}
		file->WriteBig( offset );
	}
	
	file->WriteBig( defaultPose.Num() );
	for( int i = 0; i < defaultPose.Num(); i++ )
	{
		file->WriteBig( defaultPose[i].q.x );
		file->WriteBig( defaultPose[i].q.y );
		file->WriteBig( defaultPose[i].q.z );
		file->WriteBig( defaultPose[i].q.w );
		file->WriteVec3( defaultPose[i].t );
	}
	
#if defined(USE_GPU_SKINNING)
	file->WriteBig( invertedDefaultPose.Num() );
	for( int i = 0; i < invertedDefaultPose.Num(); i++ )
	{
		file->WriteBigArray( invertedDefaultPose[ i ].ToFloatPtr(), JOINTMAT_TYPESIZE );
	}
#endif
	
	file->WriteBig( meshes.Num() );
	for( int i = 0; i < meshes.Num(); i++ )
	{
	
		if( meshes[i].shader != NULL && meshes[i].shader->GetName() != NULL )
		{
			file->WriteString( meshes[i].shader->GetName() );
		}
		else
		{
			file->WriteString( "" );
		}
		
		// RB begin
		file->WriteBig( meshes[i].texCoords.Num() );
		for( int j = 0; j < meshes[i].texCoords.Num(); j++ )
		{
			file->WriteBig( meshes[i].texCoords[j] );
		}
		
		file->WriteBig( meshes[i].numWeights );
		
		if( meshes[i].numWeights > 0 )
		{
			file->WriteBigArray( meshes[i].scaledWeights, meshes[i].numWeights );
			file->WriteBigArray( meshes[i].weightIndex, meshes[i].numWeights * 2 );
		}
		// RB end
		
#if defined(USE_GPU_SKINNING)
		file->WriteBig( meshes[i].numVerts );
#endif
		file->WriteBig( meshes[i].numTris );
		
#if defined(USE_GPU_SKINNING)
		file->WriteBig( meshes[i].numMeshJoints );
		file->WriteBigArray( meshes[i].meshJoints, meshes[i].numMeshJoints );
		file->WriteBig( meshes[i].maxJointVertDist );
#endif
		
		deformInfo_t& deform = *meshes[i].deformInfo;
		
		file->WriteBig( deform.numSourceVerts );
		file->WriteBig( deform.numOutputVerts );
		file->WriteBig( deform.numIndexes );
		file->WriteBig( deform.numMirroredVerts );
		file->WriteBig( deform.numDupVerts );
		file->WriteBig( deform.numSilEdges );
		
		if( deform.numOutputVerts > 0 )
		{
			//file->WriteBigArray( deform.verts, deform.numOutputVerts );
		}
		
		if( deform.numIndexes > 0 )
		{
			file->WriteBigArray( deform.indexes, deform.numIndexes );
			file->WriteBigArray( deform.silIndexes, deform.numIndexes );
		}
		
		if( deform.numMirroredVerts > 0 )
		{
			file->WriteBigArray( deform.mirroredVerts, deform.numMirroredVerts );
		}
		
		if( deform.numDupVerts > 0 )
		{
			file->WriteBigArray( deform.dupVerts, deform.numDupVerts * 2 );
		}
		
		if( deform.numSilEdges > 0 )
		{
			for( int j = 0; j < deform.numSilEdges; j++ )
			{
				file->WriteBig( deform.silEdges[j].p1 );
				file->WriteBig( deform.silEdges[j].p2 );
				file->WriteBig( deform.silEdges[j].v1 );
				file->WriteBig( deform.silEdges[j].v2 );
			}
		}
		
		file->WriteBig( deform.dominantTris != NULL );
		if( deform.dominantTris != NULL )
		{
			for( int j = 0; j < deform.numSourceVerts; j++ )
			{
				file->WriteBig( deform.dominantTris[j].v2 );
				file->WriteBig( deform.dominantTris[j].v3 );
				file->WriteFloat( deform.dominantTris[j].normalizationScale[0] );
				file->WriteFloat( deform.dominantTris[j].normalizationScale[1] );
				file->WriteFloat( deform.dominantTris[j].normalizationScale[2] );
			}
		}
		
		file->WriteBig( meshes[i].surfaceNum );
	}
}

/*
====================
idRenderModelMD5::LoadModel

used for initial loads, reloadModel, and reloading the data of purged models
Upon exit, the model will absolutely be valid, but possibly as a default model
====================
*/
void idRenderModelMD5::LoadModel()
{

	int			version;
	int			num;
	int			parentNum;
	idToken		token;
	idLexer		parser( LEXFL_ALLOWPATHNAMES | LEXFL_NOSTRINGESCAPECHARS );
	
	if( !purged )
	{
		PurgeModel();
	}
	purged = false;
	
	if( !parser.LoadFile( name ) )
	{
		MakeDefaultModel();
		return;
	}
	
	parser.ExpectTokenString( MD5_VERSION_STRING );
	version = parser.ParseInt();
	
	if( version != MD5_VERSION )
	{
		parser.Error( "Invalid version %d.  Should be version %d\n", version, MD5_VERSION );
	}
	
	//
	// skip commandline
	//
	parser.ExpectTokenString( "commandline" );
	parser.ReadToken( &token );
	
	// parse num joints
	parser.ExpectTokenString( "numJoints" );
	num  = parser.ParseInt();
	joints.SetGranularity( 1 );
	joints.SetNum( num );
	defaultPose.SetGranularity( 1 );
	defaultPose.SetNum( num );
	
	// parse num meshes
	parser.ExpectTokenString( "numMeshes" );
	num = parser.ParseInt();
	if( num < 0 )
	{
		parser.Error( "Invalid size: %d", num );
	}
	meshes.SetGranularity( 1 );
	meshes.SetNum( num );
	
	//
	// parse joints
	//
	parser.ExpectTokenString( "joints" );
	parser.ExpectTokenString( "{" );
	idJointMat* poseMat = ( idJointMat* )_alloca16( joints.Num() * sizeof( poseMat[0] ) );
	for( int i = 0; i < joints.Num(); i++ )
	{
		idMD5Joint* joint = &joints[i];
		idJointQuat*	 pose = &defaultPose[i];
		
		ParseJoint( parser, joint, pose );
		poseMat[ i ].SetRotation( pose->q.ToMat3() );
		poseMat[ i ].SetTranslation( pose->t );
		if( joint->parent )
		{
			parentNum = joint->parent - joints.Ptr();
			pose->q = ( poseMat[ i ].ToMat3() * poseMat[ parentNum ].ToMat3().Transpose() ).ToQuat();
			pose->t = ( poseMat[ i ].ToVec3() - poseMat[ parentNum ].ToVec3() ) * poseMat[ parentNum ].ToMat3().Transpose();
		}
	}
	parser.ExpectTokenString( "}" );
	
	//-----------------------------------------
	// create the inverse of the base pose joints to support tech6 style deformation
	// of base pose vertexes, normals, and tangents.
	//
	// vertex * joints * inverseJoints == vertex when joints is the base pose
	// When the joints are in another pose, it gives the animated vertex position
	//-----------------------------------------
#if defined(USE_GPU_SKINNING)
	invertedDefaultPose.SetNum( SIMD_ROUND_JOINTS( joints.Num() ) );
	for( int i = 0; i < joints.Num(); i++ )
	{
		invertedDefaultPose[i] = poseMat[i];
		invertedDefaultPose[i].Invert();
	}
	SIMD_INIT_LAST_JOINT( invertedDefaultPose.Ptr(), joints.Num() );
#endif
	
	for( int i = 0; i < meshes.Num(); i++ )
	{
		parser.ExpectTokenString( "mesh" );
		meshes[i].ParseMesh( parser, defaultPose.Num(), poseMat );
	}
	
	// calculate the bounds of the model
	CalculateBounds( poseMat );
	
	// set the timestamp for reloadmodels
	fileSystem->ReadFile( name, NULL, &timeStamp );
}

/*
==============
idRenderModelMD5::Print
==============
*/
void idRenderModelMD5::Print() const
{
	common->Printf( "%s\n", name.c_str() );
	common->Printf( "Dynamic model.\n" );
	common->Printf( "Generated smooth normals.\n" );
	common->Printf( "    verts  tris weights material\n" );
	int	totalVerts = 0;
	int	totalTris = 0;
	int	totalWeights = 0;
	const idMD5Mesh* mesh = meshes.Ptr();
	for( int i = 0; i < meshes.Num(); i++, mesh++ )
	{
		totalVerts += mesh->NumVerts();
		totalTris += mesh->NumTris();
		totalWeights += mesh->NumWeights();
		common->Printf( "%2i: %5i %5i %7i %s\n", i, mesh->NumVerts(), mesh->NumTris(), mesh->NumWeights(), mesh->shader->GetName() );
	}
	common->Printf( "-----\n" );
	common->Printf( "%4i verts.\n", totalVerts );
	common->Printf( "%4i tris.\n", totalTris );
	common->Printf( "%4i weights.\n", totalWeights );
	common->Printf( "%4i joints.\n", joints.Num() );
}

/*
==============
idRenderModelMD5::List
==============
*/
void idRenderModelMD5::List() const
{
	int			i;
	const idMD5Mesh*	mesh;
	int			totalTris = 0;
	int			totalVerts = 0;
	
	for( mesh = meshes.Ptr(), i = 0; i < meshes.Num(); i++, mesh++ )
	{
		totalTris += mesh->numTris;
		totalVerts += mesh->NumVerts();
	}
	common->Printf( " %4ik %3i %4i %4i %s(MD5)", Memory() / 1024, meshes.Num(), totalVerts, totalTris, Name() );
	
	if( defaulted )
	{
		common->Printf( " (DEFAULTED)" );
	}
	
	common->Printf( "\n" );
}

/*
====================
idRenderModelMD5::CalculateBounds
====================
*/
void idRenderModelMD5::CalculateBounds( const idJointMat* entJoints )
{
	int			i;
	idMD5Mesh*	mesh;
	
	bounds.Clear();
	for( mesh = meshes.Ptr(), i = 0; i < meshes.Num(); i++, mesh++ )
	{
		bounds.AddBounds( mesh->CalcBounds( entJoints ) );
	}
}

/*
====================
idRenderModelMD5::Bounds

This calculates a rough bounds by using the joint radii without
transforming all the points
====================
*/
idBounds idRenderModelMD5::Bounds( const renderEntity_t* ent ) const
{
#if 0
	// we can't calculate a rational bounds without an entity,
	// because joints could be positioned to deform it into an
	// arbitrarily large shape
	if( !ent )
	{
		common->Error( "idRenderModelMD5::Bounds: called without entity" );
	}
#endif
	
	if( !ent )
	{
		// this is the bounds for the reference pose
		return bounds;
	}
	
	return ent->bounds;
}

/*
====================
idRenderModelMD5::DrawJoints
====================
*/
void idRenderModelMD5::DrawJoints( const renderEntity_t* ent, const viewDef_t* view ) const
{
	int					i;
	int					num;
	idVec3				pos;
	const idJointMat*	joint;
	const idMD5Joint*	md5Joint;
	int					parentNum;
	
	num = ent->numJoints;
	joint = ent->joints;
	md5Joint = joints.Ptr();
	for( i = 0; i < num; i++, joint++, md5Joint++ )
	{
		pos = ent->origin + joint->ToVec3() * ent->axis;
		if( md5Joint->parent )
		{
			parentNum = md5Joint->parent - joints.Ptr();
			session->rw->DebugLine( colorWhite, ent->origin + ent->joints[ parentNum ].ToVec3() * ent->axis, pos );
		}
		
		session->rw->DebugLine( colorRed,	pos, pos + joint->ToMat3()[ 0 ] * 2.0f * ent->axis );
		session->rw->DebugLine( colorGreen,	pos, pos + joint->ToMat3()[ 1 ] * 2.0f * ent->axis );
		session->rw->DebugLine( colorBlue,	pos, pos + joint->ToMat3()[ 2 ] * 2.0f * ent->axis );
	}
	
	idBounds bounds;
	
	bounds.FromTransformedBounds( ent->bounds, vec3_zero, ent->axis );
	session->rw->DebugBounds( colorMagenta, bounds, ent->origin );
	
	if( ( r_jointNameScale.GetFloat() != 0.0f ) && ( bounds.Expand( 128.0f ).ContainsPoint( view->renderView.vieworg - ent->origin ) ) )
	{
		idVec3	offset( 0, 0, r_jointNameOffset.GetFloat() );
		float	scale;
		
		scale = r_jointNameScale.GetFloat();
		joint = ent->joints;
		num = ent->numJoints;
		for( i = 0; i < num; i++, joint++ )
		{
			pos = ent->origin + joint->ToVec3() * ent->axis;
			session->rw->DrawText( joints[ i ].name, pos + offset, scale, colorWhite, view->renderView.viewaxis, 1 );
		}
	}
}

/*
====================
idRenderModelMD5::InstantiateDynamicModel
====================
*/
idRenderModel* idRenderModelMD5::InstantiateDynamicModel( const struct renderEntity_s* ent, const struct viewDef_t* view, idRenderModel* cachedModel )
{



	if( cachedModel != NULL && !r_useCachedDynamicModels.GetBool() )
	{
		delete cachedModel;
		cachedModel = NULL;
	}
	
	if( purged )
	{
		common->DWarning( "model %s instantiated while purged", Name() );
		LoadModel();
	}
	
	if( !ent->joints )
	{
		common->Printf( "idRenderModelMD5::InstantiateDynamicModel: NULL joints on renderEntity for '%s'\n", Name() );
		delete cachedModel;
		return NULL;
	}
	else if( ent->numJoints != joints.Num() )
	{
		common->Printf( "idRenderModelMD5::InstantiateDynamicModel: renderEntity has different number of joints than model for '%s'\n", Name() );
		delete cachedModel;
		return NULL;
	}
	
	tr.pc.c_generateMd5++;
	
	idRenderModelStatic* staticModel;
	if( cachedModel != NULL )
	{
		assert( dynamic_cast<idRenderModelStatic*>( cachedModel ) != NULL );
		assert( idStr::Icmp( cachedModel->Name(), MD5_SnapshotName ) == 0 );
		staticModel = static_cast<idRenderModelStatic*>( cachedModel );
	}
	else
	{
		staticModel = new idRenderModelStatic;
		staticModel->InitEmpty( MD5_SnapshotName );
	}
	
	staticModel->bounds.Clear();
	
	if( r_showSkel.GetInteger() )
	{
		if( ( view != NULL ) && ( !r_skipSuppress.GetBool() || !ent->suppressSurfaceInViewID || ( ent->suppressSurfaceInViewID != view->renderView.viewID ) ) )
		{
			// only draw the skeleton
			DrawJoints( ent, view );
		}
		
		if( r_showSkel.GetInteger() > 1 )
		{
			// turn off the model when showing the skeleton
			staticModel->InitEmpty( MD5_SnapshotName );
			return staticModel;
		}
	}
	
	// create all the surfaces
	idMD5Mesh* mesh = meshes.Ptr();
	for( int i = 0; i < meshes.Num(); i++, mesh++ )
	{
		// avoid deforming the surface if it will be a nodraw due to a skin remapping
		// FIXME: may have to still deform clipping hulls
		const idMaterial* shader = mesh->shader;
		
		shader = R_RemapShaderBySkin( shader, ent->customSkin, ent->customShader );
		
		if( !shader || ( !shader->IsDrawn() && !shader->SurfaceCastsShadow() ) )
		{
			staticModel->DeleteSurfaceWithId( i );
			mesh->surfaceNum = -1;
			continue;
		}
		
		modelSurface_t* surf;
		
		int surfaceNum = 0;
		if( staticModel->FindSurfaceWithId( i, surfaceNum ) )
		{
			mesh->surfaceNum = surfaceNum;
			surf = &staticModel->surfaces[surfaceNum];
		}
		else
		{
		
			// Remove Overlays before adding new surfaces
			idRenderModelOverlay::RemoveOverlaySurfacesFromModel( staticModel );
			
			mesh->surfaceNum = staticModel->NumSurfaces();
			surf = &staticModel->surfaces.Alloc();
			surf->geometry = NULL;
			surf->shader = NULL;
			surf->id = i;
		}
		
		mesh->UpdateSurface( ent, ent->joints, surf );
		
		staticModel->bounds.AddPoint( surf->geometry->bounds[0] );
		staticModel->bounds.AddPoint( surf->geometry->bounds[1] );
	}
	
	return staticModel;
}

/*
====================
idRenderModelMD5::IsDynamicModel
====================
*/
dynamicModel_t idRenderModelMD5::IsDynamicModel() const
{
	return DM_CACHED;
}

/*
====================
idRenderModelMD5::NumJoints
====================
*/
int idRenderModelMD5::NumJoints() const
{
	return joints.Num();
}

/*
====================
idRenderModelMD5::GetJoints
====================
*/
const idMD5Joint* idRenderModelMD5::GetJoints() const
{
	return joints.Ptr();
}

/*
====================
idRenderModelMD5::GetDefaultPose
====================
*/
const idJointQuat* idRenderModelMD5::GetDefaultPose() const
{
	return defaultPose.Ptr();
}

/*
====================
idRenderModelMD5::GetJointHandle
====================
*/
jointHandle_t idRenderModelMD5::GetJointHandle( const char* name ) const
{
	const idMD5Joint* joint = joints.Ptr();
	for( int i = 0; i < joints.Num(); i++, joint++ )
	{
		if( idStr::Icmp( joint->name.c_str(), name ) == 0 )
		{
			return ( jointHandle_t )i;
		}
	}
	
	return INVALID_JOINT;
}

/*
=====================
idRenderModelMD5::GetJointName
=====================
*/
const char* idRenderModelMD5::GetJointName( jointHandle_t handle ) const
{
	if( ( handle < 0 ) || ( handle >= joints.Num() ) )
	{
		return "<invalid joint>";
	}
	
	return joints[ handle ].name;
}

/*
====================
idRenderModelMD5::NearestJoint
====================
*/
int idRenderModelMD5::NearestJoint( int surfaceNum, int a, int b, int c ) const
{
	if( surfaceNum > meshes.Num() )
	{
		common->Error( "idRenderModelMD5::NearestJoint: surfaceNum > meshes.Num()" );
	}
	
	const idMD5Mesh* mesh = meshes.Ptr();
	for( int i = 0; i < meshes.Num(); i++, mesh++ )
	{
		if( mesh->surfaceNum == surfaceNum )
		{
			return mesh->NearestJoint( a, b, c );
		}
	}
	return 0;
}

/*
====================
idRenderModelMD5::TouchData

models that are already loaded at level start time
will still touch their materials to make sure they
are kept loaded
====================
*/
void idRenderModelMD5::TouchData()
{
	for( int i = 0; i < meshes.Num(); i++ )
	{
		declManager->FindMaterial( meshes[i].shader->GetName() );
	}
}

/*
===================
idRenderModelMD5::PurgeModel

frees all the data, but leaves the class around for dangling references,
which can regenerate the data with LoadModel()
===================
*/
void idRenderModelMD5::PurgeModel()
{
	purged = true;
	joints.Clear();
	defaultPose.Clear();
	meshes.Clear();
}

/*
===================
idRenderModelMD5::Memory
===================
*/
int	idRenderModelMD5::Memory() const
{
	int total = sizeof( *this );
	total += joints.MemoryUsed() + defaultPose.MemoryUsed() + meshes.MemoryUsed();
	
	// count up strings
	for( int i = 0; i < joints.Num(); i++ )
	{
		total += joints[i].name.DynamicMemoryUsed();
	}
	
	// count up meshes
	for( int i = 0; i < meshes.Num(); i++ )
	{
		const idMD5Mesh* mesh = &meshes[i];
		
		total += mesh->texCoords.MemoryUsed() + mesh->numWeights * ( sizeof( mesh->scaledWeights[0] ) + sizeof( mesh->weightIndex[0] ) * 2 );
		
		// sum up deform info
		total += sizeof( mesh->deformInfo );
		total += R_DeformInfoMemoryUsed( mesh->deformInfo );
	}
	return total;
}
