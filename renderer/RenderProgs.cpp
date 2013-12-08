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



idRenderProgManager renderProgManager;

/*
================================================================================================
idRenderProgManager::idRenderProgManager()
================================================================================================
*/
idRenderProgManager::idRenderProgManager()
{
}

/*
================================================================================================
idRenderProgManager::~idRenderProgManager()
================================================================================================
*/
idRenderProgManager::~idRenderProgManager()
{
}

/*
================================================================================================
R_ReloadShaders
================================================================================================
*/
static void R_ReloadShaders( const idCmdArgs& args )
{
	renderProgManager.KillAllShaders();
	renderProgManager.LoadAllShaders();
}

/*
================================================================================================
idRenderProgManager::Init()
================================================================================================
*/
void idRenderProgManager::Init()
{
	common->Printf( "----- Initializing Render Shaders -----\n" );
	
	
	for( int i = 0; i < MAX_BUILTINS; i++ )
	{
		builtinShaders[i] = -1;
	}
	struct builtinShaders_t
	{
		int index;
		const char* name;
	} builtins[] =
	{
		{ BUILTIN_GUI, "builtin/gui.vfp" },
		{ BUILTIN_COLOR, "builtin/color.vfp" },
		// RB begin
#if defined(USE_GPU_SKINNING)
		{ BUILTIN_COLOR_SKINNED, "builtin/color_skinned.vfp" },
#endif
		{ BUILTIN_VERTEX_COLOR, "builtin/vertex_color.vfp" },
		{ BUILTIN_VERTEX_LIGHTING, "builtin/vertex_lighting.vfp" },
		{ BUILTIN_GRID_LIGHTING, "builtin/grid_lighting.vfp" },
#if defined(USE_GPU_SKINNING)
		{ BUILTIN_GRID_LIGHTING_SKINNED, "builtin/grid_lighting_skinned.vfp" },
#endif
		// RB end
#if !defined(USE_GLES2) && !defined(USE_GLES3)
		{ BUILTIN_SIMPLESHADE, "builtin/simpleshade.vfp" },
#endif
		{ BUILTIN_TEXTURED, "builtin/texture.vfp" },
		{ BUILTIN_TEXTURE_VERTEXCOLOR, "builtin/texture_color.vfp" },
#if defined(USE_GPU_SKINNING)
		{ BUILTIN_TEXTURE_VERTEXCOLOR_SKINNED, "builtin/texture_color_skinned.vfp" },
#endif
		{ BUILTIN_TEXTURE_TEXGEN_VERTEXCOLOR, "builtin/texture_color_texgen.vfp" },
		// RB begin
		{ BUILTIN_TEXTURE_YCOCG, "builtin/texture_ycocg.vfp" },
		// RB end
		{ BUILTIN_INTERACTION, "builtin/interaction.vfp" },
#if defined(USE_GPU_SKINNING)
		{ BUILTIN_INTERACTION_SKINNED, "builtin/interaction_skinned.vfp" },
#endif
		{ BUILTIN_INTERACTION_AMBIENT, "builtin/interactionAmbient.vfp" },
#if defined(USE_GPU_SKINNING)
		{ BUILTIN_INTERACTION_AMBIENT_SKINNED, "builtin/interactionAmbient_skinned.vfp" },
#endif
		{ BUILTIN_ENVIRONMENT, "builtin/environment.vfp" },
#if defined(USE_GPU_SKINNING)
		{ BUILTIN_ENVIRONMENT_SKINNED, "builtin/environment_skinned.vfp" },
#endif
		{ BUILTIN_BUMPY_ENVIRONMENT, "builtin/bumpyEnvironment.vfp" },
#if defined(USE_GPU_SKINNING)
		{ BUILTIN_BUMPY_ENVIRONMENT_SKINNED, "builtin/bumpyEnvironment_skinned.vfp" },
#endif
		
		{ BUILTIN_DEPTH, "builtin/depth.vfp" },
#if defined(USE_GPU_SKINNING)
		{ BUILTIN_DEPTH_SKINNED, "builtin/depth_skinned.vfp" },
#endif
		{ BUILTIN_SHADOW_DEBUG, "builtin/shadowDebug.vfp" },
#if defined(USE_GPU_SKINNING)
		{ BUILTIN_SHADOW_DEBUG_SKINNED, "builtin/shadowDebug_skinned.vfp" },
#endif
		
		// RB begin
		{ BUILTIN_BLENDLIGHT, "builtin/blendLight.vfp" },
#if defined(USE_GPU_SKINNING)
		{ BUILTIN_BLENDLIGHT_SKINNED, "builtin/blendLight_skinned.vfp" },
#endif
		// RB end
		{ BUILTIN_FOG, "builtin/fog.vfp" },
#if defined(USE_GPU_SKINNING)
		{ BUILTIN_FOG_SKINNED, "builtin/fog_skinned.vfp" },
#endif
		{ BUILTIN_SKYBOX, "builtin/skybox.vfp" },
		{ BUILTIN_WOBBLESKY, "builtin/wobblesky.vfp" },
		{ BUILTIN_POSTPROCESS, "builtin/postprocess.vfp" },
		{ BUILTIN_STEREO_DEGHOST, "builtin/stereoDeGhost.vfp" },
		{ BUILTIN_STEREO_WARP, "builtin/stereoWarp.vfp" },
//		{ BUILTIN_ZCULL_RECONSTRUCT, "zcullReconstruct.vfp" },
		{ BUILTIN_BINK, "builtin/bink.vfp" },
		{ BUILTIN_BINK_GUI, "builtin/bink_gui.vfp" },
		// RB begin
		{ BUILTIN_ROQ, "builtin/roq.vfp" },
		// RB end
		{ BUILTIN_STEREO_INTERLACE, "builtin/stereoInterlace.vfp" },
#if 1//defined(USE_GLES2) && !defined(USE_GLES3)
		{ BUILTIN_SHADOW, "builtin/shadow.vfp" },
#endif
		
#if !defined(USE_GLES2) && !defined(USE_GLES3)
		{ BUILTIN_MOTION_BLUR, "builtin/motionBlur.vfp" },
#endif
	};
	int numBuiltins = sizeof( builtins ) / sizeof( builtins[0] );
	vertexShaders.SetNum( numBuiltins );
	fragmentShaders.SetNum( numBuiltins );
	glslPrograms.SetNum( numBuiltins );
	
	for( int i = 0; i < numBuiltins; i++ )
	{
		vertexShaders[i].name = builtins[i].name;
		fragmentShaders[i].name = builtins[i].name;
		builtinShaders[builtins[i].index] = i;
		LoadVertexShader( i );
		LoadFragmentShader( i );
		LoadGLSLProgram( i, i, i );
	}
	
	// Special case handling for fastZ shaders
#if defined(USE_GPU_SKINNING)
	builtinShaders[BUILTIN_SHADOW] = FindVertexShader( "builtin/shadow.vp" );
	builtinShaders[BUILTIN_SHADOW_SKINNED] = FindVertexShader( "builtin/shadow_skinned.vp" );
	
	FindGLSLProgram( "builtin/shadow.vp", builtinShaders[BUILTIN_SHADOW], -1 );
	FindGLSLProgram( "builtin/shadow_skinned.vp", builtinShaders[BUILTIN_SHADOW_SKINNED], -1 );
#endif
	
	glslUniforms.SetNum( RENDERPARM_USER + MAX_GLSL_USER_PARMS, vec4_zero );
	
#if defined(USE_GPU_SKINNING)
	vertexShaders[builtinShaders[BUILTIN_TEXTURE_VERTEXCOLOR_SKINNED]].usesJoints = true;
	vertexShaders[builtinShaders[BUILTIN_INTERACTION_SKINNED]].usesJoints = true;
	vertexShaders[builtinShaders[BUILTIN_INTERACTION_AMBIENT_SKINNED]].usesJoints = true;
	vertexShaders[builtinShaders[BUILTIN_ENVIRONMENT_SKINNED]].usesJoints = true;
	vertexShaders[builtinShaders[BUILTIN_BUMPY_ENVIRONMENT_SKINNED]].usesJoints = true;
	vertexShaders[builtinShaders[BUILTIN_DEPTH_SKINNED]].usesJoints = true;
	vertexShaders[builtinShaders[BUILTIN_SHADOW_SKINNED]].usesJoints = true;
	vertexShaders[builtinShaders[BUILTIN_SHADOW_DEBUG_SKINNED]].usesJoints = true;
	vertexShaders[builtinShaders[BUILTIN_FOG_SKINNED]].usesJoints = true;
	// RB begin
	vertexShaders[builtinShaders[BUILTIN_COLOR_SKINNED]].usesJoints = true;
	vertexShaders[builtinShaders[BUILTIN_GRID_LIGHTING_SKINNED]].usesJoints = true;
	vertexShaders[builtinShaders[BUILTIN_BLENDLIGHT_SKINNED]].usesJoints = true;
	// RB end
#endif
	
	cmdSystem->AddCommand( "reloadShaders", R_ReloadShaders, CMD_FL_RENDERER, "reloads shaders" );
}

/*
================================================================================================
idRenderProgManager::LoadAllShaders()
================================================================================================
*/
void idRenderProgManager::LoadAllShaders()
{
	for( int i = 0; i < vertexShaders.Num(); i++ )
	{
		LoadVertexShader( i );
	}
	for( int i = 0; i < fragmentShaders.Num(); i++ )
	{
		LoadFragmentShader( i );
	}
	
	for( int i = 0; i < glslPrograms.Num(); ++i )
	{
		LoadGLSLProgram( i, glslPrograms[i].vertexShaderIndex, glslPrograms[i].fragmentShaderIndex );
	}
}

/*
================================================================================================
idRenderProgManager::KillAllShaders()
================================================================================================
*/
void idRenderProgManager::KillAllShaders()
{
	Unbind();
	for( int i = 0; i < vertexShaders.Num(); i++ )
	{
		if( vertexShaders[i].progId != INVALID_PROGID )
		{
			glDeleteShader( vertexShaders[i].progId );
			vertexShaders[i].progId = INVALID_PROGID;
		}
	}
	for( int i = 0; i < fragmentShaders.Num(); i++ )
	{
		if( fragmentShaders[i].progId != INVALID_PROGID )
		{
			glDeleteShader( fragmentShaders[i].progId );
			fragmentShaders[i].progId = INVALID_PROGID;
		}
	}
	for( int i = 0; i < glslPrograms.Num(); ++i )
	{
		if( glslPrograms[i].progId != INVALID_PROGID )
		{
			glDeleteProgram( glslPrograms[i].progId );
			glslPrograms[i].progId = INVALID_PROGID;
		}
	}
}

/*
================================================================================================
idRenderProgManager::Shutdown()
================================================================================================
*/
void idRenderProgManager::Shutdown()
{
	KillAllShaders();
}

/*
================================================================================================
idRenderProgManager::FindVertexShader
================================================================================================
*/
int idRenderProgManager::FindVertexShader( const char* name )
{
	for( int i = 0; i < vertexShaders.Num(); i++ )
	{
		if( vertexShaders[i].name.Icmp( name ) == 0 )
		{
			LoadVertexShader( i );
			return i;
		}
	}
	vertexShader_t shader;
	shader.name = name;
	int index = vertexShaders.Append( shader );
	LoadVertexShader( index );
	currentVertexShader = index;
	
	// RB: removed idStr::Icmp( name, "heatHaze.vfp" ) == 0  hack
	// this requires r_useUniformArrays 1
	for( int i = 0; i < vertexShaders[index].uniforms.Num(); i++ )
	{
		if( vertexShaders[index].uniforms[i] == RENDERPARM_ENABLE_SKINNING )
		{
			vertexShaders[index].usesJoints = true;
			vertexShaders[index].optionalSkinning = true;
		}
	}
	// RB end
	
	return index;
}

/*
================================================================================================
idRenderProgManager::FindFragmentShader
================================================================================================
*/
int idRenderProgManager::FindFragmentShader( const char* name )
{
	for( int i = 0; i < fragmentShaders.Num(); i++ )
	{
		if( fragmentShaders[i].name.Icmp( name ) == 0 )
		{
			LoadFragmentShader( i );
			return i;
		}
	}
	fragmentShader_t shader;
	shader.name = name;
	int index = fragmentShaders.Append( shader );
	LoadFragmentShader( index );
	currentFragmentShader = index;
	return index;
}




/*
================================================================================================
idRenderProgManager::LoadVertexShader
================================================================================================
*/
void idRenderProgManager::LoadVertexShader( int index )
{
	if( vertexShaders[index].progId != INVALID_PROGID )
	{
		return; // Already loaded
	}
	vertexShaders[index].progId = ( GLuint ) LoadGLSLShader( GL_VERTEX_SHADER, vertexShaders[index].name, vertexShaders[index].uniforms );
}

/*
================================================================================================
idRenderProgManager::LoadFragmentShader
================================================================================================
*/
void idRenderProgManager::LoadFragmentShader( int index )
{
	if( fragmentShaders[index].progId != INVALID_PROGID )
	{
		return; // Already loaded
	}
	fragmentShaders[index].progId = ( GLuint ) LoadGLSLShader( GL_FRAGMENT_SHADER, fragmentShaders[index].name, fragmentShaders[index].uniforms );
}

/*
================================================================================================
idRenderProgManager::BindShader
================================================================================================
*/
// RB begin
void idRenderProgManager::BindShader( int progIndex, int vIndex, int fIndex, bool builtin )
{
	if( currentVertexShader == vIndex && currentFragmentShader == fIndex )
	{
		return;
	}
	
	if( builtin )
	{
		currentVertexShader = vIndex;
		currentFragmentShader = fIndex;
		
		// vIndex denotes the GLSL program
		if( vIndex >= 0 && vIndex < glslPrograms.Num() )
		{
			currentRenderProgram = vIndex;
			RENDERLOG_PRINTF( "Binding GLSL Program %s\n", glslPrograms[vIndex].name.c_str() );
			glUseProgram( glslPrograms[vIndex].progId );
		}
	}
	else
	{
		if( progIndex == -1 )
		{
			// RB: FIXME linear search
			for( int i = 0; i < glslPrograms.Num(); ++i )
			{
				if( ( glslPrograms[i].vertexShaderIndex == vIndex ) && ( glslPrograms[i].fragmentShaderIndex == fIndex ) )
				{
					progIndex = i;
					break;
				}
			}
		}
		
		currentVertexShader = vIndex;
		currentFragmentShader = fIndex;
		
		if( progIndex >= 0 && progIndex < glslPrograms.Num() )
		{
			currentRenderProgram = progIndex;
			RENDERLOG_PRINTF( "Binding GLSL Program %s\n", glslPrograms[progIndex].name.c_str() );
			glUseProgram( glslPrograms[progIndex].progId );
		}
	}
}
// RB end

/*
================================================================================================
idRenderProgManager::Unbind
================================================================================================
*/
void idRenderProgManager::Unbind()
{
	currentVertexShader = -1;
	currentFragmentShader = -1;
	
	glUseProgram( 0 );
}

// RB begin
bool idRenderProgManager::IsShaderBound() const
{
	return ( currentVertexShader != -1 );
}
// RB end

/*
================================================================================================
idRenderProgManager::SetRenderParms
================================================================================================
*/
void idRenderProgManager::SetRenderParms( renderParm_t rp, const float* value, int num )
{
	for( int i = 0; i < num; i++ )
	{
		SetRenderParm( ( renderParm_t )( rp + i ), value + ( i * 4 ) );
	}
}

/*
================================================================================================
idRenderProgManager::SetRenderParm
================================================================================================
*/
void idRenderProgManager::SetRenderParm( renderParm_t rp, const float* value )
{
	SetUniformValue( rp, value );
}

