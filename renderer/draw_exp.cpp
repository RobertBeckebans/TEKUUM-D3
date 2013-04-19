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

#include "precompiled.h"
#pragma hdrstop

#include "tr_local.h"
#include "GLShader.h"

/*

strictly experimental / research codepaths

!!!if we use front facing occluders, we can portal flow from light centers

try depth_component_16 rendering

do we care about portals from light perspective? back / front face issues.

how do we do weapon depth hacks with shadow buffers?
	distort their world space vertexes instead of offsetting their depth?

jittering off the side of a projection will give wrong shadows

really huge lights, like sunlight, are going to be problematic with fixed projections
	we could tile the projections and let the auto-resize cut them down as necessary

It sucks that depth buffers are non-linear, because the bias and compares change with distance

polygon offset factor causes occasional texture holes from highly angled textures

*/

static	bool		initialized;

#if 1
static	int shadowMapResolutions[MAX_SHADOWMAPS] = { 1024, 1024, 512, 256, 128 };
#else
static	int shadowMapResolutions[MAX_SHADOWMAPS] = { 1024, 512, 256, 128, 64 };
#endif
static	int	maxLightBufferSize = 1024;
static float lightBufferSizeFraction[MAX_SHADOWMAPS] = {0.5, 0.5, 0.5, 0.5, 0.5};

static	int viewBufferSize = 1024;
static	int	viewBufferHeight = 768;
static	int	maxViewBufferSize = 1024;
static float viewBufferSizeFraction = 0.5;
static float viewBufferHeightFraction = 0.5;
static	bool	nativeViewBuffer = false;		// true if viewBufferSize is the viewport width


static	Framebuffer*	downScaleFBO_quarter;
static	Framebuffer*	downScaleFBO_64x64;
//static	Framebuffer	*downScaleFBO_16x16;
//static	Framebuffer	*downScaleFBO_4x4;
//static	Framebuffer	*downScaleFBO_1x1;

static	Framebuffer*	shadowMapFBO[MAX_SHADOWMAPS];


static	idImage*		hdrRenderImage;
static	idImage*		downScaleImage_quarter;
static	idImage*		downScaleImage_64x64;
static	idImage*		shadowMapImage[MAX_SHADOWMAPS];
static	idImage*		shadowCubeImage[MAX_SHADOWMAPS];

static	idImage*		jitterImage16;
static	idImage*		jitterImage4;
static	idImage*		jitterImage1;

static	idImage*		random256Image;

static	float		viewLightAxialSize;

idCVar r_sb_lightResolutionUltra( "r_sb_lightResolutionUltra", "1024", CVAR_RENDERER | CVAR_INTEGER, "Pixel dimensions for each shadow buffer, 64 - 2048" );
idCVar r_sb_lightResolutionVeryHigh( "r_sb_lightResolutionVeryHigh", "512", CVAR_RENDERER | CVAR_INTEGER, "Pixel dimensions for each shadow buffer, 64 - 2048" );
idCVar r_sb_lightResolutionHigh( "r_sb_lightResolutionHigh", "256", CVAR_RENDERER | CVAR_INTEGER, "Pixel dimensions for each shadow buffer, 64 - 2048" );
idCVar r_sb_lightResolutionMedium( "r_sb_lightResolutionMedium", "128", CVAR_RENDERER | CVAR_INTEGER, "Pixel dimensions for each shadow buffer, 64 - 2048" );
idCVar r_sb_lightResolutionLow( "r_sb_lightResolutionLow", "64", CVAR_RENDERER | CVAR_INTEGER, "Pixel dimensions for each shadow buffer, 64 - 2048" );

idCVar r_sb_viewResolution( "r_sb_viewResolution", "1024", CVAR_RENDERER | CVAR_INTEGER, "Width of screen space shadow sampling" );
idCVar r_sb_noShadows( "r_sb_noShadows", "0", CVAR_RENDERER | CVAR_BOOL, "don't draw any occluders" );
idCVar r_sb_usePbuffer( "r_sb_usePbuffer", "1", CVAR_RENDERER | CVAR_BOOL, "draw offscreen" );
idCVar r_sb_jitterScale( "r_sb_jitterScale", "0.006", CVAR_RENDERER | CVAR_FLOAT, "scale factor for jitter offset" );
idCVar r_sb_biasScale( "r_sb_biasScale", "0.0001", CVAR_RENDERER | CVAR_FLOAT, "scale factor for jitter bias" );
idCVar r_sb_samples( "r_sb_samples", "4", CVAR_RENDERER | CVAR_INTEGER, "0, 1, 4, or 16" );

// polyOfsFactor causes holes in low res images
idCVar r_sb_polyOfsFactor( "r_sb_polyOfsFactor", "2", CVAR_RENDERER | CVAR_FLOAT, "polygonOffset factor for drawing shadow buffer" );
idCVar r_sb_polyOfsUnits( "r_sb_polyOfsUnits", "3000", CVAR_RENDERER | CVAR_FLOAT, "polygonOffset units for drawing shadow buffer" );
idCVar r_sb_occluderFacing( "r_sb_occluderFacing", "0", CVAR_RENDERER | CVAR_INTEGER, "0 = front faces, 1 = back faces, 2 = midway between" );
// r_sb_randomizeBufferOrientation?

idCVar r_sb_frustomFOV( "r_sb_frustomFOV", "90", CVAR_RENDERER | CVAR_FLOAT, "oversize FOV for point light side matching" );
idCVar r_sb_showFrustumPixels( "r_sb_showFrustumPixels", "0", CVAR_RENDERER | CVAR_BOOL, "color the pixels contained in the frustum" );
idCVar r_sb_singleSide( "r_sb_singleSide", "-1", CVAR_RENDERER | CVAR_INTEGER, "only draw a single side (0-5) of point lights" );
idCVar r_sb_useCulling( "r_sb_useCulling", "1", CVAR_RENDERER | CVAR_BOOL, "cull geometry to individual side frustums" );
idCVar r_sb_linearFilter( "r_sb_linearFilter", "1", CVAR_RENDERER | CVAR_BOOL, "use GL_LINEAR instead of GL_NEAREST on shadow maps" );

idCVar r_sb_screenSpaceShadow( "r_sb_screenSpaceShadow", "0", CVAR_RENDERER | CVAR_BOOL, "build shadows in screen space instead of on surfaces" );

idCVar r_useHighDynamicRange( "r_useHighDynamicRange", "0", CVAR_RENDERER | CVAR_BOOL, "use a floating point rendering buffer" );
idCVar r_hdrMinLuminance( "r_hdrMinLuminance", "0.18", CVAR_RENDERER | CVAR_FLOAT, "minimum light scale" );
idCVar r_hdrMaxLuminance( "r_hdrMaxLuminance", "3000", CVAR_RENDERER | CVAR_FLOAT, "maximum light scale" );
idCVar r_hdrKey( "r_hdrKey", "0.28", CVAR_RENDERER | CVAR_FLOAT, "" );
idCVar r_hdrContrastThreshold( "r_hdrContrastThreshold", "0.28", CVAR_RENDERER | CVAR_FLOAT, "" );
idCVar r_hdrContrastOffset( "r_hdrContrastOffset", "3.0", CVAR_RENDERER | CVAR_FLOAT, "" );
idCVar r_hdrGamma( "r_hdrGamma", "1", CVAR_RENDERER | CVAR_FLOAT, "monitor gamma power" );
idCVar r_showHDR( "r_showHDR", "0", CVAR_RENDERER | CVAR_BOOL, "show HDR luminance values" );

//idCVar r_hdr_monitorDither( "r_hdr_monitorDither", "0.01", CVAR_RENDERER | CVAR_FLOAT, "random dither in monitor space" );











// from world space to light origin, looking down the X axis
static matrix_t	unflippedLightMatrix;

// from world space to OpenGL view space, looking down the negative Z axis
static matrix_t	lightMatrix;

// from OpenGL view space to OpenGL NDC ( -1 : 1 in XYZ )
static matrix_t	lightProjectionMatrix;


/*
====================
RB_CreateBloomTable
====================
*/
static const int	BLOOM_RADIUS = 8;
static void RB_CreateBloomTable()
{
	float	bloom[BLOOM_RADIUS];
	float	total = 0;
	
	// gaussian
	float	stdDev = 2.0;
	for( int i = 0 ; i < BLOOM_RADIUS ; i++ )
	{
		float	f = ( float )i / stdDev;
		bloom[i] = exp( -0.5 * f * f );
		total += bloom[i];
	}
	
	total = ( total - bloom[0] ) * 2 + bloom[0];
	
	// normalize to 1.0 contribution, so a full row or column will equal 1.0
	for( int i = 0 ; i < BLOOM_RADIUS ; i++ )
	{
		bloom[i] *= 1.0 / total;
		common->Printf( "PARAM bloom%i = { %f };\n", i, bloom[i] );
	}
}

/*
====================
GL_SelectTextureNoClient
====================
*/
static void GL_SelectTextureNoClient( int unit )
{
	backEnd.glState.currenttmu = unit;
	glActiveTextureARB( GL_TEXTURE0_ARB + unit );
	RB_LogComment( "glActiveTextureARB( %i )\n", unit );
}


static void R_CreateHDRRenderImage( idImage* image )
{
	int width, height;
	
	if( glConfig.textureNonPowerOfTwoAvailable )
	{
		width = glConfig.vidWidth;
		height = glConfig.vidHeight;
	}
	else
	{
		width = MakePowerOfTwo( glConfig.vidWidth );
		height = MakePowerOfTwo( glConfig.vidHeight );
	}
	
#if 1
	if( r_useHighDynamicRange.GetBool() )
	{
		image->GenerateImage( NULL, width, height, TF_LINEAR, false, TR_CLAMP, TD_FBO_RGBA16F );
	}
	else
#endif
	{
		byte* data = ( byte* )Mem_Alloc( width * height * 4 );
		image->GenerateImage( data, width, height, TF_LINEAR, false, TR_CLAMP, TD_HIGH_QUALITY );
		Mem_Free( data );
	}
}

static void R_CreateDownScaleImage_Quarter( idImage* image )
{
	int width, height;
	
	if( glConfig.textureNonPowerOfTwoAvailable )
	{
		width = glConfig.vidWidth * 0.25f;
		height = glConfig.vidHeight * 0.25f;
	}
	else
	{
		width = MakePowerOfTwo( glConfig.vidWidth * 0.25f );
		height = MakePowerOfTwo( glConfig.vidHeight * 0.25f );
	}
	
#if 1
	if( r_useHighDynamicRange.GetBool() )
	{
		image->GenerateImage( NULL, width, height, TF_LINEAR, false, TR_CLAMP, TD_FBO_RGBA16F );
	}
	else
#endif
	{
		byte* data = ( byte* )Mem_Alloc( width * height * 4 );
		image->GenerateImage( data, width, height, TF_LINEAR, false, TR_CLAMP, TD_HIGH_QUALITY );
		Mem_Free( data );
	}
}

static void R_CreateDownScaleImage_64x64( idImage* image )
{
	int width, height;
	
	width = height = 64;
	
#if 1
	if( r_useHighDynamicRange.GetBool() )
	{
		image->GenerateImage( NULL, width, height, TF_LINEAR, false, TR_CLAMP, TD_FBO_RGBA16F );
	}
	else
#endif
	{
		byte* data = ( byte* )Mem_Alloc( width * height * 4 );
		image->GenerateImage( data, width, height, TF_LINEAR, false, TR_CLAMP, TD_HIGH_QUALITY );
		Mem_Free( data );
	}
}

/*
================
R_CreateShadowBufferImage
================
*/
static void R_CreateShadowBufferImage( idImage* image, int lightBufferSize )
{
	if( r_sb_mode.GetInteger() == SHADOWING_ESM16 )
	{
		image->GenerateImage( NULL, lightBufferSize, lightBufferSize, TF_LINEAR, false, TR_CLAMP, TD_FBO_R16F );
	}
	else if( r_sb_mode.GetInteger() == SHADOWING_ESM32 )
	{
		image->GenerateImage( NULL, lightBufferSize, lightBufferSize, TF_LINEAR, false, TR_CLAMP, TD_FBO_R32F );
	}
	else if( r_sb_mode.GetInteger() == SHADOWING_VSM16 )
	{
		image->GenerateImage( NULL, lightBufferSize, lightBufferSize, TF_LINEAR, false, TR_CLAMP, TD_FBO_RG16F );
	}
	else if( r_sb_mode.GetInteger() == SHADOWING_VSM32 )
	{
		image->GenerateImage( NULL, lightBufferSize, lightBufferSize, TF_LINEAR, false, TR_CLAMP, TD_FBO_RG32F );
	}
	else if( r_sb_mode.GetInteger() == SHADOWING_EVSM32 )
	{
		image->GenerateImage( NULL, lightBufferSize, lightBufferSize, TF_LINEAR, false, TR_CLAMP, TD_FBO_RGBA32F );
	}
	else
	{
		image->GenerateImage( NULL, lightBufferSize, lightBufferSize, TF_LINEAR, false, TR_CLAMP, TD_FBO_RGBA16F );
	}
	
	GL_CheckErrors();
	
	// explicit zero depth border
	/*
	float	color[4];
	color[0] = color[1] = color[2] = color[3] = 0;
	glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color );
	
	GL_CheckErrors();
	*/
}

static void R_CreateShadowBufferImage_Res0( idImage* image )
{
	R_CreateShadowBufferImage( image, shadowMapResolutions[0] );
}

static void R_CreateShadowBufferImage_Res1( idImage* image )
{
	R_CreateShadowBufferImage( image, shadowMapResolutions[1] );
}

static void R_CreateShadowBufferImage_Res2( idImage* image )
{
	R_CreateShadowBufferImage( image, shadowMapResolutions[2] );
}

static void R_CreateShadowBufferImage_Res3( idImage* image )
{
	R_CreateShadowBufferImage( image, shadowMapResolutions[3] );
}

static void R_CreateShadowBufferImage_Res4( idImage* image )
{
	R_CreateShadowBufferImage( image, shadowMapResolutions[4] );
}




static void R_CreateShadowCubeImage( idImage* image, int lightBufferSize )
{
	const byte*	pics[6];
	for( int i = 0 ; i < 6 ; i++ )
	{
		pics[i] = NULL;
	}
	
	if( r_sb_mode.GetInteger() == SHADOWING_ESM16 )
	{
		image->GenerateCubeImage( pics, lightBufferSize, TF_LINEAR, false, TD_FBO_R16F );
	}
	else if( r_sb_mode.GetInteger() == SHADOWING_ESM32 )
	{
		image->GenerateCubeImage( pics, lightBufferSize, TF_LINEAR, false, TD_FBO_R32F );
	}
	else if( r_sb_mode.GetInteger() == SHADOWING_VSM16 )
	{
		image->GenerateCubeImage( pics, lightBufferSize, TF_LINEAR, false, TD_FBO_RG16F );
	}
	else if( r_sb_mode.GetInteger() == SHADOWING_VSM32 )
	{
		image->GenerateCubeImage( pics, lightBufferSize, TF_LINEAR, false, TD_FBO_RG32F );
	}
	else if( r_sb_mode.GetInteger() == SHADOWING_EVSM32 )
	{
		image->GenerateCubeImage( pics, lightBufferSize, TF_LINEAR, false, TD_FBO_RGBA32F );
	}
	else
	{
		image->GenerateCubeImage( pics, lightBufferSize, TF_LINEAR, false, TD_FBO_RGBA16F );
	}
	
	GL_CheckErrors();
	
	// explicit zero depth border
	/*
	float	color[4];
	color[0] = color[1] = color[2] = color[3] = 0;
	glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color );
	
	GL_CheckErrors();
	*/
}

static void R_CreateShadowCubeImage_Res0( idImage* image )
{
	R_CreateShadowCubeImage( image, shadowMapResolutions[0] );
}

static void R_CreateShadowCubeImage_Res1( idImage* image )
{
	R_CreateShadowCubeImage( image, shadowMapResolutions[1] );
}

static void R_CreateShadowCubeImage_Res2( idImage* image )
{
	R_CreateShadowCubeImage( image, shadowMapResolutions[2] );
}

static void R_CreateShadowCubeImage_Res3( idImage* image )
{
	R_CreateShadowCubeImage( image, shadowMapResolutions[3] );
}

static void R_CreateShadowCubeImage_Res4( idImage* image )
{
	R_CreateShadowCubeImage( image, shadowMapResolutions[4] );
}







static void R_CreateStubImage( idImage* image )
{
	float	data[3][4][4];
	
	// generate the texture number
	glGenTextures( 1, &image->texnum );
	glBindTexture( GL_TEXTURE_RECTANGLE_NV, image->texnum );
	memset( data, 0, sizeof( data ) );
	glTexImage2D( GL_TEXTURE_RECTANGLE_NV, 0, GL_FLOAT_RGBA16_NV, 4, 3, 0, GL_RGBA, GL_FLOAT, &data );
}

/*
================
R_CreateJitterImage
================
*/
const static	int JITTER_SIZE = 128;
static void R_CreateJitterImage16( idImage* image )
{
	byte	data[JITTER_SIZE][JITTER_SIZE * 16][4];
	
	for( int i = 0 ; i < JITTER_SIZE ; i++ )
	{
		for( int s = 0 ; s < 16 ; s++ )
		{
			int sOfs = 64 * ( s & 3 );
			int tOfs = 64 * ( ( s >> 2 ) & 3 );
			
			for( int j = 0 ; j < JITTER_SIZE ; j++ )
			{
				data[i][s * JITTER_SIZE + j][0] = ( rand() & 63 ) | sOfs;
				data[i][s * JITTER_SIZE + j][1] = ( rand() & 63 ) | tOfs;
				data[i][s * JITTER_SIZE + j][2] = ( rand() & 0xFF );
				data[i][s * JITTER_SIZE + j][3] = 0;
			}
		}
	}
	
	image->GenerateImage( ( byte* )data, JITTER_SIZE * 16, JITTER_SIZE,
						  TF_NEAREST, false, TR_REPEAT, TD_HIGH_QUALITY );
}

static void R_CreateJitterImage4( idImage* image )
{
	byte	data[JITTER_SIZE][JITTER_SIZE * 4][4];
	
	for( int i = 0 ; i < JITTER_SIZE ; i++ )
	{
		for( int s = 0 ; s < 4 ; s++ )
		{
			int sOfs = 128 * ( s & 1 );
			int tOfs = 128 * ( ( s >> 1 ) & 1 );
			
			for( int j = 0 ; j < JITTER_SIZE ; j++ )
			{
				data[i][s * JITTER_SIZE + j][0] = ( rand() & 127 ) | sOfs;
				data[i][s * JITTER_SIZE + j][1] = ( rand() & 127 ) | tOfs;
				data[i][s * JITTER_SIZE + j][2] = ( rand() & 0xFF );
				data[i][s * JITTER_SIZE + j][3] = 0;
			}
		}
	}
	
	image->GenerateImage( ( byte* )data, JITTER_SIZE * 4, JITTER_SIZE,
						  TF_NEAREST, false, TR_REPEAT, TD_HIGH_QUALITY );
}

static void R_CreateJitterImage1( idImage* image )
{
	byte	data[JITTER_SIZE][JITTER_SIZE][4];
	
	for( int i = 0 ; i < JITTER_SIZE ; i++ )
	{
		for( int j = 0 ; j < JITTER_SIZE ; j++ )
		{
			data[i][j][0] = ( rand() & 0xFF );
			data[i][j][1] = ( rand() & 0xFF );
			data[i][j][2] = ( rand() & 0xFF );
			data[i][j][3] = 0;
		}
	}
	
	image->GenerateImage( ( byte* )data, JITTER_SIZE, JITTER_SIZE,
						  TF_NEAREST, false, TR_REPEAT, TD_HIGH_QUALITY );
}

static void R_CreateRandom256Image( idImage* image )
{
	byte	data[256][256][4];
	
	for( int i = 0 ; i < 256 ; i++ )
	{
		for( int j = 0 ; j < 256 ; j++ )
		{
			data[i][j][0] = ( rand() & 0xFF );
			data[i][j][1] = ( rand() & 0xFF );
			data[i][j][2] = ( rand() & 0xFF );
			data[i][j][3] = ( rand() & 0xFF );
		}
	}
	
	image->GenerateImage( ( byte* )data, 256, 256,
						  TF_NEAREST, false, TR_REPEAT, TD_HIGH_QUALITY );
}


/*
==================
R_Exp_Allocate
==================
*/
void R_Exp_Allocate()
{
	int				width, height;
	
	initialized = true;
	
	common->Printf( "\ncreating FBO targets...\n" );
	
	// generate the texture number
	shadowMapImage[0] = globalImages->ImageFromFunction( va( "_shadowBuffer%i_0", shadowMapResolutions[0] ), R_CreateShadowBufferImage_Res0 );
	shadowMapImage[1] = globalImages->ImageFromFunction( va( "_shadowBuffer%i_1", shadowMapResolutions[1] ), R_CreateShadowBufferImage_Res1 );
	shadowMapImage[2] = globalImages->ImageFromFunction( va( "_shadowBuffer%i_2", shadowMapResolutions[2] ), R_CreateShadowBufferImage_Res2 );
	shadowMapImage[3] = globalImages->ImageFromFunction( va( "_shadowBuffer%i_3", shadowMapResolutions[3] ), R_CreateShadowBufferImage_Res3 );
	shadowMapImage[4] = globalImages->ImageFromFunction( va( "_shadowBuffer%i_4", shadowMapResolutions[4] ), R_CreateShadowBufferImage_Res4 );
	
	shadowCubeImage[0] = globalImages->ImageFromFunction( va( "_shadowCube%i_0", shadowMapResolutions[0] ), R_CreateShadowCubeImage_Res0 );
	shadowCubeImage[1] = globalImages->ImageFromFunction( va( "_shadowCube%i_1", shadowMapResolutions[1] ), R_CreateShadowCubeImage_Res1 );
	shadowCubeImage[2] = globalImages->ImageFromFunction( va( "_shadowCube%i_2", shadowMapResolutions[2] ), R_CreateShadowCubeImage_Res2 );
	shadowCubeImage[3] = globalImages->ImageFromFunction( va( "_shadowCube%i_3", shadowMapResolutions[3] ), R_CreateShadowCubeImage_Res3 );
	shadowCubeImage[4] = globalImages->ImageFromFunction( va( "_shadowCube%i_4", shadowMapResolutions[4] ), R_CreateShadowCubeImage_Res4 );
	
	//-----------------------------------
	
	//lightBufferSize = maxViewBufferSize;
	
	//-----------------------------------
	
	// generate the jitter image
	jitterImage16 = globalImages->ImageFromFunction( "_jitter16", R_CreateJitterImage16 );
	jitterImage4 = globalImages->ImageFromFunction( "_jitter4", R_CreateJitterImage4 );
	jitterImage1 = globalImages->ImageFromFunction( "_jitter1", R_CreateJitterImage1 );
	
	random256Image = globalImages->ImageFromFunction( "_random256", R_CreateRandom256Image );
	
	// ----------------------------------
	hdrRenderImage = globalImages->ImageFromFunction( "_hdrRender", R_CreateHDRRenderImage );
	downScaleImage_quarter = globalImages->ImageFromFunction( "_downScale_quarter", R_CreateDownScaleImage_Quarter );
	downScaleImage_64x64 = globalImages->ImageFromFunction( "_downScale_64x64", R_CreateDownScaleImage_64x64 );
	
	
	
	common->Printf( "\ncreating FBOs...\n" );
	
	if( glConfig.textureNonPowerOfTwoAvailable )
	{
		width = glConfig.vidWidth;
		height = glConfig.vidHeight;
	}
	else
	{
		width = MakePowerOfTwo( glConfig.vidWidth );
		height = MakePowerOfTwo( glConfig.vidHeight );
	}
	
#if 0
	//if(r_useDeferredShading.GetBool())
	{
		globalFramebuffers.gbuffer = new Framebuffer( "_geometryBuffer", width, height );
		globalFramebuffers.gbuffer->Bind();
		
		globalFramebuffers.gbuffer->AddColorBuffer( GL_RGBA16F_ARB, 0 );
		globalFramebuffers.gbuffer->AttachImage2D( GL_TEXTURE_2D, globalImages->hdrRenderImage, 0 );
		
		globalFramebuffers.gbuffer->AddDepthBuffer( GL_DEPTH_COMPONENT24 );
		globalFramebuffers.gbuffer->AttachImageDepth( globalImages->currentDepthImage );
		
		globalFramebuffers.gbuffer->Check();
	}
#endif
	
	// create HDR rendering context
	if( r_useHighDynamicRange.GetBool() )
	{
		globalFramebuffers.hdrRender = new Framebuffer( "_hdrRender", width, height );
		globalFramebuffers.hdrRender->Bind();
		
		globalFramebuffers.hdrRender->AddDepthBuffer( GL_DEPTH_COMPONENT24 );
		globalFramebuffers.hdrRender->AttachImageDepth( globalImages->currentDepthImage );
		
		globalFramebuffers.hdrRender->AddColorBuffer( GL_RGBA8, 0 );
		globalFramebuffers.hdrRender->AttachImage2D( GL_TEXTURE_2D, hdrRenderImage, 0 );
		
		globalFramebuffers.hdrRender->Check();
	}
	
	// create downscale FBOs
	if( glConfig.textureNonPowerOfTwoAvailable )
	{
		width = glConfig.vidWidth * 0.25f;
		height = glConfig.vidHeight * 0.25f;
	}
	else
	{
		width = MakePowerOfTwo( glConfig.vidWidth * 0.25f );
		height = MakePowerOfTwo( glConfig.vidHeight * 0.25f );
	}
	downScaleFBO_quarter = new Framebuffer( "_downScale_quarter", width, height );
	downScaleFBO_quarter->Bind();
#if 1
	if( r_useHighDynamicRange.GetBool() )
	{
		downScaleFBO_quarter->AddColorBuffer( GL_RGBA16F, 0 );
	}
	else
#endif
	{
		downScaleFBO_quarter->AddColorBuffer( GL_RGBA, 0 );
	}
	downScaleFBO_quarter->AttachImage2D( GL_TEXTURE_2D, downScaleImage_quarter, 0 );
	downScaleFBO_quarter->Check();
	
	
	downScaleFBO_64x64 = new Framebuffer( "_downScale_64x64", 64, 64 );
	downScaleFBO_64x64->Bind();
#if 1
	if( r_useHighDynamicRange.GetBool() )
	{
		downScaleFBO_64x64->AddColorBuffer( GL_RGBA16F, 0 );
	}
	else
#endif
	{
		downScaleFBO_64x64->AddColorBuffer( GL_RGBA, 0 );
	}
	downScaleFBO_64x64->AttachImage2D( GL_TEXTURE_2D, downScaleImage_64x64, 0 );
	downScaleFBO_64x64->Check();
	
	// create shadow maps
	for( int i = 0; i < MAX_SHADOWMAPS; i++ )
	{
		width = height = shadowMapResolutions[i];
		
		shadowMapFBO[i] = new Framebuffer( va( "_shadowMap%d", i ), width, height );
		shadowMapFBO[i]->Bind();
		
		
		//if((glConfig.driverType == GLDRV_OPENGL3) || (glConfig.hardwareType == GLHW_NV_DX10 || glConfig.hardwareType == GLHW_ATI_DX10))
		/*
		{
			if(r_sb_mode.GetInteger() == SHADOWING_ESM32)
			{
				shadowMapFBO[i]->AddColorBuffer(GL_ALPHA32F_ARB, 0);
			}
			else if(r_sb_mode.GetInteger() == SHADOWING_VSM32)
			{
				shadowMapFBO[i]->AddColorBuffer(GL_LUMINANCE_ALPHA32F_ARB, 0);
			}
			else if(r_sb_mode.GetInteger() == SHADOWING_EVSM32)
			{
				if(r_evsm_postProcess.GetBool())
				{
					shadowMapFBO[i]->AddColorBuffer(GL_ALPHA32F_ARB, 0);
				}
				else
				{
					shadowMapFBO[i]->AddColorBuffer(GL_RGBA32F_ARB, 0);
				}
			}
			else
			{
				shadowMapFBO[i]->AddColorBuffer(GL_RGBA16F_ARB, 0);
			}
		}
		else
		*/
		{
			if( r_sb_mode.GetInteger() == SHADOWING_ESM16 )
			{
				shadowMapFBO[i]->AddColorBuffer( GL_R16F, 0 );
			}
			else if( r_sb_mode.GetInteger() == SHADOWING_ESM32 )
			{
				shadowMapFBO[i]->AddColorBuffer( GL_R32F, 0 );
			}
			else if( r_sb_mode.GetInteger() == SHADOWING_VSM16 )
			{
				shadowMapFBO[i]->AddColorBuffer( GL_RG16F, 0 );
			}
			else if( r_sb_mode.GetInteger() == SHADOWING_VSM32 )
			{
				shadowMapFBO[i]->AddColorBuffer( GL_RG32F, 0 );
			}
			else
			{
				shadowMapFBO[i]->AddColorBuffer( GL_RGBA16F_ARB, 0 );
			}
		}
		
		shadowMapFBO[i]->AddDepthBuffer( GL_DEPTH_COMPONENT32 );
		shadowMapFBO[i]->Check();
	}
	
	Framebuffer::BindNull();
	
	
	GL_CheckErrors();
}

//===========================================================================================

static const int CULL_RECEIVER = 1;	// still draw occluder, but it is out of the view
static const int CULL_OCCLUDER_AND_RECEIVER = 2;	// the surface doesn't effect the view at all

/*
==================
RB_EXP_CullInteractions

Sets surfaceInteraction_t->cullBits
==================
*/
void RB_EXP_CullInteractions( viewLight_t* vLight, idPlane frustumPlanes[6] )
{

	if( vLight->lightDef == NULL )
	{
		return;
	}
	
	for( idInteraction* inter = vLight->lightDef->firstInteraction ; inter != NULL ; inter = inter->lightNext )
	{
		const idRenderEntityLocal* entityDef = inter->entityDef;
		if( !entityDef )
		{
			continue;
		}
		if( inter->numSurfaces < 1 )
		{
			continue;
		}
		
		int	culled = 0;
		
		if( r_sb_useCulling.GetBool() )
		{
			// transform light frustum into object space, positive side points outside the light
			idPlane	localPlanes[6];
			int		plane;
			for( plane = 0 ; plane < 6 ; plane++ )
			{
				R_GlobalPlaneToLocal( entityDef->modelMatrix, frustumPlanes[plane], localPlanes[plane] );
			}
			
			// cull the entire entity bounding box
			// has referenceBounds been tightened to the actual model bounds?
			idVec3	corners[8];
			for( int i = 0 ; i < 8 ; i++ )
			{
				corners[i][0] = entityDef->referenceBounds[i & 1][0];
				corners[i][1] = entityDef->referenceBounds[( i >> 1 ) & 1][1];
				corners[i][2] = entityDef->referenceBounds[( i >> 2 ) & 1][2];
			}
			
			for( plane = 0 ; plane < 6 ; plane++ )
			{
				int		j;
				for( j = 0 ; j < 8 ; j++ )
				{
					// if a corner is on the negative side (inside) of the frustum, the surface is not culled
					// by this plane
					if( corners[j] * localPlanes[plane].ToVec4().ToVec3() + localPlanes[plane][3] < 0 )
					{
						break;
					}
				}
				if( j == 8 )
				{
					break;			// all points outside the light
				}
			}
			if( plane < 6 )
			{
				culled = CULL_OCCLUDER_AND_RECEIVER;
			}
		}
		
		for( int i = 0 ; i < inter->numSurfaces ; i++ )
		{
			surfaceInteraction_t*	surfInt = &inter->surfaces[i];
			
			if( !surfInt->ambientTris )
			{
				continue;
			}
			surfInt->expCulled = culled;
		}
		
	}
}

/*
==================
RB_EXP_RenderOccluders
==================
*/
void RB_EXP_RenderOccluders( viewLight_t* vLight )
{
	if( vLight->lightDef == NULL )
	{
		return;
	}
	
	for( idInteraction* inter = vLight->lightDef->firstInteraction ; inter != NULL ; inter = inter->lightNext )
	{
		const idRenderEntityLocal* entityDef = inter->entityDef;
		if( !entityDef )
		{
			continue;
		}
		if( inter->numSurfaces < 1 )
		{
			continue;
		}
		
		if( entityDef->parms.suppressShadowInViewID && entityDef->parms.suppressShadowInViewID == backEnd.viewDef->renderView.viewID )
		{
			continue;
		}
		
		if( entityDef->parms.suppressShadowInLightID && entityDef->parms.suppressShadowInLightID == vLight->lightDef->parms.lightId )
		{
			continue;
		}
		
		// no need to check for current on this, because each interaction is always
		// a different space
		matrix_t	modelToLightMatrix;
		myGlMultMatrix( inter->entityDef->modelMatrix, lightMatrix, modelToLightMatrix );
		glLoadMatrixf( modelToLightMatrix );
		
#if 0
		if( !vLight->lightDef->parms.pointLight )
		{
			matrix_t	modelToLightMatrix;
			//myGlMultMatrix( inter->entityDef->modelMatrix, lightMatrix, modelToLightMatrix );
			glLoadMatrixf( lightMatrix );
			
			idPlane lightProject[4];
			for( int i = 0 ; i < 4 ; i++ )
			{
				R_GlobalPlaneToLocal( inter->entityDef->modelMatrix, vLight->lightDef->lightProject[i], lightProject[i] );
			}
			
			idMat4 lProj( lightProject[0].ToVec4(), lightProject[1].ToVec4(), lightProject[3].ToVec4(), lightProject[2].ToVec4() );
			lProj.TransposeSelf();
			
			glMatrixMode( GL_PROJECTION );
			glLoadMatrixf( lProj.ToFloatPtr() );
			glMatrixMode( GL_MODELVIEW );
		}
#endif
		
		gl_shadowMapShader->SetUniform_ModelMatrix( make_idMat4Transposed( inter->entityDef->modelMatrix ) );
		
		// draw each surface
		for( int i = 0 ; i < inter->numSurfaces ; i++ )
		{
			surfaceInteraction_t*	surfInt = &inter->surfaces[i];
			
			if( !surfInt->ambientTris )
			{
				continue;
			}
			if( surfInt->shader && !surfInt->shader->SurfaceCastsShadow() )
			{
				continue;
			}
			
			// cull it
			if( vLight->lightDef->parms.pointLight )
			{
				if( surfInt->expCulled == CULL_OCCLUDER_AND_RECEIVER )
				{
					continue;
				}
			}
			
			// render it
			const srfTriangles_t* tri = surfInt->ambientTris;
			if( !tri->ambientCache )
			{
				R_CreateAmbientCache( const_cast<srfTriangles_t*>( tri ), false );
			}
			
			idDrawVert* ac = ( idDrawVert* )vertexCache.Position( tri->ambientCache );
			glVertexPointer( 3, GL_FLOAT, sizeof( idDrawVert ), ac->xyz.ToFloatPtr() );
			glVertexAttribPointerARB( VA_INDEX_TEXCOORD0, 2, GL_FLOAT, false, sizeof( idDrawVert ), ac->st.ToFloatPtr() );
			//glTexCoordPointer( 2, GL_FLOAT, sizeof( idDrawVert ), ac->st.ToFloatPtr() );
			
			if( surfInt->shader )
			{
				surfInt->shader->GetEditorImage()->Bind();
			}
			
			RB_DrawElementsWithCounters( tri );
		}
	}
}


/*
==================
RB_EXP_RenderShadowBuffer
==================
*/
void    RB_EXP_RenderShadowBuffer( viewLight_t*	vLight, int side )
{
	float	xmin, xmax, ymin, ymax;
	float	width, height;
	float	zNear;
	
	float	fov = r_sb_frustomFOV.GetFloat();
	
	if( r_logFile.GetBool() )
	{
		RB_LogComment( "--- RB_EXP_RenderShadowBuffer( side = %i ) ---\n", side );
	}
	
	GL_CheckErrors();
	
	if( !vLight->lightDef->parms.pointLight )
	{
#if 0
		idPlane lightProject[4];
		for( int i = 0 ; i < 4 ; i++ )
		{
			R_GlobalPlaneToLocal( vLight->lightDef->modelMatrix, vLight->lightProject[i], lightProject[i] );
		}
		
		// set the old style light projection where Z and W are flipped and
		// for projected lights lightProject[3] is divided by ( zNear + zFar )
		
		// S or X
		lightProjectionMatrix[ 0] = lightProject[0][0];
		lightProjectionMatrix[ 1] = lightProject[0][1];
		lightProjectionMatrix[ 2] = lightProject[0][2];
		lightProjectionMatrix[ 3] = lightProject[0][3];
		
		// T or Y
		lightProjectionMatrix[ 4] = lightProject[1][0];
		lightProjectionMatrix[ 5] = lightProject[1][1];
		lightProjectionMatrix[ 6] = lightProject[1][2];
		lightProjectionMatrix[ 7] = lightProject[1][3];
		
		// R or Z
		lightProjectionMatrix[ 8] = lightProject[3][0];
		lightProjectionMatrix[ 9] = lightProject[3][1];
		lightProjectionMatrix[10] = lightProject[3][2];
		lightProjectionMatrix[11] = lightProject[3][3];
		
		// Q or W
		lightProjectionMatrix[12] = lightProject[2][0];
		lightProjectionMatrix[13] = lightProject[2][1];
		lightProjectionMatrix[14] = lightProject[2][2];
		lightProjectionMatrix[15] = lightProject[2][3];
		
#if 1
		matrix_t tmp;
		MatrixTranspose( lightProjectionMatrix, tmp );
		MatrixCopy( tmp, lightProjectionMatrix );
#endif
		
		// RB: FIXME mache vLight->lightProject zur lightProjectionMatrix
#else
		MatrixCopy( vLight->lightDef->projectionMatrix, lightProjectionMatrix );
#endif
	}
	else
	{
		// set up 90 degree projection matrix
		
		zNear	= 4;
		
		ymax = zNear * tan( fov * idMath::PI / 360.0f );
		ymin = -ymax;
		
		xmax = zNear * tan( fov * idMath::PI / 360.0f );
		xmin = -xmax;
		
		width = xmax - xmin;
		height = ymax - ymin;
		
		lightProjectionMatrix[0] = 2 * zNear / width;
		lightProjectionMatrix[4] = 0;
		lightProjectionMatrix[8] = 0;
		lightProjectionMatrix[12] = 0;
		
		lightProjectionMatrix[1] = 0;
		lightProjectionMatrix[5] = 2 * zNear / height;
		lightProjectionMatrix[9] = 0;
		lightProjectionMatrix[13] = 0;
		
		// this is the far-plane-at-infinity formulation, and
		// crunches the Z range slightly so w=0 vertexes do not
		// rasterize right at the wraparound point
		lightProjectionMatrix[2] = 0;
		lightProjectionMatrix[6] = 0;
		lightProjectionMatrix[10] = -0.999f;
		lightProjectionMatrix[14] = -2.0f * zNear;
		
		lightProjectionMatrix[3] = 0;
		lightProjectionMatrix[7] = 0;
		lightProjectionMatrix[11] = -1;
		lightProjectionMatrix[15] = 0;
	}
	
	if( r_sb_usePbuffer.GetBool() )
	{
		// set the current openGL drawable to the shadow buffer
		if( vLight->lightDef->parms.pointLight )
		{
			shadowMapFBO[vLight->shadowLOD]->Bind();
			shadowMapFBO[vLight->shadowLOD]->AttachImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + side, shadowCubeImage[vLight->shadowLOD], 0 );
		}
		else
		{
			shadowMapFBO[vLight->shadowLOD]->Bind();
			shadowMapFBO[vLight->shadowLOD]->AttachImage2D( GL_TEXTURE_2D, shadowMapImage[vLight->shadowLOD], 0 );
		}
		
		if( !r_ignoreGLErrors.GetBool() )
		{
			shadowMapFBO[vLight->shadowLOD]->Check();
		}
	}
	
	GL_CheckErrors();
	
	glMatrixMode( GL_PROJECTION );
	glLoadMatrixf( lightProjectionMatrix );
	glMatrixMode( GL_MODELVIEW );
	
	// TODO vLight->shadowLOD
	glViewport( 0, 0, shadowMapResolutions[vLight->shadowLOD], shadowMapResolutions[vLight->shadowLOD] );
	
	if( r_useScissor.GetBool() )
	{
		glScissor( 0, 0, shadowMapResolutions[vLight->shadowLOD], shadowMapResolutions[vLight->shadowLOD] );
	}
	
	//glDisable( GL_STENCIL_TEST );
	glStencilFunc( GL_ALWAYS, 0, 255 );
	
	glClearColor( 1, 1, 1, 1 );
	GL_State( GLS_DEPTHFUNC_LESS | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO );	// make sure depth mask is off before clear
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
// draw all the occluders
	glColor3f( 1, 1, 1 );
	GL_SelectTexture( 0 );
//glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	GL_CheckErrors();
	
	backEnd.currentSpace = NULL;
	
	static const float	s_flipMatrix[16] =
	{
		// convert from our coordinate system (looking down X)
		// to OpenGL's coordinate system (looking down -Z)
		0, 0, -1, 0,
		-1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 0, 1
	};
	
	matrix_t	viewMatrix;
	
	idVec3	vec;
	idVec3	origin = vLight->lightDef->globalLightOrigin;
	
	if( side == -1 )
	{
#if 1
		// RB: worldToLight is already in the projection matrix
		//MatrixIdentity( viewMatrix );
		//MatrixAffineInverse( vLight->lightDef->modelMatrix, viewMatrix );
		MatrixFullInverse( vLight->lightDef->modelMatrix, viewMatrix );
#else
		// projected light
		vec = vLight->lightDef->parms.target;
		vec.Normalize();
		viewMatrix[0] = vec[0];
		viewMatrix[4] = vec[1];
		viewMatrix[8] = vec[2];
		
		vec = vLight->lightDef->parms.right;
		vec.Normalize();
		viewMatrix[1] = -vec[0];
		viewMatrix[5] = -vec[1];
		viewMatrix[9] = -vec[2];
		
		vec = vLight->lightDef->parms.up;
		vec.Normalize();
		viewMatrix[2] = vec[0];
		viewMatrix[6] = vec[1];
		viewMatrix[10] = vec[2];
#endif
	}
	else
	{
		// side of a point light
		memset( viewMatrix, 0, sizeof( viewMatrix ) );
		switch( side )
		{
			case 0:
				viewMatrix[0] = 1;
				viewMatrix[9] = 1;
				viewMatrix[6] = -1;
				break;
			case 1:
				viewMatrix[0] = -1;
				viewMatrix[9] = -1;
				viewMatrix[6] = -1;
				break;
			case 2:
				viewMatrix[4] = 1;
				viewMatrix[1] = -1;
				viewMatrix[10] = 1;
				break;
			case 3:
				viewMatrix[4] = -1;
				viewMatrix[1] = -1;
				viewMatrix[10] = -1;
				break;
			case 4:
				viewMatrix[8] = 1;
				viewMatrix[1] = -1;
				viewMatrix[6] = -1;
				break;
			case 5:
				viewMatrix[8] = -1;
				viewMatrix[1] = 1;
				viewMatrix[6] = -1;
				break;
		}
	}
	
	viewMatrix[12] = -origin[0] * viewMatrix[0] + -origin[1] * viewMatrix[4] + -origin[2] * viewMatrix[8];
	viewMatrix[13] = -origin[0] * viewMatrix[1] + -origin[1] * viewMatrix[5] + -origin[2] * viewMatrix[9];
	viewMatrix[14] = -origin[0] * viewMatrix[2] + -origin[1] * viewMatrix[6] + -origin[2] * viewMatrix[10];
	
	viewMatrix[3] = 0;
	viewMatrix[7] = 0;
	viewMatrix[11] = 0;
	viewMatrix[15] = 1;
	
	memcpy( unflippedLightMatrix, viewMatrix, sizeof( unflippedLightMatrix ) );
	if( vLight->lightDef->parms.pointLight )
	{
		myGlMultMatrix( viewMatrix, s_flipMatrix, lightMatrix );
	}
	else
	{
		// projected light
		MatrixCopy( viewMatrix, lightMatrix );
		//myGlMultMatrix( viewMatrix, s_flipMatrix, lightMatrix );
	}
	
	
	if( vLight->lightDef->parms.pointLight )
	{
		// create frustum planes
		idPlane	globalFrustum[6];
		
		// near clip
		globalFrustum[0][0] = -viewMatrix[0];
		globalFrustum[0][1] = -viewMatrix[4];
		globalFrustum[0][2] = -viewMatrix[8];
		globalFrustum[0][3] = -( origin[0] * globalFrustum[0][0] + origin[1] * globalFrustum[0][1] + origin[2] * globalFrustum[0][2] );
		
		// far clip
		globalFrustum[1][0] = viewMatrix[0];
		globalFrustum[1][1] = viewMatrix[4];
		globalFrustum[1][2] = viewMatrix[8];
		globalFrustum[1][3] = -globalFrustum[0][3] - viewLightAxialSize;
		
		// side clips
		globalFrustum[2][0] = -viewMatrix[0] + viewMatrix[1];
		globalFrustum[2][1] = -viewMatrix[4] + viewMatrix[5];
		globalFrustum[2][2] = -viewMatrix[8] + viewMatrix[9];
		
		globalFrustum[3][0] = -viewMatrix[0] - viewMatrix[1];
		globalFrustum[3][1] = -viewMatrix[4] - viewMatrix[5];
		globalFrustum[3][2] = -viewMatrix[8] - viewMatrix[9];
		
		globalFrustum[4][0] = -viewMatrix[0] + viewMatrix[2];
		globalFrustum[4][1] = -viewMatrix[4] + viewMatrix[6];
		globalFrustum[4][2] = -viewMatrix[8] + viewMatrix[10];
		
		globalFrustum[5][0] = -viewMatrix[0] - viewMatrix[2];
		globalFrustum[5][1] = -viewMatrix[4] - viewMatrix[6];
		globalFrustum[5][2] = -viewMatrix[8] - viewMatrix[10];
		
		// is this nromalization necessary?
		for( int i = 0 ; i < 6 ; i++ )
		{
			globalFrustum[i].ToVec4().ToVec3().Normalize();
		}
		
		for( int i = 2 ; i < 6 ; i++ )
		{
			globalFrustum[i][3] = - ( origin * globalFrustum[i].ToVec4().ToVec3() );
		}
		
		// FIXME
		RB_EXP_CullInteractions( vLight, globalFrustum );
	}
	
	// FIXME: we want to skip the sampling as well as the generation when not casting shadows
	if( !r_sb_noShadows.GetBool() && r_shadows.GetBool() && vLight->lightShader->LightCastsShadows() )
	{
	
		gl_shadowMapShader->BindProgram();
		gl_shadowMapShader->SetUniform_GlobalLightOrigin( origin );
		//gl_shadowMapShader->SetUniform_LightRadius(vLight->lightDef->frustumTris->bounds.GetRadius());
		gl_shadowMapShader->SetUniform_LightRadius( viewLightAxialSize );
		
		//
		// set polygon offset for the rendering
		//
		switch( r_sb_occluderFacing.GetInteger() )
		{
			case 0:		// front sides
				//glPolygonOffset( r_sb_polyOfsFactor.GetFloat(), r_sb_polyOfsUnits.GetFloat() );
				//glEnable( GL_POLYGON_OFFSET_FILL );
				RB_EXP_RenderOccluders( vLight );
				//glDisable( GL_POLYGON_OFFSET_FILL );
				break;
			case 1:		// back sides
				glPolygonOffset( -r_sb_polyOfsFactor.GetFloat(), -r_sb_polyOfsUnits.GetFloat() );
				glEnable( GL_POLYGON_OFFSET_FILL );
				GL_Cull( CT_BACK_SIDED );
				RB_EXP_RenderOccluders( vLight );
				GL_Cull( CT_FRONT_SIDED );
				glDisable( GL_POLYGON_OFFSET_FILL );
				break;
				/*
				case 2:		// both sides
					GL_Cull( CT_BACK_SIDED );
					RB_EXP_RenderOccluders( vLight );
					GL_Cull( CT_FRONT_SIDED );
					shadowImage[2]->Bind();
					glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, lightBufferSize, lightBufferSize );
				
					RB_EXP_RenderOccluders( vLight );
					shadowImage[1]->Bind();
					glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, lightBufferSize, lightBufferSize );
				
					// fragment program to combine the two depth images
					glBindProgramARB( GL_VERTEX_PROGRAM_ARB, depthMidpointVertexProgram );
					glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, depthMidpointFragmentProgram );
					glEnable(GL_VERTEX_PROGRAM_ARB);
					glEnable(GL_FRAGMENT_PROGRAM_ARB);
				
					GL_SelectTextureNoClient( 1 );
					shadowImage[1]->Bind();
					glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE );
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
				
					GL_SelectTextureNoClient( 0 );
					shadowImage[2]->Bind();
					glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE );
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
				
					// draw a full screen quad
					glMatrixMode( GL_PROJECTION );
					glLoadIdentity();
					glOrtho( 0, 1, 0, 1, -1, 1 );
					glMatrixMode( GL_MODELVIEW );
					glLoadIdentity();
				
					GL_State( GLS_DEPTHFUNC_ALWAYS );
				
					glBegin( GL_TRIANGLE_FAN );
					glTexCoord2f( 0, 0 );
					glVertex2f( 0, 0 );
					glTexCoord2f( 0, lightBufferSizeFraction );
					glVertex2f( 0, 1 );
					glTexCoord2f( lightBufferSizeFraction, lightBufferSizeFraction );
					glVertex2f( 1, 1 );
					glTexCoord2f( lightBufferSizeFraction, 0 );
					glVertex2f( 1, 0 );
					glEnd();
				
					glDisable( GL_VERTEX_PROGRAM_ARB );
					glDisable( GL_FRAGMENT_PROGRAM_ARB );
				
					break;
					*/
		}
	}
	
	GL_CheckErrors();
	
	// copy to the texture
	//shadowImage[0]->Bind();
	//glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, lightBufferSize, lightBufferSize );
	
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	
	
	// reset the normal view matrix
	
	glMatrixMode( GL_PROJECTION );
	glLoadMatrixf( backEnd.viewDef->projectionMatrix );
	glMatrixMode( GL_MODELVIEW );
	
	// the current modelView matrix is not valid
	backEnd.currentSpace = NULL;
	
	GL_CheckErrors();
}

void RB_EXP_CoverScreen()
{
	// draw a full screen quad
	glPushMatrix();
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0, 1, 0, 1, -1, 1 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	
	glBegin( GL_TRIANGLE_FAN );
	glVertex2f( 0, 0 );
	glVertex2f( 0, 1 );
	glVertex2f( 1, 1 );
	glVertex2f( 1, 0 );
	glEnd();
	
	glPopMatrix();
}

/*
==================
RB_EXP_DrawInteraction
==================
*/
static void	RB_EXP_DrawInteraction( const drawInteraction_t* din )
{

	bool shadowCompare = ( !r_sb_noShadows.GetBool() && r_shadows.GetBool() &&
						   backEnd.vLight->lightShader->LightCastsShadows() &&
						   !backEnd.vLight->lightDef->parms.noShadows && backEnd.vLight->shadowLOD >= 0 );
						   
	// choose and bind the vertex program
	// TODO gl_forwardLightingShader->SetAmbientLighting(backEnd.vLight->lightShader->IsAmbientLight());
	gl_forwardLightingShader->SetMacro_LIGHT_PROJ( !backEnd.vLight->lightDef->parms.pointLight );
	gl_forwardLightingShader->SetShadowing( shadowCompare );
	gl_forwardLightingShader->SetNormalMapping( !r_skipBump.GetBool() || backEnd.vLight->lightShader->IsAmbientLight() );
	gl_forwardLightingShader->BindProgram();
	
	// load all the vertex program parameters
	gl_forwardLightingShader->SetUniform_ModelMatrix( make_idMat4Transposed( din->surf->space->modelMatrix ) );
	
	gl_forwardLightingShader->SetUniform_LocalViewOrigin( din->localViewOrigin.ToVec3() );
	
	gl_forwardLightingShader->SetUniform_LocalLightOrigin( din->localLightOrigin.ToVec3() );
	gl_forwardLightingShader->SetUniform_GlobalLightOrigin( din->globalLightOrigin.ToVec3() );
	
	//gl_forwardLightingShader->SetUniform_LightRadius(backEnd.vLight->lightDef->frustumTris->bounds.GetRadius());
	gl_forwardLightingShader->SetUniform_LightRadius( viewLightAxialSize );
	
	gl_forwardLightingShader->SetUniform_LightProjectS( din->lightProjection[0] );
	gl_forwardLightingShader->SetUniform_LightProjectT( din->lightProjection[1] );
	gl_forwardLightingShader->SetUniform_LightProjectQ( din->lightProjection[2] );
	gl_forwardLightingShader->SetUniform_LightFalloffS( din->lightProjection[3] );
	
	gl_forwardLightingShader->SetUniform_DiffuseMatrixS( din->diffuseMatrix[0] );
	gl_forwardLightingShader->SetUniform_DiffuseMatrixT( din->diffuseMatrix[1] );
	
	gl_forwardLightingShader->SetUniform_BumpMatrixS( din->bumpMatrix[0] );
	gl_forwardLightingShader->SetUniform_BumpMatrixT( din->bumpMatrix[1] );
	
	gl_forwardLightingShader->SetUniform_SpecularMatrixS( din->specularMatrix[0] );
	gl_forwardLightingShader->SetUniform_SpecularMatrixT( din->specularMatrix[1] );
	
	
	
	
	if( shadowCompare )
	{
		float shadowTexelSize = 1.0f / shadowMapResolutions[backEnd.vLight->shadowLOD];
		gl_forwardLightingShader->SetUniform_ShadowTexelSize( shadowTexelSize );
		gl_forwardLightingShader->SetUniform_ShadowBlur( r_sb_samples.GetInteger() );
		
		// calculate depth projection for shadow buffer
		matrix_t	modelToLightMatrix;
		matrix_t	lightMVP;
		
		
#if 1
		if( !backEnd.vLight->lightDef->parms.pointLight )
		{
			/*
			idPlane lightProject[4];
			for( int i = 0 ; i < 4 ; i++ )
			{
				R_GlobalPlaneToLocal( din->surf->space->modelMatrix, backEnd.vLight->lightDef->lightProject[i], lightProject[i] );
			}
			
			idMat4 lProj( lightProject[0].ToVec4(), lightProject[1].ToVec4(), lightProject[3].ToVec4(), lightProject[2].ToVec4() );
			lProj.TransposeSelf();
			*/
			
			//myGlMultMatrix( din->surf->space->modelMatrix, lightMatrix, modelToLightMatrix );
			//myGlMultMatrix( modelToLightMatrix, lProj.ToFloatPtr(), lightMVP );
			
			idMat4 lProj( din->shadowProjection[0], din->shadowProjection[1], din->shadowProjection[3], din->shadowProjection[2] );
			lProj.TransposeSelf();
			
			MatrixCopy( lProj.ToFloatPtr(), lightMVP );
		}
		else
#endif
		{
			myGlMultMatrix( din->surf->space->modelMatrix, lightMatrix, modelToLightMatrix );
			myGlMultMatrix( modelToLightMatrix, lightProjectionMatrix, lightMVP );
		}
		
		idMat4 shadowMat = make_idMat4Transposed( lightMVP );
		
		gl_forwardLightingShader->SetUniform_ShadowMatrix( shadowMat );
	}
	
	
	static const idVec4 zero( 0, 0, 0, 0 );
	static const idVec4 one( 1, 1, 1, 1 );
	static const idVec4 negOne( -1, -1, -1, -1 );
	
	switch( din->vertexColor )
	{
		case SVC_IGNORE:
			gl_forwardLightingShader->SetUniform_ColorModulate( zero );
			gl_forwardLightingShader->SetUniform_Color( one );
			break;
		case SVC_MODULATE:
			gl_forwardLightingShader->SetUniform_ColorModulate( one );
			gl_forwardLightingShader->SetUniform_Color( zero );
			break;
		case SVC_INVERSE_MODULATE:
			gl_forwardLightingShader->SetUniform_ColorModulate( negOne );
			gl_forwardLightingShader->SetUniform_Color( one );
			break;
	}
	
	// set the constant colors
	gl_forwardLightingShader->SetUniform_DiffuseColor( din->diffuseColor );
	gl_forwardLightingShader->SetUniform_SpecularColor( din->specularColor );
	
	//-----------------------------------------------------
	// screen power of two correction factor
	
	idVec4	parm;
	parm[0] = 1.0 / ( JITTER_SIZE * r_sb_samples.GetInteger() ) ;
	parm[1] = 1.0 / JITTER_SIZE;
	parm[2] = 0;
	parm[3] = 1;
	gl_forwardLightingShader->SetUniform_PositionToJitterTexScale( parm );
	
	// jitter tex scale
	parm[0] =
		parm[1] = r_sb_jitterScale.GetFloat() * lightBufferSizeFraction[0];
	parm[2] = -r_sb_biasScale.GetFloat();
	parm[3] = 0;
	gl_forwardLightingShader->SetUniform_JitterTexScale( parm );
	
	// jitter tex offset
	if( r_sb_randomize.GetBool() )
	{
		parm[0] = ( rand() & 255 ) / 255.0;
		parm[1] = ( rand() & 255 ) / 255.0;
	}
	else
	{
		parm[0] = parm[1] = 0;
	}
	parm[2] = 0;
	parm[3] = 0;
	gl_forwardLightingShader->SetUniform_JitterTexOffset( parm );
	//-----------------------------------------------------
	
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
RB_EXP_CreateDrawInteractions
=============
*/
void RB_EXP_CreateDrawInteractions( const drawSurf_t* surf )
{
	if( !surf )
	{
		return;
	}
	
	RB_LogComment( "---------- RB_EXP_CreateDrawInteractions ----------\n" );
	
	// perform setup here that will be constant for all interactions
	GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHMASK | backEnd.depthFunc );
//GL_State( GLS_DEPTHMASK | GLS_DEPTHFUNC_ALWAYS );//!@#

	// bind the vertex program
	
	/*
	glBindProgramARB( GL_VERTEX_PROGRAM_ARB, shadowVertexProgram );
	if ( r_sb_samples.GetInteger() == 16 ) {
		glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, shadowFragmentProgram16 );
	} else if ( r_sb_samples.GetInteger() == 4 ) {
		glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, shadowFragmentProgram4 );
	} else if ( r_sb_samples.GetInteger() == 1 ) {
		glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, shadowFragmentProgram1 );
	} else {
		glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, shadowFragmentProgram0 );
	}
	*/
	
	// enable the vertex arrays
	glEnableVertexAttribArrayARB( VA_INDEX_TEXCOORD0 );
	glEnableVertexAttribArrayARB( VA_INDEX_TANGENT );
	glEnableVertexAttribArrayARB( VA_INDEX_BITANGENT );
	glEnableVertexAttribArrayARB( VA_INDEX_NORMAL );
	glEnableClientState( GL_COLOR_ARRAY );
	
	// texture 0 is the normalization cube map for the vector towards the light
	GL_SelectTextureNoClient( 0 );
	if( backEnd.vLight->lightShader->IsAmbientLight() )
	{
		globalImages->ambientNormalMap->Bind();
	}
	else
	{
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
	
	// texture 6 is the random samples image
	GL_SelectTextureNoClient( 6 );
	if( r_sb_samples.GetInteger() == 16 )
	{
		jitterImage16->Bind();
	}
	else if( r_sb_samples.GetInteger() == 4 )
	{
		jitterImage4->Bind();
	}
	else
	{
		jitterImage1->Bind();
	}
	
	// bind the program
	
	for( ; surf ; surf = surf->nextOnLight )
	{
		// perform setup here that will not change over multiple interaction passes
		if( backEnd.vLight->lightShader->IsAmbientLight() )
		{
			float	parm[4];
			
			GL_SelectTextureNoClient( 0 );
			const shaderStage_t* stage = backEnd.vLight->lightShader->GetStage( 0 );
			if( stage->newStage )
			{
				stage->newStage->fragmentProgramImages[7]->BindFragment();
			}
		}
		
		// set the vertex pointers
		idDrawVert*	ac = ( idDrawVert* )vertexCache.Position( surf->geo->ambientCache );
		glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( idDrawVert ), ac->color );
		glVertexAttribPointerARB( VA_INDEX_NORMAL, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->normal.ToFloatPtr() );
		glVertexAttribPointerARB( VA_INDEX_BITANGENT, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->tangents[1].ToFloatPtr() );
		glVertexAttribPointerARB( VA_INDEX_TANGENT, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->tangents[0].ToFloatPtr() );
		glVertexAttribPointerARB( VA_INDEX_TEXCOORD0, 2, GL_FLOAT, false, sizeof( idDrawVert ), ac->st.ToFloatPtr() );
		glVertexPointer( 3, GL_FLOAT, sizeof( idDrawVert ), ac->xyz.ToFloatPtr() );
		
		RB_CreateSingleDrawInteractions( surf, RB_EXP_DrawInteraction );
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
	
	glDisable( GL_VERTEX_PROGRAM_ARB );
	glDisable( GL_FRAGMENT_PROGRAM_ARB );
}

static void RB_EXP_DrawLightDeferred( viewLight_t* vLight )
{
	const idMaterial*	lightShader = vLight->lightShader;
	const float*			lightRegs = vLight->shaderRegisters;
	
	if( r_logFile.GetBool() )
	{
		RB_LogComment( "---------- RB_EXP_DrawLightDeferred ----------\n" );
	}
	
	if( vLight->viewInsideLight )
	{
		GL_Cull( CT_BACK_SIDED );
		glDisable( GL_DEPTH_TEST );
	}
	else
	{
		GL_Cull( CT_FRONT_SIDED );
	}
	
	GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHMASK );// | backEnd.depthFunc );
	//GL_State( GLS_DEPTHMASK );// | backEnd.depthFunc );
	
	// go back to world space
	if( backEnd.currentSpace != &backEnd.viewDef->worldSpace )
	{
		backEnd.currentSpace = &backEnd.viewDef->worldSpace;
		glLoadMatrixf( backEnd.viewDef->worldSpace.modelViewMatrix );
	}
	
	// change the scissor if needed
#if 0
	// scissor values out of range
	if( r_useScissor.GetBool() && !backEnd.currentScissor.Equals( vLight->scissorRect ) )
	{
		backEnd.currentScissor = vLight->scissorRect;
		glScissor( backEnd.viewDef->viewport.x1 + backEnd.currentScissor.x1,
				   backEnd.viewDef->viewport.y1 + backEnd.currentScissor.y1,
				   backEnd.currentScissor.x2 + 1 - backEnd.currentScissor.x1,
				   backEnd.currentScissor.y2 + 1 - backEnd.currentScissor.y1 );
	}
#endif
	
#if 1
	bool shadowCompare = ( !r_sb_noShadows.GetBool() && r_shadows.GetBool() && vLight->lightShader->LightCastsShadows() && !vLight->lightDef->parms.noShadows && vLight->shadowLOD >= 0 );
#else
	bool shadowCompare = false;
#endif
	
	// choose and bind the vertex program
	// TODO gl_deferredLightingShader->SetAmbientLighting(vLight->lightShader->IsAmbientLight());
	gl_deferredLightingShader->SetMacro_LIGHT_PROJ( !vLight->lightDef->parms.pointLight );
	gl_deferredLightingShader->SetShadowing( shadowCompare );
	gl_deferredLightingShader->SetNormalMapping( !r_skipBump.GetBool() || vLight->lightShader->IsAmbientLight() );
	gl_deferredLightingShader->SetFrustumClipping( /*vLight->viewInsideLight ||*/ r_deferredShadingGPUFrustumCulling.GetBool() ); // FIXME expensive
	gl_deferredLightingShader->BindProgram();
	
	// load all the vertex program parameters
	gl_deferredLightingShader->SetUniform_UnprojectMatrix( make_idMat4Transposed( backEnd.viewDef->unprojectionMatrix ) );
	
	gl_deferredLightingShader->SetUniform_GlobalViewOrigin( backEnd.viewDef->renderView.vieworg );
	gl_deferredLightingShader->SetUniform_GlobalLightOrigin( vLight->lightDef->globalLightOrigin );
	
	//gl_deferredLightingShader->SetUniform_LightRadius(vLight->lightDef->frustumTris->bounds.GetRadius());
	gl_deferredLightingShader->SetUniform_LightRadius( viewLightAxialSize );
	
	gl_deferredLightingShader->SetUniform_LightFrustum( vLight->lightDef->frustum );
	
	gl_deferredLightingShader->SetUniform_InvertedFramebufferResolution( backEnd.viewDef->viewport );
	gl_deferredLightingShader->SetUniform_NonPowerOfTwoScale( backEnd.viewDef->viewport,
			globalImages->currentNormalsImage->uploadWidth,
			globalImages->currentNormalsImage->uploadHeight );
			
	gl_deferredLightingShader->SetUniform_Viewport( backEnd.viewDef->viewport );
	
#if 1
	if( shadowCompare )
	{
		float shadowTexelSize = 1.0f / shadowMapResolutions[backEnd.vLight->shadowLOD];
		gl_deferredLightingShader->SetUniform_ShadowTexelSize( shadowTexelSize );
		gl_deferredLightingShader->SetUniform_ShadowBlur( r_sb_samples.GetInteger() );
		
		// calculate depth projection for shadow buffer
		
		// TODO
		//gl_deferredLightingShader->SetUniform_ShadowMatrix( shadowMat );
	}
#endif
	
	//-----------------------------------------------------
	// screen power of two correction factor
	
	idVec4	parm;
	parm[0] = 1.0 / ( JITTER_SIZE * r_sb_samples.GetInteger() ) ;
	parm[1] = 1.0 / JITTER_SIZE;
	parm[2] = 0;
	parm[3] = 1;
	gl_deferredLightingShader->SetUniform_PositionToJitterTexScale( parm );
	
	// jitter tex scale
	parm[0] =
		parm[1] = r_sb_jitterScale.GetFloat() * lightBufferSizeFraction[0];
	parm[2] = -r_sb_biasScale.GetFloat();
	parm[3] = 0;
	gl_deferredLightingShader->SetUniform_JitterTexScale( parm );
	
	// jitter tex offset
	if( r_sb_randomize.GetBool() )
	{
		parm[0] = ( rand() & 255 ) / 255.0;
		parm[1] = ( rand() & 255 ) / 255.0;
	}
	else
	{
		parm[0] = parm[1] = 0;
	}
	parm[2] = 0;
	parm[3] = 0;
	gl_deferredLightingShader->SetUniform_JitterTexOffset( parm );
	//-----------------------------------------------------
	
	// set the textures
	
	// texture 0 will be the _currentNormals
	GL_SelectTextureNoClient( 0 );
	globalImages->currentNormalsImage->Bind();
	
	// texture 1 will be the _currentDepth
	GL_SelectTextureNoClient( 1 );
	globalImages->currentDepthImage->Bind();
	
	// texture 2 will be the light falloff texture
	GL_SelectTextureNoClient( 2 );
	vLight->falloffImage->Bind();
	
	for( int lightStageNum = 0 ; lightStageNum < lightShader->GetNumStages() ; lightStageNum++ )
	{
		const shaderStage_t*	lightStage = lightShader->GetStage( lightStageNum );
		
		// ignore stages that fail the condition
		if( !lightRegs[ lightStage->conditionRegister ] )
		{
			continue;
		}
		
		idVec4 lightColor;
		
		// backEnd.lightScale is calculated so that lightColor[] will never exceed
		// tr.backEndRendererMaxLight
#if 0
		lightColor[0] = backEnd.lightScale * lightRegs[ lightStage->color.registers[0] ];
		lightColor[1] = backEnd.lightScale * lightRegs[ lightStage->color.registers[1] ];
		lightColor[2] = backEnd.lightScale * lightRegs[ lightStage->color.registers[2] ];
#else
		lightColor[0] = lightRegs[ lightStage->color.registers[0] ];
		lightColor[1] = lightRegs[ lightStage->color.registers[1] ];
		lightColor[2] = lightRegs[ lightStage->color.registers[2] ];
#endif
		lightColor[3] = lightRegs[ lightStage->color.registers[3] ];
		
		// if we wouldn't draw anything, don't call the Draw function
		if( lightColor.LengthSqr() == 0 )
		{
			continue;
		}
		
		idPlane lightProject[4];
		for( int i = 0 ; i < 4 ; i++ )
		{
			lightProject[i] = vLight->lightDef->lightProject[i];
		}
		
		// now multiply the texgen by the light texture matrix
		if( lightStage->texture.hasMatrix )
		{
			RB_GetShaderTextureMatrix( lightRegs, &lightStage->texture, backEnd.lightTextureMatrix );
			RB_BakeTextureMatrixIntoTexgen( lightProject, backEnd.lightTextureMatrix );
		}
		
		gl_deferredLightingShader->SetUniform_LightColor( lightColor );
		
		gl_deferredLightingShader->SetUniform_LightProjectS( lightProject[0].ToVec4() );
		gl_deferredLightingShader->SetUniform_LightProjectT( lightProject[1].ToVec4() );
		gl_deferredLightingShader->SetUniform_LightProjectQ( lightProject[2].ToVec4() );
		gl_deferredLightingShader->SetUniform_LightFalloffS( lightProject[3].ToVec4() );
		
		// texture 3 will be the light projection texture
		GL_SelectTextureNoClient( 3 );
		lightStage->texture.image->Bind();
		
		// draw it
		RB_RenderTriangleSurface( vLight->lightDef->frustumTris );
	}
	
	if( vLight->viewInsideLight )
	{
		GL_Cull( CT_FRONT_SIDED );
		glEnable( GL_DEPTH_TEST );
	}
	
	GL_BindNullProgram();
}


void InvertByTranspose( const float a[16], float r[16] )
{
	r[ 0] = a[ 0];
	r[ 1] = a[ 4];
	r[ 2] = a[ 8];
	r[ 3] = 0;
	r[ 4] = a[ 1];
	r[ 5] = a[ 5];
	r[ 6] = a[ 9];
	r[ 7] = 0;
	r[ 8] = a[ 2];
	r[ 9] = a[ 6];
	r[10] = a[10];
	r[11] = 0;
	r[12] = -( r[ 0] * a[12] + r[ 4] * a[13] + r[ 8] * a[14] );
	r[13] = -( r[ 1] * a[12] + r[ 5] * a[13] + r[ 9] * a[14] );
	r[14] = -( r[ 2] * a[12] + r[ 6] * a[13] + r[10] * a[14] );
	r[15] = 1;
}

void FullInvert( const float a[16], float r[16] )
{
	idMat4	am;
	
	for( int i = 0 ; i < 4 ; i++ )
	{
		for( int j = 0 ; j < 4 ; j++ )
		{
			am[i][j] = a[j * 4 + i];
		}
	}
	
//	idVec4 test( 100, 100, 100, 1 );
//	idVec4	transformed, inverted;
//	transformed = test * am;

	if( !am.InverseSelf() )
	{
		common->Error( "Invert failed" );
	}
//	inverted = transformed * am;

	for( int i = 0 ; i < 4 ; i++ )
	{
		for( int j = 0 ; j < 4 ; j++ )
		{
			r[j * 4 + i] = am[i][j];
		}
	}
}

/*
==================
RB_Exp_TrianglesForFrustum
==================
*/
const srfTriangles_t*	RB_Exp_TrianglesForFrustum( viewLight_t* vLight, int side )
{
	const srfTriangles_t* tri;
	
	static srfTriangles_t	frustumTri;
	static idDrawVert		verts[5];
	static glIndex_t		indexes[18] = { 0, 1, 2,	0, 2, 3,	0, 3, 4,	0, 4, 1,	2, 1, 4,	2, 4, 3 };
	
	if( side == -1 )
	{
		tri = vLight->frustumTris;
	}
	else
	{
		memset( verts, 0, sizeof( verts ) );
		
		for( int i = 0 ; i < 5 ; i++ )
		{
			verts[i].xyz = vLight->globalLightOrigin;
		}
		
		memset( &frustumTri, 0, sizeof( frustumTri ) );
		frustumTri.indexes = indexes;
		frustumTri.verts = verts;
		frustumTri.numIndexes = 18;
		frustumTri.numVerts = 5;
		
		tri = &frustumTri;
		
		float	size = viewLightAxialSize;
		
		switch( side )
		{
			case 0:
				verts[1].xyz[0] += size;
				verts[2].xyz[0] += size;
				verts[3].xyz[0] += size;
				verts[4].xyz[0] += size;
				verts[1].xyz[1] += size;
				verts[1].xyz[2] += size;
				verts[2].xyz[1] -= size;
				verts[2].xyz[2] += size;
				verts[3].xyz[1] -= size;
				verts[3].xyz[2] -= size;
				verts[4].xyz[1] += size;
				verts[4].xyz[2] -= size;
				break;
			case 1:
				verts[1].xyz[0] -= size;
				verts[2].xyz[0] -= size;
				verts[3].xyz[0] -= size;
				verts[4].xyz[0] -= size;
				verts[1].xyz[1] -= size;
				verts[1].xyz[2] += size;
				verts[2].xyz[1] += size;
				verts[2].xyz[2] += size;
				verts[3].xyz[1] += size;
				verts[3].xyz[2] -= size;
				verts[4].xyz[1] -= size;
				verts[4].xyz[2] -= size;
				break;
			case 2:
				verts[1].xyz[1] += size;
				verts[2].xyz[1] += size;
				verts[3].xyz[1] += size;
				verts[4].xyz[1] += size;
				verts[1].xyz[0] -= size;
				verts[1].xyz[2] += size;
				verts[2].xyz[0] += size;
				verts[2].xyz[2] += size;
				verts[3].xyz[0] += size;
				verts[3].xyz[2] -= size;
				verts[4].xyz[0] -= size;
				verts[4].xyz[2] -= size;
				break;
			case 3:
				verts[1].xyz[1] -= size;
				verts[2].xyz[1] -= size;
				verts[3].xyz[1] -= size;
				verts[4].xyz[1] -= size;
				verts[1].xyz[0] += size;
				verts[1].xyz[2] += size;
				verts[2].xyz[0] -= size;
				verts[2].xyz[2] += size;
				verts[3].xyz[0] -= size;
				verts[3].xyz[2] -= size;
				verts[4].xyz[0] += size;
				verts[4].xyz[2] -= size;
				break;
			case 4:
				verts[1].xyz[2] += size;
				verts[2].xyz[2] += size;
				verts[3].xyz[2] += size;
				verts[4].xyz[2] += size;
				verts[1].xyz[0] += size;
				verts[1].xyz[1] += size;
				verts[2].xyz[0] -= size;
				verts[2].xyz[1] += size;
				verts[3].xyz[0] -= size;
				verts[3].xyz[1] -= size;
				verts[4].xyz[0] += size;
				verts[4].xyz[1] -= size;
				break;
			case 5:
				verts[1].xyz[2] -= size;
				verts[2].xyz[2] -= size;
				verts[3].xyz[2] -= size;
				verts[4].xyz[2] -= size;
				verts[1].xyz[0] -= size;
				verts[1].xyz[1] += size;
				verts[2].xyz[0] += size;
				verts[2].xyz[1] += size;
				verts[3].xyz[0] += size;
				verts[3].xyz[1] -= size;
				verts[4].xyz[0] -= size;
				verts[4].xyz[1] -= size;
				break;
		}
		
		frustumTri.ambientCache = vertexCache.AllocFrameTemp( verts, sizeof( verts ) );
	}
	
	return tri;
}




/*
==================
R_EXP_CalcLightAxialSize

all light side projections must currently match, so non-centered
and non-cubic lights must take the largest length
==================
*/
float	R_EXP_CalcLightAxialSize( viewLight_t* vLight )
{
	float	max = 0;
	
	if( !vLight->lightDef->parms.pointLight && !vLight->lightDef->parms.parallel )
	{
#if 0
		idVec3	dir = vLight->lightDef->parms.target - vLight->lightDef->parms.origin;
		max = dir.Length();
#else
		//max = vLight->lightDef->frustumTris->bounds.GetRadius();
		max = vLight->lightDef->falloffLength;
#endif
		return max;
	}
	
	for( int i = 0 ; i < 3 ; i++ )
	{
		float	dist = fabs( vLight->lightDef->parms.lightCenter[i] );
		dist += vLight->lightDef->parms.lightRadius[i];
		if( dist > max )
		{
			max = dist;
		}
	}
	return max;
}

/*
==================
R_EXP_RenderViewDepthImage

This could be avoided by drop sampling the native view depth buffer with render to texture
Bilerp might even be aprorpiate, although it would cause issues at edges
==================
*/
void RB_T_FillDepthBuffer( const drawSurf_t* surf );

#if 0
void R_EXP_RenderViewDepthImage()
{


	// if the screen resolution is exactly the window width, we can
	// use the depth buffer we already have
	if( 1 )    // nativeViewBuffer ) {
	{
		viewDepthImage->CopyDepthbuffer( backEnd.viewDef->viewport.x1,
										 backEnd.viewDef->viewport.y1,  backEnd.viewDef->viewport.x2 -  backEnd.viewDef->viewport.x1 + 1,
										 backEnd.viewDef->viewport.y2 -  backEnd.viewDef->viewport.y1 + 1 );
	}
	else
	{
		RB_LogComment( "---------- R_EXP_RenderViewDepthImage ----------\n" );
		
		if( r_sb_usePbuffer.GetBool() )
		{
			GL_CheckErrors();
			// set the current openGL drawable to the shadow buffer
			R_MakeCurrent( viewPbufferDC, win32.hGLRC, NULL /* !@# viewPbuffer */ );
		}
		
		// render the depth to the new size
		glViewport( 0, 0, viewBufferSize, viewBufferHeight );
		glScissor( 0, 0, viewBufferSize, viewBufferHeight );
		glClear( GL_DEPTH_BUFFER_BIT );
		glStencilFunc( GL_ALWAYS, 0, 255 );
		
		// the first texture will be used for alpha tested surfaces
		GL_SelectTexture( 0 );
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
		
		GL_State( GLS_DEPTHFUNC_LESS );
		
		RB_RenderDrawSurfListWithFunction( backEnd.viewDef->drawSurfs, backEnd.viewDef->numDrawSurfs, RB_T_FillDepthBuffer );
		
		//
		// copy it to a texture
		//
		viewDepthImage->Bind();
		glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, viewBufferSize, viewBufferHeight );
		
		if( r_sb_usePbuffer.GetBool() )
		{
			// set the normal screen drawable current
			R_MakeCurrent( win32.hDC, win32.hGLRC, NULL );
		}
		
		// reset the window clipping
		glMatrixMode( GL_PROJECTION );
		glLoadMatrixf( backEnd.viewDef->projectionMatrix );
		glMatrixMode( GL_MODELVIEW );
		
		glViewport( tr.viewportOffset[0] + backEnd.viewDef->viewport.x1,
					tr.viewportOffset[1] + backEnd.viewDef->viewport.y1,
					backEnd.viewDef->viewport.x2 + 1 - backEnd.viewDef->viewport.x1,
					backEnd.viewDef->viewport.y2 + 1 - backEnd.viewDef->viewport.y1 );
		glScissor( tr.viewportOffset[0] + backEnd.viewDef->viewport.x1,
				   tr.viewportOffset[1] + backEnd.viewDef->viewport.y1,
				   backEnd.viewDef->viewport.x2 + 1 - backEnd.viewDef->viewport.x1,
				   backEnd.viewDef->viewport.y2 + 1 - backEnd.viewDef->viewport.y1 );
				   
		// the current modelView matrix is not valid
		backEnd.currentSpace = NULL;
	}
	
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
}
#endif

/*
==================
RB_EXP_SetNativeBuffer

This is always the back buffer, and scissor is set full screen
==================
*/
void RB_EXP_SetNativeBuffer()
{
	// set the normal screen drawable current
	Framebuffer::BindNull();
	
	glViewport( tr.viewportOffset[0] + backEnd.viewDef->viewport.x1,
				tr.viewportOffset[1] + backEnd.viewDef->viewport.y1,
				backEnd.viewDef->viewport.x2 + 1 - backEnd.viewDef->viewport.x1,
				backEnd.viewDef->viewport.y2 + 1 - backEnd.viewDef->viewport.y1 );
				
	backEnd.currentScissor = backEnd.viewDef->viewport;
	if( r_useScissor.GetBool() )
	{
		glScissor( backEnd.viewDef->viewport.x1 + backEnd.currentScissor.x1,
				   backEnd.viewDef->viewport.y1 + backEnd.currentScissor.y1,
				   backEnd.currentScissor.x2 + 1 - backEnd.currentScissor.x1,
				   backEnd.currentScissor.y2 + 1 - backEnd.currentScissor.y1 );
	}
}

/*
==================
RB_EXP_SetRenderBuffer

This may be to a float pBuffer, and scissor is set to cover only the light
==================
*/
void RB_EXP_SetRenderBuffer( viewLight_t* vLight )
{

	if( r_useHighDynamicRange.GetBool() )
	{
		globalFramebuffers.hdrRender->Bind();
	}
	else
	{
		Framebuffer::BindNull();
	}
	
	glViewport( tr.viewportOffset[0] + backEnd.viewDef->viewport.x1,
				tr.viewportOffset[1] + backEnd.viewDef->viewport.y1,
				backEnd.viewDef->viewport.x2 + 1 - backEnd.viewDef->viewport.x1,
				backEnd.viewDef->viewport.y2 + 1 - backEnd.viewDef->viewport.y1 );
				
	if( !vLight )
	{
		backEnd.currentScissor = backEnd.viewDef->viewport;
	}
	else
	{
		backEnd.currentScissor = vLight->scissorRect;
	}
	
	if( r_useScissor.GetBool() )
	{
		glScissor( backEnd.viewDef->viewport.x1 + backEnd.currentScissor.x1,
				   backEnd.viewDef->viewport.y1 + backEnd.currentScissor.y1,
				   backEnd.currentScissor.x2 + 1 - backEnd.currentScissor.x1,
				   backEnd.currentScissor.y2 + 1 - backEnd.currentScissor.y1 );
	}
}

/*
==================
RB_shadowResampleAlpha
==================
*/
#if 0
void	RB_shadowResampleAlpha()
{
	viewAlphaImage->Bind();
	// we could make this a subimage, but it isn't relevent once we have render-to-texture
	glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, viewBufferSize, viewBufferHeight );
	
	RB_EXP_SetRenderBuffer( backEnd.vLight );
	
//=====================

	glLoadMatrixf( backEnd.viewDef->worldSpace.modelViewMatrix );
	
	// this uses the full light, not side frustums
	const srfTriangles_t* tri = backEnd.vLight->frustumTris;
	
	idDrawVert* ac = ( idDrawVert* )vertexCache.Position( tri->ambientCache );
	glVertexPointer( 3, GL_FLOAT, sizeof( idDrawVert ), ac->xyz.ToFloatPtr() );
	
	// clear stencil buffer
	glEnable( GL_SCISSOR_TEST );
	glEnable( GL_STENCIL_TEST );
	glClearStencil( 1 );
	glClear( GL_STENCIL_BUFFER_BIT );
	
	// draw front faces of the light frustum, incrementing the stencil buffer on depth fail
	// so we can't draw on those pixels
	GL_State( GLS_COLORMASK | GLS_ALPHAMASK | GLS_DEPTHMASK | GLS_DEPTHFUNC_LESS );
	glStencilFunc( GL_ALWAYS, 0, 255 );
	glStencilOp( GL_KEEP, GL_INCR, GL_KEEP );
	GL_Cull( CT_FRONT_SIDED );
	
	// set fragment / vertex program?
	
	RB_DrawElementsWithCounters( tri );
	
	// draw back faces of the light frustum with
	// depth test greater
	// stencil test of equal 1
	// zero stencil stencil when depth test passes, so subsequent interaction drawing
	// can occur on those pixels
	
	// this pass does all the shadow filtering
	glStencilFunc( GL_EQUAL, 1, 255 );
	glStencilOp( GL_KEEP, GL_KEEP, GL_ZERO );
	
	// write to destination alpha
	if( r_sb_showFrustumPixels.GetBool() )
	{
		GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHMASK | GLS_DEPTHFUNC_LESS );
		glDisable( GL_TEXTURE_2D );
		glColor4f( 0, 0.25, 0, 1 );
	}
	else
	{
		GL_State( GLS_COLORMASK | GLS_DEPTHMASK | GLS_DEPTHFUNC_LESS );
		glEnable( GL_VERTEX_PROGRAM_ARB );
		glEnable( GL_FRAGMENT_PROGRAM_ARB );
		glBindProgramARB( GL_VERTEX_PROGRAM_ARB, shadowResampleVertexProgram );
		glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, shadowResampleFragmentProgram );
		
		// convert 0..viewport-1 sizes to fractions inside the POT screen depth texture
		// shrink by one unit for bilerp
		float	parm[4];
		parm[0] = 1.0 / ( maxViewBufferSize + 1 ) * viewBufferSize / maxViewBufferSize;
		parm[1] = parm[0];
		parm[2] = 0;
		parm[3] = 1;
		glProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 0, parm );
	}
	
	GL_Cull( CT_BACK_SIDED );
	glDepthFunc( GL_GREATER );
	
	RB_DrawElementsWithCounters( tri );
	
	glDisable( GL_VERTEX_PROGRAM_ARB );
	glDisable( GL_FRAGMENT_PROGRAM_ARB );
	
	GL_Cull( CT_FRONT_SIDED );
	
	glDepthFunc( GL_LEQUAL );
	if( r_sb_showFrustumPixels.GetBool() )
	{
		glEnable( GL_TEXTURE_2D );
		glColor3f( 1, 1, 1 );
	}
	
	// after all the frustums have been drawn, the surfaces that have been drawn on will get interactions
	// scissor may still be a win even with the stencil test for very fast rejects
	glStencilFunc( GL_EQUAL, 0, 255 );
	glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
}
#endif



/*
==================
RB_EXP_ReadFloatBuffer
==================
*/
void RB_EXP_ReadFloatBuffer()
{
	int		pixels = glConfig.vidWidth * glConfig.vidHeight;
	float*	buf = ( float* )R_StaticAlloc( pixels * 4 * sizeof( float ) );
	
	glReadPixels( 0, 0, glConfig.vidWidth, glConfig.vidHeight, GL_RGBA, GL_FLOAT, buf );
	
	float	mins[4] = { 9999, 9999, 9999, 9999 };
	float	maxs[4] = { -9999, -9999, -9999, -9999 };
	for( int i = 0 ; i < pixels ; i++ )
	{
		for( int j = 0 ; j < 4 ; j++ )
		{
			float	v = buf[ i * 4 + j ];
			if( v < mins[j] )
			{
				mins[j] = v;
			}
			if( v > maxs[j] )
			{
				maxs[j] = v;
			}
		}
	}
	
	RB_EXP_SetNativeBuffer();
	
	glLoadIdentity();
	glMatrixMode( GL_PROJECTION );
	GL_State( GLS_DEPTHFUNC_ALWAYS );
	glColor3f( 1, 1, 1 );
	glPushMatrix();
	glLoadIdentity();
	glDisable( GL_TEXTURE_2D );
	glOrtho( 0, 1, 0, 1, -1, 1 );
	glRasterPos2f( 0.01f, 0.01f );
	glDrawPixels( glConfig.vidWidth, glConfig.vidHeight, GL_RGBA, GL_FLOAT, buf );
	glPopMatrix();
	glEnable( GL_TEXTURE_2D );
	glMatrixMode( GL_MODELVIEW );
	
	R_StaticFree( buf );
}

static void RB_CalculateHDRAdaptation()
{
	int				i;
	static float	image[64 * 64 * 4];
	float           curTime;
	float			deltaTime;
	float           luminance;
	float			avgLuminance;
	float			maxLuminance;
	double			sum;
	const idVec3    LUMINANCE_VECTOR( 0.2125f, 0.7154f, 0.0721f );
	idVec4			color;
	float			newAdaptation;
	float			newMaximum;
	
	RB_LogComment( "---------- RB_CalculateHDRAdaptation ----------\n" );
	
	if( glConfig.framebufferBlitAvailable )
	{
		glBindFramebuffer( GL_READ_FRAMEBUFFER, globalFramebuffers.hdrRender->GetFramebuffer() );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, downScaleFBO_quarter->GetFramebuffer() );
		glBlitFramebuffer( 0, 0, glConfig.vidWidth, glConfig.vidHeight,
						   0, 0, glConfig.vidWidth * 0.25f, glConfig.vidHeight * 0.25f,
						   GL_COLOR_BUFFER_BIT,
						   GL_LINEAR );
						   
		glBindFramebuffer( GL_READ_FRAMEBUFFER, globalFramebuffers.hdrRender->GetFramebuffer() );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, downScaleFBO_64x64->GetFramebuffer() );
		glBlitFramebuffer( 0, 0, glConfig.vidWidth, glConfig.vidHeight,
						   0, 0, 64, 64,
						   GL_COLOR_BUFFER_BIT,
						   GL_LINEAR );
	}
	else
	{
		// FIXME add non EXT_framebuffer_blit code
	}
	
	curTime = Sys_Milliseconds() / 1000.0f;
	
	// calculate the average scene luminance
	downScaleFBO_64x64->Bind();
	
	// read back the contents
//	glFinish();
	glReadPixels( 0, 0, 64, 64, GL_RGBA, GL_FLOAT, image );
	
	sum = 0.0f;
	maxLuminance = 0.0f;
	for( i = 0; i < ( 64 * 64 * 4 ); i += 4 )
	{
		color[0] = image[i + 0];
		color[1] = image[i + 1];
		color[2] = image[i + 2];
		color[3] = image[i + 3];
		
		luminance = DotProduct( color, LUMINANCE_VECTOR ) + 0.0001f;
		if( luminance > maxLuminance )
			maxLuminance = luminance;
			
		sum += log( luminance );
	}
	sum /= ( 64.0f * 64.0f );
	avgLuminance = exp( sum );
	
	// the user's adapted luminance level is simulated by closing the gap between
	// adapted luminance and current luminance by 2% every frame, based on a
	// 30 fps rate. This is not an accurate model of human adaptation, which can
	// take longer than half an hour.
	if( backEnd.hdrTime > curTime )
		backEnd.hdrTime = curTime;
		
	deltaTime = curTime - backEnd.hdrTime;
	
	//if(r_hdrMaxLuminance->value)
	{
		backEnd.hdrAverageLuminance = idMath::ClampFloat( r_hdrMinLuminance.GetFloat(), r_hdrMaxLuminance.GetFloat(), backEnd.hdrAverageLuminance );
		avgLuminance = idMath::ClampFloat( r_hdrMinLuminance.GetFloat(), r_hdrMaxLuminance.GetFloat(), avgLuminance );
		
		backEnd.hdrMaxLuminance = idMath::ClampFloat( r_hdrMinLuminance.GetFloat(), r_hdrMaxLuminance.GetFloat(), backEnd.hdrMaxLuminance );
		maxLuminance = idMath::ClampFloat( r_hdrMinLuminance.GetFloat(), r_hdrMaxLuminance.GetFloat(), maxLuminance );
	}
	
	newAdaptation = backEnd.hdrAverageLuminance + ( avgLuminance - backEnd.hdrAverageLuminance ) * ( 1.0f - powf( 0.98f, 30.0f * deltaTime ) );
	newMaximum = backEnd.hdrMaxLuminance + ( maxLuminance - backEnd.hdrMaxLuminance ) * ( 1.0f - powf( 0.98f, 30.0f * deltaTime ) );
	
	if( !FLOAT_IS_NAN( newAdaptation ) && !FLOAT_IS_NAN( newMaximum ) )
	{
#if 1
		backEnd.hdrAverageLuminance = newAdaptation;
		backEnd.hdrMaxLuminance = newMaximum;
#else
		backEnd.hdrAverageLuminance = avgLuminance;
		backEnd.hdrMaxLuminance = maxLuminance;
#endif
	}
	
	backEnd.hdrTime = curTime;
	
	// calculate HDR image key
	if( r_hdrKey.GetFloat() <= 0 )
	{
		// calculation from: Perceptual Effects in Real-time Tone Mapping - Krawczyk et al.
		backEnd.hdrKey = 1.03 - 2.0 / ( 2.0 + log10f( backEnd.hdrAverageLuminance + 1.0f ) );
	}
	else
	{
		backEnd.hdrKey = r_hdrKey.GetFloat();
	}
	
	if( r_showHDR.GetBool() )
	{
		common->Printf( "HDR luminance avg = %f, max = %f, key = %f\n", backEnd.hdrAverageLuminance, backEnd.hdrMaxLuminance, backEnd.hdrKey );
	}
	
	GL_CheckErrors();
}


void RB_TestGamma();

/*
==================
RB_EXP_GammaDither
==================
*/
void	RB_EXP_GammaDither()
{
	if( !r_useHighDynamicRange.GetBool() )
	{
		return;
	}
	
	RB_LogComment( "---------- RB_EXP_GammaDither ----------\n" );
	
#if 0
	r_testGamma.SetBool( true );
	RB_TestGamma();
	r_testGamma.SetBool( false );
#endif
	
	RB_EXP_SetNativeBuffer();
	
	gl_toneMappingShader->DisableMacro_BRIGHTPASS_FILTER();
	gl_toneMappingShader->BindProgram();
	
	gl_toneMappingShader->SetUniform_HDRKey( backEnd.hdrKey );
	gl_toneMappingShader->SetUniform_HDRAverageLuminance( backEnd.hdrAverageLuminance );
	gl_toneMappingShader->SetUniform_HDRMaxLuminance( backEnd.hdrMaxLuminance );
	
	// bind u_CurrentImage
	gl_toneMappingShader->SetUniform_CurrentRenderImage( 0 );
	GL_SelectTextureNoClient( 0 );
	hdrRenderImage->BindFragment();
	
	glDisable( GL_STENCIL_TEST );
	glDisable( GL_SCISSOR_TEST );
	glDisable( GL_DEPTH_TEST );
	
	RB_EXP_CoverScreen();
	
	glEnable( GL_DEPTH_TEST );
	
	glDisable( GL_VERTEX_PROGRAM_ARB );
	glDisable( GL_FRAGMENT_PROGRAM_ARB );
}

static void RB_EXP_FXAA()
{
#if 0
	if( !r_useFXAA.GetBool() )
	{
		return;
	}
	
	RB_LogComment( "---------- RB_EXP_FXAA ----------\n" );
	
	//RB_EXP_SetNativeBuffer();
	
	gl_FXAAShader->BindProgram();
	
	gl_FXAAShader->SetUniform_InvertedFramebufferResolution( backEnd.viewDef->viewport );
	gl_FXAAShader->SetUniform_FxaaInvertedFramebufferResolutionOpt( backEnd.viewDef->viewport );
	gl_FXAAShader->SetUniform_NonPowerOfTwoScale( backEnd.viewDef->viewport,
			globalImages->currentNormalsImage->uploadWidth,
			globalImages->currentNormalsImage->uploadHeight );
			
	gl_FXAAShader->SetUniform_Viewport( backEnd.viewDef->viewport );
	
	
	// bind u_CurrentRenderImage
#if 1
	globalImages->currentRenderImage->CopyFramebuffer( backEnd.viewDef->viewport.x1,
			backEnd.viewDef->viewport.y1,  backEnd.viewDef->viewport.x2 -  backEnd.viewDef->viewport.x1 + 1,
			backEnd.viewDef->viewport.y2 -  backEnd.viewDef->viewport.y1 + 1, !glConfig.textureNonPowerOfTwoAvailable, false );
#endif
	gl_FXAAShader->SetUniform_CurrentRenderImage( 2 );
	GL_SelectTextureNoClient( 2 );
	
	//int stateBits = backEnd.glState.glStateBits;
	//GL_State(GLS_DEPTHMASK);
	
	
	//glDisable( GL_STENCIL_TEST );
	//glDisable( GL_SCISSOR_TEST );
	//glDisable( GL_DEPTH_TEST );
	
	RB_EXP_CoverScreen();
	
	//glEnable( GL_DEPTH_TEST );
	//glEnable( GL_STENCIL_TEST );
	
	//GL_State(stateBits);
	GL_BindNullProgram();
#endif
}

/*
==================
RB_EXP_Bloom
==================
*/
#if 0
void	RB_EXP_Bloom()
{
	if( !r_useHighDynamicRange.GetBool() )
	{
		return;
	}
	
	if( r_hdr_bloomFraction.GetFloat() == 0 )
	{
		return;
	}
	
	GL_CheckErrors();
	
	//
	// mip map
	//
	
	// draw to the second floatPbuffer
	R_MakeCurrent( floatPbuffer2DC, floatContext, floatPbuffer2 );
	
	GL_State( 0 );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_SCISSOR_TEST );
	
	glEnable( GL_VERTEX_PROGRAM_ARB );
	glEnable( GL_FRAGMENT_PROGRAM_ARB );
	
	glClearColor( 1.0, 0.5, 0, 0 );
	glClear( GL_COLOR_BUFFER_BIT );
	glViewport( 0, 0, glConfig.vidWidth >> 1, glConfig.vidHeight >> 1 );
	
	// read from the original floatPbuffer
	glActiveTextureARB( GL_TEXTURE0_ARB );
	glBindTexture( GL_TEXTURE_RECTANGLE_NV, floatPbufferImage->texnum );
	R_BindTexImage( floatPbuffer );
	
	glBindProgramARB( GL_VERTEX_PROGRAM_ARB, downSampleVertexProgram );
	glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, downSampleFragmentProgram );
	
	RB_EXP_CoverScreen();
	
	//
	// mip map again
	//
	glViewport( 0, 0, glConfig.vidWidth >> 2, glConfig.vidHeight >> 2 );
	
	// draw to the second floatPbuffer
	R_MakeCurrent( floatPbufferQuarterDC, floatContext, floatPbufferQuarter );
	
	// read from the original floatPbuffer
	glBindTexture( GL_TEXTURE_RECTANGLE_NV, floatPbuffer2Image->texnum );
	R_BindTexImage( floatPbuffer2 );
	
	RB_EXP_CoverScreen();
	
	//
	// blur horizontally
	//
	/*
	# texture 0 is the high dynamic range buffer
	# writes result.color as the fp16 result of a smeared bloom
	
	PARAM	step =		program.local[0];		# { 1, 0 } or { 0, 1 } for horizontal / vertical separation
	*/
	
	// draw to the second floatPbuffer
	R_MakeCurrent( floatPbuffer2DC, floatContext, floatPbuffer2 );
	
	glBindProgramARB( GL_VERTEX_PROGRAM_ARB, bloomVertexProgram );
	glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, bloomFragmentProgram );
	glEnable( GL_VERTEX_PROGRAM_ARB );
	glEnable( GL_FRAGMENT_PROGRAM_ARB );
	
	GL_SelectTextureNoClient( 0 );
	
	// blur horizontally first to the second floatPbuffer
	glBindTexture( GL_TEXTURE_RECTANGLE_NV, floatPbufferQuarterImage->texnum );
	R_BindTexImage( floatPbufferQuarter );
	
	float	parm[4];
	
	parm[0] = 1;
	parm[1] = 0;
	parm[2] = 0;
	parm[3] = 0;
	glProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 0, parm );
	
	RB_EXP_CoverScreen();
	
	//
	// now blur vertically back to the quarter pbuffer
	//
	R_MakeCurrent( floatPbufferQuarterDC, floatContext, floatPbufferQuarter );
	
	glBindTexture( GL_TEXTURE_RECTANGLE_NV, floatPbuffer2Image->texnum );
	R_BindTexImage( floatPbuffer2 );
	
	parm[0] = 0;
	parm[1] = 1;
	parm[2] = 0;
	parm[3] = 0;
	glProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 0, parm );
	
	RB_EXP_CoverScreen();
	
	//========================
	
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_SCISSOR_TEST );
	
	glDisable( GL_VERTEX_PROGRAM_ARB );
	glDisable( GL_FRAGMENT_PROGRAM_ARB );
	
	GL_CheckErrors();
}
#endif

static void AssertCvarRange( idCVar* cv, float minVal, float maxVal )
{

	bool isInteger = ( cv->GetFlags() & CVAR_INTEGER ) != 0;
	if( isInteger )
	{
		if( cv->GetInteger() < minVal )
		{
			common->Warning( "cvar '%s' out of range (%i < %f)\n", cv->GetName(), cv->GetInteger(), minVal );
			cv->SetInteger( ( int ) minVal );
		}
		else if( cv->GetInteger() > maxVal )
		{
			common->Warning( "cvar '%s' out of range (%i > %f)\n", cv->GetName(), cv->GetInteger(), maxVal );
			cv->SetInteger( ( int ) maxVal );
		}
	}
	else
	{
		if( cv->GetFloat() < minVal )
		{
			common->Warning( "cvar '%s' out of range (%f < %f)\n", cv->GetName(), cv->GetFloat(), minVal );
			cv->SetFloat( minVal );
		}
		else if( cv->GetFloat() > maxVal )
		{
			common->Warning( "cvar '%s' out of range (%f > %f)\n", cv->GetName(), cv->GetFloat(), maxVal );
			cv->SetFloat( maxVal );
		}
	}
}

/*
==================
RB_Exp_DrawInteractions
==================
*/
void    RB_Exp_DrawInteractions()
{

	GL_CheckErrors();
	
	if( !initialized )
	{
		R_Exp_Allocate();
	}
	
	RB_LogComment( "---------- RB_Exp_DrawInteractions ----------\n" );
	
	GL_CheckErrors();
	
	if( !backEnd.viewDef->viewLights )
	{
		return;
	}
	
	// validate the samples
	if( r_sb_samples.GetInteger() != 16 && r_sb_samples.GetInteger() != 4 && r_sb_samples.GetInteger() != 1 )
	{
		r_sb_samples.SetInteger( 0 );
	}
	
	// validate the light resolution
	AssertCvarRange( &r_sb_lightResolutionUltra, 64, maxLightBufferSize );
	AssertCvarRange( &r_sb_lightResolutionVeryHigh, 64, maxLightBufferSize );
	AssertCvarRange( &r_sb_lightResolutionHigh, 64, maxLightBufferSize );
	AssertCvarRange( &r_sb_lightResolutionMedium, 64, maxLightBufferSize );
	AssertCvarRange( &r_sb_lightResolutionLow, 64, maxLightBufferSize );
	
	for( int i = 0; i < MAX_SHADOWMAPS; i++ )
	{
		lightBufferSizeFraction[i] = ( float )shadowMapResolutions[i] / maxLightBufferSize;
	}
	
	// validate the view resolution
	if( r_sb_viewResolution.GetInteger() < 64 )
	{
		r_sb_viewResolution.SetInteger( 64 );
	}
	else if( r_sb_viewResolution.GetInteger() > maxViewBufferSize )
	{
		r_sb_viewResolution.SetInteger( maxViewBufferSize );
	}
	viewBufferSize = r_sb_viewResolution.GetInteger();
	viewBufferHeight = viewBufferSize * 3 / 4;
	viewBufferSizeFraction = ( float )viewBufferSize / maxViewBufferSize;
	viewBufferHeightFraction = ( float )viewBufferHeight / maxViewBufferSize;
	if( viewBufferSize == backEnd.viewDef->viewport.x2 -  backEnd.viewDef->viewport.x1 + 1 )
	{
		nativeViewBuffer = true;
	}
	else
	{
		nativeViewBuffer = false;
	}
	
	// set up for either point sampled or percentage-closer filtering for the shadow sampling
	if( r_sb_linearFilter.IsModified() )
	{
		if( r_sb_linearFilter.GetBool() )
		{
			for( int i = 0; i < MAX_SHADOWMAPS; i++ )
			{
				shadowMapImage[i]->BindFragment();
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			}
			
			for( int i = 0; i < MAX_SHADOWMAPS; i++ )
			{
				shadowCubeImage[i]->BindFragment();
				glTexParameterf( GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
				glTexParameterf( GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			}
		}
		else
		{
			for( int i = 0; i < MAX_SHADOWMAPS; i++ )
			{
				shadowMapImage[i]->BindFragment();
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			}
			
			for( int i = 0; i < MAX_SHADOWMAPS; i++ )
			{
				shadowCubeImage[i]->BindFragment();
				glTexParameterf( GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
				glTexParameterf( GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			}
		}
		
		r_sb_linearFilter.ClearModified();
	}
	
	globalImages->BindNull();
	
	// copy the current depth buffer to a texture for image-space shadowing,
	// or re-render at a lower resolution
	
	if( r_useDeferredShading.GetBool() && !r_skipDeferredLighting.GetBool() )
	{
		//R_EXP_RenderViewDepthImage();
		
		RB_EXP_FXAA();
		
		globalImages->currentNormalsImage->CopyFramebuffer( backEnd.viewDef->viewport.x1,
				backEnd.viewDef->viewport.y1,  backEnd.viewDef->viewport.x2 -  backEnd.viewDef->viewport.x1 + 1,
				backEnd.viewDef->viewport.y2 -  backEnd.viewDef->viewport.y1 + 1, !glConfig.textureNonPowerOfTwoAvailable, true );
				
		globalImages->currentDepthImage->CopyDepthbuffer( backEnd.viewDef->viewport.x1,
				backEnd.viewDef->viewport.y1,  backEnd.viewDef->viewport.x2 -  backEnd.viewDef->viewport.x1 + 1,
				backEnd.viewDef->viewport.y2 -  backEnd.viewDef->viewport.y1 + 1 );
				
		glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
		glClear( GL_COLOR_BUFFER_BIT );
	}
	
	GL_SelectTexture( 0 );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	
	// disable stencil shadow test
	glStencilFunc( GL_ALWAYS, 128, 255 );
	
	// the jitter image will be used to offset sample centers
	GL_SelectTextureNoClient( 8 );
	if( r_sb_samples.GetInteger() == 16 )
	{
		jitterImage16->BindFragment();
	}
	else if( r_sb_samples.GetInteger() == 4 )
	{
		jitterImage4->BindFragment();
	}
	else
	{
		jitterImage1->BindFragment();
	}
	
	// if we are using a float buffer, clear it now
#if 1
	if( r_useHighDynamicRange.GetBool() )
	{
		RB_EXP_SetRenderBuffer( NULL );
		// we need to set a lot of things, because this is a completely different context
		RB_SetDefaultGLState();
		glClearColor( 0.001f, 1.0f, 0.01f, 0.1f );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		// clear the z buffer, set the projection matrix, etc
		RB_BeginDrawingView();
		RB_STD_FillDepthBuffer( ( drawSurf_t** )&backEnd.viewDef->drawSurfs[0], backEnd.viewDef->numDrawSurfs );
	}
#endif
	
	GL_CheckErrors();
	
	// for each light, perform adding and shadowing
	for( viewLight_t* vLight = backEnd.viewDef->viewLights ; vLight ; vLight = vLight->next )
	{
		backEnd.vLight = vLight;
		
		const idMaterial*	lightShader = vLight->lightShader;
		
		// do fogging later
		if( lightShader->IsFogLight() )
		{
			continue;
		}
		if( lightShader->IsBlendLight() )
		{
			continue;
		}
		
		if( !vLight->localInteractions && !vLight->globalInteractions && !vLight->translucentInteractions )
		{
			if( r_useDeferredShading.GetBool() )
			{
				if( !vLight->translucentInteractions )
				{
					//continue;
				}
			}
			else
			{
				continue;
			}
		}
		
		if( !vLight->frustumTris->ambientCache )
		{
			R_CreateAmbientCache( const_cast<srfTriangles_t*>( vLight->frustumTris ), false );
		}
		
		// all light side projections must currently match, so non-centered
		// and non-cubic lights must take the largest length
		viewLightAxialSize = R_EXP_CalcLightAxialSize( vLight );
		
		if( r_shadows.GetBool() && vLight->shadowLOD >= 0 )
		{
			int	side, sideStop;
			
			if( vLight->lightDef->parms.pointLight )
			{
				if( r_sb_singleSide.GetInteger() != -1 )
				{
					side = r_sb_singleSide.GetInteger();
					sideStop = side + 1;
				}
				else
				{
					side = 0;
					sideStop = 6;
				}
			}
			else
			{
				side = -1;
				sideStop = 0;
			}
			
			for( ; side < sideStop ; side++ )
			{
				// FIXME: check for frustums completely off the screen
				
				//GL_BindNullProgram();
				//Framebuffer::BindNull();
				
				// render a shadow buffer
				RB_EXP_RenderShadowBuffer( vLight, side );
			}
		}
		
		GL_CheckErrors();
		
		GL_BindNullProgram();
		
		// back to view rendering, possibly in the off-screen buffer
		RB_EXP_SetRenderBuffer( NULL );
		
		// bind shadow buffer to texture
		if( r_shadows.GetBool() && vLight->shadowLOD >= 0 )
		{
			if( vLight->lightDef->parms.pointLight )
			{
				GL_SelectTextureNoClient( 8 );
				shadowCubeImage[vLight->shadowLOD]->BindFragment();
			}
			else
			{
				GL_SelectTextureNoClient( 7 );
				shadowMapImage[vLight->shadowLOD]->BindFragment();
			}
		}
		
		if( r_useDeferredShading.GetBool() )
		{
			if( !r_skipDeferredLighting.GetBool() )
			{
				RB_EXP_DrawLightDeferred( vLight );
			}
		}
		else
		{
			RB_EXP_CreateDrawInteractions( vLight->localInteractions );
			RB_EXP_CreateDrawInteractions( vLight->globalInteractions );
			
			// RB: FIXME translucent interactions should be rendered in a complete different loop
			backEnd.depthFunc = GLS_DEPTHFUNC_LESS;
			RB_EXP_CreateDrawInteractions( vLight->translucentInteractions );
		}
		
		backEnd.depthFunc = GLS_DEPTHFUNC_EQUAL;
	}
	
	GL_CheckErrors();
	
	RB_EXP_SetRenderBuffer( NULL );
	
	GL_CheckErrors();
	
	if( r_useHighDynamicRange.GetBool() )
	{
		RB_CalculateHDRAdaptation();
		
		RB_EXP_GammaDither();
	}
	
	//RB_EXP_FXAA();
	
	if( r_useDeferredShading.GetBool() && !r_skipDeferredLighting.GetBool() )
	{
		GL_SelectTexture( 0 );
		
		globalImages->currentLightImage->CopyFramebuffer( backEnd.viewDef->viewport.x1,
				backEnd.viewDef->viewport.y1,  backEnd.viewDef->viewport.x2 -  backEnd.viewDef->viewport.x1 + 1,
				backEnd.viewDef->viewport.y2 -  backEnd.viewDef->viewport.y1 + 1, !glConfig.textureNonPowerOfTwoAvailable, false );
				
		// TODO restore original colors
		if( !r_skipPostLighting.GetBool() )
		{
			glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
			glClear( GL_COLOR_BUFFER_BIT );
		}
	}
	
//	RB_EXP_Bloom();
//	RB_EXP_GammaDither();

	GL_SelectTexture( 0 );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	
	// experimental transfer function
	for( int i = 8 ; i >= 0 ; i-- )
	{
		GL_SelectTextureNoClient( i );
		globalImages->BindNull();
	}
	GL_State( 0 );
	
	GL_BindNullProgram();
	
	// these haven't been state saved
	for( int i = 0 ; i < 8 ; i++ )
	{
		backEnd.glState.tmu[i].current2DMap = -1;
		backEnd.glState.tmu[i].current3DMap = -1;
		backEnd.glState.tmu[i].currentCubeMap = -1;
	}
	
	GL_CheckErrors();
}


// ======================================================================================

static void	RB_T_DrawInteractionWithLightbuffer( const drawInteractionMaterialOnly_t* din )
{
	// choose and bind the vertex program
	//gl_postLightingShader->SetNormalMapping(!r_skipBump.GetBool());
	gl_postLightingShader->BindProgram();
	
	// load all the vertex program parameters
	//gl_postLightingShader->SetUniform_GlobalViewOrigin(backEnd.viewDef->renderView.vieworg);
//	gl_postLightingShader->SetUniform_AmbientColor(ambientColor);

	gl_postLightingShader->SetUniform_InvertedFramebufferResolution( backEnd.viewDef->viewport );
	gl_postLightingShader->SetUniform_NonPowerOfTwoScale( backEnd.viewDef->viewport,
			globalImages->currentLightImage->uploadWidth,
			globalImages->currentLightImage->uploadHeight );
			
	gl_postLightingShader->SetUniform_Viewport( backEnd.viewDef->viewport );
	
	gl_postLightingShader->SetUniform_ModelMatrix( make_idMat4Transposed( din->surf->space->modelMatrix ) );
	
	gl_postLightingShader->SetUniform_BumpMatrixS( din->bumpMatrix[0] );
	gl_postLightingShader->SetUniform_BumpMatrixT( din->bumpMatrix[1] );
	
	gl_postLightingShader->SetUniform_DiffuseMatrixS( din->diffuseMatrix[0] );
	gl_postLightingShader->SetUniform_DiffuseMatrixT( din->diffuseMatrix[1] );
	
	gl_postLightingShader->SetUniform_SpecularMatrixS( din->specularMatrix[0] );
	gl_postLightingShader->SetUniform_SpecularMatrixT( din->specularMatrix[1] );
	
	static const idVec4 zero( 0, 0, 0, 0 );
	static const idVec4 one( 1, 1, 1, 1 );
	static const idVec4 negOne( -1, -1, -1, -1 );
	
	switch( din->vertexColor )
	{
		case SVC_IGNORE:
			gl_postLightingShader->SetUniform_ColorModulate( zero );
			gl_postLightingShader->SetUniform_Color( one );
			break;
		case SVC_MODULATE:
			gl_postLightingShader->SetUniform_ColorModulate( one );
			gl_postLightingShader->SetUniform_Color( zero );
			break;
		case SVC_INVERSE_MODULATE:
			gl_postLightingShader->SetUniform_ColorModulate( negOne );
			gl_postLightingShader->SetUniform_Color( one );
			break;
	}
	
	// set the constant colors
	gl_postLightingShader->SetUniform_DiffuseColor( din->diffuseColor );
	gl_postLightingShader->SetUniform_SpecularColor( din->specularColor );
	
	// set the textures
	
	// texture 0 will be the per-surface bump map
	//gl_postLightingShader->SetUniform_NormalImage(0);
	//GL_SelectTexture(0);
	//din->bumpImage->Bind();
	
	// texture 1 is the per-surface diffuse map
	//gl_postLightingShader->SetUniform_DiffuseImage(1);
	GL_SelectTexture( 1 );
	din->diffuseImage->Bind();
	
	// texture 2 is the per-surface specular map
	//gl_postLightingShader->SetUniform_SpecularImage(2);
	GL_SelectTexture( 2 );
	din->specularImage->Bind();
	
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
	if( !din->specularImage || r_skipSpecular.GetBool() )
	{
		din->specularImage = globalImages->blackImage;
	}
	if( !din->bumpImage || r_skipBump.GetBool() )
	{
		din->bumpImage = globalImages->flatNormalMap;
	}
	
	// if we wouldn't draw anything, don't call the Draw function
	//if (
	//	( ( din->diffuseColor[0] > 0 || din->diffuseColor[1] > 0 || din->diffuseColor[2] > 0 ) && din->diffuseImage != globalImages->blackImage )
	//	|| ( ( din->specularColor[0] > 0 || din->specularColor[1] > 0 || din->specularColor[2] > 0 ) && din->specularImage != globalImages->blackImage ) )
	{
		/*
		if (din->alphaTest > 0.0f) {
			glEnable( GL_ALPHA_TEST );
			glAlphaFunc( GL_GREATER, din->alphaTest );
		}
		*/
		
		DrawInteraction( din );
		
		/*
		if (din->alphaTest > 0.0f) {
			glDisable( GL_ALPHA_TEST );
		}
		*/
		return true;
	}
}

/*
==================
RB_T_PostLightGeometry
==================
*/
static void RB_T_PostLightGeometry( const drawSurf_t* surf )
{
	int			stage;
	const idMaterial*	shader;
	const shaderStage_t* pStage;
	const float*	regs;
	float		color[4];
	const srfTriangles_t*	tri;
	
	tri = surf->geo;
	shader = surf->material;
	
	
	if( !shader->IsDrawn() )
	{
		return;
	}
	
	// some deforms may disable themselves by setting numIndexes = 0
	if( !tri->numIndexes )
	{
		return;
	}
	
	if( shader->Coverage() == MC_TRANSLUCENT /* != C_PERFORATED */ )
	{
		GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHMASK | GLS_DEPTHFUNC_LESS );
	}
	else
	{
		// only draw on the alpha tested pixels that made it to the depth buffer
		// GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHMASK | GLS_DEPTHFUNC_EQUAL );
		GL_State( GLS_DEPTHMASK | backEnd.depthFunc );
	}
	
	if( !tri->ambientCache )
	{
		common->Printf( "RB_T_PostLightGeometry: !tri->ambientCache\n" );
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
	
	idDrawVert* ac = ( idDrawVert* )vertexCache.Position( tri->ambientCache );
	glVertexPointer( 3, GL_FLOAT, sizeof( idDrawVert ), ac->xyz.ToFloatPtr() );
	glVertexAttribPointerARB( VA_INDEX_TEXCOORD0, 2, GL_FLOAT, false, sizeof( idDrawVert ), ac->st.ToFloatPtr() );
	glVertexAttribPointerARB( VA_INDEX_NORMAL, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->normal.ToFloatPtr() );
	glVertexAttribPointerARB( VA_INDEX_BITANGENT, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->tangents[1].ToFloatPtr() );
	glVertexAttribPointerARB( VA_INDEX_TANGENT, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->tangents[0].ToFloatPtr() );
	glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( idDrawVert ), ac->color );
	
	drawInteractionMaterialOnly_t inter;
	
	inter.surf = surf;
	
	inter.bumpImage = NULL;
	inter.specularImage = NULL;
	inter.diffuseImage = NULL;
	inter.diffuseColor[0] = inter.diffuseColor[1] = inter.diffuseColor[2] = inter.diffuseColor[3] = 0;
	inter.specularColor[0] = inter.specularColor[1] = inter.specularColor[2] = inter.specularColor[3] = 0;
	
	bool	didDraw = false;
	
	for( stage = 0; stage < shader->GetNumStages() ; stage++ )
	{
		pStage = shader->GetStage( stage );
		
		switch( pStage->lighting )
		{
			case SL_AMBIENT:
			{
				// ignore ambient stages while drawing interactions
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
				didDraw = didDraw || RB_T_SubmittInteraction( &inter, RB_T_DrawInteractionWithLightbuffer );
				
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
					didDraw = didDraw || RB_T_SubmittInteraction( &inter, RB_T_DrawInteractionWithLightbuffer );
				}
				R_SetDrawInteraction( pStage, regs, &inter.diffuseImage, inter.diffuseMatrix, inter.diffuseColor.ToFloatPtr() );
				inter.diffuseColor[0] *= backEnd.lightScale;
				inter.diffuseColor[1] *= backEnd.lightScale;
				inter.diffuseColor[2] *= backEnd.lightScale;
				inter.diffuseColor[3] *= backEnd.lightScale;
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
					didDraw = didDraw || RB_T_SubmittInteraction( &inter, RB_T_DrawInteractionWithLightbuffer );
				}
				R_SetDrawInteraction( pStage, regs, &inter.specularImage, inter.specularMatrix, inter.specularColor.ToFloatPtr() );
				inter.specularColor[0] *= backEnd.lightScale;
				inter.specularColor[1] *= backEnd.lightScale;
				inter.specularColor[2] *= backEnd.lightScale;
				inter.specularColor[3] *= backEnd.lightScale;
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
	didDraw = didDraw || RB_T_SubmittInteraction( &inter, RB_T_DrawInteractionWithLightbuffer );
	
#if 0
	if( !didDraw )
	{
		inter.alphaTest = 0;
		inter.bumpImage = globalImages->flatNormalMap;
		inter.diffuseImage = NULL;
		inter.specularImage = NULL;
		
		RB_T_SubmittInteraction( &inter, RB_T_DrawInteractionWithLightbuffer );
	}
#endif
	
	// reset polygon offset
	if( shader->TestMaterialFlag( MF_POLYGONOFFSET ) )
	{
		glDisable( GL_POLYGON_OFFSET_FILL );
	}
}


void RB_EXP_ResolveLightFromLightBuffer( drawSurf_t** drawSurfs, int numDrawSurfs )
{
	// if we are just doing 2D rendering, no need to fill the color buffer
	if( !backEnd.viewDef->viewEntitys )
	{
		return;
	}
	
	if( r_skipPostLighting.GetBool() )
	{
		return;
	}
	
	RB_LogComment( "---------- RB_EXP_ResolveLightFromLightBuffer ----------\n" );
	
	// the first texture will be used for the light buffer
	GL_SelectTexture( 0 );
	globalImages->currentLightImage->Bind();
	
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	
	glEnableVertexAttribArrayARB( VA_INDEX_TEXCOORD0 );
	glEnableVertexAttribArrayARB( VA_INDEX_TANGENT );
	glEnableVertexAttribArrayARB( VA_INDEX_BITANGENT );
	glEnableVertexAttribArrayARB( VA_INDEX_NORMAL );
	glEnableClientState( GL_COLOR_ARRAY );
	
	// decal surfaces may enable polygon offset
	glPolygonOffset( r_offsetFactor.GetFloat(), r_offsetUnits.GetFloat() );
	
	//GL_State( GLS_DEPTHFUNC_LESS );
	//GL_State( GLS_DEPTHMASK | backEnd.depthFunc );
	
	RB_RenderDrawSurfListWithFunction( drawSurfs, numDrawSurfs, RB_T_PostLightGeometry );
	
	glDisableVertexAttribArrayARB( VA_INDEX_TEXCOORD0 );
	glDisableVertexAttribArrayARB( VA_INDEX_TANGENT );
	glDisableVertexAttribArrayARB( VA_INDEX_BITANGENT );
	glDisableVertexAttribArrayARB( VA_INDEX_NORMAL );
	glDisableClientState( GL_COLOR_ARRAY );
	
	glColor4f( 1, 1, 1, 1 );
	
	// FIXME
	GL_SelectTexture( 0 );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	
	// experimental transfer function
	for( int i = 8 ; i >= 0 ; i-- )
	{
		GL_SelectTextureNoClient( i );
		globalImages->BindNull();
	}
	GL_State( 0 );
	
	GL_BindNullProgram();
	
	// these haven't been state saved
	for( int i = 0 ; i < 8 ; i++ )
	{
		backEnd.glState.tmu[i].current2DMap = -1;
		backEnd.glState.tmu[i].current3DMap = -1;
		backEnd.glState.tmu[i].currentCubeMap = -1;
	}
	
	GL_CheckErrors();
}

// ======================================================================================

/*
==================
R_Exp_Init
==================
*/
void R_Exp_Init()
{
	glConfig.allowExpPath = false;
	
	common->Printf( "---------- R_Exp_Init ----------\n" );
	
	if( !glConfig.ARBVertexProgramAvailable || !glConfig.ARBFragmentProgramAvailable ||
			!glConfig.framebufferObjectAvailable ||
			!glConfig.framebufferBlitAvailable ||
			!GLEW_ARB_texture_float ||
			!GLEW_ARB_texture_rg )
	{
		common->Printf( "Not available.\n" );
		return;
	}
	
	RB_CreateBloomTable();
	
#if 0
	if( !R_CheckExtension( "GL_NV_float_buffer" ) )
	{
		common->Printf( "Not available.\n" );
		return;
	}
	if( !R_CheckExtension( "GL_NV_texture_rectangle" ) )
	{
		common->Printf( "Not available.\n" );
		return;
	}
#endif
	
#if 0
	glCombinerParameterfvNV = ( void ( APIENTRY* )( GLenum pname, const GLfloat * params ) )
							  GLimp_ExtensionPointer( "glCombinerParameterfvNV" );
#endif
							  
	common->Printf( "Available.\n" );
	
	if( !idStr::Icmp( r_renderer.GetString(), "exp" ) )
	{
		R_Exp_Allocate();
	}
	
	common->Printf( "--------------------------------------------\n" );
	
	glConfig.allowExpPath = true;
}
