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

#if defined(USE_LUA)

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"


idThread*			idThread::currentThread = NULL;
int					idThread::threadIndex = 0;
idList<idThread*>	idThread::threadList;
trace_t				idThread::trace;

/*
================
idThread::CurrentThread
================
*/
idThread* idThread::CurrentThread( void )
{
	return currentThread;
}

/*
================
idThread::CurrentThreadNum
================
*/
int idThread::CurrentThreadNum( void )
{
	if( currentThread )
	{
		return currentThread->GetThreadNum();
	}
	else
	{
		return 0;
	}
}

/*
================
idThread::BeginMultiFrameEvent
================
*/
bool idThread::BeginMultiFrameEvent( idEntity* ent, const idEventDef* event )
{
	if( !currentThread )
	{
		gameLocal.Error( "idThread::BeginMultiFrameEvent called without a current thread" );
	}

	// FIXME
	//return currentThread->interpreter.BeginMultiFrameEvent( ent, event );
	return false;
}

/*
================
idThread::EndMultiFrameEvent
================
*/
void idThread::EndMultiFrameEvent( idEntity* ent, const idEventDef* event )
{
	if( !currentThread )
	{
		gameLocal.Error( "idThread::EndMultiFrameEvent called without a current thread" );
	}

	// FIXME
	//currentThread->interpreter.EndMultiFrameEvent( ent, event );
}

/*
================
idThread::idThread
================
*/
idThread::idThread()
{
	Init();
	SetThreadName( va( "thread_%d", threadIndex ) );
	if( g_debugScript.GetBool() )
	{
		gameLocal.Printf( "%d: create thread (%d) '%s'\n", gameLocal.time, threadNum, threadName.c_str() );
	}
}

/*
================
idThread::idThread
================
*/
idThread::idThread( idEntity* self, const char* funcName )
{
	assert( self );
	
	Init();
	SetThreadName( self->name );

	// FIXME
	//interpreter.EnterObjectFunction( self, func, false );
	
	if( g_debugScript.GetBool() )
	{
		gameLocal.Printf( "%d: create thread (%d) '%s'\n", gameLocal.time, threadNum, threadName.c_str() );
	}
}

/*
================
idThread::idThread
================
*/
idThread::idThread( idEntity* self, const function_t* func )
{
	assert( self );
	
	Init();
	SetThreadName( self->name );

	// FIXME
	//interpreter.EnterObjectFunction( self, func, false );
	
	if( g_debugScript.GetBool() )
	{
		gameLocal.Printf( "%d: create thread (%d) '%s'\n", gameLocal.time, threadNum, threadName.c_str() );
	}
}

/*
================
idThread::idThread
================
*/
idThread::idThread( const function_t* func )
{
	assert( func );
	
	Init();
	SetThreadName( func->Name() );

	// FIXME
	//interpreter.EnterFunction( func, false );
	
	if( g_debugScript.GetBool() )
	{
		gameLocal.Printf( "%d: create thread (%d) '%s'\n", gameLocal.time, threadNum, threadName.c_str() );
	}
}

/*
================
idThread::idThread
================
*/
/*
idThread::idThread( idInterpreter* source, const function_t* func, int args )
{
	Init();
	interpreter.ThreadCall( source, func, args );
	if( g_debugScript.GetBool() )
	{
		gameLocal.Printf( "%d: create thread (%d) '%s'\n", gameLocal.time, threadNum, threadName.c_str() );
	}
}
*/

/*
================
idThread::idThread
================
*/
/*
idThread::idThread( idInterpreter* source, idEntity* self, const function_t* func, int args )
{
	assert( self );
	
	Init();
	SetThreadName( self->name );
	interpreter.ThreadCall( source, func, args );
	if( g_debugScript.GetBool() )
	{
		gameLocal.Printf( "%d: create thread (%d) '%s'\n", gameLocal.time, threadNum, threadName.c_str() );
	}
}
*/

/*
================
idThread::~idThread
================
*/
idThread::~idThread()
{
	idThread*	thread;
	int			i;
	int			n;
	
	if( g_debugScript.GetBool() )
	{
		gameLocal.Printf( "%d: end thread (%d) '%s'\n", gameLocal.time, threadNum, threadName.c_str() );
	}

	threadList.Remove( this );
	n = threadList.Num();
	for( i = 0; i < n; i++ )
	{
		thread = threadList[ i ];
		if( thread->WaitingOnThread() == this )
		{
			thread->ThreadCallback( this );
		}
	}
	
	if( currentThread == this )
	{
		currentThread = NULL;
	}
}

/*
================
idThread::ManualDelete
================
*/
void idThread::ManualDelete( void )
{
	// FIXME
	//interpreter.terminateOnExit = false;
}

/*
================
idThread::Save
================
*/
void idThread::Save( idSaveGame* savefile ) const
{

	// We will check on restore that threadNum is still the same,
	//  threads should have been restored in the same order.
	savefile->WriteInt( threadNum );
	
	savefile->WriteObject( waitingForThread );
	savefile->WriteInt( waitingFor );
	savefile->WriteInt( waitingUntil );
	
	// FIXME
	//interpreter.Save( savefile );
	
	savefile->WriteDict( &spawnArgs );
	savefile->WriteString( threadName );
	
	savefile->WriteInt( lastExecuteTime );
	savefile->WriteInt( creationTime );
	
	savefile->WriteBool( manualControl );
}

/*
================
idThread::Restore
================
*/
void idThread::Restore( idRestoreGame* savefile )
{
	savefile->ReadInt( threadNum );
	
	savefile->ReadObject( reinterpret_cast<idClass*&>( waitingForThread ) );
	savefile->ReadInt( waitingFor );
	savefile->ReadInt( waitingUntil );
	
	// FIXME
	//interpreter.Restore( savefile );
	
	savefile->ReadDict( &spawnArgs );
	savefile->ReadString( threadName );
	
	savefile->ReadInt( lastExecuteTime );
	savefile->ReadInt( creationTime );
	
	savefile->ReadBool( manualControl );
}

/*
================
idThread::Init
================
*/
void idThread::Init( void )
{
	// create a unique threadNum
	do
	{
		threadIndex++;
		if( threadIndex == 0 )
		{
			threadIndex = 1;
		}
	}
	while( GetThread( threadIndex ) );
	
	threadNum = threadIndex;
	threadList.Append( this );
	
	creationTime = gameLocal.time;
	lastExecuteTime = 0;
	manualControl = false;
	
	ClearWaitFor();
	
	luaThread = lua_newthread( gameLocal.program.GetLuaState() );

	// FIXME
	//interpreter.SetThread( this );
}

/*
================
idThread::GetThread
================
*/
idThread* idThread::GetThread( int num )
{
	int			i;
	int			n;
	idThread*	thread;
	
	n = threadList.Num();
	for( i = 0; i < n; i++ )
	{
		thread = threadList[ i ];
		if( thread->GetThreadNum() == num )
		{
			return thread;
		}
	}
	
	return NULL;
}

/*
================
idThread::DisplayInfo
================
*/
void idThread::DisplayInfo( void )
{
#if 0
	gameLocal.Printf(
		"%12i: '%s'\n"
		//"        File: %s(%d)\n"
		"     Created: %d (%d ms ago)\n"
		"      Status: ",
		threadNum, threadName.c_str(),
		//interpreter.CurrentFile(), interpreter.CurrentLine(),
		creationTime, gameLocal.time - creationTime );
		
	if( interpreter.threadDying )
	{
		gameLocal.Printf( "Dying\n" );
	}
	else if( interpreter.doneProcessing )
	{
		gameLocal.Printf(
			"Paused since %d (%d ms)\n"
			"      Reason: ",  lastExecuteTime, gameLocal.time - lastExecuteTime );
		if( waitingForThread )
		{
			gameLocal.Printf( "Waiting for thread #%3i '%s'\n", waitingForThread->GetThreadNum(), waitingForThread->GetThreadName() );
		}
		else if( ( waitingFor != ENTITYNUM_NONE ) && ( gameLocal.entities[ waitingFor ] ) )
		{
			gameLocal.Printf( "Waiting for entity #%3i '%s'\n", waitingFor, gameLocal.entities[ waitingFor ]->name.c_str() );
		}
		else if( waitingUntil )
		{
			gameLocal.Printf( "Waiting until %d (%d ms total wait time)\n", waitingUntil, waitingUntil - lastExecuteTime );
		}
		else
		{
			gameLocal.Printf( "None\n" );
		}
	}
	else
	{
		gameLocal.Printf( "Processing\n" );
	}
	
	interpreter.DisplayInfo();
	
	gameLocal.Printf( "\n" );
#endif
}

/*
================
idThread::ListThreads_f
================
*/
void idThread::ListThreads_f( const idCmdArgs& args )
{
	int	i;
	int	n;
	
	n = threadList.Num();
	for( i = 0; i < n; i++ )
	{
		//threadList[ i ]->DisplayInfo();
		//gameLocal.Printf( "%3i: %-20s : %s(%d)\n", threadList[ i ]->threadNum, threadList[ i ]->threadName.c_str(), threadList[ i ]->interpreter.CurrentFile(), threadList[ i ]->interpreter.CurrentLine() );
		gameLocal.Printf( "%3i: %-20s\n", threadList[ i ]->threadNum, threadList[ i ]->threadName.c_str() );
	}
	gameLocal.Printf( "%d active threads\n\n", n );
}

/*
================
idThread::Restart
================
*/
void idThread::Restart( void )
{
	int	i;
	int	n;
	
	// reset the threadIndex
	threadIndex = 0;
	
	currentThread = NULL;
	n = threadList.Num();
	for( i = n - 1; i >= 0; i-- )
	{
		delete threadList[ i ];
	}
	threadList.Clear();
	
	memset( &trace, 0, sizeof( trace ) );
	trace.c.entityNum = ENTITYNUM_NONE;
}

/*
================
idThread::DelayedStart
================
*/
void idThread::DelayedStart( int delay )
{
	CancelEvents( &EV_Thread_Execute );
	if( gameLocal.time <= 0 )
	{
		delay++;
	}
	PostEventMS( &EV_Thread_Execute, delay );
}

/*
================
idThread::Start
================
*/
bool idThread::Start( void )
{
	bool result;
	
	CancelEvents( &EV_Thread_Execute );
	result = Execute();
	
	return result;
}

/*
================
idThread::SetThreadName
================
*/
void idThread::SetThreadName( const char* name )
{
	threadName = name;
}

/*
================
idThread::ObjectMoveDone
================
*/
void idThread::ObjectMoveDone( int threadnum, idEntity* obj )
{
	idThread* thread;
	
	if( !threadnum )
	{
		return;
	}
	
	thread = GetThread( threadnum );
	if( thread )
	{
		thread->ObjectMoveDone( obj );
	}
}

/*
================
idThread::End
================
*/
void idThread::End( void )
{
	// Tell thread to die.  It will exit on its own.
	Pause();

	// FIXME
	//interpreter.threadDying	= true;
}

/*
================
idThread::KillThread
================
*/
void idThread::KillThread( const char* name )
{
	int			i;
	int			num;
	int			len;
	const char*	ptr;
	idThread*	thread;
	
	// see if the name uses a wild card
	ptr = strchr( name, '*' );
	if( ptr )
	{
		len = ptr - name;
	}
	else
	{
		len = strlen( name );
	}
	
	// kill only those threads whose name matches name
	num = threadList.Num();
	for( i = 0; i < num; i++ )
	{
		thread = threadList[ i ];
		if( !idStr::Cmpn( thread->GetThreadName(), name, len ) )
		{
			thread->End();
		}
	}
}

/*
================
idThread::KillThread
================
*/
void idThread::KillThread( int num )
{
	idThread* thread;
	
	thread = GetThread( num );
	if( thread )
	{
		// Tell thread to die.  It will delete itself on it's own.
		thread->End();
	}
}

/*
================
idThread::Execute
================
*/
bool idThread::Execute( void )
{
	idThread*	oldThread;
	bool		done;
	
	if( manualControl && ( waitingUntil > gameLocal.time ) )
	{
		return false;
	}
	
	oldThread = currentThread;
	currentThread = this;
	
	lastExecuteTime = gameLocal.time;
	ClearWaitFor();

	// FIXME

	//done = interpreter.Execute();
	int status = lua_resume( luaThread, gameLocal.program.GetLuaState(), 0 );
	switch( status )
	{
		case LUA_OK:
		{
			End();
			/*
			if( interpreter.terminateOnExit )
			{
				PostEventMS( &EV_Remove, 0 );
			}
			*/

			done = true;
			break;
		}

		case LUA_YIELD:
			done = false;
			break;
	};

	/*
	if( done )
	{
		End();
		
		if( interpreter.terminateOnExit )
		{
			PostEventMS( &EV_Remove, 0 );
		}
	}
	else if( !manualControl )
	{
		if( waitingUntil > lastExecuteTime )
		{
			PostEventMS( &EV_Thread_Execute, waitingUntil - lastExecuteTime );
		}
		else if( interpreter.MultiFrameEventInProgress() )
		{
			PostEventMS( &EV_Thread_Execute, gameLocal.msec );
		}
	}
	*/
	
	currentThread = oldThread;
	
	return done;
}

/*
================
idThread::IsWaiting

Checks if thread is still waiting for some event to occur.
================
*/
bool idThread::IsWaiting( void )
{
	if( waitingForThread || ( waitingFor != ENTITYNUM_NONE ) )
	{
		return true;
	}
	
	if( waitingUntil && ( waitingUntil > gameLocal.time ) )
	{
		return true;
	}
	
	return false;
}

/*
================
idThread::CallFunction

NOTE: If this is called from within a event called by this thread, the function arguments will be invalid after calling this function.
================
*/
void idThread::CallFunction( const function_t* func, bool clearStack )
{
	ClearWaitFor();
	
	// FIXME
	//interpreter.EnterFunction( func, clearStack );
}

/*
================
idThread::CallFunction

NOTE: If this is called from within a event called by this thread, the function arguments will be invalid after calling this function.
================
*/
void idThread::CallFunction( idEntity* self, const function_t* func, bool clearStack )
{
	assert( self );
	ClearWaitFor();

	// FIXME
	//interpreter.EnterObjectFunction( self, func, clearStack );
}

/*
================
idThread::ClearWaitFor
================
*/
void idThread::ClearWaitFor( void )
{
	waitingFor			= ENTITYNUM_NONE;
	waitingForThread	= NULL;
	waitingUntil		= 0;
}

/*
================
idThread::IsWaitingFor
================
*/
bool idThread::IsWaitingFor( idEntity* obj )
{
	assert( obj );
	return waitingFor == obj->entityNumber;
}

/*
================
idThread::ObjectMoveDone
================
*/
void idThread::ObjectMoveDone( idEntity* obj )
{
	assert( obj );
	
	if( IsWaitingFor( obj ) )
	{
		ClearWaitFor();
		DelayedStart( 0 );
	}
}

/*
================
idThread::ThreadCallback
================
*/
void idThread::ThreadCallback( idThread* thread )
{
	/*
	// FIXME
	if( interpreter.threadDying )
	{
		return;
	}
	*/
	
	if( thread == waitingForThread )
	{
		ClearWaitFor();
		DelayedStart( 0 );
	}
}

/*
================
idThread::Event_SetThreadName
================
*/
void idThread::Event_SetThreadName( const char* name )
{
	SetThreadName( name );
}

/*
================
idThread::Error
================
*/
void idThread::Error( const char* fmt, ... ) const
{
	va_list	argptr;
	char	text[ 1024 ];
	
	va_start( argptr, fmt );
	vsprintf( text, fmt, argptr );
	va_end( argptr );
	
	// FIXME
	//interpreter.Error( text );
}

/*
================
idThread::Warning
================
*/
void idThread::Warning( const char* fmt, ... ) const
{
	va_list	argptr;
	char	text[ 1024 ];
	
	va_start( argptr, fmt );
	vsprintf( text, fmt, argptr );
	va_end( argptr );
	
	// FIXME
	//interpreter.Warning( text );
}

/*
================
idThread::ReturnString
================
*/
void idThread::ReturnString( const char* text )
{
	gameLocal.program.ReturnString( text );
}

/*
================
idThread::ReturnFloat
================
*/
void idThread::ReturnFloat( float value )
{
	gameLocal.program.ReturnFloat( value );
}

/*
================
idThread::ReturnInt
================
*/
void idThread::ReturnInt( int value )
{
	// true integers aren't supported in the compiler,
	// so int values are stored as floats
	gameLocal.program.ReturnFloat( value );
}

/*
================
idThread::ReturnVector
================
*/
void idThread::ReturnVector( idVec3 const& vec )
{
	gameLocal.program.ReturnVector( vec );
}

/*
================
idThread::ReturnEntity
================
*/
void idThread::ReturnEntity( idEntity* ent )
{
	gameLocal.program.ReturnEntity( ent );
}

/*
================
idThread::Event_Execute
================
*/
void idThread::Event_Execute( void )
{
	Execute();
}

/*
================
idThread::Pause
================
*/
void idThread::Pause( void )
{
	ClearWaitFor();

	lua_yield( luaThread, 0 );

	// FIXME
	//interpreter.doneProcessing = true;
}

/*
================
idThread::WaitMS
================
*/
void idThread::WaitMS( int time )
{
	Pause();
	waitingUntil = gameLocal.time + time;
}

/*
================
idThread::WaitSec
================
*/
void idThread::WaitSec( float time )
{
	WaitMS( SEC2MS( time ) );
}

/*
================
idThread::WaitFrame
================
*/
void idThread::WaitFrame( void )
{
	Pause();
	
	// manual control threads don't set waitingUntil so that they can be run again
	// that frame if necessary.
	if( !manualControl )
	{
		waitingUntil = gameLocal.time + gameLocal.msec;
	}
}


#endif // #if defined(USE_LUA)