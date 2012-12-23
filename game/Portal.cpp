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
/*

  Portal.cpp

  Dynamic game portal

*/

#include "precompiled.h"
#pragma hdrstop

#include "Game_local.h"


/***********************************************************************

  tyPortal

***********************************************************************/

const idEventDef EV_Portal_AddLight( "<addLight>" );

CLASS_DECLARATION( idEntity, tyPortal )
EVENT( EV_Portal_AddLight,			tyPortal::Event_AddLight )
EVENT( EV_Touch,					tyPortal::Event_Touch )
END_CLASS

/*
================
tyPortal::Save
================
*/
void tyPortal::Save( idSaveGame* savefile ) const
{
	owner.Save( savefile );
	destinationPortal.Save( savefile );
	
	savefile->WriteInt( modelAxis );
	savefile->WriteBool( flipAxis );
	savefile->WriteFloat( scanDist );
	savefile->WriteFloat( scanFov );
	savefile->WriteFloat( scanFovCos );
	
	savefile->WriteVec3( viewOffset );
	
	savefile->WriteInt( pvsArea );
	savefile->WriteStaticObject( physicsObj );
	//savefile->WriteTraceModel( trm );
	
	savefile->WriteInt( teleportStage );
}

/*
================
tyPortal::Restore
================
*/
void tyPortal::Restore( idRestoreGame* savefile )
{
	owner.Restore( savefile );
	destinationPortal.Restore( savefile );
	
	savefile->ReadInt( modelAxis );
	savefile->ReadBool( flipAxis );
	savefile->ReadFloat( scanDist );
	savefile->ReadFloat( scanFov );
	savefile->ReadFloat( scanFovCos );
	
	savefile->ReadVec3( viewOffset );
	
	savefile->ReadInt( pvsArea );
	savefile->ReadStaticObject( physicsObj );
	//savefile->ReadTraceModel( trm );
	
	savefile->ReadInt( teleportStage );
}

/*
================
tyPortal::Spawn
================
*/
void tyPortal::Spawn()
{
	idStr	str;
	
	health		= spawnArgs.GetInt( "health", "100" );
	scanFov		= spawnArgs.GetFloat( "scanFov", "90" );
	scanDist	= spawnArgs.GetFloat( "scanDist", "200" );
	flipAxis	= spawnArgs.GetBool( "flipAxis" );
	
	modelAxis	= spawnArgs.GetInt( "modelAxis" );
	if( modelAxis < 0 || modelAxis > 2 )
	{
		modelAxis = 0;
	}
	
	spawnArgs.GetVector( "viewOffset", "0 0 0", viewOffset );
	
	if( spawnArgs.GetBool( "spotLight" ) )
	{
		PostEventMS( &EV_Portal_AddLight, 0 );
	}
	
	scanFovCos = cos( scanFov * idMath::PI / 360.0f );
	BecomeActive( TH_THINK );
	
	if( health )
	{
		fl.takedamage = true;
	}
	
	pvsArea = gameLocal.pvs.GetPVSArea( GetPhysics()->GetOrigin() );
	
	// if no target specified use ourself
	str = spawnArgs.GetString( "cameraTarget" );
	if( str.Length() == 0 )
	{
		const char* name = spawnArgs.GetString( "name" );
		spawnArgs.Set( "cameraTarget", name );
	}
	
	// check if a clip model is set
	spawnArgs.GetString( "clipmodel", "", str );
	if( !str[0] )
	{
		str = spawnArgs.GetString( "model" );		// use the visual model
	}
	
#if 0
	if( !collisionModelManager->TrmFromModel( str, trm ) )
	{
		gameLocal.Error( "tyPortal '%s': cannot load collision model %s", name.c_str(), str.c_str() );
		return;
	}
#endif
	
	GetPhysics()->GetClipModel()->Link( gameLocal.clip );
	
	GetPhysics()->SetContents( CONTENTS_TRIGGER );
	//GetPhysics()->SetClipMask( MASK_SOLID | CONTENTS_BODY | CONTENTS_CORPSE | CONTENTS_MOVEABLECLIP );
	
	// TRIGGER -------------
	spawnArgs.GetFloat( "wait", "0.5", wait );
	spawnArgs.GetFloat( "delay", "0.5", delay );
	nextTriggerTime = 0;
	// TRIGGER -------------
	
	// TELEPORTER ----------
	teleportStage = 0;
	// TELEPORTER ----------
	
	
	// setup the physics
	UpdateChangeableSpawnArgs( NULL );
}


void tyPortal::Create( idEntity* owner, tyPortal* otherPortal, const idVec3& start, const idMat3& axis )
{
	Unbind();
	
	GetPhysics()->SetOrigin( start );
	GetPhysics()->SetAxis( axis );
	
	this->owner = owner;
	
	if( otherPortal )
	{
		destinationPortal = otherPortal;
		
		otherPortal->destinationPortal = this;
		otherPortal->spawnArgs.Set( "cameraTarget", otherPortal->GetName() );
		otherPortal->UpdateChangeableSpawnArgs( NULL );
	}
	
	UpdateChangeableSpawnArgs( NULL );
	UpdateVisuals();
}

/*
================
tyPortal::Event_AddLight
================
*/
void tyPortal::Event_AddLight()
{
	idDict	args;
	idVec3	right, up, target, temp;
	idVec3	dir;
	float	radius;
	idVec3	lightOffset;
	idLight*	spotLight;
	
	dir = GetAxis();
	dir.NormalVectors( right, up );
	target = GetPhysics()->GetOrigin() + dir * scanDist;
	
	radius = tan( scanFov * idMath::PI / 360.0f );
	up = dir + up * radius;
	up.Normalize();
	up = GetPhysics()->GetOrigin() + up * scanDist;
	up -= target;
	
	right = dir + right * radius;
	right.Normalize();
	right = GetPhysics()->GetOrigin() + right * scanDist;
	right -= target;
	
	spawnArgs.GetVector( "lightOffset", "0 0 0", lightOffset );
	
	args.Set( "origin", ( GetPhysics()->GetOrigin() + lightOffset ).ToString() );
	args.Set( "light_target", target.ToString() );
	args.Set( "light_right", right.ToString() );
	args.Set( "light_up", up.ToString() );
	args.SetFloat( "angle", GetPhysics()->GetAxis()[0].ToYaw() );
	
	spotLight = static_cast<idLight*>( gameLocal.SpawnEntityType( idLight::Type, &args ) );
	spotLight->Bind( this, true );
	spotLight->UpdateVisuals();
}

/*
================
tyPortal::DrawFov
================
*/
void tyPortal::DrawFov()
{
	int i;
	float radius, a, s, c, halfRadius;
	idVec3 right, up;
	idVec4 color( 1, 0, 0, 1 ), color2( 0, 0, 1, 1 );
	idVec3 lastPoint, point, lastHalfPoint, halfPoint, center;
	
	idVec3 dir = GetAxis();
	dir.NormalVectors( right, up );
	
	radius = tan( scanFov * idMath::PI / 360.0f );
	halfRadius = radius * 0.5f;
	lastPoint = dir + up * radius;
	lastPoint.Normalize();
	lastPoint = GetPhysics()->GetOrigin() + lastPoint * scanDist;
	lastHalfPoint = dir + up * halfRadius;
	lastHalfPoint.Normalize();
	lastHalfPoint = GetPhysics()->GetOrigin() + lastHalfPoint * scanDist;
	center = GetPhysics()->GetOrigin() + dir * scanDist;
	for( i = 1; i < 12; i++ )
	{
		a = idMath::TWO_PI * i / 12.0f;
		idMath::SinCos( a, s, c );
		point = dir + right * s * radius + up * c * radius;
		point.Normalize();
		point = GetPhysics()->GetOrigin() + point * scanDist;
		gameRenderWorld->DebugLine( color, lastPoint, point );
		gameRenderWorld->DebugLine( color, GetPhysics()->GetOrigin(), point );
		lastPoint = point;
		
		halfPoint = dir + right * s * halfRadius + up * c * halfRadius;
		halfPoint.Normalize();
		halfPoint = GetPhysics()->GetOrigin() + halfPoint * scanDist;
		gameRenderWorld->DebugLine( color2, point, halfPoint );
		gameRenderWorld->DebugLine( color2, lastHalfPoint, halfPoint );
		lastHalfPoint = halfPoint;
		
		gameRenderWorld->DebugLine( color2, halfPoint, center );
	}
}

/*
================
tyPortal::GetRenderView
================
*/
renderView_t* tyPortal::GetRenderView()
{
	renderView_t* rv = NULL;
	
	/*
	if ( owner.GetEntity() && owner.GetEntity()->IsType( idPlayer::Type ) )
	{
		idPlayer *player = static_cast<idPlayer*>( owner.GetEntity() );
	
	
		return player->GetRenderView();
	}
	*/
	
	if( destinationPortal.GetEntity() )
	{
		tyPortal* otherPortal = static_cast<tyPortal*>( destinationPortal.GetEntity() );
		
		rv = idEntity::GetRenderView();
		rv->fov_x = scanFov;
		rv->fov_y = scanFov;
		rv->viewaxis = otherPortal->GetAxis().ToAngles().ToMat3();
		rv->vieworg = otherPortal->GetPhysics()->GetOrigin() + otherPortal->viewOffset;
	}
	else
	{
		rv = idEntity::GetRenderView();
		rv->fov_x = scanFov;
		rv->fov_y = scanFov;
		rv->viewaxis = GetAxis().ToAngles().ToMat3();
		rv->vieworg = GetPhysics()->GetOrigin() + viewOffset;
	}
	
	return rv;
}

/*
================
tyPortal::CanSeePlayer
================
*/
bool tyPortal::CanSeePlayer()
{
	int i;
	float dist;
	idPlayer* ent;
	trace_t tr;
	idVec3 dir;
	pvsHandle_t handle;
	
	handle = gameLocal.pvs.SetupCurrentPVS( pvsArea );
	
	for( i = 0; i < gameLocal.numClients; i++ )
	{
		ent = static_cast<idPlayer*>( gameLocal.entities[ i ] );
		
		if( !ent || ( ent->fl.notarget ) )
		{
			continue;
		}
		
		// if there is no way we can see this player
		if( !gameLocal.pvs.InCurrentPVS( handle, ent->GetPVSAreas(), ent->GetNumPVSAreas() ) )
		{
			continue;
		}
		
		dir = ent->GetPhysics()->GetOrigin() - GetPhysics()->GetOrigin();
		dist = dir.Normalize();
		
		if( dist > scanDist )
		{
			continue;
		}
		
		if( dir * GetAxis() < scanFovCos )
		{
			continue;
		}
		
		idVec3 eye;
		
		eye = ent->EyeOffset();
		
		gameLocal.clip.TracePoint( tr, GetPhysics()->GetOrigin(), ent->GetPhysics()->GetOrigin() + eye, MASK_OPAQUE, this );
		if( tr.fraction == 1.0 || ( gameLocal.GetTraceEntity( tr ) == ent ) )
		{
			gameLocal.pvs.FreeCurrentPVS( handle );
			return true;
		}
	}
	
	gameLocal.pvs.FreeCurrentPVS( handle );
	
	return false;
}


/*
================
tyPortal::Think
================
*/
void tyPortal::Think()
{
	float pct;
	float travel;
	
	if( thinkFlags & TH_THINK )
	{
		if( g_showEntityInfo.GetBool() )
		{
			DrawFov();
		}
		
		if( health <= 0 )
		{
			BecomeInactive( TH_THINK );
			return;
		}
	}
	
	// run physics
	RunPhysics();
	
	if( thinkFlags & TH_THINK )
	{
		if( CanSeePlayer() )
		{
		
		}
		else
		{
		
		}
	}
	Present();
}

/*
================
tyPortal::GetAxis
================
*/
const idVec3 tyPortal::GetAxis() const
{
	return ( flipAxis ) ? -GetPhysics()->GetAxis()[modelAxis] : GetPhysics()->GetAxis()[modelAxis];
};



/*
============
tyPortal::Killed
============
*/
void tyPortal::Killed( idEntity* inflictor, idEntity* attacker, int damage, const idVec3& dir, int location )
{

	StopSound( SND_CHANNEL_ANY, false );
	const char* fx = spawnArgs.GetString( "fx_destroyed" );
	if( fx[0] != '\0' )
	{
		idEntityFx::StartFx( fx, NULL, NULL, this, true );
	}
	
	physicsObj.SetSelf( this );
//	physicsObj.SetClipModel( new idClipModel( trm ), 0.02f );
	physicsObj.SetOrigin( GetPhysics()->GetOrigin() );
	physicsObj.SetAxis( GetPhysics()->GetAxis() );
	physicsObj.SetBouncyness( 0.2f );
	physicsObj.SetFriction( 0.6f, 0.6f, 0.2f );
	physicsObj.SetGravity( gameLocal.GetGravity() );
	physicsObj.SetContents( CONTENTS_SOLID );
	physicsObj.SetClipMask( MASK_SOLID | CONTENTS_BODY | CONTENTS_CORPSE | CONTENTS_MOVEABLECLIP );
	SetPhysics( &physicsObj );
	physicsObj.DropToFloor();
}


/*
============
tyPortal::Pain
============
*/
bool tyPortal::Pain( idEntity* inflictor, idEntity* attacker, int damage, const idVec3& dir, int location )
{
	const char* fx = spawnArgs.GetString( "fx_damage" );
	if( fx[0] != '\0' )
	{
		idEntityFx::StartFx( fx, NULL, NULL, this, true );
	}
	return true;
}


/*
================
tyPortal::Present

Present is called to allow entities to generate refEntities, lights, etc for the renderer.
================
*/
void tyPortal::Present()
{
#if 1
	// don't present to the renderer if the entity hasn't changed
	if( !( thinkFlags & TH_UPDATEVISUALS ) )
	{
		return;
	}
	BecomeInactive( TH_UPDATEVISUALS );
#endif
	
	// camera target for remote render views
	if( cameraTarget )
	{
		renderEntity.remoteRenderView = GetRenderView();
	}
	
	// if set to invisible, skip
	if( !renderEntity.hModel || IsHidden() )
	{
		return;
	}
	
	// add to refresh list
	if( modelDefHandle == -1 )
	{
		modelDefHandle = gameRenderWorld->AddEntityDef( &renderEntity );
	}
	else
	{
		gameRenderWorld->UpdateEntityDef( modelDefHandle, &renderEntity );
	}
}


/*
================
tyPortal::CheckFacing
================
*/
bool tyPortal::CheckFacing( idEntity* activator )
{
	if( spawnArgs.GetBool( "facing" ) )
	{
		if( !activator->IsType( idPlayer::Type ) )
		{
			return true;
		}
		idPlayer* player = static_cast< idPlayer* >( activator );
		float dot = player->viewAngles.ToForward() * GetPhysics()->GetAxis()[0];
		float angle = RAD2DEG( idMath::ACos( dot ) );
		if( angle  > spawnArgs.GetFloat( "angleLimit", "30" ) )
		{
			return false;
		}
	}
	return true;
}

/*
================
tyPortal::Event_Touch
================
*/
void tyPortal::Event_Touch( idEntity* other, trace_t* trace )
{
	//if( triggerFirst ) {
	//	return;
	//}
	
	bool isPlayer = other->IsType( idPlayer::Type );
	idPlayer* player = static_cast< idPlayer* >( other );
	if( isPlayer )
	{
		//if ( !touchClient ) {
		//	return;
		//}
		if( static_cast< idPlayer* >( other )->spectating )
		{
			return;
		}
	}
	//else if ( !touchOther ) {
	//	return;
	//}
	
	if( nextTriggerTime > gameLocal.time || ( destinationPortal.GetEntity() && destinationPortal.GetEntity()->nextTriggerTime > gameLocal.time ) )
	{
		// can't retrigger until the wait is over
		return;
	}
	
	// see if this trigger requires an item
	if( !gameLocal.RequirementMet( other, requires, removeItem ) )
	{
		return;
	}
	
	if( !CheckFacing( other ) )
	{
		return;
	}
	
	//if ( spawnArgs.GetBool( "toggleTriggerFirst" ) ) {
	//	triggerFirst = true;
	//}
	
	nextTriggerTime = gameLocal.time + SEC2MS( delay );
	/*
	if ( delay > 0 ) {
		// don't allow it to trigger again until our delay has passed
		nextTriggerTime += SEC2MS( delay );
		PostEventSec( &EV_TriggerAction, delay, other );
	} else {
	*/
	if( isPlayer && destinationPortal.GetEntity() )
	{
		destinationPortal.GetEntity()->TeleportPlayer( player );
	}
}


/*
================
tyPortal::ClientReceiveEvent
================
*/
bool tyPortal::ClientReceiveEvent( int event, int time, const idBitMsg& msg )
{
	int entityNumber;
	
	switch( event )
	{
		case EVENT_TELEPORTPLAYER:
		{
			entityNumber = msg.ReadBits( GENTITYNUM_BITS );
			idPlayer* player = static_cast<idPlayer*>( gameLocal.entities[entityNumber] );
			if( player != NULL && player->IsType( idPlayer::Type ) )
			{
				Event_TeleportPlayer( player );
			}
			return true;
		}
		default:
		{
			return idEntity::ClientReceiveEvent( event, time, msg );
		}
	}
	return false;
}

/*
===============
tyPortal::Event_TeleportStage

FIXME: add functionality to fx system ( could be done with player scripting too )
================
*/
/*
void tyPortal::Event_TeleportStage( idEntity *_player ) {
	idPlayer *player;
	if ( !_player->IsType( idPlayer::Type ) ) {
		common->Warning( "idPlayerStart::Event_TeleportStage: entity is not an idPlayer\n" );
		return;
	}
	player = static_cast<idPlayer*>(_player);
	float teleportDelay = spawnArgs.GetFloat( "teleportDelay" );
	switch ( teleportStage ) {
		case 0:
			player->playerView.Flash( colorWhite, 125 );
			player->SetInfluenceLevel( INFLUENCE_LEVEL3 );
			player->SetInfluenceView( spawnArgs.GetString( "mtr_teleportFx" ), NULL, 0.0f, NULL );
			gameSoundWorld->FadeSoundClasses( 0, -20.0f, teleportDelay );
			player->StartSound( "snd_teleport_start", SND_CHANNEL_BODY2, 0, false, NULL );
			teleportStage++;
			PostEventSec( &EV_TeleportStage, teleportDelay, player );
			break;
		case 1:
			gameSoundWorld->FadeSoundClasses( 0, 0.0f, 0.25f );
			teleportStage++;
			PostEventSec( &EV_TeleportStage, 0.25f, player );
			break;
		case 2:
			player->SetInfluenceView( NULL, NULL, 0.0f, NULL );
			TeleportPlayer( player );
			player->StopSound( SND_CHANNEL_BODY2, false );
			player->SetInfluenceLevel( INFLUENCE_NONE );
			teleportStage = 0;
			break;
		default:
			break;
	}
}
*/

/*
===============
tyPortal::TeleportPlayer
================
*/
void tyPortal::TeleportPlayer( idPlayer* player )
{
	float pushVel = spawnArgs.GetFloat( "push", "300" );
	float f = spawnArgs.GetFloat( "visualEffect", "0" );
	const char* viewName = spawnArgs.GetString( "visualView", "" );
	idEntity* ent = viewName ? gameLocal.FindEntity( viewName ) : NULL;
	
	if( f && ent )
	{
		// place in private camera view for some time
		// the entity needs to teleport to where the camera view is to have the PVS right
		player->Teleport( ent->GetPhysics()->GetOrigin(), ang_zero, this );
		player->StartSound( "snd_teleport_enter", SND_CHANNEL_ANY, 0, false, NULL );
		player->SetPrivateCameraView( static_cast<idCamera*>( ent ) );
		
		// the player entity knows where to spawn from the previous Teleport call
		if( !gameLocal.isClient )
		{
			player->PostEventSec( &EV_Player_ExitTeleporter, f );
		}
	}
	else
	{
		// direct to exit, Teleport will take care of the killbox
		player->Teleport( GetPhysics()->GetOrigin(), GetPhysics()->GetAxis().ToAngles(), NULL );
		
		// multiplayer hijacked this entity, so only push the player in multiplayer
		//if ( gameLocal.isMultiplayer ) {
		player->GetPhysics()->SetLinearVelocity( GetPhysics()->GetAxis()[0] * pushVel );
		//}
	}
}

/*
===============
tyPortal::Event_TeleportPlayer
================
*/
void tyPortal::Event_TeleportPlayer( idEntity* activator )
{
	idPlayer* player;
	
	if( activator->IsType( idPlayer::Type ) )
	{
		player = static_cast<idPlayer*>( activator );
	}
	else
	{
		player = gameLocal.GetLocalPlayer();
	}
	if( player )
	{
		/*
		if ( spawnArgs.GetBool( "visualFx" ) ) {
		
			teleportStage = 0;
			Event_TeleportStage( player );
		
		} else
		*/
		{
		
			if( gameLocal.isServer )
			{
				idBitMsg	msg;
				byte		msgBuf[MAX_EVENT_PARAM_SIZE];
				
				msg.Init( msgBuf, sizeof( msgBuf ) );
				msg.BeginWriting();
				msg.WriteBits( player->entityNumber, GENTITYNUM_BITS );
				ServerSendEvent( EVENT_TELEPORTPLAYER, &msg, false, -1 );
			}
			
			TeleportPlayer( player );
		}
	}
}