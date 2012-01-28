/*
===========================================================================
Copyright (C) 2010-2012 Robert Beckebans

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// gl_shader.cpp -- GLSL shader handling

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "GLShader.h"

// *INDENT-OFF*


//GLShader_generic* gl_genericShader = NULL;
GLShader_forwardLighting* gl_forwardLightingShader = NULL;
GLShader_shadowVolume* gl_shadowVolumeShader = NULL;
//GLShader_screen* gl_screenShader = NULL;
//GLShader_portal* gl_portalShader = NULL;
//GLShader_toneMapping* gl_toneMappingShader = NULL;
//GLShader_contrast* gl_contrastShader = NULL;
//GLShader_cameraEffects* gl_cameraEffectsShader = NULL;
//GLShader_blurX* gl_blurXShader = NULL;
//GLShader_blurY* gl_blurYShader = NULL;
//GLShader_debugShadowMap* gl_debugShadowMapShader = NULL;



bool GLCompileMacro_USE_VERTEX_SKINNING::HasConflictingMacros(int permutation, const idList<GLCompileMacro*>& macros) const
{
	for(int i = 0; i < macros.Num(); i++)
	{
		GLCompileMacro* macro = macros[i];

		//if(GLCompileMacro_USE_VERTEX_ANIMATION* m = dynamic_cast<GLCompileMacro_USE_VERTEX_ANIMATION*>(macro))
		if((permutation & macro->GetBit()) != 0 && macro->GetType() == USE_VERTEX_ANIMATION)
		{
			//common->Printf("conflicting macro! canceling '%s' vs. '%s' \n", GetName(), macro->GetName());
			return true;
		}
	}

	return false;
}

bool GLCompileMacro_USE_VERTEX_SKINNING::MissesRequiredMacros(int permutation, const idList<GLCompileMacro*>& macros) const
{
	// FIXME
	return true; //!glConfig.vboVertexSkinningAvailable;
}



bool GLCompileMacro_USE_VERTEX_ANIMATION::HasConflictingMacros(int permutation, const idList<GLCompileMacro*>& macros) const
{
#if 1
	for(int i = 0; i < macros.Num(); i++)
	{
		GLCompileMacro* macro = macros[i];

		if((permutation & macro->GetBit()) != 0 && macro->GetType() == USE_VERTEX_SKINNING)
		{
			//common->Printf("conflicting macro! canceling '%s' vs. '%s' \n", GetName(), macro->GetName());
			return true;
		}
	}
#endif
	return false;
}

uint32_t	GLCompileMacro_USE_VERTEX_ANIMATION::GetRequiredVertexAttributes() const
{
	uint32_t attribs = VA_NORMAL | VA_POSITION2 | VA_NORMAL2;

	if(!r_skipBump.GetBool())
	{
		attribs |= VA_TANGENT2 | VA_BINORMAL2;
	}

	return attribs;
}


/*
bool GLCompileMacro_USE_DEFORM_VERTEXES::HasConflictingMacros(int permutation, const idList<GLCompileMacro*>& macros) const
{
	return (glConfig.driverType != GLDRV_OPENGL3 || !r_vboDeformVertexes->integer);
}
*/

bool GLCompileMacro_USE_PARALLAX_MAPPING::MissesRequiredMacros(int permutation, const idList<GLCompileMacro*>& macros) const
{
	bool foundUSE_NORMAL_MAPPING = false;
	for(int i = 0; i < macros.Num(); i++)
	{
		GLCompileMacro* macro = macros[i];

		if((permutation & macro->GetBit()) != 0 && macro->GetType() == USE_NORMAL_MAPPING)
		{
			foundUSE_NORMAL_MAPPING = true;
		}
	}

	if(!foundUSE_NORMAL_MAPPING)
	{
		//common->Printf("missing macro! canceling '%s' <= '%s' \n", GetName(), "USE_NORMAL_MAPPING");
		return true;
	}

	return false;
}

bool GLCompileMacro_USE_REFLECTIVE_SPECULAR::MissesRequiredMacros(int permutation, const idList<GLCompileMacro*>& macros) const
{
	bool foundUSE_NORMAL_MAPPING = false;
	for(int i = 0; i < macros.Num(); i++)
	{
		GLCompileMacro* macro = macros[i];

		if((permutation & macro->GetBit()) != 0 && macro->GetType() == USE_NORMAL_MAPPING)
		{
			foundUSE_NORMAL_MAPPING = true;
		}
	}

	if(!foundUSE_NORMAL_MAPPING)
	{
		//common->Printf("missing macro! canceling '%s' <= '%s' \n", GetName(), "USE_NORMAL_MAPPING");
		return true;
	}

	return false;
}



bool GLShader::GetCompileMacrosString(int permutation, idStrList& compileMacrosOut) const
{
	compileMacrosOut.Clear();
	for(int j = 0; j < _compileMacros.Num(); j++)
	{
		GLCompileMacro* macro = _compileMacros[j];

		if(permutation & macro->GetBit())
		{
			if(macro->HasConflictingMacros(permutation, _compileMacros))
			{
				//common->Printf("conflicting macro! canceling '%s'\n", macro->GetName());
				return false;
			}

			if(macro->MissesRequiredMacros(permutation, _compileMacros))
			{
				return false;
			}

			compileMacrosOut.Append(idStr(macro->GetName()));
		}
	}

	return true;
}

void GLShader::UpdateShaderProgramUniformLocations(shaderProgram_t *shaderProgram) const
{
	for(int j = 0; j < _uniforms.Num(); j++)
	{
		GLUniform* uniform = _uniforms[j];

		uniform->UpdateShaderProgramUniformLocation(shaderProgram);
	}
}

idStr	GLShader::BuildGPUShaderText(	const char *mainShaderName,
										const idStrList& libShaderNames,
										GLenum shaderType) const
{
	idStr           fileName;
	char           *fileBuffer;

	idStr			shaderText;
	idStr			libsShaderText;

	GL_CheckErrors();

	// load libs
	for(int i = 0; i < libShaderNames.Num(); i++)
	{
		const idStr& libName = libShaderNames[i];

		if(shaderType == GL_VERTEX_SHADER_ARB)
		{
			fileName = va("shaders/glsl/%s_vs.glsl", libName.c_str());
			common->Printf("...loading vertex shader '%s'\n", fileName.c_str());
		}
		else
		{
			fileName = va("shaders/glsl/%s_fs.glsl", libName.c_str());
			common->Printf("...loading fragment shader '%s'\n", fileName.c_str());
		}

		fileSystem->ReadFile( fileName.c_str(), (void **)&fileBuffer, NULL );
		if ( !fileBuffer ) {
			common->Printf( ": File not found\n" );
			continue;
		}

		//char *buffer = (char *)_alloca( strlen( fileBuffer ) + 1 );
		libsShaderText.Append(fileBuffer, strlen(fileBuffer));
		
		fileSystem->FreeFile( fileBuffer );
	}

	shaderText += libsShaderText;

	// load main() program
	if(shaderType == GL_VERTEX_SHADER_ARB)
	{
		fileName = va("shaders/glsl/%s_vs.glsl", mainShaderName);
		common->Printf("...loading vertex shader '%s'\n", fileName.c_str());
	}
	else
	{
		fileName = va("shaders/glsl/%s_fs.glsl", mainShaderName);
		common->Printf("...loading fragment shader '%s'\n", fileName.c_str());
	}

	fileSystem->ReadFile( fileName.c_str(), (void **)&fileBuffer, NULL );
	if ( !fileBuffer ) {
		common->Error("Couldn't load %s\n", fileName.c_str());
	}
	
	// HACK: add some macros to avoid extra uniforms and save speed and code maintenance
	/*
	Q_strcat(bufferExtra, sizeof(bufferExtra),
				va("#ifndef r_SpecularExponent\n#define r_SpecularExponent %f\n#endif\n", r_specularExponent->value));

	Q_strcat(bufferExtra, sizeof(bufferExtra),
				va("#ifndef r_SpecularExponent2\n#define r_SpecularExponent2 %f\n#endif\n", r_specularExponent2->value));

	Q_strcat(bufferExtra, sizeof(bufferExtra),
				va("#ifndef r_SpecularScale\n#define r_SpecularScale %f\n#endif\n", r_specularScale->value));

	//Q_strcat(bufferExtra, sizeof(bufferExtra),
	//       va("#ifndef r_NormalScale\n#define r_NormalScale %f\n#endif\n", r_normalScale->value));
	*/

	shaderText += "#ifndef M_PI\n#define M_PI 3.14159265358979323846f\n#endif\n";

	/*
	fbufWidthScale = Q_recip((float)glConfig.vidWidth);
	fbufHeightScale = Q_recip((float)glConfig.vidHeight);
	Q_strcat(bufferExtra, sizeof(bufferExtra),
				va("#ifndef r_FBufScale\n#define r_FBufScale vec2(%f, %f)\n#endif\n", fbufWidthScale, fbufHeightScale));

	if(glConfig2.textureNPOTAvailable)
	{
		npotWidthScale = 1;
		npotHeightScale = 1;
	}
	else
	{
		npotWidthScale = (float)glConfig.vidWidth / (float)NearestPowerOfTwo(glConfig.vidWidth);
		npotHeightScale = (float)glConfig.vidHeight / (float)NearestPowerOfTwo(glConfig.vidHeight);
	}
	Q_strcat(bufferExtra, sizeof(bufferExtra),
				va("#ifndef r_NPOTScale\n#define r_NPOTScale vec2(%f, %f)\n#endif\n", npotWidthScale, npotHeightScale));
	*/

	/*
	if(glConfig.driverType == GLDRV_MESA)
	{
		Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef GLDRV_MESA\n#define GLDRV_MESA 1\n#endif\n");
	}

	if(glConfig.hardwareType == GLHW_ATI)
	{
		Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef GLHW_ATI\n#define GLHW_ATI 1\n#endif\n");
	}
	else if(glConfig.hardwareType == GLHW_ATI_DX10)
	{
		Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef GLHW_ATI_DX10\n#define GLHW_ATI_DX10 1\n#endif\n");
	}
	else if(glConfig.hardwareType == GLHW_NV_DX10)
	{
		Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef GLHW_NV_DX10\n#define GLHW_NV_DX10 1\n#endif\n");
	}

	if(r_shadows->integer >= SHADOWING_ESM16 && glConfig2.textureFloatAvailable && glConfig2.framebufferObjectAvailable)
	{
		if(r_shadows->integer == SHADOWING_ESM16 || r_shadows->integer == SHADOWING_ESM32)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef ESM\n#define ESM 1\n#endif\n");
		}
		else if(r_shadows->integer == SHADOWING_EVSM32)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef EVSM\n#define EVSM 1\n#endif\n");

			// The exponents for the EVSM techniques should be less than ln(FLT_MAX/FILTER_SIZE)/2 {ln(FLT_MAX/1)/2 ~44.3}
			//         42.9 is the maximum possible value for FILTER_SIZE=15
			//         42.0 is the truncated value that we pass into the sample
			Q_strcat(bufferExtra, sizeof(bufferExtra),
					va("#ifndef r_EVSMExponents\n#define r_EVSMExponents vec2(%f, %f)\n#endif\n", 42.0f, 42.0f));

			if(r_evsmPostProcess->integer)
			{
				Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef r_EVSMPostProcess\n#define r_EVSMPostProcess 1\n#endif\n");
			}
		}
		else
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef VSM\n#define VSM 1\n#endif\n");

			if(glConfig.hardwareType == GLHW_ATI)
			{
				Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef VSM_CLAMP\n#define VSM_CLAMP 1\n#endif\n");
			}
		}

		if((glConfig.hardwareType == GLHW_NV_DX10 || glConfig.hardwareType == GLHW_ATI_DX10) && r_shadows->integer == SHADOWING_VSM32)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef VSM_EPSILON\n#define VSM_EPSILON 0.000001\n#endif\n");
		}
		else
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef VSM_EPSILON\n#define VSM_EPSILON 0.0001\n#endif\n");
		}

		if(r_lightBleedReduction->value)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra),
						va("#ifndef r_LightBleedReduction\n#define r_LightBleedReduction %f\n#endif\n",
						r_lightBleedReduction->value));
		}

		if(r_overDarkeningFactor->value)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra),
						va("#ifndef r_OverDarkeningFactor\n#define r_OverDarkeningFactor %f\n#endif\n",
						r_overDarkeningFactor->value));
		}

		if(r_shadowMapDepthScale->value)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra),
						va("#ifndef r_ShadowMapDepthScale\n#define r_ShadowMapDepthScale %f\n#endif\n",
						r_shadowMapDepthScale->value));
		}

		if(r_debugShadowMaps->integer)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra),
						va("#ifndef r_DebugShadowMaps\n#define r_DebugShadowMaps %i\n#endif\n", r_debugShadowMaps->integer));
		}
		if(r_softShadows->integer == 6)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef PCSS\n#define PCSS 1\n#endif\n");
		}
		else if(r_softShadows->integer)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra),
				va("#ifndef r_PCFSamples\n#define r_PCFSamples %1.1f\n#endif\n", r_softShadows->value + 1.0f));
		}

		if(r_parallelShadowSplits->integer)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra),
						va("#ifndef r_ParallelShadowSplits_%i\n#define r_ParallelShadowSplits_%i\n#endif\n", r_parallelShadowSplits->integer, r_parallelShadowSplits->integer));
		}

		if(r_showParallelShadowSplits->integer)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef r_ShowParallelShadowSplits\n#define r_ShowParallelShadowSplits 1\n#endif\n");
		}
	}
	*/

	/*
	if(r_deferredShading->integer && glConfig2.maxColorAttachments >= 4 && glConfig2.textureFloatAvailable &&
		glConfig2.drawBuffersAvailable && glConfig2.maxDrawBuffers >= 4)
	{

		if(r_deferredShading->integer == DS_STANDARD)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef r_DeferredShading\n#define r_DeferredShading 1\n#endif\n");
		}
	}

	if(r_hdrRendering->integer && glConfig2.framebufferObjectAvailable && glConfig2.textureFloatAvailable)
	{
		Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef r_HDRRendering\n#define r_HDRRendering 1\n#endif\n");

		Q_strcat(bufferExtra, sizeof(bufferExtra),
					va("#ifndef r_HDRContrastThreshold\n#define r_HDRContrastThreshold %f\n#endif\n",
					r_hdrContrastThreshold->value));

		Q_strcat(bufferExtra, sizeof(bufferExtra),
					va("#ifndef r_HDRContrastOffset\n#define r_HDRContrastOffset %f\n#endif\n",
					r_hdrContrastOffset->value));

		Q_strcat(bufferExtra, sizeof(bufferExtra),
					va("#ifndef r_HDRToneMappingOperator\n#define r_HDRToneMappingOperator_%i\n#endif\n",
					r_hdrToneMappingOperator->integer));

		Q_strcat(bufferExtra, sizeof(bufferExtra),
					va("#ifndef r_HDRGamma\n#define r_HDRGamma %f\n#endif\n",
					r_hdrGamma->value));
	}
	*/

	/*
	if(r_precomputedLighting->integer)
	{
		Q_strcat(bufferExtra, sizeof(bufferExtra),
					"#ifndef r_precomputedLighting\n#define r_precomputedLighting 1\n#endif\n");
	}
	*/

	/*
	if(r_halfLambertLighting->integer)
	{
		Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef r_HalfLambertLighting\n#define r_HalfLambertLighting 1\n#endif\n");
	}
	*/

	/*
	if(r_rimLighting->integer)
	{
		Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef r_RimLighting\n#define r_RimLighting 1\n#endif\n");
		Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef r_RimColor\n#define r_RimColor vec4(0.26, 0.19, 0.16, 0.0)\n#endif\n");
		Q_strcat(bufferExtra, sizeof(bufferExtra), va("#ifndef r_RimExponent\n#define r_RimExponent %f\n#endif\n",
													r_rimExponent->value));
	}
	*/

	// OK we added a lot of stuff but if we do something bad in the GLSL shaders then we want the proper line
	// so we have to reset the line counting
	shaderText += "#line 0\n";

	shaderText.Append(fileBuffer, strlen(fileBuffer));

	fileSystem->FreeFile( fileBuffer );

	//common->Printf("GLSL extra: %s\n", bufferExtra);

#if 0
	{
		static char     msgPart[1024];
		int             i;
		ri.Printf(PRINT_WARNING, "----------------------------------------------------------\n", filename);
		ri.Printf(PRINT_WARNING, "CONCATENATED shader '%s' ----------\n", filename);
		ri.Printf(PRINT_WARNING, " BEGIN ---------------------------------------------------\n", filename);

		for(i = 0; i < sizeFinal; i += 1024)
		{
			Q_strncpyz(msgPart, bufferFinal + i, sizeof(msgPart));
			common->Printf("%s", msgPart);
		}

		ri.Printf(PRINT_WARNING, " END-- ---------------------------------------------------\n", filename);
	}
#endif

#if defined(USE_GLSL_OPTIMIZER)
	if(glConfig.driverType != GLDRV_OPENGL3 && optimize)
	{
		static char     msgPart[1024];
		int             length = 0;
		int             i;
			

		glslopt_shader_type glsloptShaderType;

		if(shaderType == GL_FRAGMENT_SHADER_ARB)
			glsloptShaderType = kGlslOptShaderFragment;
		else
			glsloptShaderType = kGlslOptShaderVertex;

		glslopt_shader* shaderOptimized = glslopt_optimize(s_glslOptimizer, 
			glsloptShaderType, bufferFinal, 0);

		if(glslopt_get_status(shaderOptimized))
		{
			const char* newSource = glslopt_get_output(shaderOptimized);

			ri.Printf(PRINT_WARNING, "----------------------------------------------------------\n", filename);
			ri.Printf(PRINT_WARNING, "OPTIMIZED shader '%s' ----------\n", filename);
			ri.Printf(PRINT_WARNING, " BEGIN ---------------------------------------------------\n", filename);

			length = strlen(newSource);
			for(i = 0; i < length; i += 1024)
			{
				Q_strncpyz(msgPart, newSource + i, sizeof(msgPart));
				common->Printf("%s\n", msgPart);
			}

			ri.Printf(PRINT_WARNING, " END-- ---------------------------------------------------\n", filename);

			glShaderSourceARB(shader, 1, (const GLcharARB **)&newSource, &length);
			shaderText = idStr(newSource, length);
		}
		else
		{
			const char* errorLog = glslopt_get_log(shaderOptimized);

			//ri.Printf(PRINT_WARNING, "Couldn't optimize '%s'", filename);

			length = strlen(errorLog);
			for(i = 0; i < length; i += 1024)
			{
				Q_strncpyz(msgPart, errorLog + i, sizeof(msgPart));
				common->Printf("%s\n", msgPart);
			}

			ri.Error(ERR_FATAL, "Couldn't optimize %s", filename);
		}
			
		glslopt_shader_delete(shaderOptimized);
	}
	else
	{
		shaderText = idStr(bufferFinal, sizeFinal);
	}
#endif

	return shaderText;
}


void GLShader::CompileAndLinkGPUShaderProgram(	shaderProgram_t * program,
												const char *programName,
												const idStr& vertexShaderText,
												const idStr& fragmentShaderText,
												const idStrList& compileMacros) const
{
	//ri.Printf(PRINT_DEVELOPER, "------- GPU shader -------\n");

	//Q_strncpyz(program->name, programName, sizeof(program->name));
	program->name = programName;

#if 0
	if(!compileMacros.empty())
	{
		program->compileMacros = (char *) ri.Hunk_Alloc(sizeof(char) * compileMacros.length() + 1, h_low);
		Q_strncpyz(program->compileMacros, compileMacros.c_str(), compileMacros.length() + 1);
	}
	else
#endif
	{
		program->compileMacros = NULL;
	}

	program->program = glCreateProgramObjectARB();
	program->attribs = _vertexAttribsRequired;// | _vertexAttribsOptional;


	// header of the glsl shader
	idStr vertexHeader;
	idStr fragmentHeader;
	/*
	if(glConfig.driverType == GLDRV_OPENGL3)
	{
		// HACK: use the GLSL preprocessor to turn GLSL 1.20 shaders into 1.30 ones

		vertexHeader += "#version 130\n";
		fragmentHeader += "#version 130\n";

		if(shaderType == GL_VERTEX_SHADER_ARB)
		{
			vertexHeader += "#define attribute in\n";
			vertexHeader += "#define varying out\n";
		}
		else
		{
			fragmentHeader += "#define varying in\n";

			fragmentHeader += "out vec4 out_Color;\n";
			fragmentHeader += "#define gl_FragColor out_Color\n";
		}

		vertexHeader += "#define textureCube texture\n";
		fragmentHeader += "#define textureCube texture\n";
	}
	else
	*/
	{
		vertexHeader += "#version 120\n";
		fragmentHeader += "#version 120\n";
	}

	// permutation macros
	idStr macrosString;

	if(compileMacros.Num())
	{
		for(int i = 0; i < compileMacros.Num(); i++)
		{
			const char *compileMacro = compileMacros[i].c_str();
		
			macrosString += va("#ifndef %s\n#define %s 1\n#endif\n", compileMacro, compileMacro);
		}
	}

	// add them
	idStr vertexShaderTextWithMacros = vertexHeader + macrosString + vertexShaderText;
	idStr fragmentShaderTextWithMacros = fragmentHeader + macrosString + fragmentShaderText;

	CompileGPUShader(program->program, programName, vertexShaderTextWithMacros, GL_VERTEX_SHADER_ARB);
	CompileGPUShader(program->program, programName, fragmentShaderTextWithMacros, GL_FRAGMENT_SHADER_ARB);

	BindAttribLocations(program->program); //, _vertexAttribsRequired | _vertexAttribsOptional);
	LinkProgram(program->program);
}

void GLShader::CompileGPUShader(GLhandleARB program, const idStr& programName, const idStr& shaderText, GLenum shaderType) const
{
	GLhandleARB shader = glCreateShaderObjectARB(shaderType);

	GL_CheckErrors();

	int shaderTextSize = shaderText.Length();
	const char* shaderTextPtr = shaderText.c_str();
	glShaderSourceARB(shader, 1, (const GLcharARB **)&shaderTextPtr, &shaderTextSize);

	// compile shader
	glCompileShaderARB(shader);

	GL_CheckErrors();

	// check if shader compiled
	GLint compiled;
	glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);
	if(!compiled)
	{
		PrintShaderSource(shader);
		PrintInfoLog(shader, false);
		common->Error("Couldn't compile %s %s", (shaderType == GL_VERTEX_SHADER_ARB ? "vertex shader" : "fragment shader"), programName.c_str());
		return;
	}

	//PrintInfoLog(shader, qtrue);
	//common->Printf("%s\n", GLSL_PrintShaderSource(shader));

	// attach shader to program
	glAttachObjectARB(program, shader);
	GL_CheckErrors();

	// delete shader, no longer needed
	glDeleteObjectARB(shader);
	GL_CheckErrors();
}

void GLShader::PrintShaderText(const idStr& shaderText) const
{
	static char     msgPart[1024];
	
	for(int i = 0; i < shaderText.Length(); i += 1024)
	{
		idStr::snPrintf(msgPart, sizeof(msgPart), shaderText.c_str() + i);
		common->Printf("%s\n", msgPart);
	}
}

void GLShader::PrintShaderSource(GLhandleARB object) const
{
	char           *msg;
	static char     msgPart[1024];
	int             maxLength = 0;
	int             i;

	glGetObjectParameterivARB(object, GL_OBJECT_SHADER_SOURCE_LENGTH_ARB, &maxLength);

	msg = (char *) Mem_Alloc(maxLength);

	glGetShaderSourceARB(object, maxLength, &maxLength, msg);

	for(i = 0; i < maxLength; i += 1024)
	{
		idStr::snPrintf(msgPart, sizeof(msgPart), msg + i);
		common->Printf("%s\n", msgPart);
	}

	Mem_Free(msg);
}

void GLShader::PrintInfoLog(GLhandleARB object, bool developerOnly) const
{
	char           *msg;
	static char     msgPart[1024];
	int             maxLength = 0;
	int             i;

	glGetObjectParameterivARB(object, GL_OBJECT_INFO_LOG_LENGTH_ARB, &maxLength);

	msg = (char *) Mem_Alloc(maxLength);

	glGetInfoLogARB(object, maxLength, &maxLength, msg);

	if(developerOnly)
	{
		common->DPrintf("compile log:\n");
	}
	else
	{
		common->Printf("compile log:\n");
	}

	for(i = 0; i < maxLength; i += 1024)
	{
		idStr::snPrintf(msgPart, sizeof(msgPart), msg + i);

		if(developerOnly)
			common->DPrintf("%s\n", msgPart);
		else
			common->Printf("%s\n", msgPart);
	}

	Mem_Free(msg);
}

void GLShader::LinkProgram(GLhandleARB program) const
{
	GLint           linked;

	glLinkProgramARB(program);

	glGetObjectParameterivARB(program, GL_OBJECT_LINK_STATUS_ARB, &linked);
	if(!linked)
	{
		PrintInfoLog(program, false);
		common->Error("Shaders failed to link!!!");
	}
}

void GLShader::ValidateProgram(GLhandleARB program) const
{
	GLint           validated;

	glValidateProgramARB(program);

	glGetObjectParameterivARB(program, GL_OBJECT_VALIDATE_STATUS_ARB, &validated);
	if(!validated)
	{
		PrintInfoLog(program, false);
		common->Error("Shaders failed to validate!!!");
	}
}

void GLShader::ShowProgramUniforms(GLhandleARB program) const
{
	int             i, count, size;
	GLenum			type;
	char            uniformName[1000];

	// install the executables in the program object as part of current state.
	glUseProgramObjectARB(program);

	// check for GL Errors

	// query the number of active uniforms
	glGetObjectParameterivARB(program, GL_OBJECT_ACTIVE_UNIFORMS_ARB, &count);

	// Loop over each of the active uniforms, and set their value
	for(i = 0; i < count; i++)
	{
		glGetActiveUniformARB(program, i, sizeof(uniformName), NULL, &size, &type, uniformName);

		common->Printf("active uniform: '%s'\n", uniformName);
	}

	glUseProgramObjectARB(0);
}

void GLShader::BindAttribLocations(GLhandleARB program) const
{
	//if(attribs & VA_POSITION)
		glBindAttribLocationARB(program, VA_INDEX_POSITION, "va_Position");

	//if(attribs & VA_TEXCOORD)
		glBindAttribLocationARB(program, VA_INDEX_TEXCOORD0, "va_TexCoord0");

	//if(attribs & VA_LIGHTCOORD)
		glBindAttribLocationARB(program, VA_INDEX_TEXCOORD1, "va_TexCoord1");

//  if(attribs & VA_TEXCOORD2)
//      glBindAttribLocationARB(program, VA_INDEX_TEXCOORD2, "va_TexCoord2");

//  if(attribs & VA_TEXCOORD3)
//      glBindAttribLocationARB(program, VA_INDEX_TEXCOORD3, "va_TexCoord3");

	//if(attribs & VA_TANGENT)
		glBindAttribLocationARB(program, VA_INDEX_TANGENT, "va_Tangent");

	//if(attribs & VA_BITANGENT)
		glBindAttribLocationARB(program, VA_INDEX_BITANGENT, "va_Bitangent");

	//if(attribs & VA_NORMAL)
		glBindAttribLocationARB(program, VA_INDEX_NORMAL, "va_Normal");

	//if(attribs & VA_COLOR)
		glBindAttribLocationARB(program, VA_INDEX_COLOR, "va_Color");

	//if(attribs & VA_LIGHTDIRECTION)
//		glBindAttribLocationARB(program, VA_INDEX_LIGHTDIRECTION, "va_LightDirection");

	//if(glConfig2.vboVertexSkinningAvailable)
	{
		glBindAttribLocationARB(program, VA_INDEX_BONE_INDEXES, "va_BoneIndexes");
		glBindAttribLocationARB(program, VA_INDEX_BONE_WEIGHTS, "va_BoneWeights");
	}

	//if(attribs & VA_POSITION2)
		glBindAttribLocationARB(program, VA_INDEX_POSITION2, "va_Position2");

	//if(attribs & VA_TANGENT2)
		glBindAttribLocationARB(program, VA_INDEX_TANGENT2, "va_Tangent2");

	//if(attribs & VA_BINORMAL2)
		glBindAttribLocationARB(program, VA_INDEX_BINORMAL2, "va_Binormal2");

	//if(attribs & VA_NORMAL2)
		glBindAttribLocationARB(program, VA_INDEX_NORMAL2, "va_Normal2");
}

void GLShader::SelectProgram()
{
	int index = 0;

	int numMacros = _compileMacros.Num();
	for(int i = 0; i < numMacros; i++)
	{
		if(_activeMacros & BIT(i))
			index += BIT(i);
	}

	_currentProgram = _shaderPrograms[index];
}

void GLShader::BindProgram()
{
	SelectProgram();

	if(_currentProgram->program == 0)
	{
		idStr activeMacros = "";
		size_t numMacros = _compileMacros.Num();
		for(size_t j = 0; j < numMacros; j++)
		{
			GLCompileMacro* macro = _compileMacros[j];

			int bit = macro->GetBit();

			if(IsMacroSet(bit))
			{
				activeMacros += macro->GetName();
				activeMacros += " ";
			}
		}

		common->Error("Invalid shader configuration: shader = '%s', macros = '%s'", _name.c_str(), activeMacros.c_str());
	}

	GL_BindProgram(_currentProgram);
}


void GLShader::SetRequiredVertexPointers()
{
	uint32_t macroVertexAttribs = 0;
	size_t numMacros = _compileMacros.Num();
	for(size_t j = 0; j < numMacros; j++)
	{
		GLCompileMacro* macro = _compileMacros[j];

		int bit = macro->GetBit();

		if(IsMacroSet(bit))
		{
			macroVertexAttribs |= macro->GetRequiredVertexAttributes();
		}
	}

	// FIXME
	//GL_VertexAttribsState((_vertexAttribsRequired | _vertexAttribs | macroVertexAttribs));// & ~_vertexAttribsUnsupported);
}








/*
GLShader_generic::GLShader_generic():
		GLShader(	"generic", VA_POSITION | VA_TEXCOORD | VA_NORMAL),
		u_ColorMap(this),
		u_ColorTextureMatrix(this),
		u_ViewOrigin(this),
		u_AlphaTest(this),
		u_ColorModulate(this),
		u_Color(this),
		u_ModelMatrix(this),
		u_ModelViewProjectionMatrix(this),
		u_BoneMatrix(this),
		u_VertexInterpolation(this),
		u_PortalPlane(this),
		GLDeformStage(this),
		GLCompileMacro_USE_PORTAL_CLIPPING(this),
		GLCompileMacro_USE_ALPHA_TESTING(this),
		GLCompileMacro_USE_VERTEX_SKINNING(this),
		GLCompileMacro_USE_VERTEX_ANIMATION(this),
		GLCompileMacro_USE_DEFORM_VERTEXES(this),
		GLCompileMacro_USE_TCGEN_ENVIRONMENT(this)
{
	common->Printf("/// -------------------------------------------------\n");
	common->Printf("/// creating generic shaders ------------------------\n");

	int startTime = ri.Milliseconds();

	_shaderPrograms = idList<shaderProgram_t>(1 << _compileMacros.size());
	
	//Com_Memset(_shaderPrograms, 0, sizeof(_shaderPrograms));

	idStr vertexInlines = "vertexSkinning vertexAnimation ";
	if(glConfig.driverType == GLDRV_OPENGL3 && r_vboDeformVertexes->integer)
	{
		vertexInlines += "deformVertexes ";
	}

	idStr vertexShaderText = BuildGPUShaderText("generic", vertexInlines.c_str(), GL_VERTEX_SHADER_ARB);
	idStr fragmentShaderText = BuildGPUShaderText("generic", "", GL_FRAGMENT_SHADER_ARB);

	size_t numPermutations = (1 << _compileMacros.size());	// same as 2^n, n = no. compile macros
	size_t numCompiled = 0;
	common->Printf("...compiling generic shaders\n");
	common->Printf("0%%  10   20   30   40   50   60   70   80   90   100%%\n");
	common->Printf("|----|----|----|----|----|----|----|----|----|----|\n");
	size_t tics = 0;
	size_t nextTicCount = 0;
	for(size_t i = 0; i < numPermutations; i++)
	{
		if((i + 1) >= nextTicCount)
		{
			size_t ticsNeeded = (size_t)(((double)(i + 1) / numPermutations) * 50.0);

			do { common->Printf("*"); } while ( ++tics < ticsNeeded );

			nextTicCount = (size_t)((tics / 50.0) * numPermutations);
			if(i == (numPermutations - 1))
			{
				if(tics < 51)
					common->Printf("*");
				common->Printf("\n");
			}
		}

		idStr compileMacros;
		if(GetCompileMacrosString(i, compileMacros))
		{
			ri.Printf(PRINT_DEVELOPER, "Compile macros: '%s'\n", compileMacros.c_str());

			shaderProgram_t *shaderProgram = &_shaderPrograms[i];

			CompileAndLinkGPUShaderProgram(	shaderProgram,
											"generic",
											vertexShaderText,
											fragmentShaderText,
											compileMacros);
			
			UpdateShaderProgramUniformLocations(shaderProgram);

			//shaderProgram->u_ColorMap = glGetUniformLocationARB(shaderProgram->program, "u_ColorMap");

			//common->Printf("u_ColorMap = %i\n", shaderProgram->u_ColorMap);

			glUseProgramObjectARB(shaderProgram->program);
			glUniform1iARB(shaderProgram->u_ColorMap, 0);
			glUseProgramObjectARB(0);

			ValidateProgram(shaderProgram->program);
			//ShowProgramUniforms(shaderProgram->program);
			
			GL_CheckErrors();

			numCompiled++;
		}
	}
	common->Printf("\n");

	SelectProgram();

	int endTime = ri.Milliseconds();
	common->Printf("...compiled %i generic shader permutations in %5.2f seconds\n", numCompiled, (endTime - startTime) / 1000.0);
}
*/


GLShader_forwardLighting::GLShader_forwardLighting():
		GLShader("forwardLighting", VA_POSITION | VA_TEXCOORD | VA_NORMAL),
		u_DiffuseMatrixS(this),
		u_DiffuseMatrixT(this),
		u_BumpMatrixS(this),
		u_BumpMatrixT(this),
		u_SpecularMatrixS(this),
		u_SpecularMatrixT(this),
		//u_AlphaTest(this),
		u_Color(this),
		u_ColorModulate(this),
		u_DiffuseColor(this),
		u_SpecularColor(this),
		u_ViewOrigin(this),
		u_LightOrigin(this),
		u_LightProjectS(this),
		u_LightProjectT(this),
		u_LightProjectQ(this),
		u_LightFalloffS(this),
		//u_LightColor(this),
		//u_LightRadius(this),
		//u_LightScale(this),
		//u_LightWrapAround(this),
		//u_LightAttenuationMatrix(this),
		//u_ShadowTexelSize(this),
		//u_ShadowBlur(this),
		//u_ModelMatrix(this),
		//u_ModelViewProjectionMatrix(this),
		//u_BoneMatrix(this),
		//u_VertexInterpolation(this),
		//u_PortalPlane(this),
		//u_DepthScale(this),
		//GLDeformStage(this),
		//GLCompileMacro_USE_PORTAL_CLIPPING(this),
		//GLCompileMacro_USE_ALPHA_TESTING(this),
		//GLCompileMacro_USE_VERTEX_SKINNING(this),
		//GLCompileMacro_USE_VERTEX_ANIMATION(this),
		//GLCompileMacro_USE_DEFORM_VERTEXES(this),
		GLCompileMacro_USE_NORMAL_MAPPING(this)
		//GLCompileMacro_USE_PARALLAX_MAPPING(this),
		//GLCompileMacro_USE_SHADOWING(this)//,
		//GLCompileMacro_TWOSIDED(this)
{
	common->Printf("/// -------------------------------------------------\n");
	common->Printf("/// creating forwardLighting shaders --------\n");

	idTimer compile_time;
	compile_time.Start();	

	//idStr vertexInlines = "vertexSkinning vertexAnimation ";
	idStrList vertexInlines;
	/*
	if(glConfig.driverType == GLDRV_OPENGL3 && r_vboDeformVertexes->integer)
	{
		vertexInlines += "deformVertexes ";
	}
	*/

	idStrList fragmentInlines; // reliefMapping

	idStr vertexShaderText = BuildGPUShaderText("forwardLighting", vertexInlines, GL_VERTEX_SHADER_ARB);
	idStr fragmentShaderText = BuildGPUShaderText("forwardLighting", fragmentInlines, GL_FRAGMENT_SHADER_ARB);

	size_t numPermutations = (1 << _compileMacros.Num());	// same as 2^n, n = no. compile macros
	size_t numCompiled = 0;
	common->Printf("...compiling forwardLighting shaders\n");
	common->Printf("0%%  10   20   30   40   50   60   70   80   90   100%%\n");
	common->Printf("|----|----|----|----|----|----|----|----|----|----|\n");
	size_t tics = 0;
	size_t nextTicCount = 0;
	for(size_t i = 0; i < numPermutations; i++)
	{
		if((i + 1) >= nextTicCount)
		{
			size_t ticsNeeded = (size_t)(((double)(i + 1) / numPermutations) * 50.0);

			do { common->Printf("*"); } while ( ++tics < ticsNeeded );

			nextTicCount = (size_t)((tics / 50.0) * numPermutations);
			if(i == (numPermutations - 1))
			{
				if(tics < 51)
					common->Printf("*");
				common->Printf("\n");
			}
		}

		idStrList compileMacros;
		if(GetCompileMacrosString(i, compileMacros))
		{
			//compileMacros.Append("TWOSIDED");
			compileMacros.Append("HALF_LAMBERT");

			//common->DPrintf("Compile macros: '%s'\n", compileMacros.To);
		
			shaderProgram_t *shaderProgram = new shaderProgram_t();
			_shaderPrograms.Append(shaderProgram);

			CompileAndLinkGPUShaderProgram(	shaderProgram,
											"forwardLighting",
											vertexShaderText,
											fragmentShaderText,
											compileMacros);

			UpdateShaderProgramUniformLocations(shaderProgram);

			shaderProgram->u_NormalCubeMapImage	= glGetUniformLocationARB(shaderProgram->program, "u_NormalCubeMapImage");
			shaderProgram->u_NormalImage = glGetUniformLocationARB(shaderProgram->program, "u_NormalImage");
			shaderProgram->u_LightFalloffImage = glGetUniformLocationARB(shaderProgram->program, "u_LightFalloffImage");
			shaderProgram->u_LightImage = glGetUniformLocationARB(shaderProgram->program, "u_LightImage");
			shaderProgram->u_DiffuseImage = glGetUniformLocationARB(shaderProgram->program, "u_DiffuseImage");
			shaderProgram->u_SpecularImage = glGetUniformLocationARB(shaderProgram->program, "u_SpecularImage");
			//if(r_shadows->integer >= SHADOWING_ESM16)
			//{
			//	shaderProgram->u_ShadowMap = glGetUniformLocationARB(shaderProgram->program, "u_ShadowMap");
			//}
			//shaderProgram->u_RandomMap = glGetUniformLocationARB(shaderProgram->program, "u_RandomMap");

			glUseProgramObjectARB(shaderProgram->program);
			glUniform1iARB(shaderProgram->u_NormalCubeMapImage, 0);
			glUniform1iARB(shaderProgram->u_NormalImage, 1);
			glUniform1iARB(shaderProgram->u_LightFalloffImage, 2);
			glUniform1iARB(shaderProgram->u_LightImage, 3);
			glUniform1iARB(shaderProgram->u_DiffuseImage, 4);
			glUniform1iARB(shaderProgram->u_SpecularImage, 5);
			//if(r_shadows->integer >= SHADOWING_ESM16)
			//{
			//	glUniform1iARB(shaderProgram->u_ShadowMap, 5);
			//}
			//glUniform1iARB(shaderProgram->u_RandomMap, 6);
			glUseProgramObjectARB(0);

			ValidateProgram(shaderProgram->program);
			//ShowProgramUniforms(shaderProgram->program);
			GL_CheckErrors();

			numCompiled++;
		}
		else
		{
			_shaderPrograms.Append(NULL);
		}
	}

	SelectProgram();

	compile_time.Stop();
	common->Printf("...compiled %i forwardLighting shader permutations in %5.2f seconds\n", numCompiled, compile_time.Milliseconds() / 1000.0);
}


GLShader_shadowVolume::GLShader_shadowVolume():
		GLShader("shadowVolume", VA_POSITION),
		u_LightOrigin(this)
		//GLCompileMacro_USE_VERTEX_SKINNING(this),
		//GLCompileMacro_USE_VERTEX_ANIMATION(this),
		//GLCompileMacro_USE_DEFORM_VERTEXES(this),
{
	common->Printf("/// -------------------------------------------------\n");
	common->Printf("/// creating shadowVolume shaders --------\n");

	idTimer compile_time;
	compile_time.Start();

	//idStr vertexInlines = "vertexSkinning vertexAnimation ";
	idStrList vertexInlines;
	/*
	if(glConfig.driverType == GLDRV_OPENGL3 && r_vboDeformVertexes->integer)
	{
		vertexInlines += "deformVertexes ";
	}
	*/

	idStrList fragmentInlines;

	idStr vertexShaderText = BuildGPUShaderText("shadowVolume", vertexInlines, GL_VERTEX_SHADER_ARB);
	idStr fragmentShaderText = BuildGPUShaderText("shadowVolume", fragmentInlines, GL_FRAGMENT_SHADER_ARB);

	size_t numPermutations = (1 << _compileMacros.Num());	// same as 2^n, n = no. compile macros
	size_t numCompiled = 0;
	common->Printf("...compiling shadowVolume shaders\n");
	common->Printf("0%%  10   20   30   40   50   60   70   80   90   100%%\n");
	common->Printf("|----|----|----|----|----|----|----|----|----|----|\n");
	size_t tics = 0;
	size_t nextTicCount = 0;
	for(size_t i = 0; i < numPermutations; i++)
	{
		if((i + 1) >= nextTicCount)
		{
			size_t ticsNeeded = (size_t)(((double)(i + 1) / numPermutations) * 50.0);

			do { common->Printf("*"); } while ( ++tics < ticsNeeded );

			nextTicCount = (size_t)((tics / 50.0) * numPermutations);
			if(i == (numPermutations - 1))
			{
				if(tics < 51)
					common->Printf("*");
				common->Printf("\n");
			}
		}

		idStrList compileMacros;
		if(GetCompileMacrosString(i, compileMacros))
		{
			//compileMacros.Append("TWOSIDED");
			//compileMacros.Append("HALF_LAMBERT");

			//common->DPrintf("Compile macros: '%s'\n", compileMacros.To);
		
			shaderProgram_t *shaderProgram = new shaderProgram_t();
			_shaderPrograms.Append(shaderProgram);

			CompileAndLinkGPUShaderProgram(	shaderProgram,
											"shadowVolume",
											vertexShaderText,
											fragmentShaderText,
											compileMacros);

			UpdateShaderProgramUniformLocations(shaderProgram);

			ValidateProgram(shaderProgram->program);
			//ShowProgramUniforms(shaderProgram->program);
			GL_CheckErrors();

			numCompiled++;
		}
		else
		{
			_shaderPrograms.Append(NULL);
		}
	}

	SelectProgram();

	compile_time.Stop();
	common->Printf("...compiled %i shadowVolume shader permutations in %5.2f seconds\n", numCompiled, compile_time.Milliseconds() / 1000.0);
}

