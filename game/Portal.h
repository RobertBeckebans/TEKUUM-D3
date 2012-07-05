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

#ifndef __GAME_PORTAL_H__
#define __GAME_PORTAL_H__

/*
===================================================================================

	PORTAL

===================================================================================
*/


class tyPortal : public idEntity
{
public:
	CLASS_PROTOTYPE( tyPortal );
	
	enum
	{
		EVENT_TELEPORTPLAYER = idEntity::EVENT_MAXEVENTS,
		EVENT_MAXEVENTS
	};
	
	void					Spawn( void );
	
	void					Create( idEntity* owner, tyPortal* otherPortal, const idVec3& start, const idMat3& axis );
	
	
	void					Save( idSaveGame* savefile ) const;
	void					Restore( idRestoreGame* savefile );
	
	virtual void			Think( void );
	
	virtual renderView_t* 	GetRenderView();
	virtual void			Killed( idEntity* inflictor, idEntity* attacker, int damage, const idVec3& dir, int location );
	virtual bool			Pain( idEntity* inflictor, idEntity* attacker, int damage, const idVec3& dir, int location );
	virtual void			Present( void );
	
	virtual bool			ClientReceiveEvent( int event, int time, const idBitMsg& msg );
	
private:
	idEntityPtr<idEntity>	owner;
	idEntityPtr<tyPortal>	destinationPortal;
	
	int						modelAxis;
	bool					flipAxis;
	float					scanDist;
	float					scanFov;
	float					scanFovCos;
	
	idVec3					viewOffset;
	
	int						pvsArea;
	idPhysics_RigidBody		physicsObj;
//	idTraceModel			trm;

	bool					CanSeePlayer( void );
	void					DrawFov( void );
	const idVec3			GetAxis( void ) const;
	
	float					wait;
	float					delay;
	int						nextTriggerTime;
	idStr					requires;
	int						removeItem;
	
	bool					CheckFacing( idEntity* activator );
	void					Event_Touch( idEntity* other, trace_t* trace );
	
	
	int						teleportStage;
	
	void					Event_TeleportPlayer( idEntity* activator );
	void					Event_TeleportStage( idEntity* player );
	void					TeleportPlayer( idPlayer* player );
	void					Event_AddLight( void );
};

#endif /* !__GAME_PORTAL_H__ */
