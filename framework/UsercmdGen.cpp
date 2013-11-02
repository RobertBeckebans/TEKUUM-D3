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

#include "precompiled.h"
#pragma hdrstop

#include "Session_local.h"
#include "../ui/Rectangle.h"
#include "../renderer/tr_local.h"

idCVar joy_mergedThreshold( "joy_mergedThreshold", "1", CVAR_BOOL | CVAR_ARCHIVE, "If the thresholds aren't merged, you drift more off center" );
idCVar joy_newCode( "joy_newCode", "1", CVAR_BOOL | CVAR_ARCHIVE, "Use the new codepath" );
idCVar joy_triggerThreshold( "joy_triggerThreshold", "0.05", CVAR_FLOAT | CVAR_ARCHIVE, "how far the joystick triggers have to be pressed before they register as down" );
idCVar joy_deadZone( "joy_deadZone", "0.2", CVAR_FLOAT | CVAR_ARCHIVE, "specifies how large the dead-zone is on the joystick" );
idCVar joy_range( "joy_range", "1.0", CVAR_FLOAT | CVAR_ARCHIVE, "allow full range to be mapped to a smaller offset" );
idCVar joy_gammaLook( "joy_gammaLook", "1", CVAR_INTEGER | CVAR_ARCHIVE, "use a log curve instead of a power curve for movement" );
idCVar joy_powerScale( "joy_powerScale", "2", CVAR_FLOAT | CVAR_ARCHIVE, "Raise joystick values to this power" );
idCVar joy_pitchSpeed( "joy_pitchSpeed", "100",	CVAR_ARCHIVE | CVAR_FLOAT, "pitch speed when pressing up or down on the joystick", 60, 600 );
idCVar joy_yawSpeed( "joy_yawSpeed", "240",	CVAR_ARCHIVE | CVAR_FLOAT, "pitch speed when pressing left or right on the joystick", 60, 600 );

// these were a bad idea!
idCVar joy_dampenLook( "joy_dampenLook", "1", CVAR_BOOL | CVAR_ARCHIVE, "Do not allow full acceleration on look" );
idCVar joy_deltaPerMSLook( "joy_deltaPerMSLook", "0.003", CVAR_FLOAT | CVAR_ARCHIVE, "Max amount to be added on look per MS" );

idCVar in_mouseSpeed( "in_mouseSpeed", "1",	CVAR_ARCHIVE | CVAR_FLOAT, "speed at which the mouse moves", 0.25f, 4.0f );
idCVar in_alwaysRun( "in_alwaysRun", "1", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_BOOL, "always run (reverse _speed button) - only in MP" );

idCVar in_useJoystick( "in_useJoystick", "0", CVAR_ARCHIVE | CVAR_BOOL, "enables/disables the gamepad for PC use" );
idCVar in_joystickRumble( "in_joystickRumble", "1", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_BOOL, "enable joystick rumble" );
idCVar in_invertLook( "in_invertLook", "0", CVAR_ARCHIVE | CVAR_BOOL, "inverts the look controls so the forward looks up (flight controls) - the proper way to play games!" );
idCVar in_mouseInvertLook( "in_mouseInvertLook", "0", CVAR_ARCHIVE | CVAR_BOOL, "inverts the look controls so the forward looks up (flight controls) - the proper way to play games!" );

/*
================
usercmd_t::ByteSwap
================
*/
void usercmd_t::ByteSwap()
{
	angles[0] = LittleShort( angles[0] );
	angles[1] = LittleShort( angles[1] );
	angles[2] = LittleShort( angles[2] );
	sequence = LittleLong( sequence );
}

/*
================
usercmd_t::operator==
================
*/
bool usercmd_t::operator==( const usercmd_t& rhs ) const
{
	return ( buttons == rhs.buttons &&
			 forwardmove == rhs.forwardmove &&
			 rightmove == rhs.rightmove &&
			 upmove == rhs.upmove &&
			 angles[0] == rhs.angles[0] &&
			 angles[1] == rhs.angles[1] &&
			 angles[2] == rhs.angles[2] &&
			 impulse == rhs.impulse &&
			 impulseSequence == rhs.impulseSequence &&
			 mx == rhs.mx &&
			 my == rhs.my );
}


const int KEY_MOVESPEED	= 127;



typedef struct
{
	const char* string;
	usercmdButton_t	button;
} userCmdString_t;

userCmdString_t	userCmdStrings[] =
{
	{ "_moveUp",		UB_MOVEUP },
	{ "_moveDown",		UB_MOVEDOWN },
	{ "_left",			UB_LOOKLEFT },
	{ "_right",			UB_LOOKRIGHT },
	{ "_forward",		UB_MOVEFORWARD },
	{ "_back",			UB_MOVEBACK },
	{ "_lookUp",		UB_LOOKUP },
	{ "_lookDown",		UB_LOOKDOWN },
	{ "_moveLeft",		UB_MOVELEFT },
	{ "_moveRight",		UB_MOVERIGHT },
	
	{ "_attack",		UB_ATTACK },
	{ "_speed",			UB_SPEED },
	{ "_zoom",			UB_ZOOM },
	{ "_showScores",	UB_SHOWSCORES },
	{ "_use",			UB_USE },
	
	{ "_impulse0",		UB_IMPULSE0 },
	{ "_impulse1",		UB_IMPULSE1 },
	{ "_impulse2",		UB_IMPULSE2 },
	{ "_impulse3",		UB_IMPULSE3 },
	{ "_impulse4",		UB_IMPULSE4 },
	{ "_impulse5",		UB_IMPULSE5 },
	{ "_impulse6",		UB_IMPULSE6 },
	{ "_impulse7",		UB_IMPULSE7 },
	{ "_impulse8",		UB_IMPULSE8 },
	{ "_impulse9",		UB_IMPULSE9 },
	{ "_impulse10",		UB_IMPULSE10 },
	{ "_impulse11",		UB_IMPULSE11 },
	{ "_impulse12",		UB_IMPULSE12 },
	{ "_impulse13",		UB_IMPULSE13 },
	{ "_impulse14",		UB_IMPULSE14 },
	{ "_impulse15",		UB_IMPULSE15 },
	{ "_impulse16",		UB_IMPULSE16 },
	{ "_impulse17",		UB_IMPULSE17 },
	{ "_impulse18",		UB_IMPULSE18 },
	{ "_impulse19",		UB_IMPULSE19 },
	{ "_impulse20",		UB_IMPULSE20 },
	{ "_impulse21",		UB_IMPULSE21 },
	{ "_impulse22",		UB_IMPULSE22 },
	{ "_impulse23",		UB_IMPULSE23 },
	{ "_impulse24",		UB_IMPULSE24 },
	{ "_impulse25",		UB_IMPULSE25 },
	{ "_impulse26",		UB_IMPULSE26 },
	{ "_impulse27",		UB_IMPULSE27 },
	{ "_impulse28",		UB_IMPULSE28 },
	{ "_impulse29",		UB_IMPULSE29 },
	{ "_impulse30",		UB_IMPULSE30 },
	{ "_impulse31",		UB_IMPULSE31 },
	{ "_impulse32",		UB_IMPULSE32 },
	{ "_impulse33",		UB_IMPULSE33 },
	{ "_impulse34",		UB_IMPULSE34 },
	{ "_impulse35",		UB_IMPULSE35 },
	{ "_impulse36",		UB_IMPULSE36 },
	{ "_impulse37",		UB_IMPULSE37 },
	{ "_impulse38",		UB_IMPULSE38 },
	{ "_impulse39",		UB_IMPULSE39 },
	{ "_impulse40",		UB_IMPULSE40 },
	{ "_impulse41",		UB_IMPULSE41 },
	{ "_impulse42",		UB_IMPULSE42 },
	{ "_impulse43",		UB_IMPULSE43 },
	{ "_impulse44",		UB_IMPULSE44 },
	{ "_impulse45",		UB_IMPULSE45 },
	{ "_impulse46",		UB_IMPULSE46 },
	{ "_impulse47",		UB_IMPULSE47 },
	{ "_impulse48",		UB_IMPULSE48 },
	{ "_impulse49",		UB_IMPULSE49 },
	{ "_impulse50",		UB_IMPULSE50 },
	{ "_impulse51",		UB_IMPULSE51 },
	{ "_impulse52",		UB_IMPULSE52 },
	{ "_impulse53",		UB_IMPULSE53 },
	{ "_impulse54",		UB_IMPULSE54 },
	{ "_impulse55",		UB_IMPULSE55 },
	{ "_impulse56",		UB_IMPULSE56 },
	{ "_impulse57",		UB_IMPULSE57 },
	{ "_impulse58",		UB_IMPULSE58 },
	{ "_impulse59",		UB_IMPULSE59 },
	{ "_impulse60",		UB_IMPULSE60 },
	{ "_impulse61",		UB_IMPULSE61 },
	{ "_impulse62",		UB_IMPULSE62 },
	{ "_impulse63",		UB_IMPULSE63 },
	
	{ NULL,				UB_NONE },
};

class buttonState_t
{
public:
	int		on;
	bool	held;
	
	buttonState_t()
	{
		Clear();
	};
	void	Clear();
	void	SetKeyState( int keystate, bool toggle );
};

/*
================
buttonState_t::Clear
================
*/
void buttonState_t::Clear()
{
	held = false;
	on = 0;
}

/*
================
buttonState_t::SetKeyState
================
*/
void buttonState_t::SetKeyState( int keystate, bool toggle )
{
	if( !toggle )
	{
		held = false;
		on = keystate;
	}
	else if( !keystate )
	{
		held = false;
	}
	else if( !held )
	{
		held = true;
		on ^= 1;
	}
}


const int NUM_USER_COMMANDS = sizeof( userCmdStrings ) / sizeof( userCmdString_t );

const int MAX_CHAT_BUFFER = 127;

class idUsercmdGenLocal : public idUsercmdGen
{
public:
	idUsercmdGenLocal();
	
	void			Init();
	
	void			InitForNewMap();
	
	void			Shutdown();
	
	void			Clear();
	
	void			ClearAngles();
	
	usercmd_t		TicCmd( int ticNumber );
	
	void			InhibitUsercmd( inhibit_t subsystem, bool inhibit );
	
	void			UsercmdInterrupt();
	
	int				CommandStringUsercmdData( const char* cmdString );
	
	void			BuildCurrentUsercmd();
	
	usercmd_t		GetCurrentUsercmd()
	{
		return cmd;
	};
	
	void			MouseState( int* x, int* y, int* button, bool* down );
	
	int				ButtonState( int key );
	int				KeyState( int key );
	
private:
	void			MakeCurrent();
	void			InitCurrent();
	
	bool			Inhibited();
	void			AdjustAngles();
	void			KeyMove();
	void			JoystickMove();
// RB begin
	void			Xbox360ControllerMove();
// RB end
	void			MouseMove();
	void			CmdButtons();
	
	void			Mouse();
	void			Keyboard();
	void			Joystick();
// RB begin
	void			Xbox360Controller();
	void			TouchScreen();
// RB end

	void			Key( int keyNum, bool down );
	
	idVec3			viewangles;
	int				impulseSequence;
	int				impulse;
	
	buttonState_t	toggled_crouch;
	buttonState_t	toggled_run;
	buttonState_t	toggled_zoom;
	
	int				buttonState[UB_MAX_BUTTONS];
	bool			keyState[K_LAST_KEY];
	
	int				inhibitCommands;	// true when in console or menu locally
	int				lastCommandTime;
	
	bool			initialized;
	
	usercmd_t		cmd;		// the current cmd being built
	usercmd_t		buffered[MAX_BUFFERED_USERCMD];
	
	int				continuousMouseX, continuousMouseY;	// for gui event generatioin, never zerod
	int				mouseButton;						// for gui event generatioin
	bool			mouseDown;
	
	int				mouseDx, mouseDy;	// added to by mouse events
	int				joystickAxis[MAX_JOYSTICK_AXIS];	// set by joystick events
	
	int				pollTime;
	int				lastPollTime;
	float			lastLookValuePitch;
	float			lastLookValueYaw;
	
	// RB: rectangles for touch screen interfaces
	idRectangle		touchMove;
	idRectangle		touchAttack;
	idRectangle		touchJump;
	idRectangle		touchCrouch;
	idRectangle		touchNextWeapon;
	idRectangle		touchPrevWeapon;
	idRectangle		touchFlashLight;
	// RB end
	
	static idCVar	in_yawSpeed;
	static idCVar	in_pitchSpeed;
	static idCVar	in_angleSpeedKey;
	static idCVar	in_alwaysRun;
	static idCVar	in_toggleRun;
	static idCVar	in_toggleCrouch;
	static idCVar	in_toggleZoom;
	static idCVar	sensitivity;
	static idCVar	m_pitch;
	static idCVar	m_yaw;
	static idCVar	m_smooth;
	static idCVar	m_showMouseRate;
};

idCVar idUsercmdGenLocal::in_yawSpeed( "in_yawspeed", "140", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_FLOAT, "yaw change speed when holding down _left or _right button" );
idCVar idUsercmdGenLocal::in_pitchSpeed( "in_pitchspeed", "140", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_FLOAT, "pitch change speed when holding down look _lookUp or _lookDown button" );
idCVar idUsercmdGenLocal::in_angleSpeedKey( "in_anglespeedkey", "1.5", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_FLOAT, "angle change scale when holding down _speed button" );
idCVar idUsercmdGenLocal::in_alwaysRun( "in_alwaysRun", "0", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_BOOL, "always run (reverse _speed button) - only in MP" );
idCVar idUsercmdGenLocal::in_toggleRun( "in_toggleRun", "0", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_BOOL, "pressing _speed button toggles run on/off - only in MP" );
idCVar idUsercmdGenLocal::in_toggleCrouch( "in_toggleCrouch", "0", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_BOOL, "pressing _movedown button toggles player crouching/standing" );
idCVar idUsercmdGenLocal::in_toggleZoom( "in_toggleZoom", "0", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_BOOL, "pressing _zoom button toggles zoom on/off" );
idCVar idUsercmdGenLocal::sensitivity( "sensitivity", "5", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_FLOAT, "mouse view sensitivity" );
idCVar idUsercmdGenLocal::m_pitch( "m_pitch", "0.022", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_FLOAT, "mouse pitch scale" );
idCVar idUsercmdGenLocal::m_yaw( "m_yaw", "0.022", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_FLOAT, "mouse yaw scale" );
idCVar idUsercmdGenLocal::m_smooth( "m_smooth", "1", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_INTEGER, "number of samples blended for mouse viewing", 1, 8, idCmdSystem::ArgCompletion_Integer<1, 8> );
idCVar idUsercmdGenLocal::m_showMouseRate( "m_showMouseRate", "0", CVAR_SYSTEM | CVAR_BOOL, "shows mouse movement" );

static idUsercmdGenLocal localUsercmdGen;
idUsercmdGen*	usercmdGen = &localUsercmdGen;

/*
================
idUsercmdGenLocal::idUsercmdGenLocal
================
*/
idUsercmdGenLocal::idUsercmdGenLocal()
{
	lastCommandTime = 0;
	initialized = false;
	
	pollTime = 0;
	lastPollTime = 0;
	lastLookValuePitch = 0.0f;
	lastLookValueYaw = 0.0f;
	
	impulseSequence = 0;
	impulse = 0;
	
	toggled_crouch.Clear();
	toggled_run.Clear();
	toggled_zoom.Clear();
	toggled_run.on = in_alwaysRun.GetBool();
	
	ClearAngles();
	Clear();
}

/*
================
idUsercmdGenLocal::InhibitUsercmd
================
*/
void idUsercmdGenLocal::InhibitUsercmd( inhibit_t subsystem, bool inhibit )
{
	if( inhibit )
	{
		inhibitCommands |= 1 << subsystem;
	}
	else
	{
		inhibitCommands &= ( 0xffffffff ^ ( 1 << subsystem ) );
	}
}

/*
===============
idUsercmdGenLocal::ButtonState

Returns (the fraction of the frame) that the key was down
===============
*/
int	idUsercmdGenLocal::ButtonState( int key )
{
	if( key < 0 || key >= UB_MAX_BUTTONS )
	{
		return -1;
	}
	return ( buttonState[key] > 0 ) ? 1 : 0;
}

/*
===============
idUsercmdGenLocal::KeyState

Returns (the fraction of the frame) that the key was down
bk20060111
===============
*/
int	idUsercmdGenLocal::KeyState( int key )
{
	if( key < 0 || key >= K_LAST_KEY )
	{
		return -1;
	}
	return ( keyState[key] ) ? 1 : 0;
}


//=====================================================================

/*
================
idUsercmdGenLocal::Inhibited

is user cmd generation inhibited
================
*/
bool idUsercmdGenLocal::Inhibited()
{
	return ( inhibitCommands != 0 );
}

/*
================
idUsercmdGenLocal::AdjustAngles

Moves the local angle positions
================
*/
void idUsercmdGenLocal::AdjustAngles()
{
	float speed = MS2SEC( USERCMD_MSEC );
	
	if( toggled_run.on ^ ( in_alwaysRun.GetBool() && idAsyncNetwork::IsActive() ) )
	{
		speed *= in_angleSpeedKey.GetFloat();
	}
	
	viewangles[YAW] -= speed * in_yawSpeed.GetFloat() * ButtonState( UB_LOOKRIGHT );
	viewangles[YAW] += speed * in_yawSpeed.GetFloat() * ButtonState( UB_LOOKLEFT );
	
	viewangles[PITCH] -= speed * in_pitchSpeed.GetFloat() * ButtonState( UB_LOOKUP );
	viewangles[PITCH] += speed * in_pitchSpeed.GetFloat() * ButtonState( UB_LOOKDOWN );
}

/*
================
idUsercmdGenLocal::KeyMove

Sets the usercmd_t based on key states
================
*/
void idUsercmdGenLocal::KeyMove()
{
	int forward = 0;
	int side = 0;
	int up = 0;
	
	side += KEY_MOVESPEED * ButtonState( UB_MOVERIGHT );
	side -= KEY_MOVESPEED * ButtonState( UB_MOVELEFT );
	
	up -= KEY_MOVESPEED * toggled_crouch.on;
	up += KEY_MOVESPEED * ButtonState( UB_MOVEUP );
	
	forward += KEY_MOVESPEED * ButtonState( UB_MOVEFORWARD );
	forward -= KEY_MOVESPEED * ButtonState( UB_MOVEBACK );
	
	cmd.forwardmove = idMath::ClampChar( forward );
	cmd.rightmove = idMath::ClampChar( side );
	cmd.upmove = idMath::ClampChar( up );
}

/*
=================
idUsercmdGenLocal::MouseMove
=================
*/
void idUsercmdGenLocal::MouseMove()
{
	float		mx, my;
	static int	history[8][2];
	static int	historyCounter;
	int			i;
	
	history[historyCounter & 7][0] = mouseDx;
	history[historyCounter & 7][1] = mouseDy;
	
	// allow mouse movement to be smoothed together
	int smooth = m_smooth.GetInteger();
	if( smooth < 1 )
	{
		smooth = 1;
	}
	if( smooth > 8 )
	{
		smooth = 8;
	}
	mx = 0;
	my = 0;
	for( i = 0 ; i < smooth ; i++ )
	{
		mx += history[( historyCounter - i + 8 ) & 7 ][0];
		my += history[( historyCounter - i + 8 ) & 7 ][1];
	}
	mx /= smooth;
	my /= smooth;
	
	historyCounter++;
	
	if( idMath::Fabs( mx ) > 1000 || idMath::Fabs( my ) > 1000 )
	{
		Sys_DebugPrintf( "idUsercmdGenLocal::MouseMove: Ignoring ridiculous mouse delta.\n" );
		mx = my = 0;
	}
	
	mx *= sensitivity.GetFloat();
	my *= sensitivity.GetFloat();
	
	if( m_showMouseRate.GetBool() )
	{
		Sys_DebugPrintf( "[%3i %3i  = %5.1f %5.1f] ", mouseDx, mouseDy, mx, my );
	}
	
	mouseDx = 0;
	mouseDy = 0;
	
	viewangles[YAW] -= m_yaw.GetFloat() * mx * in_mouseSpeed.GetFloat();
	viewangles[PITCH] += m_pitch.GetFloat() * in_mouseSpeed.GetFloat() * ( in_mouseInvertLook.GetBool() ? -my : my );
}

/*
=================
idUsercmdGenLocal::JoystickMove
=================
*/
void idUsercmdGenLocal::JoystickMove()
{
#if 0
	float	anglespeed;
	
	if( toggled_run.on ^ ( in_alwaysRun.GetBool() && idAsyncNetwork::IsActive() ) )
	{
		anglespeed = idMath::M_MS2SEC * USERCMD_MSEC * in_angleSpeedKey.GetFloat();
	}
	else
	{
		anglespeed = idMath::M_MS2SEC * USERCMD_MSEC;
	}
	
	if( !ButtonState( UB_STRAFE ) )
	{
		viewangles[YAW] += anglespeed * in_yawSpeed.GetFloat() * joystickAxis[AXIS_SIDE];
		viewangles[PITCH] += anglespeed * in_pitchSpeed.GetFloat() * joystickAxis[AXIS_FORWARD];
	}
	else
	{
		cmd.rightmove = idMath::ClampChar( cmd.rightmove + joystickAxis[AXIS_SIDE] );
		cmd.forwardmove = idMath::ClampChar( cmd.forwardmove + joystickAxis[AXIS_FORWARD] );
	}
	
	cmd.upmove = idMath::ClampChar( cmd.upmove + joystickAxis[AXIS_UP] );
#endif
}

// RB begin
void idUsercmdGenLocal::Xbox360ControllerMove()
{
	float	anglespeed;
	
	if( toggled_run.on ^ ( in_alwaysRun.GetBool() && idAsyncNetwork::IsActive() ) )
	{
		anglespeed = idMath::M_MS2SEC * USERCMD_MSEC * in_angleSpeedKey.GetFloat();
	}
	else
	{
		anglespeed = idMath::M_MS2SEC * USERCMD_MSEC;
	}
	
	viewangles[YAW] += anglespeed * in_yawSpeed.GetFloat() * ( joystickAxis[AXIS_YAW] / 127.0f );
	viewangles[PITCH] += anglespeed * in_pitchSpeed.GetFloat() * ( joystickAxis[AXIS_PITCH] / 127.0f );
	
	if( joystickAxis[AXIS_SIDE] != 0.0f || joystickAxis[AXIS_FORWARD] != 0.0f )
	{
		cmd.rightmove = idMath::ClampChar( cmd.rightmove + joystickAxis[AXIS_SIDE] );
		cmd.forwardmove = idMath::ClampChar( cmd.forwardmove + joystickAxis[AXIS_FORWARD] );
		//cmd.upmove = idMath::ClampChar( cmd.upmove + joystickAxis[AXIS_UP] );
		
		// always run because we have a fine control with the analog stick
		//cmd.buttons |= BUTTON_RUN;
	}
}
// RB end

/*
==============
idUsercmdGenLocal::CmdButtons
==============
*/
void idUsercmdGenLocal::CmdButtons()
{
	cmd.buttons = 0;
	
	// check the attack button
	if( ButtonState( UB_ATTACK ) )
	{
		cmd.buttons |= BUTTON_ATTACK;
	}
	
	// check the use button
	if( ButtonState( UB_USE ) )
	{
		cmd.buttons |= BUTTON_USE;
	}
	
	// check the run button
	if( toggled_run.on ^ ( in_alwaysRun.GetBool() && idAsyncNetwork::IsActive() ) )
	{
		cmd.buttons |= BUTTON_RUN;
	}
	
	// check the zoom button
	if( toggled_zoom.on )
	{
		cmd.buttons |= BUTTON_ZOOM;
	}
	
	// check the scoreboard button
	if( ButtonState( UB_SHOWSCORES ) || ButtonState( UB_IMPULSE19 ) )
	{
		// the button is toggled in SP mode as well but without effect
		cmd.buttons |= BUTTON_SCORES;
	}
	
	if( ButtonState( UB_MOVEUP ) )
	{
		cmd.buttons |= BUTTON_JUMP;
	}
	if( toggled_crouch.on )
	{
		cmd.buttons |= BUTTON_CROUCH;
	}
}

/*
================
idUsercmdGenLocal::InitCurrent

inits the current command for this frame
================
*/
void idUsercmdGenLocal::InitCurrent()
{
	memset( &cmd, 0, sizeof( cmd ) );
	cmd.impulseSequence = impulseSequence;
	cmd.impulse = impulse;
	cmd.buttons |= ( in_alwaysRun.GetBool() && idAsyncNetwork::IsActive() ) ? BUTTON_RUN : 0;
}

/*
================
idUsercmdGenLocal::MakeCurrent

creates the current command for this frame
================
*/
void idUsercmdGenLocal::MakeCurrent()
{
	idVec3 oldAngles = viewangles;
	
	if( !Inhibited() )
	{
		// update toggled key states
		toggled_crouch.SetKeyState( ButtonState( UB_MOVEDOWN ), in_toggleCrouch.GetBool() );
		toggled_run.SetKeyState( ButtonState( UB_SPEED ), in_toggleRun.GetBool() && idAsyncNetwork::IsActive() );
		toggled_zoom.SetKeyState( ButtonState( UB_ZOOM ), in_toggleZoom.GetBool() );
		
		// get basic movement from mouse
		MouseMove();
		
		// get basic movement from joystick and set key bits
		// must be done before CmdButtons!
		if( joy_newCode.GetBool() )
		{
			//JoystickMove2();
		}
		else
		{
			JoystickMove();
		}
		
// RB begin
		// get basic movement from xbox 360 controller
		Xbox360ControllerMove();
// RB end
		// keyboard angle adjustment
		AdjustAngles();
		
		// set button bits
		CmdButtons();
		
		// get basic movement from keyboard
		KeyMove();
		
		// aim assist
		// TODO AimAssist();
		
		// check to make sure the angles haven't wrapped
		if( viewangles[PITCH] - oldAngles[PITCH] > 90 )
		{
			viewangles[PITCH] = oldAngles[PITCH] + 90;
		}
		else if( oldAngles[PITCH] - viewangles[PITCH] > 90 )
		{
			viewangles[PITCH] = oldAngles[PITCH] - 90;
		}
	}
	else
	{
		mouseDx = 0;
		mouseDy = 0;
	}
	
	for( int i = 0; i < 3; i++ )
	{
		cmd.angles[i] = ANGLE2SHORT( viewangles[i] );
	}
	
	cmd.mx = continuousMouseX;
	cmd.my = continuousMouseY;
	
	impulseSequence = cmd.impulseSequence;
	impulse = cmd.impulse;
	
}

//=====================================================================


/*
================
idUsercmdGenLocal::CommandStringUsercmdData

Returns the button if the command string is used by the usercmd generator.
================
*/
int	idUsercmdGenLocal::CommandStringUsercmdData( const char* cmdString )
{
	for( userCmdString_t* ucs = userCmdStrings ; ucs->string ; ucs++ )
	{
		if( idStr::Icmp( cmdString, ucs->string ) == 0 )
		{
			return ucs->button;
		}
	}
	return UB_NONE;
}

/*
================
idUsercmdGenLocal::Init
================
*/
void idUsercmdGenLocal::Init()
{
	initialized = true;
}

/*
================
idUsercmdGenLocal::InitForNewMap
================
*/
void idUsercmdGenLocal::InitForNewMap()
{
	impulseSequence = 0;
	impulse = 0;
	
	toggled_crouch.Clear();
	toggled_run.Clear();
	toggled_zoom.Clear();
	toggled_run.on = in_alwaysRun.GetBool();
	
	Clear();
	ClearAngles();
}

/*
================
idUsercmdGenLocal::Shutdown
================
*/
void idUsercmdGenLocal::Shutdown()
{
	initialized = false;
}

/*
================
idUsercmdGenLocal::Clear
================
*/
void idUsercmdGenLocal::Clear()
{
	// clears all key states
	memset( buttonState, 0, sizeof( buttonState ) );
	memset( keyState, false, sizeof( keyState ) );
	memset( joystickAxis, 0, sizeof( joystickAxis ) );
	
	inhibitCommands = false;
	
	mouseDx = mouseDy = 0;
	mouseButton = 0;
	mouseDown = false;
}

/*
================
idUsercmdGenLocal::ClearAngles
================
*/
void idUsercmdGenLocal::ClearAngles()
{
	viewangles.Zero();
}

/*
================
idUsercmdGenLocal::TicCmd

Returns a buffered usercmd
================
*/
usercmd_t idUsercmdGenLocal::TicCmd( int ticNumber )
{

	// the packetClient code can legally ask for com_ticNumber+1, because
	// it is in the async code and com_ticNumber hasn't been updated yet,
	// but all other code should never ask for anything > com_ticNumber
	if( ticNumber > com_ticNumber + 1 )
	{
		common->Error( "idUsercmdGenLocal::TicCmd ticNumber > com_ticNumber" );
	}
	
	if( ticNumber <= com_ticNumber - MAX_BUFFERED_USERCMD )
	{
		// this can happen when something in the game code hitches badly, allowing the
		// async code to overflow the buffers
		//common->Printf( "warning: idUsercmdGenLocal::TicCmd ticNumber <= com_ticNumber - MAX_BUFFERED_USERCMD\n" );
	}
	
	return buffered[ ticNumber & ( MAX_BUFFERED_USERCMD - 1 ) ];
}

//======================================================================


/*
===================
idUsercmdGenLocal::Key

Handles mouse/keyboard button actions
===================
*/
void idUsercmdGenLocal::Key( int keyNum, bool down )
{

	// Sanity check, sometimes we get double message :(
	if( keyState[ keyNum ] == down )
	{
		return;
	}
	keyState[ keyNum ] = down;
	
	int action = idKeyInput::GetUsercmdAction( keyNum );
	
	if( down )
	{
		buttonState[ action ]++;
		if( !Inhibited() )
		{
			if( action >= UB_IMPULSE0 && action <= UB_IMPULSE61 )
			{
				cmd.impulse = action - UB_IMPULSE0;
				cmd.impulseSequence++;
			}
		}
	}
	else
	{
		buttonState[ action ]--;
		// we might have one held down across an app active transition
		if( buttonState[ action ] < 0 )
		{
			buttonState[ action ] = 0;
		}
	}
}

/*
===================
idUsercmdGenLocal::Mouse
===================
*/
void idUsercmdGenLocal::Mouse()
{
	int i, numEvents;
	
	numEvents = Sys_PollMouseInputEvents();
	
	if( numEvents )
	{
		//
		// Study each of the buffer elements and process them.
		//
		for( i = 0; i < numEvents; i++ )
		{
			int action, value;
			if( Sys_ReturnMouseInputEvent( i, action, value ) )
			{
				if( action >= M_ACTION1 && action <= M_ACTION8 )
				{
					mouseButton = K_MOUSE1 + ( action - M_ACTION1 );
					mouseDown = ( value != 0 );
					Key( mouseButton, mouseDown );
				}
				else
				{
					switch( action )
					{
						case M_DELTAX:
							mouseDx += value;
							continuousMouseX += value;
							break;
						case M_DELTAY:
							mouseDy += value;
							continuousMouseY += value;
							break;
						case M_DELTAZ:
							int key = value < 0 ? K_MWHEELDOWN : K_MWHEELUP;
							value = abs( value );
							while( value-- > 0 )
							{
								Key( key, true );
								Key( key, false );
								mouseButton = key;
								mouseDown = true;
							}
							break;
					}
				}
			}
		}
	}
	
	Sys_EndMouseInputEvents();
}

/*
===============
idUsercmdGenLocal::Keyboard
===============
*/
void idUsercmdGenLocal::Keyboard()
{

	int numEvents = Sys_PollKeyboardInputEvents();
	
	// Study each of the buffer elements and process them.
	for( int i = 0; i < numEvents; i++ )
	{
		int key;
		bool state;
		if( Sys_ReturnKeyboardInputEvent( i, key, state ) )
		{
			Key( key, state );
		}
	}
	
	Sys_EndKeyboardInputEvents();
}

/*
===============
idUsercmdGenLocal::Joystick
===============
*/
void idUsercmdGenLocal::Joystick()
{
	memset( joystickAxis, 0, sizeof( joystickAxis ) );
}

// RB begin
void idUsercmdGenLocal::Xbox360Controller()
{
	int numEvents = Sys_PollXbox360ControllerInputEvents();
	
	if( numEvents )
	{
		for( int i = 0; i < numEvents; i++ )
		{
			int action, value, value2;
			if( Sys_ReturnXbox360ControllerInputEvent( i, action, value, value2 ) )
			{
				/*
				if ( action >= M_ACTION1 && action <= M_ACTION8 )
				{
					mouseButton = K_MOUSE1 + ( action - M_ACTION1 );
					mouseDown = ( value != 0 );
					Key( mouseButton, mouseDown );
				}
				*/
				
				// convert xbox 360 controller axis to generic joystick axis
				switch( action )
				{
					case GP_AXIS_SIDE:
						joystickAxis[AXIS_SIDE] = value;
						break;
						
					case GP_AXIS_FORWARD:
						joystickAxis[AXIS_FORWARD] = value;
						break;
						
					case GP_AXIS_UP:
						joystickAxis[AXIS_UP] = value;
						break;
						
					case GP_AXIS_ROLL:
						joystickAxis[AXIS_ROLL] = value;
						break;
						
					case GP_AXIS_YAW:
						joystickAxis[AXIS_YAW] = value;
						break;
						
					case GP_AXIS_PITCH:
						joystickAxis[AXIS_PITCH] = value;
						break;
						
					case GP_BUTTON:
						Key( value, value2 != 0 );
						break;
				}
				
			}
		}
	}
	
	Sys_EndXbox360ControllerInputEvents();
}

void idUsercmdGenLocal::TouchScreen()
{
	int numEvents = Sys_PollTouchScreenInputEvents();
	
	if( numEvents )
	{
		int sysWidth = glConfig.nativeScreenWidth;
		int sysHeight = glConfig.nativeScreenHeight;
		
		touchMove.x = 0;
		touchMove.y = sysHeight - 400;
		touchMove.w = 400;
		touchMove.h = 400;
		
		touchAttack.x = sysWidth / 2 - 150;
		touchAttack.y = sysHeight - 300;
		touchAttack.w = 300;
		touchAttack.h = 300;
		
		touchJump.x = sysWidth - 200;
		touchJump.y = sysHeight - 400;
		touchJump.w = 200;
		touchJump.h = 200;
		
		touchCrouch.x = sysWidth - 200;
		touchCrouch.y = sysHeight - 200;
		touchCrouch.w = 200;
		touchCrouch.h = 200;
		
		touchNextWeapon.x = sysWidth - 200;
		touchNextWeapon.y = 0;
		touchNextWeapon.w = 200;
		touchNextWeapon.h = 200;
		
		touchPrevWeapon.x = sysWidth - 400;
		touchPrevWeapon.y = 0;
		touchPrevWeapon.w = 200;
		touchPrevWeapon.h = 200;
		
		touchFlashLight.x = sysWidth - 600;
		touchFlashLight.y = 0;
		touchFlashLight.w = 200;
		touchFlashLight.h = 200;
		
		//common->Printf( "idUsercmdGenLocal::TouchScreen() touchAttack: %s\n", touchAttack.String() );
		
		static int lastAction = -1;
		
		for( int i = 0; i < numEvents; i++ )
		{
			int action, value, value2, value3, value4;
			
			if( Sys_ReturnTouchScreenInputEvent( i, action, value, value2, value3, value4 ) )
			{
				if( action == TOUCH_MOTION_DOWN || action == TOUCH_MOTION_UP )
				{
					float x = idMath::ClampFloat( 0, sysWidth - 1, value * 0.001f * sysWidth );
					float y = idMath::ClampFloat( 0, sysHeight - 1, value2 * 0.001f * sysHeight );
					
					if( touchAttack.Contains( x, y ) )
					{
						common->Printf( "idUsercmdGenLocal::TouchScreen() ( %i , %i ) inside touchAttack %s\n", ( int ) x, ( int ) y, touchAttack.String() );
						
						mouseButton = K_MOUSE1;
						mouseDown = ( action == TOUCH_MOTION_DOWN );
						Key( mouseButton, mouseDown );
					}
					else if( touchJump.Contains( x, y ) )
					{
						common->Printf( "idUsercmdGenLocal::TouchScreen() ( %i , %i ) inside touchJump %s\n", ( int ) x, ( int ) y, touchJump.String() );
						
						Key( K_SPACE, action == TOUCH_MOTION_DOWN );
					}
					else if( touchCrouch.Contains( x, y ) )
					{
						common->Printf( "idUsercmdGenLocal::TouchScreen() ( %i , %i ) inside touchCrouch %s\n", ( int ) x, ( int ) y, touchCrouch.String() );
						
						Key( 'c' , action == TOUCH_MOTION_DOWN );
					}
					else if( touchNextWeapon.Contains( x, y ) )
					{
						common->Printf( "idUsercmdGenLocal::TouchScreen() ( %i , %i ) inside touchNextWeapon %s\n", ( int ) x, ( int ) y, touchNextWeapon.String() );
						
						Key( K_MWHEELDOWN , action == TOUCH_MOTION_DOWN );
					}
					else if( touchPrevWeapon.Contains( x, y ) )
					{
						common->Printf( "idUsercmdGenLocal::TouchScreen() ( %i , %i ) inside touchPrevWeapon %s\n", ( int ) x, ( int ) y, touchPrevWeapon.String() );
						
						Key( K_MWHEELUP, action == TOUCH_MOTION_DOWN );
					}
					else if( touchFlashLight.Contains( x, y ) )
					{
						common->Printf( "idUsercmdGenLocal::TouchScreen() ( %i , %i ) inside touchFlashLight %s\n", ( int ) x, ( int ) y, touchFlashLight.String() );
						
						Key( 'f' , action == TOUCH_MOTION_DOWN );
					}
					else if( action == TOUCH_MOTION_UP )
					{
						// reset all keys
						mouseButton = K_MOUSE1;
						mouseDown = false;
						Key( mouseButton, mouseDown );
						
						Key( K_SPACE , false );
						Key( 'c' , false );
						Key( K_MWHEELDOWN , false );
						Key( K_MWHEELUP , false );
						Key( 'f' , false );
					}
				}
				else if( action == TOUCH_MOTION_DELTA_XY )
				{
					float x = idMath::ClampFloat( 0, sysWidth - 1, value * 0.001f * sysWidth );
					float y = idMath::ClampFloat( 0, sysHeight - 1, value2 * 0.001f * sysHeight );
					
					/*
					if( lastAction == TOUCH_MOTION_DOWN )
					{
						if( touchAttack.Contains( x, y ) )
						{
							common->Printf( "idUsercmdGenLocal::TouchScreen() ( %i , %i ) inside touchAttack %s\n", ( int ) x, ( int ) y, touchAttack.String() );
					
							mouseButton = K_MOUSE1;
							mouseDown = true; // continue fire ( action == TOUCH_MOTION_DOWN );
							Key( mouseButton, mouseDown );
						}
					}
					else
					*/
					{
						if( !touchMove.Contains( x, y ) ) //&& !touchJump.Contains( x, y ) && !touchCrouch.Contains( x, y ) && )
						{
							mouseDx += value3 * 4.0f;
							continuousMouseX += value3 * 4.0f;
							
							mouseDy += value4 * 4.0f;
							continuousMouseY += value4 * 4.0f;
						}
					}
				}
				
				lastAction = action;
			}
		}
	}
	
	Sys_EndTouchScreenInputEvents();
}
// RB end

/*
================
idUsercmdGenLocal::UsercmdInterrupt

Called asyncronously
================
*/
void idUsercmdGenLocal::UsercmdInterrupt()
{
	// dedicated servers won't create usercmds
	if( !initialized )
	{
		return;
	}
	
	// init the usercmd for com_ticNumber+1
	InitCurrent();
	
	// process the system mouse events
	Mouse();
	
	// process the system keyboard events
	Keyboard();
	
	// process the system joystick events
	Joystick();
	
	// create the usercmd for com_ticNumber+1
	MakeCurrent();
	
	// save a number for debugging cmdDemos and networking
	cmd.sequence = com_ticNumber + 1;
	
	buffered[( com_ticNumber + 1 ) & ( MAX_BUFFERED_USERCMD - 1 )] = cmd;
}

/*
================
idUsercmdGenLocal::MouseState
================
*/
void idUsercmdGenLocal::MouseState( int* x, int* y, int* button, bool* down )
{
	*x = continuousMouseX;
	*y = continuousMouseY;
	*button = mouseButton;
	*down = mouseDown;
}

/*
================
idUsercmdGenLocal::BuildCurrentUsercmd
================
*/
void idUsercmdGenLocal::BuildCurrentUsercmd()
{
	pollTime = Sys_Milliseconds();
	if( pollTime - lastPollTime > 100 )
	{
		lastPollTime = pollTime - 100;
	}
	
	// initialize current usercmd
	InitCurrent();
	
	// process the system mouse events
	Mouse();
	
	// process the system keyboard events
	Keyboard();
	
	// process the system joystick events
	Joystick();
	
	// RB begin
	Xbox360Controller();
	
	TouchScreen();
	// RB end
	
	// create the usercmd
	MakeCurrent();
	
	cmd.duplicateCount = 0;
	
	lastPollTime = pollTime;
}
