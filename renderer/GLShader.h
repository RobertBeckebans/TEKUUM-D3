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

#ifndef GL_SHADER_H
#define GL_SHADER_H

#include "tr_local.h"

// *INDENT-OFF*

//#define USE_UNIFORM_FIREWALL
#define LOG_GLSL_UNIFORMS

class GLUniform;
class GLCompileMacro;

class GLShader
{
	//friend class GLCompileMacro_USE_ALPHA_TESTING;
	
private:
	GLShader& operator = ( const GLShader& );
	
	idStr								_name;
protected:
	int									_activeMacros;
	
	idList<shaderProgram_t*>			_shaderPrograms;
	shaderProgram_t*					_currentProgram;
	
	idList<GLUniform*>					_uniforms;
	idList<GLCompileMacro*>				_compileMacros;
	
	const uint32_t						_vertexAttribsRequired;
//	const uint32_t						_vertexAttribsOptional;
//	const uint32_t						_vertexAttribsUnsupported;
	uint32_t							_vertexAttribs;	// can be set by uniforms
	
	
	GLShader( const idStr& name, uint32_t vertexAttribsRequired/*, uint32_t vertexAttribsOptional, uint32_t vertexAttribsUnsupported*/ ):
		_name( name ),
		_vertexAttribsRequired( vertexAttribsRequired )
		//_vertexAttribsOptional(vertexAttribsOptional),
		//_vertexAttribsUnsupported(vertexAttribsUnsupported)
	{
		_activeMacros = 0;
		_currentProgram = NULL;
		_vertexAttribs = 0;
		
		//ri.Printf(PRINT_ALL, "/// -------------------------------------------------\n");
	}
	
	~GLShader()
	{
		for( int i = 0; i < _shaderPrograms.Num(); i++ )
		{
			shaderProgram_t* shaderProgram = _shaderPrograms[i];
			if( shaderProgram != NULL && shaderProgram->program )
			{
				glDeleteProgram( shaderProgram->program );
				delete shaderProgram;
			}
		}
	}
	
public:

	void RegisterUniform( GLUniform* uniform )
	{
		_uniforms.Append( uniform );
	}
	
	void RegisterCompileMacro( GLCompileMacro* compileMacro )
	{
		if( _compileMacros.Num() >= 9 )
		{
			common->Error( "Can't register more than 9 compile macros for a single shader" );
		}
		
		_compileMacros.Append( compileMacro );
	}
	
	size_t				GetNumOfCompiledMacros() const
	{
		return _compileMacros.Num();
	}
	shaderProgram_t*	GetProgram() const
	{
		return _currentProgram;
	}
	
protected:
	bool				GetCompileMacrosStringList( int permutation, idStrList& compileMacrosOut ) const;
	void				UpdateShaderProgramUniformLocations( shaderProgram_t* shaderProgram ) const;
	
	idStr				BuildGPUShaderText(	const char* mainShader,
											const idStrList& libShaderNames,
											GLenum shaderType,
											const char* preIncludeText = NULL ) const;
											
	void				CompileAndLinkGPUShaderProgram(	shaderProgram_t* program,
			const char* programName,
			const idStr& vertexShaderText,
			const idStr& fragmentShaderText,
			const idStrList& compileMacros ) const;
			
	void				CompilePermutations();
	virtual void		GetVertexShaderLibNames( idStrList& vertexInlines ) const {}
	virtual void		GetFragmentShaderLibNames( idStrList& fragmentInlines ) const {}
	virtual void		GetConstantCompileMacros( idStrList& compileMacros ) const {}
	
	// TODO remove
	virtual void		SetShaderProgramUniforms( shaderProgram_t* shaderProgram ) const {}
	
private:
	static const char*	FindEmbeddedShaderText( const idStr& shaderName, GLenum shaderType );
	
	void				CompileGPUShader( uint32_t program, const idStr& programName, const idStr& shaderText, GLenum shaderType ) const;
	
	void				PrintShaderText( const idStr& shaderText ) const;
	void				PrintShaderSource( uint32_t object ) const;
	void				PrintInfoLog( uint32_t object, bool developerOnly ) const;
	
	bool				LinkProgram( uint32_t program ) const;
	void				BindAttribLocations( uint32_t program ) const;
	
protected:
	void				ValidateProgram( uint32_t program ) const;
	void				ShowProgramUniforms( uint32_t program ) const;
	
	
public:
	void				SelectProgram();
	void				BindProgram();
	void				SetRequiredVertexPointers();
	
	bool IsMacroSet( int bit )
	{
		return ( _activeMacros & bit ) != 0;
	}
	
	void AddMacroBit( int bit )
	{
		_activeMacros |= bit;
	}
	
	void DelMacroBit( int bit )
	{
		_activeMacros &= ~bit;
	}
	
	bool IsVertexAtttribSet( int bit )
	{
		return ( _vertexAttribs & bit ) != 0;
	}
	
	void AddVertexAttribBit( int bit )
	{
		_vertexAttribs |= bit;
	}
	
	void DelVertexAttribBit( int bit )
	{
		_vertexAttribs &= ~bit;
	}
};

class GLUniform
{
protected:
	GLShader*				_shader;
	
	GLUniform( GLShader* shader ):
		_shader( shader )
	{
		_shader->RegisterUniform( this );
	}
	
public:
	virtual const char* GetName() const = 0;
	virtual void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const = 0;
};


class GLCompileMacro
{
private:
	int						_bit;
	
protected:
	GLShader*				_shader;
	
	GLCompileMacro( GLShader* shader ):
		_shader( shader )
	{
		_bit = BIT( _shader->GetNumOfCompiledMacros() );
		_shader->RegisterCompileMacro( this );
	}
	
	// RB: This is not good oo design, but it can be a workaround and its cost is more or less only a virtual function call.
	// It also works regardless of RTTI is enabled or not.
	enum EGLCompileMacro
	{
		USE_ALPHA_TESTING,
		USE_PORTAL_CLIPPING,
		USE_FRUSTUM_CLIPPING,
		USE_VERTEX_SKINNING,
		USE_VERTEX_ANIMATION,
		USE_DEFORM_VERTEXES,
		USE_TCGEN_ENVIRONMENT,
		USE_NORMAL_MAPPING,
		USE_PARALLAX_MAPPING,
		USE_REFLECTIVE_SPECULAR,
		USE_SHADOWING,
		TWOSIDED,
		EYE_OUTSIDE,
		BRIGHTPASS_FILTER,
		LIGHT_DIRECTIONAL,
		LIGHT_PROJ,
		USE_GBUFFER
	};
	
public:
	virtual const char* GetName() const = 0;
	virtual EGLCompileMacro GetType() const = 0;
	virtual bool		HasConflictingMacros( int permutation, const idList<GLCompileMacro*>& macros ) const
	{
		return false;
	}
	virtual bool		MissesRequiredMacros( int permutation, const idList<GLCompileMacro*>& macros ) const
	{
		return false;
	}
	virtual uint32_t	GetRequiredVertexAttributes() const
	{
		return 0;
	}
	
	void EnableMacro()
	{
		int bit = GetBit();
		
		if( !_shader->IsMacroSet( bit ) )
		{
			_shader->AddMacroBit( bit );
			//_shader->SelectProgram();
		}
	}
	
	void DisableMacro()
	{
		int bit = GetBit();
		
		if( _shader->IsMacroSet( bit ) )
		{
			_shader->DelMacroBit( bit );
			//_shader->SelectProgram();
		}
	}
	
public:
	const int GetBit()
	{
		return _bit;
	}
};


class GLCompileMacro_USE_ALPHA_TESTING:
	GLCompileMacro
{
public:
	GLCompileMacro_USE_ALPHA_TESTING( GLShader* shader ):
		GLCompileMacro( shader )
	{
	}
	
	const char* GetName() const
	{
		return "USE_ALPHA_TESTING";
	}
	EGLCompileMacro GetType() const
	{
		return USE_ALPHA_TESTING;
	}
	
	void EnableAlphaTesting()
	{
		EnableMacro();
	}
	void DisableAlphaTesting()
	{
		DisableMacro();
	}
	
	void SetAlphaTesting( bool enable )
	{
		if( enable )
			EnableMacro();
		else
			DisableMacro();
	}
};

class GLCompileMacro_USE_PORTAL_CLIPPING:
	GLCompileMacro
{
public:
	GLCompileMacro_USE_PORTAL_CLIPPING( GLShader* shader ):
		GLCompileMacro( shader )
	{
	}
	
	const char* GetName() const
	{
		return "USE_PORTAL_CLIPPING";
	}
	EGLCompileMacro GetType() const
	{
		return USE_PORTAL_CLIPPING;
	}
	
	void EnablePortalClipping()
	{
		EnableMacro();
	}
	void DisablePortalClipping()
	{
		DisableMacro();
	}
	
	void SetPortalClipping( bool enable )
	{
		if( enable )
			EnableMacro();
		else
			DisableMacro();
	}
};

class GLCompileMacro_USE_FRUSTUM_CLIPPING:
	GLCompileMacro
{
public:
	GLCompileMacro_USE_FRUSTUM_CLIPPING( GLShader* shader ):
		GLCompileMacro( shader )
	{
	}
	
	const char* GetName() const
	{
		return "USE_FRUSTUM_CLIPPING";
	}
	EGLCompileMacro GetType() const
	{
		return USE_FRUSTUM_CLIPPING;
	}
	
	void EnableFrustumClipping()
	{
		EnableMacro();
	}
	void DisableFrustumClipping()
	{
		DisableMacro();
	}
	
	void SetFrustumClipping( bool enable )
	{
		if( enable )
			EnableMacro();
		else
			DisableMacro();
	}
};

class GLCompileMacro_USE_VERTEX_SKINNING:
	GLCompileMacro
{
public:
	GLCompileMacro_USE_VERTEX_SKINNING( GLShader* shader ):
		GLCompileMacro( shader )
	{
	}
	
	const char* GetName() const
	{
		return "USE_VERTEX_SKINNING";
	}
	EGLCompileMacro GetType() const
	{
		return USE_VERTEX_SKINNING;
	}
	bool		HasConflictingMacros( int permutation, const idList<GLCompileMacro*>& macros ) const;
	bool		MissesRequiredMacros( int permutation, const idList<GLCompileMacro*>& macros ) const;
	uint32_t	GetRequiredVertexAttributes() const
	{
		return VA_BONE_INDEXES | VA_BONE_WEIGHTS;
	}
	
	
	void EnableVertexSkinning()
	{
		EnableMacro();
	}
	void DisableVertexSkinning()
	{
		DisableMacro();
	}
	
	void SetVertexSkinning( bool enable )
	{
		if( enable )
			EnableVertexSkinning();
		else
			DisableVertexSkinning();
	}
};

class GLCompileMacro_USE_VERTEX_ANIMATION:
	GLCompileMacro
{
public:
	GLCompileMacro_USE_VERTEX_ANIMATION( GLShader* shader ):
		GLCompileMacro( shader )
	{
	}
	
	const char* GetName() const
	{
		return "USE_VERTEX_ANIMATION";
	}
	EGLCompileMacro GetType() const
	{
		return USE_VERTEX_ANIMATION;
	}
	bool		HasConflictingMacros( int permutation, const idList<GLCompileMacro*>& macros ) const;
	uint32_t	GetRequiredVertexAttributes() const;
	
	
	void EnableVertexAnimation()
	{
		EnableMacro();
	}
	
	void DisableVertexAnimation()
	{
		DisableMacro();
	}
	
	void SetVertexAnimation( bool enable )
	{
		if( enable )
			EnableVertexAnimation();
		else
			DisableVertexAnimation();
	}
};

/*
class GLCompileMacro_USE_DEFORM_VERTEXES:
GLCompileMacro
{
public:
	GLCompileMacro_USE_DEFORM_VERTEXES(GLShader* shader):
	  GLCompileMacro(shader)
	{
	}

	const char* GetName() const { return "USE_DEFORM_VERTEXES"; }
	EGLCompileMacro GetType() const { return USE_DEFORM_VERTEXES; }
	bool		HasConflictingMacros(int permutation, const idList<GLCompileMacro*>& macros) const;
	uint32_t	GetRequiredVertexAttributes() const { return VA_NORMAL; }

	void EnableDeformVertexes()
	{
		if(glConfig.driverType == GLDRV_OPENGL3 && r_vboDeformVertexes->integer)
		{
			EnableMacro();
		}
		else
		{
			DisableMacro();
		}
	}

	void DisableDeformVertexes()
	{
		DisableMacro();
	}

	void SetDeformVertexes(bool enable)
	{
		if(enable && (glConfig.driverType == GLDRV_OPENGL3 && r_vboDeformVertexes->integer))
			EnableMacro();
		else
			DisableMacro();
	}
};
*/

class GLCompileMacro_USE_TCGEN_ENVIRONMENT:
	GLCompileMacro
{
public:
	GLCompileMacro_USE_TCGEN_ENVIRONMENT( GLShader* shader ):
		GLCompileMacro( shader )
	{
	}
	
	const char* GetName() const
	{
		return "USE_TCGEN_ENVIRONMENT";
	}
	EGLCompileMacro GetType() const
	{
		return USE_TCGEN_ENVIRONMENT;
	}
	uint32_t	GetRequiredVertexAttributes() const
	{
		return VA_NORMAL;
	}
	
	void EnableTCGenEnvironment()
	{
		EnableMacro();
	}
	
	void DisableTCGenEnvironment()
	{
		DisableMacro();
	}
	
	void SetTCGenEnvironment( bool enable )
	{
		if( enable )
			EnableMacro();
		else
			DisableMacro();
	}
};


class GLCompileMacro_USE_NORMAL_MAPPING:
	GLCompileMacro
{
public:
	GLCompileMacro_USE_NORMAL_MAPPING( GLShader* shader ):
		GLCompileMacro( shader )
	{
	}
	
	const char* GetName() const
	{
		return "USE_NORMAL_MAPPING";
	}
	EGLCompileMacro GetType() const
	{
		return USE_NORMAL_MAPPING;
	}
	uint32_t	GetRequiredVertexAttributes() const
	{
		return VA_NORMAL | VA_TANGENT | VA_BITANGENT;
	}
	
	void EnableNormalMapping()
	{
		EnableMacro();
	}
	void DisableNormalMapping()
	{
		DisableMacro();
	}
	
	void SetNormalMapping( bool enable )
	{
		if( enable )
			EnableMacro();
		else
			DisableMacro();
	}
};


class GLCompileMacro_USE_PARALLAX_MAPPING:
	GLCompileMacro
{
public:
	GLCompileMacro_USE_PARALLAX_MAPPING( GLShader* shader ):
		GLCompileMacro( shader )
	{
	}
	
	const char* GetName() const
	{
		return "USE_PARALLAX_MAPPING";
	}
	EGLCompileMacro GetType() const
	{
		return USE_PARALLAX_MAPPING;
	}
	bool		MissesRequiredMacros( int permutation, const idList<GLCompileMacro*>& macros ) const;
	
	void EnableParallaxMapping()
	{
		EnableMacro();
	}
	void DisableParallaxMapping()
	{
		DisableMacro();
	}
	
	void SetParallaxMapping( bool enable )
	{
		if( enable )
			EnableMacro();
		else
			DisableMacro();
	}
};


class GLCompileMacro_USE_REFLECTIVE_SPECULAR:
	GLCompileMacro
{
public:
	GLCompileMacro_USE_REFLECTIVE_SPECULAR( GLShader* shader ):
		GLCompileMacro( shader )
	{
	}
	
	const char* GetName() const
	{
		return "USE_REFLECTIVE_SPECULAR";
	}
	EGLCompileMacro GetType() const
	{
		return USE_REFLECTIVE_SPECULAR;
	}
	bool		MissesRequiredMacros( int permutation, const idList<GLCompileMacro*>& macros ) const;
	
	void EnableReflectiveSpecular()
	{
		EnableMacro();
	}
	void DisableReflectiveSpecular()
	{
		DisableMacro();
	}
	
	void SetReflectiveSpecular( bool enable )
	{
		if( enable )
			EnableMacro();
		else
			DisableMacro();
	}
};


class GLCompileMacro_TWOSIDED:
	GLCompileMacro
{
public:
	GLCompileMacro_TWOSIDED( GLShader* shader ):
		GLCompileMacro( shader )
	{
	}
	
	const char* GetName() const
	{
		return "TWOSIDED";
	}
	EGLCompileMacro GetType() const
	{
		return TWOSIDED;
	}
	//bool		MissesRequiredMacros(int permutation, const idList<GLCompileMacro*>& macros) const;
	uint32_t	GetRequiredVertexAttributes() const
	{
		return VA_NORMAL;
	}
	
	void EnableMacro_TWOSIDED()
	{
		EnableMacro();
	}
	void DisableMacro_TWOSIDED()
	{
		DisableMacro();
	}
	
	void SetMacro_TWOSIDED( cullType_t cullType )
	{
		if( cullType == CT_TWO_SIDED || cullType == CT_BACK_SIDED )
			EnableMacro();
		else
			DisableMacro();
	}
};

class GLCompileMacro_EYE_OUTSIDE:
	GLCompileMacro
{
public:
	GLCompileMacro_EYE_OUTSIDE( GLShader* shader ):
		GLCompileMacro( shader )
	{
	}
	
	const char* GetName() const
	{
		return "EYE_OUTSIDE";
	}
	EGLCompileMacro GetType() const
	{
		return EYE_OUTSIDE;
	}
	
	void EnableMacro_EYE_OUTSIDE()
	{
		EnableMacro();
	}
	void DisableMacro_EYE_OUTSIDE()
	{
		DisableMacro();
	}
	
	void SetMacro_EYE_OUTSIDE( bool enable )
	{
		if( enable )
			EnableMacro();
		else
			DisableMacro();
	}
};

class GLCompileMacro_BRIGHTPASS_FILTER:
	GLCompileMacro
{
public:
	GLCompileMacro_BRIGHTPASS_FILTER( GLShader* shader ):
		GLCompileMacro( shader )
	{
	}
	
	const char* GetName() const
	{
		return "BRIGHTPASS_FILTER";
	}
	EGLCompileMacro GetType() const
	{
		return BRIGHTPASS_FILTER;
	}
	
	void EnableMacro_BRIGHTPASS_FILTER()
	{
		EnableMacro();
	}
	void DisableMacro_BRIGHTPASS_FILTER()
	{
		DisableMacro();
	}
	
	void SetMacro_BRIGHTPASS_FILTER( bool enable )
	{
		if( enable )
			EnableMacro();
		else
			DisableMacro();
	}
};

class GLCompileMacro_LIGHT_DIRECTIONAL:
	GLCompileMacro
{
public:
	GLCompileMacro_LIGHT_DIRECTIONAL( GLShader* shader ):
		GLCompileMacro( shader )
	{
	}
	
	const char* GetName() const
	{
		return "LIGHT_DIRECTIONAL";
	}
	EGLCompileMacro GetType() const
	{
		return LIGHT_DIRECTIONAL;
	}
	bool		HasConflictingMacros( int permutation, const idList<GLCompileMacro*>& macros ) const;
	
	void EnableMacro_LIGHT_DIRECTIONAL()
	{
		EnableMacro();
	}
	void DisableMacro_LIGHT_DIRECTIONAL()
	{
		DisableMacro();
	}
	
	void SetMacro_LIGHT_DIRECTIONAL( bool enable )
	{
		if( enable )
			EnableMacro();
		else
			DisableMacro();
	}
};

class GLCompileMacro_LIGHT_PROJ:
	GLCompileMacro
{
public:
	GLCompileMacro_LIGHT_PROJ( GLShader* shader ):
		GLCompileMacro( shader )
	{
	}
	
	const char* GetName() const
	{
		return "LIGHT_PROJ";
	}
	EGLCompileMacro GetType() const
	{
		return LIGHT_PROJ;
	}
	//bool		HasConflictingMacros(int permutation, const idList<GLCompileMacro*>& macros) const;
	
	void EnableMacro_LIGHT_PROJ()
	{
		EnableMacro();
	}
	void DisableMacro_LIGHT_PROJ()
	{
		DisableMacro();
	}
	
	void SetMacro_LIGHT_PROJ( bool enable )
	{
		if( enable )
			EnableMacro();
		else
			DisableMacro();
	}
};


class GLCompileMacro_USE_SHADOWING:
	GLCompileMacro
{
public:
	GLCompileMacro_USE_SHADOWING( GLShader* shader ):
		GLCompileMacro( shader )
	{
	}
	
	const char* GetName() const
	{
		return "USE_SHADOWING";
	}
	EGLCompileMacro GetType() const
	{
		return USE_SHADOWING;
	}
	
	void EnableShadowing()
	{
		EnableMacro();
	}
	void DisableShadowing()
	{
		DisableMacro();
	}
	
	void SetShadowing( bool enable )
	{
		if( enable )
			EnableMacro();
		else
			DisableMacro();
	}
};


class GLCompileMacro_USE_GBUFFER:
	GLCompileMacro
{
public:
	GLCompileMacro_USE_GBUFFER( GLShader* shader ):
		GLCompileMacro( shader )
	{
	}
	
	const char* GetName() const
	{
		return "USE_GBUFFER";
	}
	EGLCompileMacro GetType() const
	{
		return USE_GBUFFER;
	}
	
	void EnableMacro_USE_GBUFFER()
	{
		EnableMacro();
	}
	void DisableMacro_USE_GBUFFER()
	{
		DisableMacro();
	}
	
	void SetMacro_USE_GBUFFER( bool enable )
	{
		if( enable )
			EnableMacro();
		else
			DisableMacro();
	}
};



class u_CurrentRenderImage:
	GLUniform
{
public:
	u_CurrentRenderImage( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_CurrentRenderImage";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_CurrentRenderImage = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_CurrentRenderImage( int value )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_CurrentRenderImage == value )
			return;
			
		program->t_CurrentRenderImage = value;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_CurrentRenderImage( program = %s, value = %d ) ---\n", program->name.c_str(), value );
		}
#endif
		glUniform1i( program->u_CurrentRenderImage, value );
	}
};

class u_CurrentNormalsImage:
	GLUniform
{
public:
	u_CurrentNormalsImage( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_CurrentNormalsImage";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_CurrentNormalsImage = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_CurrentNormalsImage( int value )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_CurrentNormalsImage == value )
			return;
			
		program->t_CurrentNormalsImage = value;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_CurrentNormalsImage( program = %s, value = %d ) ---\n", program->name.c_str(), value );
		}
#endif
		glUniform1i( program->u_CurrentNormalsImage, value );
	}
};



class u_CurrentDepthImage:
	GLUniform
{
public:
	u_CurrentDepthImage( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_CurrentDepthImage";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_CurrentDepthImage = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_CurrentDepthImage( int value )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_CurrentDepthImage == value )
			return;
			
		program->t_CurrentDepthImage = value;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_CurrentDepthImage( program = %s, value = %d ) ---\n", program->name.c_str(), value );
		}
#endif
		glUniform1i( program->u_CurrentDepthImage, value );
	}
};

class u_CurrentLightImage:
	GLUniform
{
public:
	u_CurrentLightImage( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_CurrentLightImage";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_CurrentLightImage = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_CurrentLightImage( int value )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_CurrentLightImage == value )
			return;
			
		program->t_CurrentLightImage = value;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_CurrentLightImage( program = %s, value = %d ) ---\n", program->name.c_str(), value );
		}
#endif
		glUniform1i( program->u_CurrentLightImage, value );
	}
};


class u_ColorImage:
	GLUniform
{
public:
	u_ColorImage( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_ColorImage";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_ColorImage = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_ColorImage( int value )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_ColorImage == value )
			return;
			
		program->t_ColorImage = value;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_ColorImage( program = %s, value = %d ) ---\n", program->name.c_str(), value );
		}
#endif
		glUniform1i( program->u_ColorImage, value );
	}
};

class u_DiffuseImage:
	GLUniform
{
public:
	u_DiffuseImage( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_DiffuseImage";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_DiffuseImage = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_DiffuseImage( int value )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_DiffuseImage == value )
			return;
			
		program->t_DiffuseImage = value;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_DiffuseImage( program = %s, value = %d ) ---\n", program->name.c_str(), value );
		}
#endif
		glUniform1i( program->u_DiffuseImage, value );
	}
};

class u_NormalImage:
	GLUniform
{
public:
	u_NormalImage( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_NormalImage";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_NormalImage = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_NormalImage( int value )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_NormalImage == value )
			return;
			
		program->t_NormalImage = value;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_NormalImage( program = %s, value = %d ) ---\n", program->name.c_str(), value );
		}
#endif
		glUniform1i( program->u_NormalImage, value );
	}
};

class u_SpecularImage:
	GLUniform
{
public:
	u_SpecularImage( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_SpecularImage";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_SpecularImage = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_SpecularImage( int value )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_SpecularImage == value )
			return;
			
		program->t_SpecularImage = value;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_SpecularImage( program = %s, value = %d ) ---\n", program->name.c_str(), value );
		}
#endif
		glUniform1i( program->u_SpecularImage, value );
	}
};



class u_ModelMatrix:
	GLUniform
{
public:
	u_ModelMatrix( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_ModelMatrix";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_ModelMatrix = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_ModelMatrix( const idMat4& m )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_ModelMatrix == m )
			return;
			
		program->t_ModelMatrix = m;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_ModelMatrix( program = %s, "
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f ) ) ---\n",
						   program->name.c_str(),
						   m[0][0], m[0][1], m[0][2], m[0][3],
						   m[1][0], m[1][1], m[1][2], m[1][3],
						   m[2][0], m[2][1], m[2][2], m[2][3],
						   m[3][0], m[3][1], m[3][2], m[3][3] );
		}
#endif
		
		glUniformMatrix4fv( program->u_ModelMatrix, 1, GL_FALSE, m.ToFloatPtr() );
	}
};

class u_ViewMatrix:
	GLUniform
{
public:
	u_ViewMatrix( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_ViewMatrix";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_ViewMatrix = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_ViewMatrix( const idMat4& m )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_ViewMatrix == m )
			return;
			
		program->t_ViewMatrix = m;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_ViewMatrix( program = %s, "
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f ) ) ---\n",
						   program->name.c_str(),
						   m[0][0], m[0][1], m[0][2], m[0][3],
						   m[1][0], m[1][1], m[1][2], m[1][3],
						   m[2][0], m[2][1], m[2][2], m[2][3],
						   m[3][0], m[3][1], m[3][2], m[3][3] );
		}
#endif
		
		glUniformMatrix4fv( program->u_ViewMatrix, 1, GL_FALSE, m.ToFloatPtr() );
	}
};

class u_ViewMatrixTranspose:
	GLUniform
{
public:
	u_ViewMatrixTranspose( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_ViewMatrixTranspose";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_ViewMatrixTranspose = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_ViewMatrixTranspose( const idMat4& m )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_ViewMatrixTranspose == m )
			return;
			
		program->t_ViewMatrixTranspose = m;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_ViewMatrixTranspose( program = %s, "
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f ) ) ---\n",
						   program->name.c_str(),
						   m[0][0], m[0][1], m[0][2], m[0][3],
						   m[1][0], m[1][1], m[1][2], m[1][3],
						   m[2][0], m[2][1], m[2][2], m[2][3],
						   m[3][0], m[3][1], m[3][2], m[3][3] );
		}
#endif
		
		glUniformMatrix4fv( program->u_ViewMatrixTranspose, 1, GL_FALSE, m.ToFloatPtr() );
	}
};

class u_ColorMatrix:
	GLUniform
{
public:
	u_ColorMatrix( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_ColorMatrix";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_ColorMatrix = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_ColorMatrix( const idMat4& m )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_ColorMatrix == m )
			return;
			
		program->t_ColorMatrix = m;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_ColorMatrix( program = %s, "
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f ) ) ---\n",
						   program->name.c_str(),
						   m[0][0], m[0][1], m[0][2], m[0][3],
						   m[1][0], m[1][1], m[1][2], m[1][3],
						   m[2][0], m[2][1], m[2][2], m[2][3],
						   m[3][0], m[3][1], m[3][2], m[3][3] );
		}
#endif
		
		glUniformMatrix4fv( program->u_ColorMatrix, 1, GL_FALSE, m.ToFloatPtr() );
	}
};

class u_ModelViewProjectionMatrix:
	GLUniform
{
public:
	u_ModelViewProjectionMatrix( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_ModelViewProjectionMatrix";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_ModelViewProjectionMatrix = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_ModelViewProjectionMatrix( const idMat4& m )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_ModelViewProjectionMatrix == m )
			return;
			
		program->t_ModelViewProjectionMatrix = m;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_ModelViewProjectionMatrix( program = %s, "
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f ) ) ---\n",
						   program->name.c_str(),
						   m[0][0], m[0][1], m[0][2], m[0][3],
						   m[1][0], m[1][1], m[1][2], m[1][3],
						   m[2][0], m[2][1], m[2][2], m[2][3],
						   m[3][0], m[3][1], m[3][2], m[3][3] );
		}
#endif
		
		glUniformMatrix4fv( program->u_ModelViewProjectionMatrix, 1, GL_FALSE, m.ToFloatPtr() );
	}
};

class u_UnprojectMatrix:
	GLUniform
{
public:
	u_UnprojectMatrix( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_UnprojectMatrix";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_UnprojectMatrix = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_UnprojectMatrix( const idMat4& m )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_UnprojectMatrix == m )
			return;
			
		program->t_UnprojectMatrix = m;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_UnprojectMatrix( program = %s, "
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f ) ) ---\n",
						   program->name.c_str(),
						   m[0][0], m[0][1], m[0][2], m[0][3],
						   m[1][0], m[1][1], m[1][2], m[1][3],
						   m[2][0], m[2][1], m[2][2], m[2][3],
						   m[3][0], m[3][1], m[3][2], m[3][3] );
		}
#endif
		
		glUniformMatrix4fv( program->u_UnprojectMatrix, 1, GL_FALSE, m.ToFloatPtr() );
	}
};


class u_ShadowMatrix:
	GLUniform
{
public:
	u_ShadowMatrix( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_ShadowMatrix";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_ShadowMatrix = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_ShadowMatrix( const idMat4& m )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_ShadowMatrix == m )
			return;
			
		program->t_ShadowMatrix = m;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_ShadowMatrix( program = %s, "
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f ) ) ---\n",
						   program->name.c_str(),
						   m[0][0], m[0][1], m[0][2], m[0][3],
						   m[1][0], m[1][1], m[1][2], m[1][3],
						   m[2][0], m[2][1], m[2][2], m[2][3],
						   m[3][0], m[3][1], m[3][2], m[3][3] );
		}
#endif
		
		glUniformMatrix4fv( program->u_ShadowMatrix, 1, GL_FALSE, m.ToFloatPtr() );
	}
};

class u_DiffuseMatrixS:
	GLUniform
{
public:
	u_DiffuseMatrixS( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_DiffuseMatrixS";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_DiffuseMatrixS = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_DiffuseMatrixS( const idVec4& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_DiffuseMatrixS == v )
			return;
			
		program->t_DiffuseMatrixS = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_DiffuseMatrixS( program = %s, vector = ( %5.3f, %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2], v[3] );
		}
#endif
		
		glUniform4f( program->u_DiffuseMatrixS, v[0], v[1], v[2], v[3] );
	}
};

class u_ColorMatrixT:
	GLUniform
{
public:
	u_ColorMatrixT( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_ColorMatrixT";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_ColorMatrixT = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_ColorMatrixT( const idVec4& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_ColorMatrixT == v )
			return;
			
		program->t_ColorMatrixT = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_ColorMatrixT( program = %s, vector = ( %5.3f, %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2], v[3] );
		}
#endif
		
		glUniform4f( program->u_ColorMatrixT, v[0], v[1], v[2], v[3] );
	}
};

class u_ColorMatrixS:
	GLUniform
{
public:
	u_ColorMatrixS( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_ColorMatrixS";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_ColorMatrixS = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_ColorMatrixS( const idVec4& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_ColorMatrixS == v )
			return;
			
		program->t_ColorMatrixS = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_ColorMatrixS( program = %s, vector = ( %5.3f, %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2], v[3] );
		}
#endif
		
		glUniform4f( program->u_ColorMatrixS, v[0], v[1], v[2], v[3] );
	}
};

class u_DiffuseMatrixT:
	GLUniform
{
public:
	u_DiffuseMatrixT( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_DiffuseMatrixT";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_DiffuseMatrixT = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_DiffuseMatrixT( const idVec4& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_DiffuseMatrixT == v )
			return;
			
		program->t_DiffuseMatrixT = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_DiffuseMatrixT( program = %s, vector = ( %5.3f, %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2], v[3] );
		}
#endif
		
		glUniform4f( program->u_DiffuseMatrixT, v[0], v[1], v[2], v[3] );
	}
};

class u_BumpMatrixS:
	GLUniform
{
public:
	u_BumpMatrixS( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_BumpMatrixS";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_BumpMatrixS = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_BumpMatrixS( const idVec4& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_BumpMatrixS == v )
			return;
			
		program->t_BumpMatrixS = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_BumpMatrixS( program = %s, vector = ( %5.3f, %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2], v[3] );
		}
#endif
		
		glUniform4f( program->u_BumpMatrixS, v[0], v[1], v[2], v[3] );
	}
};

class u_BumpMatrixT:
	GLUniform
{
public:
	u_BumpMatrixT( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_BumpMatrixT";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_BumpMatrixT = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_BumpMatrixT( const idVec4& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_BumpMatrixT == v )
			return;
			
		program->t_BumpMatrixT = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_BumpMatrixT( program = %s, vector = ( %5.3f, %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2], v[3] );
		}
#endif
		
		glUniform4f( program->u_BumpMatrixT, v[0], v[1], v[2], v[3] );
	}
};


class u_SpecularMatrixS:
	GLUniform
{
public:
	u_SpecularMatrixS( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_SpecularMatrixS";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_SpecularMatrixS = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_SpecularMatrixS( const idVec4& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_SpecularMatrixS == v )
			return;
			
		program->t_SpecularMatrixS = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_SpecularMatrixS( program = %s, vector = ( %5.3f, %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2], v[3] );
		}
#endif
		
		glUniform4f( program->u_SpecularMatrixS, v[0], v[1], v[2], v[3] );
	}
};

class u_SpecularMatrixT:
	GLUniform
{
public:
	u_SpecularMatrixT( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_SpecularMatrixT";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_SpecularMatrixT = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_SpecularMatrixT( const idVec4& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_SpecularMatrixT == v )
			return;
			
		program->t_SpecularMatrixT = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_SpecularMatrixT( program = %s, vector = ( %5.3f, %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2], v[3] );
		}
#endif
		
		glUniform4f( program->u_SpecularMatrixT, v[0], v[1], v[2], v[3] );
	}
};


class u_Color:
	GLUniform
{
public:
	u_Color( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_Color";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_Color = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_Color( const idVec4& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_Color == v )
			return;
			
		program->t_Color = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_Color( program = %s, vector = ( %5.3f, %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2], v[3] );
		}
#endif
		
		glUniform4f( program->u_Color, v[0], v[1], v[2], v[3] );
	}
};

class u_ColorModulate:
	GLUniform
{
public:
	u_ColorModulate( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_ColorModulate";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_ColorModulate = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void ResetUniform_ColorModulate()
	{
		SetUniform_ColorModulate( idVec4( 0, 0, 0, 0 ) );
	}
	
	void SetUniform_ColorModulate( const idVec4& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_ColorModulate == v )
			return;
			
		program->t_ColorModulate = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_ColorModulate( program = %s, vector = ( %5.3f, %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2], v[3] );
		}
#endif
		
		glUniform4f( program->u_ColorModulate, v[0], v[1], v[2], v[3] );
	}
};

class u_AmbientColor:
	GLUniform
{
public:
	u_AmbientColor( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_AmbientColor";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_AmbientColor = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_AmbientColor( const idVec4& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_AmbientColor == v )
			return;
			
		program->t_AmbientColor = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_AmbientColor( program = %s, vector = ( %5.3f, %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2], v[3] );
		}
#endif
		
		glUniform4f( program->u_AmbientColor, v[0], v[1], v[2], v[3] );
	}
};

class u_DiffuseColor:
	GLUniform
{
public:
	u_DiffuseColor( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_DiffuseColor";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_DiffuseColor = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_DiffuseColor( const idVec4& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_DiffuseColor == v )
			return;
			
		program->t_DiffuseColor = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_DiffuseColor( program = %s, vector = ( %5.3f, %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2], v[3] );
		}
#endif
		
		glUniform4f( program->u_DiffuseColor, v[0], v[1], v[2], v[3] );
	}
};

class u_SpecularColor:
	GLUniform
{
public:
	u_SpecularColor( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_SpecularColor";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_SpecularColor = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_SpecularColor( const idVec4& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_SpecularColor == v )
			return;
			
		program->t_SpecularColor = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_SpecularColor( program = %s, vector = ( %5.3f, %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2], v[3] );
		}
#endif
		
		glUniform4f( program->u_SpecularColor, v[0], v[1], v[2], v[3] );
	}
};

class u_LightColor:
	GLUniform
{
public:
	u_LightColor( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_LightColor";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_LightColor = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_LightColor( const idVec4& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_LightColor == v )
			return;
			
		program->t_LightColor = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_LightColor( program = %s, vector = ( %5.3f, %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2], v[3] );
		}
#endif
		
		glUniform4f( program->u_LightColor, v[0], v[1], v[2], v[3] );
	}
};


/*
class u_AlphaTest:
GLUniform
{
public:
	u_AlphaTest(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_AlphaTest"; }
	void				UpdateShaderProgramUniformLocation(shaderProgram_t *shaderProgram) const
	{
		shaderProgram->u_AlphaTest = glGetUniformLocation(shaderProgram->program, GetName());
	}

	void SetUniform_AlphaTest(uint32_t stateBits)
	{
		GLSL_SetUniform_AlphaTest(_shader->GetProgram(), stateBits);
	}
};
*/
class u_AlphaTest:
	GLUniform
{
public:
	u_AlphaTest( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_AlphaTest";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_AlphaTest = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_AlphaTest( float value )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_AlphaTest == value )
			return;
			
		program->t_AlphaTest = value;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_AlphaTest( program = %s, value = %f ) ---\n", program->name.c_str(), value );
		}
#endif
		
		glUniform1f( program->u_AlphaTest, value );
	}
};


class u_LocalViewOrigin:
	GLUniform
{
public:
	u_LocalViewOrigin( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_LocalViewOrigin";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_LocalViewOrigin = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_LocalViewOrigin( const idVec3 v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_LocalViewOrigin == v )
			return;
			
		program->t_LocalViewOrigin = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_LocalViewOrigin( program = %s, vector = ( %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2] );
		}
#endif
		
		glUniform3f( program->u_LocalViewOrigin, v[0], v[1], v[2] );
	}
};

class u_GlobalViewOrigin:
	GLUniform
{
public:
	u_GlobalViewOrigin( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_GlobalViewOrigin";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_GlobalViewOrigin = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_GlobalViewOrigin( const idVec3 v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_GlobalViewOrigin == v )
			return;
			
		program->t_GlobalViewOrigin = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_GlobalViewOrigin( program = %s, vector = ( %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2] );
		}
#endif
		
		glUniform3f( program->u_GlobalViewOrigin, v[0], v[1], v[2] );
	}
};

/*
class u_LightDir:
GLUniform
{
public:
	u_LightDir(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_LightDir"; }
	void				UpdateShaderProgramUniformLocation(shaderProgram_t *shaderProgram) const
	{
		shaderProgram->u_LightDir = glGetUniformLocation(shaderProgram->program, GetName());
	}

	void SetUniform_LightDir(const vec3_t v)
	{
		GLSL_SetUniform_LightDir(_shader->GetProgram(), v);
	}
};
*/

class u_LocalLightOrigin:
	GLUniform
{
public:
	u_LocalLightOrigin( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_LocalLightOrigin";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_LocalLightOrigin = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_LocalLightOrigin( const idVec3& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_LocalLightOrigin == v )
			return;
			
		program->t_LocalLightOrigin = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_LocalLightOrigin( program = %s, vector = ( %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2] );
		}
#endif
		
		glUniform3f( program->u_LocalLightOrigin, v[0], v[1], v[2] );
	}
};

class u_GlobalLightOrigin:
	GLUniform
{
public:
	u_GlobalLightOrigin( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_GlobalLightOrigin";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_GlobalLightOrigin = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_GlobalLightOrigin( const idVec3& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_GlobalLightOrigin == v )
			return;
			
		program->t_GlobalLightOrigin = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_GlobalLightOrigin( program = %s, vector = ( %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2] );
		}
#endif
		
		glUniform3f( program->u_GlobalLightOrigin, v[0], v[1], v[2] );
	}
};

class u_LightProjectS:
	GLUniform
{
public:
	u_LightProjectS( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_LightProjectS";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_LightProjectS = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_LightProjectS( const idVec4& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_LightProjectS == v )
			return;
			
		program->t_LightProjectS = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_LightProjectS( program = %s, vector = ( %5.3f, %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2], v[3] );
		}
#endif
		
		glUniform4f( program->u_LightProjectS, v[0], v[1], v[2], v[3] );
	}
};

class u_LightProjectT:
	GLUniform
{
public:
	u_LightProjectT( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_LightProjectT";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_LightProjectT = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_LightProjectT( const idVec4& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_LightProjectT == v )
			return;
			
		program->t_LightProjectT = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_LightProjectT( program = %s, vector = ( %5.3f, %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2], v[3] );
		}
#endif
		
		glUniform4f( program->u_LightProjectT, v[0], v[1], v[2], v[3] );
	}
};

class u_LightProjectQ:
	GLUniform
{
public:
	u_LightProjectQ( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_LightProjectQ";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_LightProjectQ = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_LightProjectQ( const idVec4& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_LightProjectQ == v )
			return;
			
		program->t_LightProjectQ = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_LightProjectQ( program = %s, vector = ( %5.3f, %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2], v[3] );
		}
#endif
		
		glUniform4f( program->u_LightProjectQ, v[0], v[1], v[2], v[3] );
	}
};

class u_LightFalloffS:
	GLUniform
{
public:
	u_LightFalloffS( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_LightFalloffS";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_LightFalloffS = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_LightFalloffS( const idVec4& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_LightFalloffS == v )
			return;
			
		program->t_LightFalloffS = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_LightFalloffS( program = %s, vector = ( %5.3f, %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2], v[3] );
		}
#endif
		
		glUniform4f( program->u_LightFalloffS, v[0], v[1], v[2], v[3] );
	}
};


class u_LightRadius:
	GLUniform
{
public:
	u_LightRadius( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_LightRadius";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_LightRadius = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_LightRadius( float value )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_LightRadius == value )
			return;
			
		program->t_LightRadius = value;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_LightRadius( program = %s, value = %f ) ---\n", program->name.c_str(), value );
		}
#endif
		
		glUniform1f( program->u_LightRadius, value );
	}
};


class u_LightFrustum:
	GLUniform
{
public:
	u_LightFrustum( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_LightFrustum";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_LightFrustum = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_LightFrustum( const idPlane lightFrustum[6] )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if 0
		if( memcmp( program->t_LightFrustum, m ) )
			return;
#endif
			
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_LightFrustum( program = %s, "
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f )\n"
						   "( %5.3f, %5.3f, %5.3f, %5.3f ) ) ---\n",
						   program->name.c_str(),
						   lightFrustum[0][0], lightFrustum[0][1], lightFrustum[0][2], lightFrustum[0][3],
						   lightFrustum[1][0], lightFrustum[1][1], lightFrustum[1][2], lightFrustum[1][3],
						   lightFrustum[2][0], lightFrustum[2][1], lightFrustum[2][2], lightFrustum[2][3],
						   lightFrustum[3][0], lightFrustum[3][1], lightFrustum[3][2], lightFrustum[3][3],
						   lightFrustum[4][0], lightFrustum[4][1], lightFrustum[4][2], lightFrustum[4][3],
						   lightFrustum[5][0], lightFrustum[5][1], lightFrustum[5][2], lightFrustum[5][3]
						 );
		}
#endif
		
		glUniform4fv( program->u_LightFrustum, 6, lightFrustum[0].ToFloatPtr() );
	}
};


class u_ShadowTexelSize:
	GLUniform
{
public:
	u_ShadowTexelSize( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_ShadowTexelSize";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_ShadowTexelSize = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_ShadowTexelSize( float value )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_ShadowTexelSize == value )
			return;
			
		program->t_ShadowTexelSize = value;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_ShadowTexelSize( program = %s, value = %f ) ---\n", program->name.c_str(), value );
		}
#endif
		
		glUniform1f( program->u_ShadowTexelSize, value );
	}
};

class u_ShadowBlur:
	GLUniform
{
public:
	u_ShadowBlur( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_ShadowBlur";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_ShadowBlur = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_ShadowBlur( float value )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_ShadowBlur == value )
			return;
			
		program->t_ShadowBlur = value;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_ShadowBlur( program = %s, value = %f ) ---\n", program->name.c_str(), value );
		}
#endif
		
		glUniform1f( program->u_ShadowBlur, value );
	}
};


class u_PositionToJitterTexScale:
	GLUniform
{
public:
	u_PositionToJitterTexScale( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_PositionToJitterTexScale";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_PositionToJitterTexScale = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_PositionToJitterTexScale( const idVec4& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_PositionToJitterTexScale == v )
			return;
			
		program->t_PositionToJitterTexScale = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_PositionToJitterTexScale( program = %s, vector = ( %5.3f, %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2], v[3] );
		}
#endif
		
		glUniform4f( program->u_PositionToJitterTexScale, v[0], v[1], v[2], v[3] );
	}
};


class u_JitterTexScale:
	GLUniform
{
public:
	u_JitterTexScale( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_JitterTexScale";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_JitterTexScale = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_JitterTexScale( const idVec4& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_JitterTexScale == v )
			return;
			
		program->t_JitterTexScale = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_JitterTexScale( program = %s, vector = ( %5.3f, %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2], v[3] );
		}
#endif
		
		glUniform4f( program->u_JitterTexScale, v[0], v[1], v[2], v[3] );
	}
};

class u_JitterTexOffset:
	GLUniform
{
public:
	u_JitterTexOffset( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_JitterTexOffset";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_JitterTexOffset = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_JitterTexOffset( const idVec4& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_JitterTexOffset == v )
			return;
			
		program->t_JitterTexOffset = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_JitterTexOffset( program = %s, vector = ( %5.3f, %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2], v[3] );
		}
#endif
		
		glUniform4f( program->u_JitterTexOffset, v[0], v[1], v[2], v[3] );
	}
};


class u_HDRKey:
	GLUniform
{
public:
	u_HDRKey( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_HDRKey";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_HDRKey = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_HDRKey( float value )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_HDRKey == value )
			return;
			
		program->t_HDRKey = value;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_HDRKey( program = %s, value = %f ) ---\n", program->name.c_str(), value );
		}
#endif
		
		glUniform1f( program->u_HDRKey, value );
	}
};


class u_HDRAverageLuminance:
	GLUniform
{
public:
	u_HDRAverageLuminance( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_HDRAverageLuminance";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_HDRAverageLuminance = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_HDRAverageLuminance( float value )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_HDRAverageLuminance == value )
			return;
			
		program->t_HDRAverageLuminance = value;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_HDRAverageLuminance( program = %s, value = %f ) ---\n", program->name.c_str(), value );
		}
#endif
		
		glUniform1f( program->u_HDRAverageLuminance, value );
	}
};


class u_HDRMaxLuminance:
	GLUniform
{
public:
	u_HDRMaxLuminance( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_HDRMaxLuminance";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_HDRMaxLuminance = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_HDRMaxLuminance( float value )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_HDRMaxLuminance == value )
			return;
			
		program->t_HDRMaxLuminance = value;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_HDRMaxLuminance( program = %s, value = %f ) ---\n", program->name.c_str(), value );
		}
#endif
		
		glUniform1f( program->u_HDRMaxLuminance, value );
	}
};

class u_InvertedFramebufferResolution:
	GLUniform
{
public:
	u_InvertedFramebufferResolution( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_InvertedFramebufferResolution";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_InvertedFramebufferResolution = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_InvertedFramebufferResolution( const idVec2& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_InvertedFramebufferResolution == v )
			return;
			
		program->t_InvertedFramebufferResolution = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_InvertedFramebufferResolution( program = %s, vector = ( %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1] );
		}
#endif
		
		glUniform2f( program->u_InvertedFramebufferResolution, v[0], v[1] );
	}
	
	void SetUniform_InvertedFramebufferResolution( const idScreenRect& viewport )
	{
		int	width = viewport.x2 - viewport.x1 + 1;
		int	height = viewport.y2 - viewport.y1 + 1;
		
		SetUniform_InvertedFramebufferResolution( idVec2( 1.0f / ( float ) width, 1.0f / ( float ) height ) );
	}
};

class u_FxaaInvertedFramebufferResolutionOpt:
	GLUniform
{
public:
	u_FxaaInvertedFramebufferResolutionOpt( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_FxaaInvertedFramebufferResolutionOpt";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_FxaaInvertedFramebufferResolutionOpt = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_FxaaInvertedFramebufferResolutionOpt( const idVec4& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_FxaaInvertedFramebufferResolutionOpt == v )
			return;
			
		program->t_FxaaInvertedFramebufferResolutionOpt = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_FxaaInvertedFramebufferResolutionOpt( program = %s, vector = ( %5.3f, %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2], v[3] );
		}
#endif
		
		glUniform4f( program->u_FxaaInvertedFramebufferResolutionOpt, v[0], v[1], v[2], v[3] );
	}
	
	void SetUniform_FxaaInvertedFramebufferResolutionOpt( const idScreenRect& viewport )
	{
		int	width = viewport.x2 - viewport.x1 + 1;
		int	height = viewport.y2 - viewport.y1 + 1;
		
		SetUniform_FxaaInvertedFramebufferResolutionOpt( idVec4( 2.0f / ( float ) width, 2.0f / ( float ) height, 0.5f / ( float ) width, 0.5f / ( float ) height ) );
	}
};

class u_NonPowerOfTwoScale:
	GLUniform
{
public:
	u_NonPowerOfTwoScale( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_NonPowerOfTwoScale";
	}
	void				UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_NonPowerOfTwoScale = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_NonPowerOfTwoScale( const idVec2& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_NonPowerOfTwoScale == v )
			return;
			
		program->t_NonPowerOfTwoScale = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_NonPowerOfTwoScale( program = %s, vector = ( %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1] );
		}
#endif
		
		glUniform2f( program->u_NonPowerOfTwoScale, v[0], v[1] );
	}
	
	void SetUniform_NonPowerOfTwoScale( const idScreenRect& viewport, int uploadWidth, int uploadHeight )
	{
		// screen power of two correction factor, assuming the copy to _currentRender
		// also copied an extra row and column for the bilerp
		int	w = viewport.x2 - viewport.x1 + 1;
		int	h = viewport.y2 - viewport.y1 + 1;
		
		SetUniform_NonPowerOfTwoScale( idVec2( ( float )w / uploadWidth, ( float )h / uploadHeight ) );
	}
};


class u_Viewport:
	GLUniform
{
public:
	u_Viewport( GLShader* shader ):
		GLUniform( shader )
	{
	}
	
	const char* GetName() const
	{
		return "u_Viewport";
	}
	void UpdateShaderProgramUniformLocation( shaderProgram_t* shaderProgram ) const
	{
		shaderProgram->u_Viewport = glGetUniformLocation( shaderProgram->program, GetName() );
	}
	
	void SetUniform_Viewport( const idVec4& v )
	{
		shaderProgram_t* program = _shader->GetProgram();
		
#if defined(USE_UNIFORM_FIREWALL)
		if( program->t_Viewport == v )
			return;
			
		program->t_Viewport = v;
#endif
		
#if defined(LOG_GLSL_UNIFORMS)
		if( r_logFile.GetBool() )
		{
			RB_LogComment( "--- SetUniform_Viewport( program = %s, vector = ( %5.3f, %5.3f, %5.3f, %5.3f  ) ) ---\n", program->name.c_str(), v[0], v[1], v[2], v[3] );
		}
#endif
		
		glUniform4f( program->u_Viewport, v[0], v[1], v[2], v[3] );
	}
	
	void SetUniform_Viewport( const idScreenRect& viewport )
	{
		int	width = viewport.x2 - viewport.x1 + 1;
		int	height = viewport.y2 - viewport.y1 + 1;
		
		SetUniform_Viewport( idVec4( viewport.x1, viewport.y1, ( float ) width, ( float ) height ) );
	}
};


class GLShader_generic:
	public GLShader,
	public u_ColorImage,
	public u_ColorMatrix,
//public u_ViewOrigin,
public u_AlphaTest,
public u_ModelMatrix,
public u_ModelViewProjectionMatrix,
public u_ColorModulate,
public u_Color,
//public u_BoneMatrix,
//public u_VertexInterpolation,
//public u_PortalPlane,
//public GLDeformStage,
//public GLCompileMacro_USE_PORTAL_CLIPPING,
public GLCompileMacro_USE_ALPHA_TESTING
//public GLCompileMacro_USE_VERTEX_SKINNING,
//public GLCompileMacro_USE_VERTEX_ANIMATION,
//public GLCompileMacro_USE_DEFORM_VERTEXES,
//public GLCompileMacro_USE_TCGEN_ENVIRONMENT
{
public:
	GLShader_generic();
};

class GLShader_geometricFill:
	public GLShader,
	public u_DiffuseImage,
	public u_NormalImage,
	public u_SpecularImage,
	public u_DiffuseMatrixS,
	public u_DiffuseMatrixT,
	public u_BumpMatrixS,
	public u_BumpMatrixT,
	public u_SpecularMatrixS,
	public u_SpecularMatrixT,
	public u_ColorModulate,
	public u_Color,
	public u_DiffuseColor,
	public u_SpecularColor,
	public u_GlobalViewOrigin,
	public u_ModelMatrix,
//public u_DepthScale,
public GLCompileMacro_USE_NORMAL_MAPPING
//public GLCompileMacro_USE_PARALLAX_MAPPING,
{
public:
	GLShader_geometricFill();
};

class GLShader_deferredLighting:
	public GLShader,
	public u_UnprojectMatrix,
	public u_Color,
	public u_ColorModulate,
	public u_LightColor,
	public u_GlobalViewOrigin,
	public u_GlobalLightOrigin,
	public u_LightRadius,
	public u_LightProjectS,
	public u_LightProjectT,
	public u_LightProjectQ,
	public u_LightFalloffS,
	public u_LightFrustum,
	public u_ShadowMatrix,
	public u_ShadowTexelSize,
	public u_ShadowBlur,
	public u_PositionToJitterTexScale,
	public u_JitterTexScale,
	public u_JitterTexOffset,
	public u_InvertedFramebufferResolution,
	public u_NonPowerOfTwoScale,
	public u_Viewport,
	public GLCompileMacro_USE_NORMAL_MAPPING,
//public GLCompileMacro_USE_PARALLAX_MAPPING,
#if !defined(USE_GLES2)
public GLCompileMacro_USE_SHADOWING,
#endif
public GLCompileMacro_USE_FRUSTUM_CLIPPING,
//public GLCompileMacro_LIGHT_DIRECTIONAL,
public GLCompileMacro_LIGHT_PROJ
{
public:
	GLShader_deferredLighting();
	
private:
	void		GetConstantCompileMacros( idStrList& compileMacros ) const;
	void		SetShaderProgramUniforms( shaderProgram_t* shaderProgram ) const;
};

class GLShader_forwardLighting:
	public GLShader,
	public u_ModelMatrix,
	public u_DiffuseMatrixS,
	public u_DiffuseMatrixT,
	public u_BumpMatrixS,
	public u_BumpMatrixT,
	public u_SpecularMatrixS,
	public u_SpecularMatrixT,
	public u_Color,
	public u_ColorModulate,
	public u_DiffuseColor,
	public u_SpecularColor,
	public u_LocalViewOrigin,
	public u_LocalLightOrigin,
	public u_GlobalLightOrigin,
	public u_LightRadius,
	public u_LightProjectS,
	public u_LightProjectT,
	public u_LightProjectQ,
	public u_LightFalloffS,
	public u_ModelViewProjectionMatrix,
	public u_ShadowMatrix,
	public u_ShadowTexelSize,
	public u_ShadowBlur,
	public u_PositionToJitterTexScale,
	public u_JitterTexScale,
	public u_JitterTexOffset,
//public u_BoneMatrix,
//public u_VertexInterpolation,
//public u_PortalPlane,
//public u_DepthScale,
//public GLDeformStage,
//public GLCompileMacro_USE_PORTAL_CLIPPING,
//public GLCompileMacro_USE_ALPHA_TESTING,
//public GLCompileMacro_USE_VERTEX_SKINNING,
//public GLCompileMacro_USE_VERTEX_ANIMATION,
//public GLCompileMacro_USE_DEFORM_VERTEXES,
public GLCompileMacro_USE_NORMAL_MAPPING,
//public GLCompileMacro_USE_PARALLAX_MAPPING,
#if !defined(USE_GLES2)
public GLCompileMacro_USE_SHADOWING,
#endif
//public GLCompileMacro_LIGHT_DIRECTIONAL,
public GLCompileMacro_LIGHT_PROJ
{
public:
	GLShader_forwardLighting();
	
private:
	void		GetConstantCompileMacros( idStrList& compileMacros ) const;
	void		SetShaderProgramUniforms( shaderProgram_t* shaderProgram ) const;
};

class GLShader_postLighting:
	public GLShader,
	public u_ModelMatrix,
	public u_DiffuseMatrixS,
	public u_DiffuseMatrixT,
	public u_BumpMatrixS,
	public u_BumpMatrixT,
	public u_SpecularMatrixS,
	public u_SpecularMatrixT,
	public u_Color,
	public u_ColorModulate,
	public u_DiffuseColor,
	public u_SpecularColor,
	public u_InvertedFramebufferResolution,
	public u_NonPowerOfTwoScale,
	public u_Viewport
{
public:
	GLShader_postLighting();
	
private:
	void		SetShaderProgramUniforms( shaderProgram_t* shaderProgram ) const;
};


class GLShader_shadowVolume:
	public GLShader,
	public u_LocalLightOrigin,
	public u_ModelViewProjectionMatrix
//public GLCompileMacro_USE_VERTEX_SKINNING,
//public GLCompileMacro_USE_VERTEX_ANIMATION,
//public GLCompileMacro_USE_DEFORM_VERTEXES,
{
public:
	GLShader_shadowVolume();
};

class GLShader_shadowMap:
	public GLShader,
	public u_ModelMatrix,
	public u_GlobalLightOrigin,
	public u_LightRadius
//public GLCompileMacro_USE_VERTEX_SKINNING,
//public GLCompileMacro_USE_VERTEX_ANIMATION,
//public GLCompileMacro_USE_DEFORM_VERTEXES,
{
public:
	GLShader_shadowMap();
	
private:
//	void			CreatePreIncludeText( idStr& preIncludeText );
};

class GLShader_FXAA:
	public GLShader,
	public u_CurrentRenderImage,
	public u_InvertedFramebufferResolution,
	public u_FxaaInvertedFramebufferResolutionOpt,
	public u_NonPowerOfTwoScale,
	public u_Viewport
{
public:
	GLShader_FXAA();
};

class GLShader_toneMapping:
	public GLShader,
	public u_CurrentRenderImage,
	public u_ModelViewProjectionMatrix,
	public u_HDRKey,
	public u_HDRAverageLuminance,
	public u_HDRMaxLuminance,
	public GLCompileMacro_BRIGHTPASS_FILTER
{
public:
	GLShader_toneMapping();
};


extern GLShader_generic* gl_genericShader;
extern GLShader_geometricFill* gl_geometricFillShader;
extern GLShader_deferredLighting* gl_deferredLightingShader;
extern GLShader_forwardLighting* gl_forwardLightingShader;
extern GLShader_postLighting* gl_postLightingShader;
extern GLShader_shadowVolume* gl_shadowVolumeShader;
extern GLShader_shadowMap* gl_shadowMapShader;
//extern GLShader_screen* gl_screenShader;
//extern GLShader_portal* gl_portalShader;
extern GLShader_FXAA* gl_FXAAShader;
extern GLShader_toneMapping* gl_toneMappingShader;
//extern GLShader_contrast* gl_contrastShader;
//extern GLShader_cameraEffects* gl_cameraEffectsShader;
//extern GLShader_blurX* gl_blurXShader;
//extern GLShader_blurY* gl_blurYShader;
//extern GLShader_debugShadowMap* gl_debugShadowMapShader;

#endif	// GL_SHADER_H
