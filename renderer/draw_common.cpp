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

#if !defined(USE_GLES1)
#include "GLShader.h"
#endif

/*
=====================
RB_BakeTextureMatrixIntoTexgen
=====================
*/
void RB_BakeTextureMatrixIntoTexgen( idPlane lightProject[3], const float* textureMatrix )
{
	float	genMatrix[16];
	float	final[16];
	
	genMatrix[0] = lightProject[0][0];
	genMatrix[4] = lightProject[0][1];
	genMatrix[8] = lightProject[0][2];
	genMatrix[12] = lightProject[0][3];
	
	genMatrix[1] = lightProject[1][0];
	genMatrix[5] = lightProject[1][1];
	genMatrix[9] = lightProject[1][2];
	genMatrix[13] = lightProject[1][3];
	
	genMatrix[2] = 0;
	genMatrix[6] = 0;
	genMatrix[10] = 0;
	genMatrix[14] = 0;
	
	genMatrix[3] = lightProject[2][0];
	genMatrix[7] = lightProject[2][1];
	genMatrix[11] = lightProject[2][2];
	genMatrix[15] = lightProject[2][3];
	
	myGlMultMatrix( genMatrix, backEnd.lightTextureMatrix, final );
	
	lightProject[0][0] = final[0];
	lightProject[0][1] = final[4];
	lightProject[0][2] = final[8];
	lightProject[0][3] = final[12];
	
	lightProject[1][0] = final[1];
	lightProject[1][1] = final[5];
	lightProject[1][2] = final[9];
	lightProject[1][3] = final[13];
}

/*
================
RB_PrepareStageTexturing
================
*/
void RB_PrepareStageTexturing( const shaderStage_t* pStage,  const drawSurf_t* surf, idDrawVert* ac )
{
	// set privatePolygonOffset if necessary
	if( pStage->privatePolygonOffset )
	{
		glEnable( GL_POLYGON_OFFSET_FILL );
		glPolygonOffset( r_offsetFactor.GetFloat(), r_offsetUnits.GetFloat() * pStage->privatePolygonOffset );
	}
	
	// set the texture matrix if needed
	if( pStage->texture.hasMatrix )
	{
		RB_LoadShaderTextureMatrix( surf->shaderRegisters, &pStage->texture );
	}
	
	// texgens
	if( pStage->texture.texgen == TG_DIFFUSE_CUBE )
	{
		glTexCoordPointer( 3, GL_FLOAT, sizeof( idDrawVert ), ac->normal.ToFloatPtr() );
	}
	if( pStage->texture.texgen == TG_SKYBOX_CUBE || pStage->texture.texgen == TG_WOBBLESKY_CUBE )
	{
		glTexCoordPointer( 3, GL_FLOAT, 0, vertexCache.Position( surf->dynamicTexCoords ) );
	}
	
#if !defined(USE_GLES1)
	if( pStage->texture.texgen == TG_SCREEN )
	{
		glEnable( GL_TEXTURE_GEN_S );
		glEnable( GL_TEXTURE_GEN_T );
		glEnable( GL_TEXTURE_GEN_Q );
		
		float	mat[16], plane[4];
		myGlMultMatrix( surf->space->modelViewMatrix, backEnd.viewDef->projectionMatrix, mat );
		
		plane[0] = mat[0];
		plane[1] = mat[4];
		plane[2] = mat[8];
		plane[3] = mat[12];
		glTexGenfv( GL_S, GL_OBJECT_PLANE, plane );
		
		plane[0] = mat[1];
		plane[1] = mat[5];
		plane[2] = mat[9];
		plane[3] = mat[13];
		glTexGenfv( GL_T, GL_OBJECT_PLANE, plane );
		
		plane[0] = mat[3];
		plane[1] = mat[7];
		plane[2] = mat[11];
		plane[3] = mat[15];
		glTexGenfv( GL_Q, GL_OBJECT_PLANE, plane );
	}
	
	if( pStage->texture.texgen == TG_SCREEN2 )
	{
		glEnable( GL_TEXTURE_GEN_S );
		glEnable( GL_TEXTURE_GEN_T );
		glEnable( GL_TEXTURE_GEN_Q );
		
		float	mat[16], plane[4];
		myGlMultMatrix( surf->space->modelViewMatrix, backEnd.viewDef->projectionMatrix, mat );
		
		plane[0] = mat[0];
		plane[1] = mat[4];
		plane[2] = mat[8];
		plane[3] = mat[12];
		glTexGenfv( GL_S, GL_OBJECT_PLANE, plane );
		
		plane[0] = mat[1];
		plane[1] = mat[5];
		plane[2] = mat[9];
		plane[3] = mat[13];
		glTexGenfv( GL_T, GL_OBJECT_PLANE, plane );
		
		plane[0] = mat[3];
		plane[1] = mat[7];
		plane[2] = mat[11];
		plane[3] = mat[15];
		glTexGenfv( GL_Q, GL_OBJECT_PLANE, plane );
	}
	
	if( pStage->texture.texgen == TG_GLASSWARP )
	{
		// RB begin
		if( tr.backEndRenderer == BE_ARB2 || tr.backEndRenderer == BE_GLSL || tr.backEndRenderer == BE_EXP )
		{
			// RB end
			glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, FPROG_GLASSWARP );
			glEnable( GL_FRAGMENT_PROGRAM_ARB );
			
			GL_SelectTexture( 2 );
			globalImages->scratchImage->Bind();
			
			GL_SelectTexture( 1 );
			globalImages->scratchImage2->Bind();
			
			glEnable( GL_TEXTURE_GEN_S );
			glEnable( GL_TEXTURE_GEN_T );
			glEnable( GL_TEXTURE_GEN_Q );
			
			float	mat[16], plane[4];
			myGlMultMatrix( surf->space->modelViewMatrix, backEnd.viewDef->projectionMatrix, mat );
			
			plane[0] = mat[0];
			plane[1] = mat[4];
			plane[2] = mat[8];
			plane[3] = mat[12];
			glTexGenfv( GL_S, GL_OBJECT_PLANE, plane );
			
			plane[0] = mat[1];
			plane[1] = mat[5];
			plane[2] = mat[9];
			plane[3] = mat[13];
			glTexGenfv( GL_T, GL_OBJECT_PLANE, plane );
			
			plane[0] = mat[3];
			plane[1] = mat[7];
			plane[2] = mat[11];
			plane[3] = mat[15];
			glTexGenfv( GL_Q, GL_OBJECT_PLANE, plane );
			
			GL_SelectTexture( 0 );
		}
	}
	
	if( pStage->texture.texgen == TG_REFLECT_CUBE )
	{
		// RB begin
		if( tr.backEndRenderer == BE_ARB2 || tr.backEndRenderer == BE_GLSL || tr.backEndRenderer == BE_EXP )
		{
			// RB end
			// see if there is also a bump map specified
			const shaderStage_t* bumpStage = surf->material->GetBumpStage();
			if( bumpStage )
			{
				// per-pixel reflection mapping with bump mapping
				GL_SelectTexture( 1 );
				bumpStage->texture.image->Bind();
				GL_SelectTexture( 0 );
				
				glNormalPointer( GL_FLOAT, sizeof( idDrawVert ), ac->normal.ToFloatPtr() );
				glVertexAttribPointerARB( 10, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->tangents[1].ToFloatPtr() );
				glVertexAttribPointerARB( 9, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->tangents[0].ToFloatPtr() );
				
				glEnableVertexAttribArrayARB( 9 );
				glEnableVertexAttribArrayARB( 10 );
				glEnableClientState( GL_NORMAL_ARRAY );
				
				// Program env 5, 6, 7, 8 have been set in RB_SetProgramEnvironmentSpace
				
				glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, FPROG_BUMPY_ENVIRONMENT );
				glEnable( GL_FRAGMENT_PROGRAM_ARB );
				glBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_BUMPY_ENVIRONMENT );
				glEnable( GL_VERTEX_PROGRAM_ARB );
			}
			else
			{
				// per-pixel reflection mapping without a normal map
				glNormalPointer( GL_FLOAT, sizeof( idDrawVert ), ac->normal.ToFloatPtr() );
				glEnableClientState( GL_NORMAL_ARRAY );
				
				glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, FPROG_ENVIRONMENT );
				glEnable( GL_FRAGMENT_PROGRAM_ARB );
				glBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_ENVIRONMENT );
				glEnable( GL_VERTEX_PROGRAM_ARB );
			}
		}
		else
		{
			glEnable( GL_TEXTURE_GEN_S );
			glEnable( GL_TEXTURE_GEN_T );
			glEnable( GL_TEXTURE_GEN_R );
			glTexGenf( GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT );
			glTexGenf( GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT );
			glTexGenf( GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT );
			glEnableClientState( GL_NORMAL_ARRAY );
			glNormalPointer( GL_FLOAT, sizeof( idDrawVert ), ac->normal.ToFloatPtr() );
			
			glMatrixMode( GL_TEXTURE );
			float	mat[16];
			
			R_TransposeGLMatrix( backEnd.viewDef->worldSpace.modelViewMatrix, mat );
			
			glLoadMatrixf( mat );
			glMatrixMode( GL_MODELVIEW );
		}
	}
#endif // #if !defined(USE_GLES1)
}

/*
================
RB_FinishStageTexturing
================
*/
void RB_FinishStageTexturing( const shaderStage_t* pStage, const drawSurf_t* surf, idDrawVert* ac )
{
	// unset privatePolygonOffset if necessary
	if( pStage->privatePolygonOffset && !surf->material->TestMaterialFlag( MF_POLYGONOFFSET ) )
	{
		glDisable( GL_POLYGON_OFFSET_FILL );
	}
	
	if( pStage->texture.texgen == TG_DIFFUSE_CUBE || pStage->texture.texgen == TG_SKYBOX_CUBE
			|| pStage->texture.texgen == TG_WOBBLESKY_CUBE )
	{
		glTexCoordPointer( 2, GL_FLOAT, sizeof( idDrawVert ), ( void* )&ac->st );
	}
	
#if !defined(USE_GLES1)
	if( pStage->texture.texgen == TG_SCREEN )
	{
		glDisable( GL_TEXTURE_GEN_S );
		glDisable( GL_TEXTURE_GEN_T );
		glDisable( GL_TEXTURE_GEN_Q );
	}
	if( pStage->texture.texgen == TG_SCREEN2 )
	{
		glDisable( GL_TEXTURE_GEN_S );
		glDisable( GL_TEXTURE_GEN_T );
		glDisable( GL_TEXTURE_GEN_Q );
	}
	
	if( pStage->texture.texgen == TG_GLASSWARP )
	{
		if( tr.backEndRenderer == BE_ARB2 || tr.backEndRenderer == BE_GLSL || tr.backEndRenderer == BE_EXP )
		{
			GL_SelectTexture( 2 );
			globalImages->BindNull();
			
			GL_SelectTexture( 1 );
			if( pStage->texture.hasMatrix )
			{
				RB_LoadShaderTextureMatrix( surf->shaderRegisters, &pStage->texture );
			}
			glDisable( GL_TEXTURE_GEN_S );
			glDisable( GL_TEXTURE_GEN_T );
			glDisable( GL_TEXTURE_GEN_Q );
			glDisable( GL_FRAGMENT_PROGRAM_ARB );
			globalImages->BindNull();
			GL_SelectTexture( 0 );
		}
	}
	
	if( pStage->texture.texgen == TG_REFLECT_CUBE )
	{
		// RB begin
		if( tr.backEndRenderer == BE_ARB2 || tr.backEndRenderer == BE_GLSL || tr.backEndRenderer == BE_EXP )
		{
			// RB end
			// see if there is also a bump map specified
			const shaderStage_t* bumpStage = surf->material->GetBumpStage();
			if( bumpStage )
			{
				// per-pixel reflection mapping with bump mapping
				GL_SelectTexture( 1 );
				globalImages->BindNull();
				GL_SelectTexture( 0 );
				
				glDisableVertexAttribArrayARB( 9 );
				glDisableVertexAttribArrayARB( 10 );
			}
			else
			{
				// per-pixel reflection mapping without bump mapping
			}
			
			glDisableClientState( GL_NORMAL_ARRAY );
			glDisable( GL_FRAGMENT_PROGRAM_ARB );
			glDisable( GL_VERTEX_PROGRAM_ARB );
			// Fixme: Hack to get around an apparent bug in ATI drivers.  Should remove as soon as it gets fixed.
			glBindProgramARB( GL_VERTEX_PROGRAM_ARB, 0 );
		}
		else
		{
			glDisable( GL_TEXTURE_GEN_S );
			glDisable( GL_TEXTURE_GEN_T );
			glDisable( GL_TEXTURE_GEN_R );
			glTexGenf( GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
			glTexGenf( GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
			glTexGenf( GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
			glDisableClientState( GL_NORMAL_ARRAY );
			
			glMatrixMode( GL_TEXTURE );
			glLoadIdentity();
			glMatrixMode( GL_MODELVIEW );
		}
	}
#endif // #if !defined(USE_GLES1)
	
	if( pStage->texture.hasMatrix )
	{
		glMatrixMode( GL_TEXTURE );
		glLoadIdentity();
		glMatrixMode( GL_MODELVIEW );
	}
}

/*
=============================================================================================

FILL DEPTH BUFFER

=============================================================================================
*/


/*
==================
RB_T_FillDepthBuffer
==================
*/
void RB_T_FillDepthBuffer( const drawSurf_t* surf )
{
	int			stage;
	const idMaterial*	shader;
	const shaderStage_t* pStage;
	const float*	regs;
	float		color[4];
	const srfTriangles_t*	tri;
	
	tri = surf->geo;
	shader = surf->material;
	
#if defined(__ANDROID__)
	GL_CheckErrors();
#endif
	
	// update the clip plane if needed
	if( backEnd.viewDef->numClipPlanes && surf->space != backEnd.currentSpace )
	{
		GL_SelectTexture( 1 );
		
		idPlane	plane;
		
		R_GlobalPlaneToLocal( surf->space->modelMatrix, backEnd.viewDef->clipPlanes[0], plane );
		plane[3] += 0.5;	// the notch is in the middle
#if !defined(USE_GLES1)
		glTexGenfv( GL_S, GL_OBJECT_PLANE, plane.ToFloatPtr() );
#endif
		GL_SelectTexture( 0 );
	}
	
	if( !shader->IsDrawn() )
	{
		return;
	}
	
	// some deforms may disable themselves by setting numIndexes = 0
	if( !tri->numIndexes )
	{
		return;
	}
	
	// translucent surfaces don't put anything in the depth buffer and don't
	// test against it, which makes them fail the mirror clip plane operation
	if( shader->Coverage() == MC_TRANSLUCENT )
	{
		return;
	}
	
	if( !tri->ambientCache )
	{
		common->Printf( "RB_T_FillDepthBuffer: !tri->ambientCache\n" );
		return;
	}
	
	// get the expressions for conditionals / color / texcoords
	regs = surf->shaderRegisters;
	
	// if all stages of a material have been conditioned off, don't do anything
	for( stage = 0; stage < shader->GetNumStages() ; stage++ )
	{
		pStage = shader->GetStage( stage );
		// check the stage enable condition
		if( regs[ pStage->conditionRegister ] != 0 )
		{
			break;
		}
	}
	if( stage == shader->GetNumStages() )
	{
		return;
	}
	
#if defined(__ANDROID__)
	GL_CheckErrors();
#endif
	
	// set polygon offset if necessary
	if( shader->TestMaterialFlag( MF_POLYGONOFFSET ) )
	{
		glEnable( GL_POLYGON_OFFSET_FILL );
		glPolygonOffset( r_offsetFactor.GetFloat(), r_offsetUnits.GetFloat() * shader->GetPolygonOffset() );
	}
	
#if defined(__ANDROID__)
	GL_CheckErrors();
#endif
	
	// subviews will just down-modulate the color buffer by overbright
	if( shader->GetSort() == SS_SUBVIEW )
	{
		GL_State( GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ZERO | GLS_DEPTHFUNC_LESS );
		color[0] =
			color[1] =
				color[2] = ( 1.0 / backEnd.overBright );
		color[3] = 1;
	}
	else
	{
		// others just draw black
		color[0] = 0;
		color[1] = 0;
		color[2] = 0;
		color[3] = 1;
	}
	
	idDrawVert* ac = ( idDrawVert* )vertexCache.Position( tri->ambientCache );
	glVertexPointer( 3, GL_FLOAT, sizeof( idDrawVert ), ac->xyz.ToFloatPtr() );
	glTexCoordPointer( 2, GL_FLOAT, sizeof( idDrawVert ), reinterpret_cast<void*>( &ac->st ) );
	
	if( r_usePrecomputedLighting.GetBool() && tr.backEndRenderer == BE_ARB )
	{
		glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( idDrawVert ), ( void* ) &ac->color );
	}
	
	bool drawSolid = false;
	
	if( shader->Coverage() == MC_OPAQUE )
	{
		drawSolid = true;
	}
	
	// we may have multiple alpha tested stages
	if( shader->Coverage() == MC_PERFORATED )
	{
		// if the only alpha tested stages are condition register omitted,
		// draw a normal opaque surface
		bool	didDraw = false;
		
#if defined(__ANDROID__)
		GL_CheckErrors();
#endif
		
#if !defined(USE_GLES2)
		glEnable( GL_ALPHA_TEST );
#endif
		
#if defined(__ANDROID__)
		GL_CheckErrors();
#endif
		
		// perforated surfaces may have multiple alpha tested stages
		for( stage = 0; stage < shader->GetNumStages() ; stage++ )
		{
			pStage = shader->GetStage( stage );
			
			if( !pStage->hasAlphaTest )
			{
				continue;
			}
			
			// check the stage enable condition
			if( regs[ pStage->conditionRegister ] == 0 )
			{
				continue;
			}
			
			// if we at least tried to draw an alpha tested stage,
			// we won't draw the opaque surface
			didDraw = true;
			
			// set the alpha modulate
			color[3] = regs[ pStage->color.registers[3] ];
			
			// skip the entire stage if alpha would be black
			if( color[3] <= 0 )
			{
				continue;
			}
			
#if !defined(USE_GLES2)
			glAlphaFunc( GL_GREATER, regs[ pStage->alphaTestRegister ] );
#endif
			
			if( r_usePrecomputedLighting.GetBool() && tr.backEndRenderer == BE_ARB )
			{
				globalImages->whiteImage->Bind();
			}
			else
			{
				glColor4f( color[0], color[1], color[2], color[3] );
				
				// bind the texture
				pStage->texture.image->Bind();
			}
			
			// set texture matrix and texGens
			RB_PrepareStageTexturing( pStage, surf, ac );
			
			// draw it
			RB_DrawElementsWithCounters( tri );
			
			RB_FinishStageTexturing( pStage, surf, ac );
		}
		
#if defined(__ANDROID__)
		GL_CheckErrors();
#endif
		
#if !defined(USE_GLES2)
		glDisable( GL_ALPHA_TEST );
#endif
		
#if defined(__ANDROID__)
		GL_CheckErrors();
#endif
		
		if( !didDraw )
		{
			drawSolid = true;
		}
	}
	
	// draw the entire surface solid
	if( drawSolid )
	{
		if( !( r_usePrecomputedLighting.GetBool() && tr.backEndRenderer == BE_ARB ) )
		{
			glColor4f( color[0], color[1], color[2], color[3] );
		}
		globalImages->whiteImage->Bind();
		
		// draw it
		RB_DrawElementsWithCounters( tri );
	}
	
	GL_CheckErrors();
	
	// reset polygon offset
	if( shader->TestMaterialFlag( MF_POLYGONOFFSET ) )
	{
		glDisable( GL_POLYGON_OFFSET_FILL );
	}
	
#if defined(__ANDROID__)
	GL_CheckErrors();
#endif
	
	// reset blending
	if( shader->GetSort() == SS_SUBVIEW )
	{
		GL_State( GLS_DEPTHFUNC_LESS );
	}
	
#if defined(__ANDROID__)
	GL_CheckErrors();
#endif
	
}

// RB begin
#if !defined(USE_GLES1)
static void	RB_T_DrawInteractionAsFillDepthBufferWithNormals( const drawInteractionMaterialOnly_t* din )
{
	// choose and bind the vertex program
	gl_geometricFillShader->SetNormalMapping( !r_skipBump.GetBool() );
	gl_geometricFillShader->BindProgram();
	
	// load all the vertex program parameters
	
	gl_geometricFillShader->SetUniform_GlobalViewOrigin( backEnd.viewDef->renderView.vieworg );
//	gl_geometricFillShader->SetUniform_AmbientColor(ambientColor);

	gl_geometricFillShader->SetUniform_ModelMatrix( make_idMat4( din->surf->space->modelMatrix ) );
	
	gl_geometricFillShader->SetUniform_NormalImage( 0 );
	
	gl_geometricFillShader->SetUniform_BumpMatrixS( din->bumpMatrix[0] );
	gl_geometricFillShader->SetUniform_BumpMatrixT( din->bumpMatrix[1] );
	
	gl_geometricFillShader->SetUniform_DiffuseMatrixS( din->diffuseMatrix[0] );
	gl_geometricFillShader->SetUniform_DiffuseMatrixT( din->diffuseMatrix[1] );
	
#if 0
	gl_geometricFillShader->SetUniform_SpecularMatrixS( din->specularMatrix[0] );
	gl_geometricFillShader->SetUniform_SpecularMatrixT( din->specularMatrix[1] );
#endif
	
	static const idVec4 zero( 0, 0, 0, 0 );
	static const idVec4 one( 1, 1, 1, 1 );
	static const idVec4 negOne( -1, -1, -1, -1 );
	
	switch( din->vertexColor )
	{
		case SVC_IGNORE:
			gl_geometricFillShader->SetUniform_ColorModulate( zero );
			gl_geometricFillShader->SetUniform_Color( one );
			break;
		case SVC_MODULATE:
			gl_geometricFillShader->SetUniform_ColorModulate( one );
			gl_geometricFillShader->SetUniform_Color( zero );
			break;
		case SVC_INVERSE_MODULATE:
			gl_geometricFillShader->SetUniform_ColorModulate( negOne );
			gl_geometricFillShader->SetUniform_Color( one );
			break;
	}
	
	// set the textures
	
	// texture 0 will be the per-surface bump map
	gl_geometricFillShader->SetUniform_NormalImage( 0 );
	GL_SelectTexture( 0 );
	din->bumpImage->Bind();
	
	// texture 1 is the per-surface diffuse map
	gl_geometricFillShader->SetUniform_DiffuseImage( 1 );
	GL_SelectTexture( 1 );
	din->diffuseImage->Bind();
	
#if 0
	// texture 2 is the per-surface specular map
	gl_geometricFillShader->SetUniform_SpecularImage( 2 );
	GL_SelectTexture( 2 );
	din->specularImage->Bind();
#endif
	
	// draw it
	RB_DrawElementsWithCounters( din->surf->geo );
}

static bool RB_T_SubmittInteraction( drawInteractionMaterialOnly_t* din, void ( *DrawInteraction )( const drawInteractionMaterialOnly_t* ) )
{
	if( !din->bumpImage )
	{
		return false;
	}
	
	if( !din->diffuseImage || r_skipDiffuse.GetBool() )
	{
		din->diffuseImage = globalImages->blackImage;
	}
#if 0
	if( !din->specularImage || r_skipSpecular.GetBool() )
	{
		din->specularImage = globalImages->blackImage;
	}
#endif
	if( !din->bumpImage || r_skipBump.GetBool() )
	{
		din->bumpImage = globalImages->flatNormalMap;
	}
	
	// if we wouldn't draw anything, don't call the Draw function
	//if (
	//	( ( din->diffuseColor[0] > 0 || din->diffuseColor[1] > 0 || din->diffuseColor[2] > 0 ) && din->diffuseImage != globalImages->blackImage )
	//	|| ( ( din->specularColor[0] > 0 || din->specularColor[1] > 0 || din->specularColor[2] > 0 ) && din->specularImage != globalImages->blackImage ) )
	{
		if( din->alphaTest > 0.0f )
		{
			glEnable( GL_ALPHA_TEST );
			glAlphaFunc( GL_GREATER, din->alphaTest );
		}
		
		DrawInteraction( din );
		
		if( din->alphaTest > 0.0f )
		{
			glDisable( GL_ALPHA_TEST );
		}
		
		return true;
	}
}

/*
==================
RB_T_FillDepthBufferWithNormals
==================
*/
void RB_T_FillDepthBufferWithNormals( const drawSurf_t* surf )
{
	int			stage;
	const idMaterial*	shader;
	const shaderStage_t* pStage;
	const float*	regs;
	float		color[4];
	const srfTriangles_t*	tri;
	
	tri = surf->geo;
	shader = surf->material;
	
	// update the clip plane if needed
	if( backEnd.viewDef->numClipPlanes && surf->space != backEnd.currentSpace )
	{
		GL_SelectTexture( 1 );
		
		idPlane	plane;
		
		R_GlobalPlaneToLocal( surf->space->modelMatrix, backEnd.viewDef->clipPlanes[0], plane );
		plane[3] += 0.5;	// the notch is in the middle
#if !defined(USE_GLES1)
		glTexGenfv( GL_S, GL_OBJECT_PLANE, plane.ToFloatPtr() );
#endif
		GL_SelectTexture( 0 );
	}
	
	if( !shader->IsDrawn() )
	{
		return;
	}
	
	// some deforms may disable themselves by setting numIndexes = 0
	if( !tri->numIndexes )
	{
		return;
	}
	
	// translucent surfaces don't put anything in the depth buffer and don't
	// test against it, which makes them fail the mirror clip plane operation
	if( shader->Coverage() == MC_TRANSLUCENT )
	{
		return;
	}
	
	if( !tri->ambientCache )
	{
		common->Printf( "RB_T_FillDepthBufferWithNormals: !tri->ambientCache\n" );
		return;
	}
	
	// get the expressions for conditionals / color / texcoords
	regs = surf->shaderRegisters;
	
	// if all stages of a material have been conditioned off, don't do anything
	for( stage = 0; stage < shader->GetNumStages() ; stage++ )
	{
		pStage = shader->GetStage( stage );
		
		// check the stage enable condition
		if( regs[ pStage->conditionRegister ] != 0 )
		{
			break;
		}
	}
	if( stage == shader->GetNumStages() )
	{
		return;
	}
	
	// set polygon offset if necessary
	if( shader->TestMaterialFlag( MF_POLYGONOFFSET ) )
	{
		glEnable( GL_POLYGON_OFFSET_FILL );
		glPolygonOffset( r_offsetFactor.GetFloat(), r_offsetUnits.GetFloat() * shader->GetPolygonOffset() );
	}
	
	// subviews will just down-modulate the color buffer by overbright
	if( shader->GetSort() == SS_SUBVIEW )
	{
		GL_State( GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ZERO | GLS_DEPTHFUNC_LESS );
		color[0] =
			color[1] =
				color[2] = ( 1.0 / backEnd.overBright );
		color[3] = 1;
	}
	else
	{
		// others just draw white
		color[0] = 1;
		color[1] = 1;
		color[2] = 1;
		color[3] = 1;
	}
	
	idDrawVert* ac = ( idDrawVert* )vertexCache.Position( tri->ambientCache );
	glVertexPointer( 3, GL_FLOAT, sizeof( idDrawVert ), ac->xyz.ToFloatPtr() );
	glVertexAttribPointerARB( VA_INDEX_TEXCOORD0, 2, GL_FLOAT, false, sizeof( idDrawVert ), ac->st.ToFloatPtr() );
	glVertexAttribPointerARB( VA_INDEX_NORMAL, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->normal.ToFloatPtr() );
	glVertexAttribPointerARB( VA_INDEX_BITANGENT, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->tangents[1].ToFloatPtr() );
	glVertexAttribPointerARB( VA_INDEX_TANGENT, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->tangents[0].ToFloatPtr() );
	
	bool drawSolid = false;
	
	//if ( shader->Coverage() == MC_OPAQUE ) {
	//	drawSolid = true;
	//}
	
	drawInteractionMaterialOnly_t inter;
	
	inter.surf = surf;
	
	inter.bumpImage = NULL;
	inter.specularImage = NULL;
	inter.diffuseImage = NULL;
	inter.diffuseColor[0] = inter.diffuseColor[1] = inter.diffuseColor[2] = inter.diffuseColor[3] = 0;
	inter.specularColor[0] = inter.specularColor[1] = inter.specularColor[2] = inter.specularColor[3] = 0;
	
	glColor4f( color[0], color[1], color[2], color[3] );
	
	bool	didDraw = false;
	
	for( stage = 0; stage < shader->GetNumStages() ; stage++ )
	{
		pStage = shader->GetStage( stage );
		
		/*
		// check the stage enable condition
		if ( regs[ pStage->conditionRegister ] == 0 ) {
			continue;
		}
		
		if ( pStage->lighting != SL_BUMP ) {
			continue;
		}
		
		color[0] = regs[ pStage->color.registers[0] ];
		color[1] = regs[ pStage->color.registers[1] ];
		color[2] = regs[ pStage->color.registers[2] ];
		color[3] = regs[ pStage->color.registers[3] ];
		*/
		
		switch( pStage->lighting )
		{
			case SL_AMBIENT:
			{
#if 1
				// ignore ambient stages while drawing interactions
				continue;
#else
				if( !pStage->hasAlphaTest )
				{
					continue;
				}
				
				// check the stage enable condition
				if( regs[ pStage->conditionRegister ] == 0 )
				{
					continue;
				}
				
				// if we at least tried to draw an alpha tested stage,
				// we won't draw the opaque surface
				didDraw = true;
				
				// set the alpha modulate
				color[3] = regs[ pStage->color.registers[3] ];
				
				// skip the entire stage if alpha would be black
				if( color[3] <= 0 )
				{
					continue;
				}
				glColor4f( color[0], color[1], color[2], color[3] );
				
				glAlphaFunc( GL_GREATER, regs[ pStage->alphaTestRegister ] );
				
				// bind the texture
				pStage->texture.image->Bind();
				
				// set texture matrix and texGens
				RB_PrepareStageTexturing( pStage, surf, ac );
				
				// draw it
				RB_DrawElementsWithCounters( tri );
				
				RB_FinishStageTexturing( pStage, surf, ac );
#endif
				break;
			}
			case SL_BUMP:
			{
				// ignore stage that fails the condition
				if( !regs[ pStage->conditionRegister ] )
				{
					break;
				}
				
				// draw any previous interaction
				didDraw = didDraw || RB_T_SubmittInteraction( &inter, RB_T_DrawInteractionAsFillDepthBufferWithNormals );
				
				if( pStage->hasAlphaTest )
				{
					inter.alphaTest = regs[ pStage->alphaTestRegister ];
				}
				else
				{
					inter.alphaTest = 0;
				}
				
				inter.diffuseImage = NULL;
				inter.specularImage = NULL;
				R_SetDrawInteraction( pStage, regs, &inter.bumpImage, inter.bumpMatrix, NULL );
				break;
			}
			case SL_DIFFUSE:
			{
				// ignore stage that fails the condition
				if( !regs[ pStage->conditionRegister ] )
				{
					break;
				}
				if( inter.diffuseImage )
				{
					didDraw = didDraw || RB_T_SubmittInteraction( &inter, RB_T_DrawInteractionAsFillDepthBufferWithNormals );
				}
				R_SetDrawInteraction( pStage, regs, &inter.diffuseImage, inter.diffuseMatrix, inter.diffuseColor.ToFloatPtr() );
				inter.vertexColor = pStage->vertexColor;
				
				if( pStage->hasAlphaTest )
				{
					if( regs[ pStage->alphaTestRegister ] > inter.alphaTest )
					{
						inter.alphaTest = regs[ pStage->alphaTestRegister ];
					}
				}
				break;
			}
			case SL_SPECULAR:
			{
				// ignore stage that fails the condition
				if( !regs[ pStage->conditionRegister ] )
				{
					break;
				}
				if( inter.specularImage )
				{
					didDraw = didDraw || RB_T_SubmittInteraction( &inter, RB_T_DrawInteractionAsFillDepthBufferWithNormals );
				}
				R_SetDrawInteraction( pStage, regs, &inter.specularImage, inter.specularMatrix, inter.specularColor.ToFloatPtr() );
				inter.vertexColor = pStage->vertexColor;
				
				if( pStage->hasAlphaTest )
				{
					if( regs[ pStage->alphaTestRegister ] > inter.alphaTest )
					{
						inter.alphaTest = regs[ pStage->alphaTestRegister ];
					}
				}
				break;
			}
		}
	}
	
	// draw the final interaction
	didDraw = didDraw || RB_T_SubmittInteraction( &inter, RB_T_DrawInteractionAsFillDepthBufferWithNormals );
	
	if( !didDraw )
	{
		inter.alphaTest = 0;
		inter.bumpImage = globalImages->flatNormalMap;
		inter.diffuseImage = NULL;
		inter.specularImage = NULL;
		
		RB_T_SubmittInteraction( &inter, RB_T_DrawInteractionAsFillDepthBufferWithNormals );
	}
	
	// reset polygon offset
	if( shader->TestMaterialFlag( MF_POLYGONOFFSET ) )
	{
		glDisable( GL_POLYGON_OFFSET_FILL );
	}
	
	// reset blending
	if( shader->GetSort() == SS_SUBVIEW )
	{
		GL_State( GLS_DEPTHFUNC_LESS );
	}
}
#endif // #if !defined(USE_GLES1)
// RB end


/*
=====================
RB_STD_FillDepthBuffer

If we are rendering a subview with a near clip plane, use a second texture
to force the alpha test to fail when behind that clip plane
=====================
*/
void RB_STD_FillDepthBuffer( drawSurf_t** drawSurfs, int numDrawSurfs )
{
	// if we are just doing 2D rendering, no need to fill the depth buffer
	if( !backEnd.viewDef->viewEntitys )
	{
		return;
	}
	
	RB_LogComment( "---------- RB_STD_FillDepthBuffer ----------\n" );
	
	// enable the second texture for mirror plane clipping if needed
	if( backEnd.viewDef->numClipPlanes )
	{
		GL_SelectTexture( 1 );
		globalImages->alphaNotchImage->Bind();
		
#if defined(__ANDROID__)
		GL_CheckErrors();
#endif
		glDisableClientState( GL_TEXTURE_COORD_ARRAY );
		
#if defined(__ANDROID__)
		GL_CheckErrors();
#endif
		
#if !defined(USE_GLES1)
		glEnable( GL_TEXTURE_GEN_S );
		glTexCoord2f( 1, 0.5 );
#endif
	}
	
	GL_CheckErrors();
	
	// the first texture will be used for alpha tested surfaces
	GL_SelectTexture( 0 );
	
	GL_CheckErrors();
	
#if !defined(USE_GLES1)
	if( r_useDeferredShading.GetBool() )
	{
		glEnableVertexAttribArrayARB( VA_INDEX_TEXCOORD0 );
		glEnableVertexAttribArrayARB( VA_INDEX_TANGENT );
		glEnableVertexAttribArrayARB( VA_INDEX_BITANGENT );
		glEnableVertexAttribArrayARB( VA_INDEX_NORMAL );
	}
	else
#endif
	{
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
		
		if( r_usePrecomputedLighting.GetBool() && tr.backEndRenderer == BE_ARB )
		{
			glEnableClientState( GL_COLOR_ARRAY );
		}
	}
	
	// decal surfaces may enable polygon offset
	glPolygonOffset( r_offsetFactor.GetFloat(), r_offsetUnits.GetFloat() );
	
#if defined(__ANDROID__)
	GL_CheckErrors();
#endif
	
	GL_State( GLS_DEPTHFUNC_LESS );
	
#if defined(__ANDROID__)
	GL_CheckErrors();
#endif
	
	// Enable stencil test if we are going to be using it for shadows.
	// If we didn't do this, it would be legal behavior to get z fighting
	// from the ambient pass and the light passes.
	glEnable( GL_STENCIL_TEST );
	glStencilFunc( GL_ALWAYS, 1, 255 );
	
#if !defined(USE_GLES1)
	if( /*tr.backEndRenderer == BE_EXP &&*/ r_useDeferredShading.GetBool() )
	{
		RB_RenderDrawSurfListWithFunction( drawSurfs, numDrawSurfs, RB_T_FillDepthBufferWithNormals );
		
		glDisableVertexAttribArrayARB( VA_INDEX_TEXCOORD0 );
		glDisableVertexAttribArrayARB( VA_INDEX_TANGENT );
		glDisableVertexAttribArrayARB( VA_INDEX_BITANGENT );
		glDisableVertexAttribArrayARB( VA_INDEX_NORMAL );
		
		GL_BindNullProgram();
	}
	else
#endif
	{
		RB_RenderDrawSurfListWithFunction( drawSurfs, numDrawSurfs, RB_T_FillDepthBuffer );
#if defined(__ANDROID__)
		GL_CheckErrors();
#endif
	}
	
	if( backEnd.viewDef->numClipPlanes )
	{
		GL_SelectTexture( 1 );
		globalImages->BindNull();
#if !defined(USE_GLES1)
		glDisable( GL_TEXTURE_GEN_S );
#endif
		GL_SelectTexture( 0 );
	}
	
	if( r_usePrecomputedLighting.GetBool() && tr.backEndRenderer == BE_ARB )
	{
		glDisableClientState( GL_COLOR_ARRAY );
	}
	
	GL_CheckErrors();
}

/*
=============================================================================================

SHADER PASSES

=============================================================================================
*/

/*
==================
RB_SetProgramEnvironment

Sets variables that can be used by all vertex programs
==================
*/
void RB_SetProgramEnvironment()
{
#if !defined(USE_GLES1)
	float	parm[4];
	int		pot;
	
	if( !glConfig.ARBVertexProgramAvailable )
	{
		return;
	}
	
#if 0
	// screen power of two correction factor, one pixel in so we don't get a bilerp
	// of an uncopied pixel
	int	 w = backEnd.viewDef->viewport.x2 - backEnd.viewDef->viewport.x1 + 1;
	pot = globalImages->currentRenderImage->uploadWidth;
	if( w == pot )
	{
		parm[0] = 1.0;
	}
	else
	{
		parm[0] = ( float )( w - 1 ) / pot;
	}
	
	int	 h = backEnd.viewDef->viewport.y2 - backEnd.viewDef->viewport.y1 + 1;
	pot = globalImages->currentRenderImage->uploadHeight;
	if( h == pot )
	{
		parm[1] = 1.0;
	}
	else
	{
		parm[1] = ( float )( h - 1 ) / pot;
	}
	
	parm[2] = 0;
	parm[3] = 1;
	glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, 0, parm );
#else
	// screen power of two correction factor, assuming the copy to _currentRender
	// also copied an extra row and column for the bilerp
	int	 w = backEnd.viewDef->viewport.x2 - backEnd.viewDef->viewport.x1 + 1;
	pot = globalImages->currentRenderImage->uploadWidth;
	parm[0] = ( float )w / pot;
	
	int	 h = backEnd.viewDef->viewport.y2 - backEnd.viewDef->viewport.y1 + 1;
	pot = globalImages->currentRenderImage->uploadHeight;
	parm[1] = ( float )h / pot;
	
	parm[2] = 0;
	parm[3] = 1;
	glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, 0, parm );
#endif
	
	glProgramEnvParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 0, parm );
	
	// window coord to 0.0 to 1.0 conversion
	parm[0] = 1.0 / w;
	parm[1] = 1.0 / h;
	parm[2] = 0;
	parm[3] = 1;
	glProgramEnvParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 1, parm );
	
	//
	// set eye position in global space
	//
	parm[0] = backEnd.viewDef->renderView.vieworg[0];
	parm[1] = backEnd.viewDef->renderView.vieworg[1];
	parm[2] = backEnd.viewDef->renderView.vieworg[2];
	parm[3] = 1.0;
	glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, 1, parm );
	
#endif // #if !defined(USE_GLES1)
}

/*
==================
RB_SetProgramEnvironmentSpace

Sets variables related to the current space that can be used by all vertex programs
==================
*/
void RB_SetProgramEnvironmentSpace()
{
#if !defined(USE_GLES1)
	if( !glConfig.ARBVertexProgramAvailable )
	{
		return;
	}
	
	const struct viewEntity_t* space = backEnd.currentSpace;
	float	parm[4];
	
	// set eye position in local space
	R_GlobalPointToLocal( space->modelMatrix, backEnd.viewDef->renderView.vieworg, *( idVec3* )parm );
	parm[3] = 1.0;
	glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, 5, parm );
	
	// we need the model matrix without it being combined with the view matrix
	// so we can transform local vectors to global coordinates
	parm[0] = space->modelMatrix[0];
	parm[1] = space->modelMatrix[4];
	parm[2] = space->modelMatrix[8];
	parm[3] = space->modelMatrix[12];
	glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, 6, parm );
	parm[0] = space->modelMatrix[1];
	parm[1] = space->modelMatrix[5];
	parm[2] = space->modelMatrix[9];
	parm[3] = space->modelMatrix[13];
	glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, 7, parm );
	parm[0] = space->modelMatrix[2];
	parm[1] = space->modelMatrix[6];
	parm[2] = space->modelMatrix[10];
	parm[3] = space->modelMatrix[14];
	glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, 8, parm );
#endif // #if !defined(USE_GLES1)
}

/*
==================
RB_STD_T_RenderShaderPasses

This is also called for the generated 2D rendering
==================
*/
void RB_STD_T_RenderShaderPasses( const drawSurf_t* surf )
{
	int			stage;
	const idMaterial*	shader;
	const shaderStage_t* pStage;
	const float*	regs;
	float		color[4];
	const srfTriangles_t*	tri;
	
	tri = surf->geo;
	shader = surf->material;
	
	if( !shader->HasAmbient() )
	{
		return;
	}
	
	if( shader->IsPortalSky() )
	{
		return;
	}
	
	// change the matrix if needed
	if( surf->space != backEnd.currentSpace )
	{
		glLoadMatrixf( surf->space->modelViewMatrix );
		backEnd.currentSpace = surf->space;
		RB_SetProgramEnvironmentSpace();
	}
	
	// change the scissor if needed
	if( r_useScissor.GetBool() && !backEnd.currentScissor.Equals( surf->scissorRect ) )
	{
		backEnd.currentScissor = surf->scissorRect;
		glScissor( backEnd.viewDef->viewport.x1 + backEnd.currentScissor.x1,
				   backEnd.viewDef->viewport.y1 + backEnd.currentScissor.y1,
				   backEnd.currentScissor.x2 + 1 - backEnd.currentScissor.x1,
				   backEnd.currentScissor.y2 + 1 - backEnd.currentScissor.y1 );
	}
	
	// some deforms may disable themselves by setting numIndexes = 0
	if( !tri->numIndexes )
	{
		return;
	}
	
	if( !tri->ambientCache )
	{
		common->Printf( "RB_T_RenderShaderPasses: !tri->ambientCache\n" );
		return;
	}
	
	// get the expressions for conditionals / color / texcoords
	regs = surf->shaderRegisters;
	
	// set face culling appropriately
	GL_Cull( shader->GetCullType() );
	
	// set polygon offset if necessary
	if( shader->TestMaterialFlag( MF_POLYGONOFFSET ) )
	{
		glEnable( GL_POLYGON_OFFSET_FILL );
		glPolygonOffset( r_offsetFactor.GetFloat(), r_offsetUnits.GetFloat() * shader->GetPolygonOffset() );
	}
	
	if( surf->space->weaponDepthHack )
	{
		RB_EnterWeaponDepthHack();
	}
	
	if( surf->space->modelDepthHack != 0.0f )
	{
		RB_EnterModelDepthHack( surf->space->modelDepthHack );
	}
	
	idDrawVert* ac = ( idDrawVert* )vertexCache.Position( tri->ambientCache );
	glVertexPointer( 3, GL_FLOAT, sizeof( idDrawVert ), ac->xyz.ToFloatPtr() );
	glTexCoordPointer( 2, GL_FLOAT, sizeof( idDrawVert ), reinterpret_cast<void*>( &ac->st ) );
	
	for( stage = 0; stage < shader->GetNumStages() ; stage++ )
	{
		pStage = shader->GetStage( stage );
		
		// check the enable condition
		if( regs[ pStage->conditionRegister ] == 0 )
		{
			continue;
		}
		
		// skip the stages involved in lighting
		if( pStage->lighting != SL_AMBIENT )
		{
			continue;
		}
		
		// skip if the stage is ( GL_ZERO, GL_ONE ), which is used for some alpha masks
		if( ( pStage->drawStateBits & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) ) == ( GLS_SRCBLEND_ZERO | GLS_DSTBLEND_ONE ) )
		{
			continue;
		}
		
		// see if we are a new-style stage
		newShaderStage_t* newStage = pStage->newStage;
		if( newStage )
		{
			//--------------------------
			//
			// new style stages
			//
			//--------------------------
			
			// completely skip the stage if we don't have the capability
			// RB begin
			if( tr.backEndRenderer != BE_ARB2 && tr.backEndRenderer != BE_GLSL && tr.backEndRenderer != BE_EXP )
			{
				// RB end
				continue;
			}
			if( r_skipNewAmbient.GetBool() )
			{
				continue;
			}
			glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( idDrawVert ), ( void* )&ac->color );
#if !defined(USE_GLES1)
			glVertexAttribPointerARB( 9, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->tangents[0].ToFloatPtr() );
			glVertexAttribPointerARB( 10, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->tangents[1].ToFloatPtr() );
#endif
			glNormalPointer( GL_FLOAT, sizeof( idDrawVert ), ac->normal.ToFloatPtr() );
			
			glEnableClientState( GL_COLOR_ARRAY );
#if !defined(USE_GLES1)
			glEnableVertexAttribArrayARB( 9 );
			glEnableVertexAttribArrayARB( 10 );
#endif
			glEnableClientState( GL_NORMAL_ARRAY );
			
			GL_State( pStage->drawStateBits );
			
#if !defined(USE_GLES1)
			glBindProgramARB( GL_VERTEX_PROGRAM_ARB, newStage->vertexProgram );
			glEnable( GL_VERTEX_PROGRAM_ARB );
#endif
			
			// megaTextures bind a lot of images and set a lot of parameters
			if( newStage->megaTexture )
			{
				newStage->megaTexture->SetMappingForSurface( tri );
				idVec3	localViewer;
				R_GlobalPointToLocal( surf->space->modelMatrix, backEnd.viewDef->renderView.vieworg, localViewer );
				newStage->megaTexture->BindForViewOrigin( localViewer );
			}
			
#if !defined(USE_GLES1)
			for( int i = 0 ; i < newStage->numVertexParms ; i++ )
			{
				float	parm[4];
				parm[0] = regs[ newStage->vertexParms[i][0] ];
				parm[1] = regs[ newStage->vertexParms[i][1] ];
				parm[2] = regs[ newStage->vertexParms[i][2] ];
				parm[3] = regs[ newStage->vertexParms[i][3] ];
				glProgramLocalParameter4fvARB( GL_VERTEX_PROGRAM_ARB, i, parm );
			}
#endif
			
			for( int i = 0 ; i < newStage->numFragmentProgramImages ; i++ )
			{
				if( newStage->fragmentProgramImages[i] )
				{
					GL_SelectTexture( i );
					newStage->fragmentProgramImages[i]->Bind();
				}
			}
			
#if !defined(USE_GLES1)
			glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, newStage->fragmentProgram );
			glEnable( GL_FRAGMENT_PROGRAM_ARB );
#endif
			
			// draw it
			RB_DrawElementsWithCounters( tri );
			
			for( int i = 1 ; i < newStage->numFragmentProgramImages ; i++ )
			{
				if( newStage->fragmentProgramImages[i] )
				{
					GL_SelectTexture( i );
					globalImages->BindNull();
				}
			}
			if( newStage->megaTexture )
			{
				newStage->megaTexture->Unbind();
			}
			
			GL_SelectTexture( 0 );
			
#if !defined(USE_GLES1)
			glDisable( GL_VERTEX_PROGRAM_ARB );
			glDisable( GL_FRAGMENT_PROGRAM_ARB );
			// Fixme: Hack to get around an apparent bug in ATI drivers.  Should remove as soon as it gets fixed.
			glBindProgramARB( GL_VERTEX_PROGRAM_ARB, 0 );
#endif
			glDisableClientState( GL_COLOR_ARRAY );
			
#if !defined(USE_GLES1)
			glDisableVertexAttribArrayARB( 9 );
			glDisableVertexAttribArrayARB( 10 );
#endif
			glDisableClientState( GL_NORMAL_ARRAY );
			continue;
		}
		
		//--------------------------
		//
		// old style stages
		//
		//--------------------------
		
		// set the color
		color[0] = regs[ pStage->color.registers[0] ];
		color[1] = regs[ pStage->color.registers[1] ];
		color[2] = regs[ pStage->color.registers[2] ];
		color[3] = regs[ pStage->color.registers[3] ];
		
		// skip the entire stage if an add would be black
		if( ( pStage->drawStateBits & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) ) == ( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE )
				&& color[0] <= 0 && color[1] <= 0 && color[2] <= 0 )
		{
			continue;
		}
		
		// skip the entire stage if a blend would be completely transparent
		if( ( pStage->drawStateBits & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) ) == ( GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA )
				&& color[3] <= 0 )
		{
			continue;
		}
		
		// select the vertex color source
		if( pStage->vertexColor == SVC_IGNORE )
		{
			glColor4f( color[0], color[1], color[2], color[3] );
		}
		else
		{
			glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( idDrawVert ), ( void* )&ac->color );
			glEnableClientState( GL_COLOR_ARRAY );
			
#if !defined(USE_GLES2)
			if( pStage->vertexColor == SVC_INVERSE_MODULATE )
			{
				GL_TexEnv( GL_COMBINE );
				glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE );
				glTexEnvi( GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE );
				glTexEnvi( GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR );
				glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR );
				glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_ONE_MINUS_SRC_COLOR );
				glTexEnvi( GL_TEXTURE_ENV, GL_RGB_SCALE, 1 );
			}
			
			// for vertex color and modulated color, we need to enable a second
			// texture stage
			if( color[0] != 1 || color[1] != 1 || color[2] != 1 || color[3] != 1 )
			{
				GL_SelectTexture( 1 );
				
				globalImages->whiteImage->Bind();
				GL_TexEnv( GL_COMBINE );
				
				glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color );
				
				glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE );
				glTexEnvi( GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS );
				glTexEnvi( GL_TEXTURE_ENV, GL_SRC1_RGB, GL_CONSTANT );
				glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR );
				glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR );
				glTexEnvi( GL_TEXTURE_ENV, GL_RGB_SCALE, 1 );
				
				glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE );
				glTexEnvi( GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PREVIOUS );
				glTexEnvi( GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_CONSTANT );
				glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA );
				glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA );
				glTexEnvi( GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1 );
				
				GL_SelectTexture( 0 );
			}
#endif
		}
		
#if 0 //defined(USE_GLES2)
		// TODO
		static const idVec4 zero( 0, 0, 0, 0 );
		static const idVec4 one( 1, 1, 1, 1 );
		static const idVec4 negOne( -1, -1, -1, -1 );
		
		switch( pStage->vertexColor )
		{
			case SVC_IGNORE:
				gl_genericShader->SetUniform_ColorModulate( zero );
				gl_genericShader->SetUniform_Color( one );
				break;
			case SVC_MODULATE:
				gl_genericShader->SetUniform_ColorModulate( one );
				gl_genericShader->SetUniform_Color( zero );
				break;
			case SVC_INVERSE_MODULATE:
				gl_genericShader->SetUniform_ColorModulate( negOne );
				gl_genericShader->SetUniform_Color( one );
				break;
		}
#endif
		
		// bind the texture
		RB_BindVariableStageImage( &pStage->texture, regs );
		
		// set the state
		GL_State( pStage->drawStateBits );
		
		RB_PrepareStageTexturing( pStage, surf, ac );
		
		// draw it
		RB_DrawElementsWithCounters( tri );
		
		RB_FinishStageTexturing( pStage, surf, ac );
		
		if( pStage->vertexColor != SVC_IGNORE )
		{
			glDisableClientState( GL_COLOR_ARRAY );
			
#if !defined(USE_GLES2)
			GL_SelectTexture( 1 );
			GL_TexEnv( GL_MODULATE );
			globalImages->BindNull();
			GL_SelectTexture( 0 );
			GL_TexEnv( GL_MODULATE );
#endif
		}
	}
	
	// reset polygon offset
	if( shader->TestMaterialFlag( MF_POLYGONOFFSET ) )
	{
		glDisable( GL_POLYGON_OFFSET_FILL );
	}
	if( surf->space->weaponDepthHack || surf->space->modelDepthHack != 0.0f )
	{
		RB_LeaveDepthHack();
	}
}

/*
=====================
RB_STD_DrawShaderPasses

Draw non-light dependent passes
=====================
*/
int RB_STD_DrawShaderPasses( drawSurf_t** drawSurfs, int numDrawSurfs )
{
	int				i;
	
	// only obey skipAmbient if we are rendering a view
	if( backEnd.viewDef->viewEntitys && r_skipAmbient.GetBool() )
	{
		return numDrawSurfs;
	}
	
	RB_LogComment( "---------- RB_STD_DrawShaderPasses ----------\n" );
	
	// if we are about to draw the first surface that needs
	// the rendering in a texture, copy it over
	if( drawSurfs[0]->material->GetSort() >= SS_POST_PROCESS )
	{
		if( r_skipPostProcess.GetBool() )
		{
			return 0;
		}
		
		// only dump if in a 3d view
		// RB begin
		if( backEnd.viewDef->viewEntitys && ( tr.backEndRenderer == BE_ARB2 || tr.backEndRenderer == BE_GLSL || tr.backEndRenderer == BE_EXP ) )
		{
			// RB end
			globalImages->currentRenderImage->CopyFramebuffer( backEnd.viewDef->viewport.x1,
					backEnd.viewDef->viewport.y1,  backEnd.viewDef->viewport.x2 -  backEnd.viewDef->viewport.x1 + 1,
					backEnd.viewDef->viewport.y2 -  backEnd.viewDef->viewport.y1 + 1, true );
		}
		backEnd.currentRenderCopied = true;
	}
	
	GL_SelectTexture( 1 );
	globalImages->BindNull();
	
	GL_SelectTexture( 0 );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	
	RB_SetProgramEnvironment();
	
	// we don't use RB_RenderDrawSurfListWithFunction()
	// because we want to defer the matrix load because many
	// surfaces won't draw any ambient passes
	backEnd.currentSpace = NULL;
	for( i = 0  ; i < numDrawSurfs ; i++ )
	{
		if( drawSurfs[i]->material->SuppressInSubview() )
		{
			continue;
		}
		
		if( backEnd.viewDef->isXraySubview && drawSurfs[i]->space->entityDef )
		{
			if( drawSurfs[i]->space->entityDef->parms.xrayIndex != 2 )
			{
				continue;
			}
		}
		
		// we need to draw the post process shaders after we have drawn the fog lights
		if( drawSurfs[i]->material->GetSort() >= SS_POST_PROCESS
				&& !backEnd.currentRenderCopied )
		{
			break;
		}
		
		RB_STD_T_RenderShaderPasses( drawSurfs[i] );
	}
	
	GL_Cull( CT_FRONT_SIDED );
	glColor4f( 1, 1, 1, 1 );
	
	return i;
}



/*
==============================================================================

BACK END RENDERING OF STENCIL SHADOWS

==============================================================================
*/

/*
=====================
RB_T_Shadow

the shadow volumes face INSIDE
=====================
*/
static void RB_T_Shadow( const drawSurf_t* surf )
{
	const srfTriangles_t*	tri;
	
	// set the light position if we are using a vertex program to project the rear surfaces
	if( tr.backEndRendererHasVertexPrograms && r_useShadowVertexProgram.GetBool()
			&& surf->space != backEnd.currentSpace )
	{
		idVec4 localLight;
		
		R_GlobalPointToLocal( surf->space->modelMatrix, backEnd.vLight->globalLightOrigin, localLight.ToVec3() );
		localLight.w = 0.0f;
#if !defined(USE_GLES1)
		switch( tr.backEndRenderer )
		{
			case BE_ARB2:
				glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_LIGHT_ORIGIN, localLight.ToFloatPtr() );
				break;
				
			case BE_GLSL:
				gl_shadowVolumeShader->SetUniform_LocalLightOrigin( localLight.ToVec3() );
				break;
		};
#endif
	}
	
	tri = surf->geo;
	
	if( !tri->shadowCache )
	{
		return;
	}
	
	glVertexPointer( 4, GL_FLOAT, sizeof( shadowCache_t ), vertexCache.Position( tri->shadowCache ) );
	
	// we always draw the sil planes, but we may not need to draw the front or rear caps
	int	numIndexes;
	bool external = false;
	
	if( !r_useExternalShadows.GetInteger() )
	{
		numIndexes = tri->numIndexes;
	}
	else if( r_useExternalShadows.GetInteger() == 2 )      // force to no caps for testing
	{
		numIndexes = tri->numShadowIndexesNoCaps;
	}
	else if( !( surf->dsFlags & DSF_VIEW_INSIDE_SHADOW ) )
	{
		// if we aren't inside the shadow projection, no caps are ever needed needed
		numIndexes = tri->numShadowIndexesNoCaps;
		external = true;
	}
	else if( !backEnd.vLight->viewInsideLight && !( surf->geo->shadowCapPlaneBits & SHADOW_CAP_INFINITE ) )
	{
		// if we are inside the shadow projection, but outside the light, and drawing
		// a non-infinite shadow, we can skip some caps
		if( backEnd.vLight->viewSeesShadowPlaneBits & surf->geo->shadowCapPlaneBits )
		{
			// we can see through a rear cap, so we need to draw it, but we can skip the
			// caps on the actual surface
			numIndexes = tri->numShadowIndexesNoFrontCaps;
		}
		else
		{
			// we don't need to draw any caps
			numIndexes = tri->numShadowIndexesNoCaps;
		}
		external = true;
	}
	else
	{
		// must draw everything
		numIndexes = tri->numIndexes;
	}
	
	// set depth bounds
#if !defined(USE_GLES1)
	if( glConfig.depthBoundsTestAvailable && r_useDepthBoundsTest.GetBool() )
	{
		glDepthBoundsEXT( surf->scissorRect.zmin, surf->scissorRect.zmax );
	}
#endif
	
	// debug visualization
	if( r_showShadows.GetInteger() )
	{
		if( r_showShadows.GetInteger() == 3 )
		{
			if( external )
			{
				glColor4f( 0.1 / backEnd.overBright, 1 / backEnd.overBright, 0.1 / backEnd.overBright, 1 );
			}
			else
			{
				// these are the surfaces that require the reverse
				glColor4f( 1 / backEnd.overBright, 0.1 / backEnd.overBright, 0.1 / backEnd.overBright, 1 );
			}
		}
		else
		{
			// draw different color for turboshadows
			if( surf->geo->shadowCapPlaneBits & SHADOW_CAP_INFINITE )
			{
				if( numIndexes == tri->numIndexes )
				{
					glColor4f( 1 / backEnd.overBright, 0.1 / backEnd.overBright, 0.1 / backEnd.overBright, 1 );
				}
				else
				{
					glColor4f( 1 / backEnd.overBright, 0.4 / backEnd.overBright, 0.1 / backEnd.overBright, 1 );
				}
			}
			else
			{
				if( numIndexes == tri->numIndexes )
				{
					glColor4f( 0.1 / backEnd.overBright, 1 / backEnd.overBright, 0.1 / backEnd.overBright, 1 );
				}
				else if( numIndexes == tri->numShadowIndexesNoFrontCaps )
				{
					glColor4f( 0.1 / backEnd.overBright, 1 / backEnd.overBright, 0.6 / backEnd.overBright, 1 );
				}
				else
				{
					glColor4f( 0.6 / backEnd.overBright, 1 / backEnd.overBright, 0.1 / backEnd.overBright, 1 );
				}
			}
		}
		
		glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
		glDisable( GL_STENCIL_TEST );
		GL_Cull( CT_TWO_SIDED );
		RB_DrawShadowElementsWithCounters( tri, numIndexes );
		GL_Cull( CT_FRONT_SIDED );
		glEnable( GL_STENCIL_TEST );
		
		return;
	}
	
	// patent-free work around
	if( !external )
	{
		// "preload" the stencil buffer with the number of volumes
		// that get clipped by the near or far clip plane
		glStencilOp( GL_KEEP, tr.stencilDecr, tr.stencilDecr );
		GL_Cull( CT_FRONT_SIDED );
		RB_DrawShadowElementsWithCounters( tri, numIndexes );
		glStencilOp( GL_KEEP, tr.stencilIncr, tr.stencilIncr );
		GL_Cull( CT_BACK_SIDED );
		RB_DrawShadowElementsWithCounters( tri, numIndexes );
	}
	
	// traditional depth-pass stencil shadows
	glStencilOp( GL_KEEP, GL_KEEP, tr.stencilIncr );
	GL_Cull( CT_FRONT_SIDED );
	RB_DrawShadowElementsWithCounters( tri, numIndexes );
	
	glStencilOp( GL_KEEP, GL_KEEP, tr.stencilDecr );
	GL_Cull( CT_BACK_SIDED );
	RB_DrawShadowElementsWithCounters( tri, numIndexes );
}

/*
=====================
RB_StencilShadowPass

Stencil test should already be enabled, and the stencil buffer should have
been set to 128 on any surfaces that might receive shadows
=====================
*/
void RB_StencilShadowPass( const drawSurf_t* drawSurfs )
{
	if( !r_shadows.GetBool() )
	{
		return;
	}
	
	if( !drawSurfs )
	{
		return;
	}
	
	RB_LogComment( "---------- RB_StencilShadowPass ----------\n" );
	
	globalImages->BindNull();
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	
	// for visualizing the shadows
	if( r_showShadows.GetInteger() )
	{
		if( r_showShadows.GetInteger() == 2 )
		{
			// draw filled in
			GL_State( GLS_DEPTHMASK | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHFUNC_LESS );
		}
		else
		{
			// draw as lines, filling the depth buffer
			GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO | GLS_POLYMODE_LINE | GLS_DEPTHFUNC_ALWAYS );
		}
	}
	else
	{
		// don't write to the color buffer, just the stencil buffer
		GL_State( GLS_DEPTHMASK | GLS_COLORMASK | GLS_ALPHAMASK | GLS_DEPTHFUNC_LESS );
	}
	
	if( r_shadowPolygonFactor.GetFloat() || r_shadowPolygonOffset.GetFloat() )
	{
		glPolygonOffset( r_shadowPolygonFactor.GetFloat(), -r_shadowPolygonOffset.GetFloat() );
		glEnable( GL_POLYGON_OFFSET_FILL );
	}
	
	glStencilFunc( GL_ALWAYS, 1, 255 );
	
#if !defined(USE_GLES1)
	if( glConfig.depthBoundsTestAvailable && r_useDepthBoundsTest.GetBool() )
	{
		glEnable( GL_DEPTH_BOUNDS_TEST_EXT );
	}
#endif
	
	RB_RenderDrawSurfChainWithFunction( drawSurfs, RB_T_Shadow );
	
	GL_Cull( CT_FRONT_SIDED );
	
	if( r_shadowPolygonFactor.GetFloat() || r_shadowPolygonOffset.GetFloat() )
	{
		glDisable( GL_POLYGON_OFFSET_FILL );
	}
	
#if !defined(USE_GLES1)
	if( glConfig.depthBoundsTestAvailable && r_useDepthBoundsTest.GetBool() )
	{
		glDisable( GL_DEPTH_BOUNDS_TEST_EXT );
	}
#endif
	
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	
	glStencilFunc( GL_GEQUAL, 128, 255 );
	glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
}



/*
=============================================================================================

BLEND LIGHT PROJECTION

=============================================================================================
*/

/*
=====================
RB_T_BlendLight

=====================
*/
static void RB_T_BlendLight( const drawSurf_t* surf )
{
	const srfTriangles_t* tri;
	
	tri = surf->geo;
	
	if( backEnd.currentSpace != surf->space )
	{
		idPlane	lightProject[4];
		int		i;
		
		for( i = 0 ; i < 4 ; i++ )
		{
			R_GlobalPlaneToLocal( surf->space->modelMatrix, backEnd.vLight->lightProject[i], lightProject[i] );
		}
		
		GL_SelectTexture( 0 );
#if !defined(USE_GLES1)
		glTexGenfv( GL_S, GL_OBJECT_PLANE, lightProject[0].ToFloatPtr() );
		glTexGenfv( GL_T, GL_OBJECT_PLANE, lightProject[1].ToFloatPtr() );
		glTexGenfv( GL_Q, GL_OBJECT_PLANE, lightProject[2].ToFloatPtr() );
#endif
		
		GL_SelectTexture( 1 );
		
#if !defined(USE_GLES1)
		glTexGenfv( GL_S, GL_OBJECT_PLANE, lightProject[3].ToFloatPtr() );
#endif
	}
	
	// this gets used for both blend lights and shadow draws
	if( tri->ambientCache )
	{
		idDrawVert*	ac = ( idDrawVert* )vertexCache.Position( tri->ambientCache );
		glVertexPointer( 3, GL_FLOAT, sizeof( idDrawVert ), ac->xyz.ToFloatPtr() );
	}
	else if( tri->shadowCache )
	{
		shadowCache_t*	sc = ( shadowCache_t* )vertexCache.Position( tri->shadowCache );
		glVertexPointer( 3, GL_FLOAT, sizeof( shadowCache_t ), sc->xyz.ToFloatPtr() );
	}
	
	RB_DrawElementsWithCounters( tri );
}


/*
=====================
RB_BlendLight

Dual texture together the falloff and projection texture with a blend
mode to the framebuffer, instead of interacting with the surface texture
=====================
*/
static void RB_BlendLight( const drawSurf_t* drawSurfs,  const drawSurf_t* drawSurfs2 )
{
	const idMaterial*	lightShader;
	const shaderStage_t*	stage;
	int					i;
	const float*	regs;
	
	if( !drawSurfs )
	{
		return;
	}
	if( r_skipBlendLights.GetBool() )
	{
		return;
	}
	RB_LogComment( "---------- RB_BlendLight ----------\n" );
	
	lightShader = backEnd.vLight->lightShader;
	regs = backEnd.vLight->shaderRegisters;
	
	// texture 1 will get the falloff texture
	GL_SelectTexture( 1 );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	
#if !defined(USE_GLES1)
	glEnable( GL_TEXTURE_GEN_S );
	glTexCoord2f( 0, 0.5 );
#endif
	backEnd.vLight->falloffImage->Bind();
	
	// texture 0 will get the projected texture
	GL_SelectTexture( 0 );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	
#if !defined(USE_GLES1)
	glEnable( GL_TEXTURE_GEN_S );
	glEnable( GL_TEXTURE_GEN_T );
	glEnable( GL_TEXTURE_GEN_Q );
#endif
	
	for( i = 0 ; i < lightShader->GetNumStages() ; i++ )
	{
		stage = lightShader->GetStage( i );
		
		if( !regs[ stage->conditionRegister ] )
		{
			continue;
		}
		
		GL_State( GLS_DEPTHMASK | stage->drawStateBits | GLS_DEPTHFUNC_EQUAL );
		
		GL_SelectTexture( 0 );
		stage->texture.image->Bind();
		
		if( stage->texture.hasMatrix )
		{
			RB_LoadShaderTextureMatrix( regs, &stage->texture );
		}
		
		// get the modulate values from the light, including alpha, unlike normal lights
		backEnd.lightColor[0] = regs[ stage->color.registers[0] ];
		backEnd.lightColor[1] = regs[ stage->color.registers[1] ];
		backEnd.lightColor[2] = regs[ stage->color.registers[2] ];
		backEnd.lightColor[3] = regs[ stage->color.registers[3] ];
		glColor4f( backEnd.lightColor[0], backEnd.lightColor[1], backEnd.lightColor[2], backEnd.lightColor[3] );
		
		RB_RenderDrawSurfChainWithFunction( drawSurfs, RB_T_BlendLight );
		RB_RenderDrawSurfChainWithFunction( drawSurfs2, RB_T_BlendLight );
		
		if( stage->texture.hasMatrix )
		{
			GL_SelectTexture( 0 );
			glMatrixMode( GL_TEXTURE );
			glLoadIdentity();
			glMatrixMode( GL_MODELVIEW );
		}
	}
	
	GL_SelectTexture( 1 );
	
#if !defined(USE_GLES1)
	glDisable( GL_TEXTURE_GEN_S );
#endif
	
	globalImages->BindNull();
	
	GL_SelectTexture( 0 );
	
#if !defined(USE_GLES1)
	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );
	glDisable( GL_TEXTURE_GEN_Q );
#endif
}


//========================================================================

static idPlane	fogPlanes[4];

/*
=====================
RB_T_BasicFog

=====================
*/
static void RB_T_BasicFog( const drawSurf_t* surf )
{
	if( backEnd.currentSpace != surf->space )
	{
		idPlane	local;
		
		GL_SelectTexture( 0 );
		
		R_GlobalPlaneToLocal( surf->space->modelMatrix, fogPlanes[0], local );
		local[3] += 0.5;
		
#if !defined(USE_GLES1)
		glTexGenfv( GL_S, GL_OBJECT_PLANE, local.ToFloatPtr() );
#endif
		
//		R_GlobalPlaneToLocal( surf->space->modelMatrix, fogPlanes[1], local );
//		local[3] += 0.5;
		local[0] = local[1] = local[2] = 0;
		local[3] = 0.5;
#if !defined(USE_GLES1)
		glTexGenfv( GL_T, GL_OBJECT_PLANE, local.ToFloatPtr() );
#endif
		
		GL_SelectTexture( 1 );
		
		// GL_S is constant per viewer
		R_GlobalPlaneToLocal( surf->space->modelMatrix, fogPlanes[2], local );
		local[3] += FOG_ENTER;
		
#if !defined(USE_GLES1)
		glTexGenfv( GL_T, GL_OBJECT_PLANE, local.ToFloatPtr() );
#endif
		
		R_GlobalPlaneToLocal( surf->space->modelMatrix, fogPlanes[3], local );
		
#if !defined(USE_GLES1)
		glTexGenfv( GL_S, GL_OBJECT_PLANE, local.ToFloatPtr() );
#endif
	}
	
	RB_T_RenderTriangleSurface( surf );
}



/*
==================
RB_FogPass
==================
*/
static void RB_FogPass( const drawSurf_t* drawSurfs,  const drawSurf_t* drawSurfs2 )
{
	const srfTriangles_t* frustumTris;
	drawSurf_t			ds;
	const idMaterial*	lightShader;
	const shaderStage_t*	stage;
	const float*			regs;
	
	RB_LogComment( "---------- RB_FogPass ----------\n" );
	
	// create a surface for the light frustom triangles, which are oriented drawn side out
	frustumTris = backEnd.vLight->frustumTris;
	
	// if we ran out of vertex cache memory, skip it
	if( !frustumTris->ambientCache )
	{
		return;
	}
	memset( &ds, 0, sizeof( ds ) );
	ds.space = &backEnd.viewDef->worldSpace;
	ds.geo = frustumTris;
	ds.scissorRect = backEnd.viewDef->scissor;
	
	// find the current color and density of the fog
	lightShader = backEnd.vLight->lightShader;
	regs = backEnd.vLight->shaderRegisters;
	// assume fog shaders have only a single stage
	stage = lightShader->GetStage( 0 );
	
	backEnd.lightColor[0] = regs[ stage->color.registers[0] ];
	backEnd.lightColor[1] = regs[ stage->color.registers[1] ];
	backEnd.lightColor[2] = regs[ stage->color.registers[2] ];
	backEnd.lightColor[3] = regs[ stage->color.registers[3] ];
	
	glColor4f( backEnd.lightColor[0], backEnd.lightColor[1], backEnd.lightColor[2], backEnd.lightColor[3] );
	
	// calculate the falloff planes
	float	a;
	
	// if they left the default value on, set a fog distance of 500
	if( backEnd.lightColor[3] <= 1.0 )
	{
		a = -0.5f / DEFAULT_FOG_DISTANCE;
	}
	else
	{
		// otherwise, distance = alpha color
		a = -0.5f / backEnd.lightColor[3];
	}
	
	GL_State( GLS_DEPTHMASK | GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA | GLS_DEPTHFUNC_EQUAL );
	
	// texture 0 is the falloff image
	GL_SelectTexture( 0 );
	globalImages->fogImage->Bind();
	//GL_Bind( tr.whiteImage );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	
#if !defined(USE_GLES1)
	glEnable( GL_TEXTURE_GEN_S );
	glEnable( GL_TEXTURE_GEN_T );
	glTexCoord2f( 0.5f, 0.5f );		// make sure Q is set
#endif
	
	fogPlanes[0][0] = a * backEnd.viewDef->worldSpace.modelViewMatrix[2];
	fogPlanes[0][1] = a * backEnd.viewDef->worldSpace.modelViewMatrix[6];
	fogPlanes[0][2] = a * backEnd.viewDef->worldSpace.modelViewMatrix[10];
	fogPlanes[0][3] = a * backEnd.viewDef->worldSpace.modelViewMatrix[14];
	
	fogPlanes[1][0] = a * backEnd.viewDef->worldSpace.modelViewMatrix[0];
	fogPlanes[1][1] = a * backEnd.viewDef->worldSpace.modelViewMatrix[4];
	fogPlanes[1][2] = a * backEnd.viewDef->worldSpace.modelViewMatrix[8];
	fogPlanes[1][3] = a * backEnd.viewDef->worldSpace.modelViewMatrix[12];
	
	
	// texture 1 is the entering plane fade correction
	GL_SelectTexture( 1 );
	globalImages->fogEnterImage->Bind();
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	
#if !defined(USE_GLES1)
	glEnable( GL_TEXTURE_GEN_S );
	glEnable( GL_TEXTURE_GEN_T );
#endif
	
	// T will get a texgen for the fade plane, which is always the "top" plane on unrotated lights
	fogPlanes[2][0] = 0.001f * backEnd.vLight->fogPlane[0];
	fogPlanes[2][1] = 0.001f * backEnd.vLight->fogPlane[1];
	fogPlanes[2][2] = 0.001f * backEnd.vLight->fogPlane[2];
	fogPlanes[2][3] = 0.001f * backEnd.vLight->fogPlane[3];
	
	// S is based on the view origin
	float s = backEnd.viewDef->renderView.vieworg * fogPlanes[2].Normal() + fogPlanes[2][3];
	
	fogPlanes[3][0] = 0;
	fogPlanes[3][1] = 0;
	fogPlanes[3][2] = 0;
	fogPlanes[3][3] = FOG_ENTER + s;
	
#if !defined(USE_GLES1)
	glTexCoord2f( FOG_ENTER + s, FOG_ENTER );
#endif
	
	// draw it
	RB_RenderDrawSurfChainWithFunction( drawSurfs, RB_T_BasicFog );
	RB_RenderDrawSurfChainWithFunction( drawSurfs2, RB_T_BasicFog );
	
	// the light frustum bounding planes aren't in the depth buffer, so use depthfunc_less instead
	// of depthfunc_equal
	GL_State( GLS_DEPTHMASK | GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA | GLS_DEPTHFUNC_LESS );
	GL_Cull( CT_BACK_SIDED );
	RB_RenderDrawSurfChainWithFunction( &ds, RB_T_BasicFog );
	GL_Cull( CT_FRONT_SIDED );
	
	GL_SelectTexture( 1 );
	
#if !defined(USE_GLES1)
	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );
#endif
	globalImages->BindNull();
	
	GL_SelectTexture( 0 );
	
#if !defined(USE_GLES1)
	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );
#endif
}


/*
==================
RB_STD_FogAllLights
==================
*/
void RB_STD_FogAllLights()
{
	viewLight_t*	vLight;
	
	if( r_skipFogLights.GetBool() || r_showOverDraw.GetInteger() != 0
			|| backEnd.viewDef->isXraySubview /* dont fog in xray mode*/
	  )
	{
		return;
	}
	
	RB_LogComment( "---------- RB_STD_FogAllLights ----------\n" );
	
	glDisable( GL_STENCIL_TEST );
	
	for( vLight = backEnd.viewDef->viewLights ; vLight ; vLight = vLight->next )
	{
		backEnd.vLight = vLight;
		
		if( !vLight->lightShader->IsFogLight() && !vLight->lightShader->IsBlendLight() )
		{
			continue;
		}
		
#if 0 // _D3XP disabled that
		if( r_ignore.GetInteger() )
		{
			// we use the stencil buffer to guarantee that no pixels will be
			// double fogged, which happens in some areas that are thousands of
			// units from the origin
			backEnd.currentScissor = vLight->scissorRect;
			if( r_useScissor.GetBool() )
			{
				glScissor( backEnd.viewDef->viewport.x1 + backEnd.currentScissor.x1,
						   backEnd.viewDef->viewport.y1 + backEnd.currentScissor.y1,
						   backEnd.currentScissor.x2 + 1 - backEnd.currentScissor.x1,
						   backEnd.currentScissor.y2 + 1 - backEnd.currentScissor.y1 );
			}
			glClear( GL_STENCIL_BUFFER_BIT );
			
			glEnable( GL_STENCIL_TEST );
			
			// only pass on the cleared stencil values
			glStencilFunc( GL_EQUAL, 128, 255 );
			
			// when we pass the stencil test and depth test and are going to draw,
			// increment the stencil buffer so we don't ever draw on that pixel again
			glStencilOp( GL_KEEP, GL_KEEP, GL_INCR );
		}
#endif
		
		if( vLight->lightShader->IsFogLight() )
		{
			RB_FogPass( vLight->globalInteractions, vLight->localInteractions );
		}
		else if( vLight->lightShader->IsBlendLight() )
		{
			RB_BlendLight( vLight->globalInteractions, vLight->localInteractions );
		}
		glDisable( GL_STENCIL_TEST );
	}
	
	glEnable( GL_STENCIL_TEST );
}

//=========================================================================================

/*
==================
RB_STD_LightScale

Perform extra blending passes to multiply the entire buffer by
a floating point value
==================
*/
void RB_STD_LightScale()
{
#if !defined(USE_GLES1)
	float	v, f;
	
	if( backEnd.overBright == 1.0f )
	{
		return;
	}
	
	if( r_skipLightScale.GetBool() )
	{
		return;
	}
	
	RB_LogComment( "---------- RB_STD_LightScale ----------\n" );
	
	// the scissor may be smaller than the viewport for subviews
	if( r_useScissor.GetBool() )
	{
		glScissor( backEnd.viewDef->viewport.x1 + backEnd.viewDef->scissor.x1,
				   backEnd.viewDef->viewport.y1 + backEnd.viewDef->scissor.y1,
				   backEnd.viewDef->scissor.x2 - backEnd.viewDef->scissor.x1 + 1,
				   backEnd.viewDef->scissor.y2 - backEnd.viewDef->scissor.y1 + 1 );
		backEnd.currentScissor = backEnd.viewDef->scissor;
	}
	
	// full screen blends
	glLoadIdentity();
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glOrtho( 0, 1, 0, 1, -1, 1 );
	
	GL_State( GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_SRC_COLOR );
	GL_Cull( CT_TWO_SIDED );	// so mirror views also get it
	globalImages->BindNull();
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_STENCIL_TEST );
	
	v = 1;
	while( idMath::Fabs( v - backEnd.overBright ) > 0.01 )  	// a little extra slop
	{
		f = backEnd.overBright / v;
		f /= 2;
		if( f > 1 )
		{
			f = 1;
		}
		glColor3f( f, f, f );
		v = v * f * 2;
		
		glBegin( GL_QUADS );
		glVertex2f( 0, 0 );
		glVertex2f( 0, 1 );
		glVertex2f( 1, 1 );
		glVertex2f( 1, 0 );
		glEnd();
	}
	
	
	glPopMatrix();
	glEnable( GL_DEPTH_TEST );
	glMatrixMode( GL_MODELVIEW );
	GL_Cull( CT_FRONT_SIDED );
#endif // #if !defined(USE_GLES1)
}

//=========================================================================================

/*
=============
RB_STD_DrawView
=============
*/
void	RB_STD_DrawView()
{
	drawSurf_t** 	drawSurfs;
	int			numDrawSurfs;
	
	RB_LogComment( "---------- RB_STD_DrawView ----------\n" );
	
	backEnd.depthFunc = GLS_DEPTHFUNC_EQUAL;
	
	drawSurfs = ( drawSurf_t** )&backEnd.viewDef->drawSurfs[0];
	numDrawSurfs = backEnd.viewDef->numDrawSurfs;
	
#if 0 // !defined(USE_GLES1)
	if( tr.backEndRenderer == BE_EXP && r_useHighDynamicRange.GetBool() )
	{
		globalFramebuffers.hdrRender->Bind();
	}
#endif
	
	GL_CheckErrors();
	
	// clear the z buffer, set the projection matrix, etc
	RB_BeginDrawingView();
	
	GL_CheckErrors();
	
	// decide how much overbrighting we are going to do
	RB_DetermineLightScale();
	
	// fill the depth buffer and clear color buffer to black except on
	// subviews
	RB_STD_FillDepthBuffer( drawSurfs, numDrawSurfs );
	
	GL_CheckErrors();
	
	// main light renderer
	switch( tr.backEndRenderer )
	{
		case BE_ARB:
			RB_ARB_DrawInteractions();
			break;
#if !defined(USE_GLES1)
		case BE_ARB2:
			RB_ARB2_DrawInteractions();
			break;
		case BE_GLSL:
			RB_GLSL_DrawInteractions();
			break;
		case BE_EXP:
			RB_Exp_DrawInteractions();
			if( r_useDeferredShading.GetBool() )
			{
				RB_EXP_ResolveLightFromLightBuffer( drawSurfs, numDrawSurfs );
			}
			break;
#endif
	}
	
	// disable stencil shadow test
	glStencilFunc( GL_ALWAYS, 128, 255 );
	
	GL_CheckErrors();
	
	// uplight the entire screen to crutch up not having better blending range
	RB_STD_LightScale();
	
	GL_CheckErrors();
	
	// now draw any non-light dependent shading passes
	int	processed = RB_STD_DrawShaderPasses( drawSurfs, numDrawSurfs );
	
	GL_CheckErrors();
	
	// fob and blend lights
	RB_STD_FogAllLights();
	
	GL_CheckErrors();
	
	// now draw any post-processing effects using _currentRender
	if( processed < numDrawSurfs )
	{
		RB_STD_DrawShaderPasses( drawSurfs + processed, numDrawSurfs - processed );
	}
	
	GL_CheckErrors();
	
	RB_RenderDebugTools( drawSurfs, numDrawSurfs );
	
	GL_CheckErrors();
}
