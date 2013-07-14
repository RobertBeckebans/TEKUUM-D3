/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

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

//#define WRITE_GUIS

typedef struct
{
	int		version;
	int		sizeofRenderEntity;
	int		sizeofRenderLight;
	char	mapname[256];
} demoHeader_t;


/*
==============
StartWritingDemo
==============
*/
void		idRenderWorldLocal::StartWritingDemo( idDemoFile* demo )
{
	int		i;
	
	// FIXME: we should track the idDemoFile locally, instead of snooping into session for it
	
	WriteLoadMap();
	
	// write the door portal state
	for( i = 0 ; i < numInterAreaPortals ; i++ )
	{
		if( doublePortals[i].blockingBits )
		{
			SetPortalState( i + 1, doublePortals[i].blockingBits );
		}
	}
	
	// clear the archive counter on all defs
	for( i = 0 ; i < lightDefs.Num() ; i++ )
	{
		if( lightDefs[i] )
		{
			lightDefs[i]->archived = false;
		}
	}
	for( i = 0 ; i < entityDefs.Num() ; i++ )
	{
		if( entityDefs[i] )
		{
			entityDefs[i]->archived = false;
		}
	}
}

void idRenderWorldLocal::StopWritingDemo()
{
//	writeDemo = NULL;
}

/*
==============
ProcessDemoCommand
==============
*/
bool		idRenderWorldLocal::ProcessDemoCommand( idDemoFile* readDemo, renderView_t* renderView, int* demoTimeOffset )
{
	bool	newMap = false;
	
	if( !readDemo )
	{
		return false;
	}
	
	demoCommand_t	dc;
	qhandle_t		h;
	
	if( !readDemo->ReadInt( ( int& )dc ) )
	{
		// a demoShot may not have an endFrame, but it is still valid
		return false;
	}
	
	switch( dc )
	{
		case DC_LOADMAP:
			// read the initial data
			demoHeader_t	header;
			
			readDemo->ReadInt( header.version );
			readDemo->ReadInt( header.sizeofRenderEntity );
			readDemo->ReadInt( header.sizeofRenderLight );
			for( int i = 0; i < 256; i++ )
				readDemo->ReadChar( header.mapname[i] );
			// the internal version value got replaced by DS_VERSION at toplevel
			if( header.version != 4 )
			{
				common->Error( "Demo version mismatch.\n" );
			}
			
			if( r_showDemo.GetBool() )
			{
				common->Printf( "DC_LOADMAP: %s\n", header.mapname );
			}
			InitFromMap( header.mapname );
			
			newMap = true;		// we will need to set demoTimeOffset
			
			break;
			
		case DC_RENDERVIEW:
			readDemo->ReadInt( renderView->viewID );
			readDemo->ReadFloat( renderView->fov_x );
			readDemo->ReadFloat( renderView->fov_y );
			readDemo->ReadVec3( renderView->vieworg );
			readDemo->ReadMat3( renderView->viewaxis );
			readDemo->ReadBool( renderView->cramZNear );
			readDemo->ReadBool( renderView->forceUpdate );
			// binary compatibility with win32 padded structures
			char tmp;
			readDemo->ReadChar( tmp );
			readDemo->ReadChar( tmp );
			readDemo->ReadInt( renderView->time[1] );
			for( int i = 0; i < MAX_GLOBAL_SHADER_PARMS; i++ )
				readDemo->ReadFloat( renderView->shaderParms[i] );
				
			if( !readDemo->ReadInt( ( int& )renderView->globalMaterial ) )
			{
				return false;
			}
			
			if( r_showDemo.GetBool() )
			{
				common->Printf( "DC_RENDERVIEW: %i\n", renderView->time );
			}
			
			// possibly change the time offset if this is from a new map
			if( newMap && demoTimeOffset )
			{
				*demoTimeOffset = renderView->time[1] - eventLoop->Milliseconds();
			}
			return false;
			
		case DC_UPDATE_ENTITYDEF:
			ReadRenderEntity();
			break;
		case DC_DELETE_ENTITYDEF:
			if( !readDemo->ReadInt( h ) )
			{
				return false;
			}
			if( r_showDemo.GetBool() )
			{
				common->Printf( "DC_DELETE_ENTITYDEF: %i\n", h );
			}
			FreeEntityDef( h );
			break;
		case DC_UPDATE_LIGHTDEF:
			ReadRenderLight();
			break;
		case DC_DELETE_LIGHTDEF:
			if( !readDemo->ReadInt( h ) )
			{
				return false;
			}
			if( r_showDemo.GetBool() )
			{
				common->Printf( "DC_DELETE_LIGHTDEF: %i\n", h );
			}
			FreeLightDef( h );
			break;
			
		case DC_CAPTURE_RENDER:
			if( r_showDemo.GetBool() )
			{
				common->Printf( "DC_CAPTURE_RENDER\n" );
			}
			renderSystem->CaptureRenderToImage( readDemo->ReadHashString() );
			break;
			
		case DC_CROP_RENDER:
			if( r_showDemo.GetBool() )
			{
				common->Printf( "DC_CROP_RENDER\n" );
			}
			int	size[3];
			readDemo->ReadInt( size[0] );
			readDemo->ReadInt( size[1] );
			readDemo->ReadInt( size[2] );
			renderSystem->CropRenderSize( size[0], size[1] );
			break;
			
		case DC_UNCROP_RENDER:
			if( r_showDemo.GetBool() )
			{
				common->Printf( "DC_UNCROP\n" );
			}
			renderSystem->UnCrop();
			break;
			
		case DC_GUI_MODEL:
			if( r_showDemo.GetBool() )
			{
				common->Printf( "DC_GUI_MODEL\n" );
			}
			break;
			
		case DC_DEFINE_MODEL:
		{
			idRenderModel*	model = renderModelManager->AllocModel();
			model->ReadFromDemoFile( session->ReadDemo() );
			// add to model manager, so we can find it
			renderModelManager->AddModel( model );
			
			// save it in the list to free when clearing this map
			localModels.Append( model );
			
			if( r_showDemo.GetBool() )
			{
				common->Printf( "DC_DEFINE_MODEL\n" );
			}
			break;
		}
		case DC_SET_PORTAL_STATE:
		{
			int		data[2];
			readDemo->ReadInt( data[0] );
			readDemo->ReadInt( data[1] );
			SetPortalState( data[0], data[1] );
			if( r_showDemo.GetBool() )
			{
				common->Printf( "DC_SET_PORTAL_STATE: %i %i\n", data[0], data[1] );
			}
		}
		
		break;
		case DC_END_FRAME:
			return true;
			
		default:
			common->Error( "Bad token in demo stream" );
	}
	
	return false;
}

/*
================
WriteLoadMap
================
*/
void	idRenderWorldLocal::WriteLoadMap()
{

	// only the main renderWorld writes stuff to demos, not the wipes or
	// menu renders
	if( this != session->RW() )
	{
		return;
	}
	
	session->WriteDemo()->WriteInt( DS_RENDER );
	session->WriteDemo()->WriteInt( DC_LOADMAP );
	
	demoHeader_t	header;
	strncpy( header.mapname, mapName.c_str(), sizeof( header.mapname ) - 1 );
	header.version = 4;
	header.sizeofRenderEntity = sizeof( renderEntity_t );
	header.sizeofRenderLight = sizeof( renderLight_t );
	session->WriteDemo()->WriteInt( header.version );
	session->WriteDemo()->WriteInt( header.sizeofRenderEntity );
	session->WriteDemo()->WriteInt( header.sizeofRenderLight );
	for( int i = 0; i < 256; i++ )
		session->WriteDemo()->WriteChar( header.mapname[i] );
		
	if( r_showDemo.GetBool() )
	{
		common->Printf( "write DC_DELETE_LIGHTDEF: %s\n", mapName.c_str() );
	}
}

/*
================
WriteVisibleDefs

================
*/
void	idRenderWorldLocal::WriteVisibleDefs( const viewDef_t* viewDef )
{
	// only the main renderWorld writes stuff to demos, not the wipes or
	// menu renders
	if( this != session->RW() )
	{
		return;
	}
	
	// make sure all necessary entities and lights are updated
	for( viewEntity_t* viewEnt = viewDef->viewEntitys ; viewEnt ; viewEnt = viewEnt->next )
	{
		idRenderEntityLocal* ent = viewEnt->entityDef;
		
		if( ent->archived )
		{
			// still up to date
			continue;
		}
		
		// write it out
		WriteRenderEntity( ent->index, &ent->parms );
		ent->archived = true;
	}
	
	for( viewLight_t* viewLight = viewDef->viewLights ; viewLight ; viewLight = viewLight->next )
	{
		idRenderLightLocal* light = viewLight->lightDef;
		
		if( light->archived )
		{
			// still up to date
			continue;
		}
		// write it out
		WriteRenderLight( light->index, &light->parms );
		light->archived = true;
	}
}


/*
================
WriteRenderView
================
*/
void	idRenderWorldLocal::WriteRenderView( const renderView_t* renderView )
{
	int i;
	
	// only the main renderWorld writes stuff to demos, not the wipes or
	// menu renders
	if( this != session->RW() )
	{
		return;
	}
	
	// write the actual view command
	session->WriteDemo()->WriteInt( DS_RENDER );
	session->WriteDemo()->WriteInt( DC_RENDERVIEW );
	session->WriteDemo()->WriteInt( renderView->viewID );
	session->WriteDemo()->WriteFloat( renderView->fov_x );
	session->WriteDemo()->WriteFloat( renderView->fov_y );
	session->WriteDemo()->WriteVec3( renderView->vieworg );
	session->WriteDemo()->WriteMat3( renderView->viewaxis );
	session->WriteDemo()->WriteBool( renderView->cramZNear );
	session->WriteDemo()->WriteBool( renderView->forceUpdate );
	// binary compatibility with old win32 version writing padded structures directly to disk
	session->WriteDemo()->WriteUnsignedChar( 0 );
	session->WriteDemo()->WriteUnsignedChar( 0 );
	session->WriteDemo()->WriteInt( renderView->time[1] );
	for( i = 0; i < MAX_GLOBAL_SHADER_PARMS; i++ )
		session->WriteDemo()->WriteFloat( renderView->shaderParms[i] );
	session->WriteDemo()->WriteInt( ( int& )renderView->globalMaterial );
	
	if( r_showDemo.GetBool() )
	{
		common->Printf( "write DC_RENDERVIEW: %i\n", renderView->time );
	}
}

/*
================
WriteFreeEntity
================
*/
void	idRenderWorldLocal::WriteFreeEntity( qhandle_t handle )
{

	// only the main renderWorld writes stuff to demos, not the wipes or
	// menu renders
	if( this != session->RW() )
	{
		return;
	}
	
	session->WriteDemo()->WriteInt( DS_RENDER );
	session->WriteDemo()->WriteInt( DC_DELETE_ENTITYDEF );
	session->WriteDemo()->WriteInt( handle );
	
	if( r_showDemo.GetBool() )
	{
		common->Printf( "write DC_DELETE_ENTITYDEF: %i\n", handle );
	}
}

/*
================
WriteFreeLightEntity
================
*/
void	idRenderWorldLocal::WriteFreeLight( qhandle_t handle )
{

	// only the main renderWorld writes stuff to demos, not the wipes or
	// menu renders
	if( this != session->RW() )
	{
		return;
	}
	
	session->WriteDemo()->WriteInt( DS_RENDER );
	session->WriteDemo()->WriteInt( DC_DELETE_LIGHTDEF );
	session->WriteDemo()->WriteInt( handle );
	
	if( r_showDemo.GetBool() )
	{
		common->Printf( "write DC_DELETE_LIGHTDEF: %i\n", handle );
	}
}

/*
================
WriteRenderLight
================
*/
void	idRenderWorldLocal::WriteRenderLight( qhandle_t handle, const renderLight_t* light )
{

	// only the main renderWorld writes stuff to demos, not the wipes or
	// menu renders
	if( this != session->RW() )
	{
		return;
	}
	
	session->WriteDemo()->WriteInt( DS_RENDER );
	session->WriteDemo()->WriteInt( DC_UPDATE_LIGHTDEF );
	session->WriteDemo()->WriteInt( handle );
	
	session->WriteDemo()->WriteMat3( light->axis );
	session->WriteDemo()->WriteVec3( light->origin );
	session->WriteDemo()->WriteInt( light->suppressLightInViewID );
	session->WriteDemo()->WriteInt( light->allowLightInViewID );
	session->WriteDemo()->WriteBool( light->noShadows );
	session->WriteDemo()->WriteBool( light->noSpecular );
	session->WriteDemo()->WriteBool( light->pointLight );
	session->WriteDemo()->WriteBool( light->parallel );
	session->WriteDemo()->WriteVec3( light->lightRadius );
	session->WriteDemo()->WriteVec3( light->lightCenter );
	session->WriteDemo()->WriteVec3( light->target );
	session->WriteDemo()->WriteVec3( light->right );
	session->WriteDemo()->WriteVec3( light->up );
	session->WriteDemo()->WriteVec3( light->start );
	session->WriteDemo()->WriteVec3( light->end );
	session->WriteDemo()->WriteInt( ( int& )light->prelightModel );
	session->WriteDemo()->WriteInt( light->lightId );
	session->WriteDemo()->WriteInt( ( int& )light->shader );
	for( int i = 0; i < MAX_ENTITY_SHADER_PARMS; i++ )
		session->WriteDemo()->WriteFloat( light->shaderParms[i] );
	session->WriteDemo()->WriteInt( ( int& )light->referenceSound );
	
	if( light->prelightModel )
	{
		session->WriteDemo()->WriteHashString( light->prelightModel->Name() );
	}
	if( light->shader )
	{
		session->WriteDemo()->WriteHashString( light->shader->GetName() );
	}
	if( light->referenceSound )
	{
		int	index = light->referenceSound->Index();
		session->WriteDemo()->WriteInt( index );
	}
	
	if( r_showDemo.GetBool() )
	{
		common->Printf( "write DC_UPDATE_LIGHTDEF: %i\n", handle );
	}
}

/*
================
ReadRenderLight
================
*/
void	idRenderWorldLocal::ReadRenderLight( )
{
	renderLight_t	light;
	int				index;
	
	session->ReadDemo()->ReadInt( index );
	if( index < 0 )
	{
		common->Error( "ReadRenderLight: index < 0 " );
	}
	
	session->ReadDemo()->ReadMat3( light.axis );
	session->ReadDemo()->ReadVec3( light.origin );
	session->ReadDemo()->ReadInt( light.suppressLightInViewID );
	session->ReadDemo()->ReadInt( light.allowLightInViewID );
	session->ReadDemo()->ReadBool( light.noShadows );
	session->ReadDemo()->ReadBool( light.noSpecular );
	session->ReadDemo()->ReadBool( light.pointLight );
	session->ReadDemo()->ReadBool( light.parallel );
	session->ReadDemo()->ReadVec3( light.lightRadius );
	session->ReadDemo()->ReadVec3( light.lightCenter );
	session->ReadDemo()->ReadVec3( light.target );
	session->ReadDemo()->ReadVec3( light.right );
	session->ReadDemo()->ReadVec3( light.up );
	session->ReadDemo()->ReadVec3( light.start );
	session->ReadDemo()->ReadVec3( light.end );
	session->ReadDemo()->ReadInt( ( int& )light.prelightModel );
	session->ReadDemo()->ReadInt( light.lightId );
	session->ReadDemo()->ReadInt( ( int& )light.shader );
	for( int i = 0; i < MAX_ENTITY_SHADER_PARMS; i++ )
		session->ReadDemo()->ReadFloat( light.shaderParms[i] );
	session->ReadDemo()->ReadInt( ( int& )light.referenceSound );
	if( light.prelightModel )
	{
		light.prelightModel = renderModelManager->FindModel( session->ReadDemo()->ReadHashString() );
	}
	if( light.shader )
	{
		light.shader = declManager->FindMaterial( session->ReadDemo()->ReadHashString() );
	}
	if( light.referenceSound )
	{
		int	index;
		session->ReadDemo()->ReadInt( index );
		light.referenceSound = session->SW()->EmitterForIndex( index );
	}
	
	UpdateLightDef( index, &light );
	
	if( r_showDemo.GetBool() )
	{
		common->Printf( "DC_UPDATE_LIGHTDEF: %i\n", index );
	}
}

/*
================
WriteRenderEntity
================
*/
void	idRenderWorldLocal::WriteRenderEntity( qhandle_t handle, const renderEntity_t* ent )
{

	// only the main renderWorld writes stuff to demos, not the wipes or
	// menu renders
	if( this != session->RW() )
	{
		return;
	}
	
	session->WriteDemo()->WriteInt( DS_RENDER );
	session->WriteDemo()->WriteInt( DC_UPDATE_ENTITYDEF );
	session->WriteDemo()->WriteInt( handle );
	
	session->WriteDemo()->WriteInt( ( int& )ent->hModel );
	session->WriteDemo()->WriteInt( ent->entityNum );
	session->WriteDemo()->WriteInt( ent->bodyId );
	session->WriteDemo()->WriteVec3( ent->bounds[0] );
	session->WriteDemo()->WriteVec3( ent->bounds[1] );
	session->WriteDemo()->WriteInt( ( int& )ent->callback );
	session->WriteDemo()->WriteInt( ( int& )ent->callbackData );
	session->WriteDemo()->WriteInt( ent->suppressSurfaceInViewID );
	session->WriteDemo()->WriteInt( ent->suppressShadowInViewID );
	session->WriteDemo()->WriteInt( ent->suppressShadowInLightID );
	session->WriteDemo()->WriteInt( ent->allowSurfaceInViewID );
	session->WriteDemo()->WriteVec3( ent->origin );
	session->WriteDemo()->WriteMat3( ent->axis );
	session->WriteDemo()->WriteInt( ( int& )ent->customShader );
	session->WriteDemo()->WriteInt( ( int& )ent->referenceShader );
	session->WriteDemo()->WriteInt( ( int& )ent->customSkin );
	session->WriteDemo()->WriteInt( ( int& )ent->referenceSound );
	for( int i = 0; i < MAX_ENTITY_SHADER_PARMS; i++ )
		session->WriteDemo()->WriteFloat( ent->shaderParms[i] );
	for( int i = 0; i < MAX_RENDERENTITY_GUI; i++ )
		session->WriteDemo()->WriteInt( ( int& )ent->gui[i] );
	session->WriteDemo()->WriteInt( ( int& )ent->remoteRenderView );
	session->WriteDemo()->WriteInt( ent->numJoints );
	session->WriteDemo()->WriteInt( ( int& )ent->joints );
	session->WriteDemo()->WriteFloat( ent->modelDepthHack );
	session->WriteDemo()->WriteBool( ent->noSelfShadow );
	session->WriteDemo()->WriteBool( ent->noShadow );
	session->WriteDemo()->WriteBool( ent->noDynamicInteractions );
	session->WriteDemo()->WriteBool( ent->weaponDepthHack );
	session->WriteDemo()->WriteInt( ent->forceUpdate );
	
	if( ent->customShader )
	{
		session->WriteDemo()->WriteHashString( ent->customShader->GetName() );
	}
	if( ent->customSkin )
	{
		session->WriteDemo()->WriteHashString( ent->customSkin->GetName() );
	}
	if( ent->hModel )
	{
		session->WriteDemo()->WriteHashString( ent->hModel->Name() );
	}
	if( ent->referenceShader )
	{
		session->WriteDemo()->WriteHashString( ent->referenceShader->GetName() );
	}
	if( ent->referenceSound )
	{
		int	index = ent->referenceSound->Index();
		session->WriteDemo()->WriteInt( index );
	}
	if( ent->numJoints )
	{
		for( int i = 0; i < ent->numJoints; i++ )
		{
			float* data = ent->joints[i].ToFloatPtr();
			for( int j = 0; j < 12; ++j )
				session->WriteDemo()->WriteFloat( data[j] );
		}
	}
	
	/*
	if ( ent->decals ) {
		ent->decals->WriteToDemoFile( session->ReadDemo() );
	}
	if ( ent->overlays ) {
		ent->overlays->WriteToDemoFile( session->WriteDemo() );
	}
	*/
	
#ifdef WRITE_GUIS
	if( ent->gui )
	{
		ent->gui->WriteToDemoFile( session->WriteDemo() );
	}
	if( ent->gui2 )
	{
		ent->gui2->WriteToDemoFile( session->WriteDemo() );
	}
	if( ent->gui3 )
	{
		ent->gui3->WriteToDemoFile( session->WriteDemo() );
	}
#endif
	
	// RENDERDEMO_VERSION >= 2 ( Doom3 1.2 )
	session->WriteDemo()->WriteInt( ent->timeGroup );
	session->WriteDemo()->WriteInt( ent->xrayIndex );
	
	if( r_showDemo.GetBool() )
	{
		common->Printf( "write DC_UPDATE_ENTITYDEF: %i = %s\n", handle, ent->hModel ? ent->hModel->Name() : "NULL" );
	}
}

/*
================
ReadRenderEntity
================
*/
void	idRenderWorldLocal::ReadRenderEntity()
{
	renderEntity_t		ent;
	int				index, i;
	
	session->ReadDemo()->ReadInt( index );
	if( index < 0 )
	{
		common->Error( "ReadRenderEntity: index < 0" );
	}
	
	session->ReadDemo()->ReadInt( ( int& )ent.hModel );
	session->ReadDemo()->ReadInt( ent.entityNum );
	session->ReadDemo()->ReadInt( ent.bodyId );
	session->ReadDemo()->ReadVec3( ent.bounds[0] );
	session->ReadDemo()->ReadVec3( ent.bounds[1] );
	session->ReadDemo()->ReadInt( ( int& )ent.callback );
	session->ReadDemo()->ReadInt( ( int& )ent.callbackData );
	session->ReadDemo()->ReadInt( ent.suppressSurfaceInViewID );
	session->ReadDemo()->ReadInt( ent.suppressShadowInViewID );
	session->ReadDemo()->ReadInt( ent.suppressShadowInLightID );
	session->ReadDemo()->ReadInt( ent.allowSurfaceInViewID );
	session->ReadDemo()->ReadVec3( ent.origin );
	session->ReadDemo()->ReadMat3( ent.axis );
	session->ReadDemo()->ReadInt( ( int& )ent.customShader );
	session->ReadDemo()->ReadInt( ( int& )ent.referenceShader );
	session->ReadDemo()->ReadInt( ( int& )ent.customSkin );
	session->ReadDemo()->ReadInt( ( int& )ent.referenceSound );
	for( i = 0; i < MAX_ENTITY_SHADER_PARMS; i++ )
	{
		session->ReadDemo()->ReadFloat( ent.shaderParms[i] );
	}
	for( i = 0; i < MAX_RENDERENTITY_GUI; i++ )
	{
		session->ReadDemo()->ReadInt( ( int& )ent.gui[i] );
	}
	session->ReadDemo()->ReadInt( ( int& )ent.remoteRenderView );
	session->ReadDemo()->ReadInt( ent.numJoints );
	session->ReadDemo()->ReadInt( ( int& )ent.joints );
	session->ReadDemo()->ReadFloat( ent.modelDepthHack );
	session->ReadDemo()->ReadBool( ent.noSelfShadow );
	session->ReadDemo()->ReadBool( ent.noShadow );
	session->ReadDemo()->ReadBool( ent.noDynamicInteractions );
	session->ReadDemo()->ReadBool( ent.weaponDepthHack );
	session->ReadDemo()->ReadInt( ent.forceUpdate );
	ent.callback = NULL;
	if( ent.customShader )
	{
		ent.customShader = declManager->FindMaterial( session->ReadDemo()->ReadHashString() );
	}
	if( ent.customSkin )
	{
		ent.customSkin = declManager->FindSkin( session->ReadDemo()->ReadHashString() );
	}
	if( ent.hModel )
	{
		ent.hModel = renderModelManager->FindModel( session->ReadDemo()->ReadHashString() );
	}
	if( ent.referenceShader )
	{
		ent.referenceShader = declManager->FindMaterial( session->ReadDemo()->ReadHashString() );
	}
	if( ent.referenceSound )
	{
		int	index;
		session->ReadDemo()->ReadInt( index );
		ent.referenceSound = session->SW()->EmitterForIndex( index );
	}
	if( ent.numJoints )
	{
		ent.joints = ( idJointMat* )Mem_Alloc16( SIMD_ROUND_JOINTS( ent.numJoints ) * sizeof( ent.joints[0] ) );
		for( int i = 0; i < ent.numJoints; i++ )
		{
			float* data = ent.joints[i].ToFloatPtr();
			for( int j = 0; j < 12; ++j )
			{
				session->ReadDemo()->ReadFloat( data[j] );
			}
		}
		SIMD_INIT_LAST_JOINT( ent.joints, ent.numJoints );
	}
	
	ent.callbackData = NULL;
	
	/*
	if ( ent.decals ) {
		ent.decals = idRenderModelDecal::Alloc();
		ent.decals->ReadFromDemoFile( session->ReadDemo() );
	}
	if ( ent.overlays ) {
		ent.overlays = idRenderModelOverlay::Alloc();
		ent.overlays->ReadFromDemoFile( session->ReadDemo() );
	}
	*/
	
	for( i = 0; i < MAX_RENDERENTITY_GUI; i++ )
	{
		if( ent.gui[ i ] )
		{
			ent.gui[ i ] = uiManager->Alloc();
#ifdef WRITE_GUIS
			ent.gui[ i ]->ReadFromDemoFile( session->ReadDemo() );
#endif
		}
	}
	
	session->ReadDemo()->ReadInt( ent.timeGroup );
	session->ReadDemo()->ReadInt( ent.xrayIndex );
	
	UpdateEntityDef( index, &ent );
	
	if( r_showDemo.GetBool() )
	{
		common->Printf( "DC_UPDATE_ENTITYDEF: %i = %s\n", index, ent.hModel ? ent.hModel->Name() : "NULL" );
	}
}
