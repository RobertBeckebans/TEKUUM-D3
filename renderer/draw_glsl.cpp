/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012 Robert Beckebans

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

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "GLShader.h"
//#include "tr_local.h"


void GL_BindProgram(shaderProgram_t * program)
{
	if(!program)
	{
		GL_BindNullProgram();
		return;
	}

	if(r_logFile.GetBool())
	{
		// don't just call LogComment, or we will get a call to va() every frame!
		RB_LogComment("--- GL_BindProgram( name = '%s', macros = '%s' ) ---\n", program->name.c_str(), program->compileMacros);
	}

	if(backEnd.glState.currentProgram != program)
	{
		glUseProgramObjectARB(program->program);
		backEnd.glState.currentProgram = program;
	}
}

void GL_BindNullProgram(void)
{
	if(r_logFile.GetBool())
	{
		RB_LogComment("--- GL_BindNullProgram ---\n");
	}

	if(backEnd.glState.currentProgram)
	{
		glUseProgramObjectARB(0);
		backEnd.glState.currentProgram = NULL;
	}
}

/*
=========================================================================================

GENERAL INTERACTION RENDERING

=========================================================================================
*/

/*
====================
GL_SelectTextureNoClient
====================
*/
static void GL_SelectTextureNoClient( int unit ) {
	backEnd.glState.currenttmu = unit;
	glActiveTextureARB( GL_TEXTURE0_ARB + unit );
	RB_LogComment( "glActiveTextureARB( %i )\n", unit );
}

/*
==================
RB_GLSL_DrawInteraction
==================
*/
static void	RB_GLSL_DrawInteraction( const drawInteraction_t *din ) {

	// choose and bind the vertex program
	// TODO gl_forwardLightingShader->SetAmbientLighting(backEnd.vLight->lightShader->IsAmbientLight());
	gl_forwardLightingShader->SetNormalMapping(!r_skipBump.GetBool());
	gl_forwardLightingShader->BindProgram();

	// load all the vertex program parameters
	gl_forwardLightingShader->SetUniform_ViewOrigin(din->localViewOrigin.ToVec3());

	gl_forwardLightingShader->SetUniform_LightOrigin(din->localLightOrigin.ToVec3());

	gl_forwardLightingShader->SetUniform_LightProjectS(din->lightProjection[0]);
	gl_forwardLightingShader->SetUniform_LightProjectT(din->lightProjection[1]);
	gl_forwardLightingShader->SetUniform_LightProjectQ(din->lightProjection[2]);
	gl_forwardLightingShader->SetUniform_LightFalloffS(din->lightProjection[3]);

	gl_forwardLightingShader->SetUniform_DiffuseMatrixS(din->diffuseMatrix[0]);
	gl_forwardLightingShader->SetUniform_DiffuseMatrixT(din->diffuseMatrix[1]);

	gl_forwardLightingShader->SetUniform_BumpMatrixS(din->bumpMatrix[0]);
	gl_forwardLightingShader->SetUniform_BumpMatrixT(din->bumpMatrix[1]);

	gl_forwardLightingShader->SetUniform_SpecularMatrixS(din->specularMatrix[0]);
	gl_forwardLightingShader->SetUniform_SpecularMatrixT(din->specularMatrix[1]);

	static const idVec4 zero( 0, 0, 0, 0 );
	static const idVec4 one( 1, 1, 1, 1 );
	static const idVec4 negOne( -1, -1, -1, -1 );

	switch ( din->vertexColor ) {
	case SVC_IGNORE:
		gl_forwardLightingShader->SetUniform_ColorModulate(zero);
		gl_forwardLightingShader->SetUniform_Color(one);
		break;
	case SVC_MODULATE:
		gl_forwardLightingShader->SetUniform_ColorModulate(one);
		gl_forwardLightingShader->SetUniform_Color(zero);
		break;
	case SVC_INVERSE_MODULATE:
		gl_forwardLightingShader->SetUniform_ColorModulate(negOne);
		gl_forwardLightingShader->SetUniform_Color(one);
		break;
	}

	// set the constant colors
	gl_forwardLightingShader->SetUniform_DiffuseColor(din->diffuseColor);
	gl_forwardLightingShader->SetUniform_SpecularColor(din->specularColor);

	// set the textures

	// texture 1 will be the per-surface bump map
	GL_SelectTextureNoClient( 1 );
	din->bumpImage->Bind();

	// texture 2 will be the light falloff texture
	GL_SelectTextureNoClient( 2 );
	din->lightFalloffImage->Bind();

	// texture 3 will be the light projection texture
	GL_SelectTextureNoClient( 3 );
	din->lightImage->Bind();

	// texture 4 is the per-surface diffuse map
	GL_SelectTextureNoClient( 4 );
	din->diffuseImage->Bind();

	// texture 5 is the per-surface specular map
	GL_SelectTextureNoClient( 5 );
	din->specularImage->Bind();

	// draw it
	RB_DrawElementsWithCounters( din->surf->geo );
}


/*
=============
RB_GLSL_CreateDrawInteractions
=============
*/
static void RB_GLSL_CreateDrawInteractions( const drawSurf_t *surf ) {
	if ( !surf ) {
		return;
	}

	// perform setup here that will be constant for all interactions
	GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHMASK | backEnd.depthFunc );

	// enable the vertex arrays
	glEnableVertexAttribArrayARB( VA_INDEX_TEXCOORD0 );
	glEnableVertexAttribArrayARB( VA_INDEX_TANGENT );
	glEnableVertexAttribArrayARB( VA_INDEX_BITANGENT );
	glEnableVertexAttribArrayARB( VA_INDEX_NORMAL );
	glEnableClientState( GL_COLOR_ARRAY );

	// texture 0 is the normalization cube map for the vector towards the light
	GL_SelectTextureNoClient( 0 );
	if ( backEnd.vLight->lightShader->IsAmbientLight() ) {
		globalImages->ambientNormalMap->Bind();
	} else {
		globalImages->normalCubeMapImage->Bind();
	}

	// texture 6 is the specular lookup table
	/*
	GL_SelectTextureNoClient( 6 );
	if ( r_testARBProgram.GetBool() ) {
		globalImages->specular2DTableImage->Bind();	// variable specularity in alpha channel
	} else {
		globalImages->specularTableImage->Bind();
	}
	*/

	for ( ; surf ; surf=surf->nextOnLight ) {
		// perform setup here that will not change over multiple interaction passes

		// set the vertex pointers
		idDrawVert	*ac = (idDrawVert *)vertexCache.Position( surf->geo->ambientCache );
		glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( idDrawVert ), ac->color );
		glVertexAttribPointerARB( VA_INDEX_NORMAL, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->normal.ToFloatPtr() );
		glVertexAttribPointerARB( VA_INDEX_BITANGENT, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->tangents[1].ToFloatPtr() );
		glVertexAttribPointerARB( VA_INDEX_TANGENT, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->tangents[0].ToFloatPtr() );
		glVertexAttribPointerARB( VA_INDEX_TEXCOORD0, 2, GL_FLOAT, false, sizeof( idDrawVert ), ac->st.ToFloatPtr() );
		glVertexPointer( 3, GL_FLOAT, sizeof( idDrawVert ), ac->xyz.ToFloatPtr() );

		// this may cause RB_GLSL_DrawInteraction to be exacuted multiple
		// times with different colors and images if the surface or light have multiple layers
		RB_CreateSingleDrawInteractions( surf, RB_GLSL_DrawInteraction );
	}

	glDisableVertexAttribArrayARB( VA_INDEX_TEXCOORD0 );
	glDisableVertexAttribArrayARB( VA_INDEX_TANGENT );
	glDisableVertexAttribArrayARB( VA_INDEX_BITANGENT );
	glDisableVertexAttribArrayARB( VA_INDEX_NORMAL );
	glDisableClientState( GL_COLOR_ARRAY );

	// disable features
	GL_SelectTextureNoClient( 6 );
	globalImages->BindNull();

	GL_SelectTextureNoClient( 5 );
	globalImages->BindNull();

	GL_SelectTextureNoClient( 4 );
	globalImages->BindNull();

	GL_SelectTextureNoClient( 3 );
	globalImages->BindNull();

	GL_SelectTextureNoClient( 2 );
	globalImages->BindNull();

	GL_SelectTextureNoClient( 1 );
	globalImages->BindNull();

	backEnd.glState.currenttmu = -1;
	GL_SelectTexture( 0 );

	GL_BindNullProgram();
}


/*
==================
RB_GLSL_DrawInteractions
==================
*/
void RB_GLSL_DrawInteractions( void ) {
	viewLight_t		*vLight;
	const idMaterial	*lightShader;

	GL_SelectTexture( 0 );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );

	//
	// for each light, perform adding and shadowing
	//
	for ( vLight = backEnd.viewDef->viewLights ; vLight ; vLight = vLight->next ) {
		backEnd.vLight = vLight;

		// do fogging later
		if ( vLight->lightShader->IsFogLight() ) {
			continue;
		}
		if ( vLight->lightShader->IsBlendLight() ) {
			continue;
		}

		if ( !vLight->localInteractions && !vLight->globalInteractions
			&& !vLight->translucentInteractions ) {
			continue;
		}

		lightShader = vLight->lightShader;

		// clear the stencil buffer if needed
		if ( vLight->globalShadows || vLight->localShadows ) {
			backEnd.currentScissor = vLight->scissorRect;
			if ( r_useScissor.GetBool() ) {
				glScissor( backEnd.viewDef->viewport.x1 + backEnd.currentScissor.x1, 
					backEnd.viewDef->viewport.y1 + backEnd.currentScissor.y1,
					backEnd.currentScissor.x2 + 1 - backEnd.currentScissor.x1,
					backEnd.currentScissor.y2 + 1 - backEnd.currentScissor.y1 );
			}
			glClear( GL_STENCIL_BUFFER_BIT );
		} else {
			// no shadows, so no need to read or write the stencil buffer
			// we might in theory want to use GL_ALWAYS instead of disabling
			// completely, to satisfy the invarience rules
			glStencilFunc( GL_ALWAYS, 128, 255 );
		}

		if ( r_useShadowVertexProgram.GetBool() ) 
		{
			gl_shadowVolumeShader->BindProgram();
			
			RB_StencilShadowPass( vLight->globalShadows );
			RB_GLSL_CreateDrawInteractions( vLight->localInteractions );
			
			gl_shadowVolumeShader->BindProgram();

			RB_StencilShadowPass( vLight->localShadows );
			RB_GLSL_CreateDrawInteractions( vLight->globalInteractions );

			//glDisable( GL_VERTEX_PROGRAM_ARB );	// if there weren't any globalInteractions, it would have stayed on
			GL_BindNullProgram();
		} else {
			RB_StencilShadowPass( vLight->globalShadows );
			RB_GLSL_CreateDrawInteractions( vLight->localInteractions );
			RB_StencilShadowPass( vLight->localShadows );
			RB_GLSL_CreateDrawInteractions( vLight->globalInteractions );
		}

		// translucent surfaces never get stencil shadowed
		if ( r_skipTranslucent.GetBool() ) {
			continue;
		}

		glStencilFunc( GL_ALWAYS, 128, 255 );

		backEnd.depthFunc = GLS_DEPTHFUNC_LESS;
		RB_GLSL_CreateDrawInteractions( vLight->translucentInteractions );

		backEnd.depthFunc = GLS_DEPTHFUNC_EQUAL;
	}

	// disable stencil shadow test
	glStencilFunc( GL_ALWAYS, 128, 255 );

	GL_SelectTexture( 0 );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
}

//===================================================================================






/*
==================
R_FindARBProgram

Returns a GL identifier that can be bound to the given target, parsing
a text file if it hasn't already been loaded.
==================
*/
int R_FindShader( GLenum target, const char *program ) {
#if 0
	int		i;
	idStr	stripped = program;

	stripped.StripFileExtension();

	// see if it is already loaded
	for ( i = 0 ; progs[i].name[0] ; i++ ) {
		if ( progs[i].target != target ) {
			continue;
		}

		idStr	compare = progs[i].name;
		compare.StripFileExtension();

		if ( !idStr::Icmp( stripped.c_str(), compare.c_str() ) ) {
			return progs[i].ident;
		}
	}

	if ( i == MAX_GLPROGS ) {
		common->Error( "R_FindARBProgram: MAX_GLPROGS" );
	}

	// add it to the list and load it
	progs[i].ident = (program_t)0;	// will be gen'd by R_LoadARBProgram
	progs[i].target = target;
	strncpy( progs[i].name, program, sizeof( progs[i].name ) - 1 );

	R_LoadARBProgram( i );

	return progs[i].ident;
#else
	return 0;
#endif
}

/*
==================
R_ReloadShaders_f
==================
*/
void R_ReloadShaders_f( const idCmdArgs &args ) {

	common->Printf( "----- R_ReloadShaders -----\n" );
	
	if ( GLEW_ARB_fragment_shader && GLEW_ARB_vertex_shader && GL_ARB_shader_objects && GLEW_ARB_shading_language_100 ) 
	{
		if(gl_forwardLightingShader)
		{
			delete gl_forwardLightingShader;
			gl_forwardLightingShader = NULL;
		}
		gl_forwardLightingShader = new GLShader_forwardLighting();

		if(gl_shadowVolumeShader)
		{
			delete gl_shadowVolumeShader;
			gl_shadowVolumeShader = NULL;
		}
		gl_shadowVolumeShader = new GLShader_shadowVolume();
	}

	common->Printf( "-------------------------------\n" );
}

/*
==================
R_GLSL_Init
==================
*/
void R_GLSL_Init( void ) {
	glConfig.allowARB2Path = false;

	common->Printf( "---------- R_GLSL_Init ----------\n" );

	if ( !GLEW_ARB_fragment_shader || !GLEW_ARB_vertex_shader || !GL_ARB_shader_objects || !GLEW_ARB_shading_language_100 ) {
		common->Printf( "Not available.\n" );
		return;
	}

	common->Printf( "Available.\n" );

	common->Printf( "---------------------------------\n" );

	glConfig.allowARB2Path = true;
}

