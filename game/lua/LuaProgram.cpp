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

/***********************************************************************

  function_t

***********************************************************************/

/*
================
function_t::function_t
================
*/
function_t::function_t()
{
	Clear();
}

/*
================
function_t::Allocated
================
*/
size_t function_t::Allocated( void ) const
{
	return name.Allocated() + parmSize.Allocated();
}

/*
================
function_t::SetName
================
*/
void function_t::SetName( const char* name )
{
	this->name = name;
}

/*
================
function_t::Name
================
*/
const char* function_t::Name( void ) const
{
	return name;
}

/*
================
function_t::Clear
================
*/
void function_t::Clear( void )
{
	eventdef		= NULL;
//	def				= NULL;
//	type			= NULL;
	firstStatement	= 0;
	numStatements	= 0;
	parmTotal		= 0;
	locals			= 0;
	filenum			= 0;
	name.Clear();
	parmSize.Clear();
}


/***********************************************************************

  idScriptObject

***********************************************************************/

/*
============
idScriptObject::idScriptObject
============
*/
idScriptObject::idScriptObject()
{
	data = NULL;
//	type = &type_object;
}

/*
============
idScriptObject::~idScriptObject
============
*/
idScriptObject::~idScriptObject()
{
	Free();
}

/*
============
idScriptObject::Free
============
*/
void idScriptObject::Free( void )
{
	if( data )
	{
		Mem_Free( data );
	}
	
	data = NULL;
//	type = &type_object;
}

/*
================
idScriptObject::Save
================
*/
void idScriptObject::Save( idSaveGame* savefile ) const
{
	// Techyon RB: 64 bit fix, changed size_t to int
	int size;
	// Techyon END
	
	// FIXME
	savefile->WriteString( "" );

	/*
	if( type == &type_object && data == NULL )
	{
		// Write empty string for uninitialized object
		savefile->WriteString( "" );
	}
	else
	{
		savefile->WriteString( type->Name() );
		size = type->Size();
		savefile->WriteInt( size );
		savefile->Write( data, size );
	}
	*/
}

/*
================
idScriptObject::Restore
================
*/
void idScriptObject::Restore( idRestoreGame* savefile )
{
	idStr typeName;
	// Techyon RB: 64 bit fix, changed size_t to int
	int size;
	// Techyon END
	
	savefile->ReadString( typeName );
	
	// Empty string signals uninitialized object
	if( typeName.Length() == 0 )
	{
		return;
	}
	
	if( !SetType( typeName ) )
	{
		savefile->Error( "idScriptObject::Restore: failed to restore object of type '%s'.", typeName.c_str() );
	}
	
	// Techyon RB: 64 bit fix, changed size_t to int
	savefile->ReadInt( size );
	// Techyon END

	// FIXME
	/*
	if( size != type->Size() )
	{
		savefile->Error( "idScriptObject::Restore: size of object '%s' doesn't match size in save game.", typeName.c_str() );
	}

	*/
	
	savefile->Read( data, size );
}

/*
============
idScriptObject::SetType

Allocates an object and initializes memory.
============
*/
bool idScriptObject::SetType( const char* typeName )
{
#if 0
	size_t size;
	idTypeDef* newtype;
	
	// lookup the type
	newtype = gameLocal.program.FindType( typeName );
	
	// only allocate memory if the object type changes
	if( newtype != type )
	{
		Free();
		if( !newtype )
		{
			gameLocal.Warning( "idScriptObject::SetType: Unknown type '%s'", typeName );
			return false;
		}
		
		if( !newtype->Inherits( &type_object ) )
		{
			gameLocal.Warning( "idScriptObject::SetType: Can't create object of type '%s'.  Must be an object type.", newtype->Name() );
			return false;
		}
		
		// set the type
		type = newtype;
		
		// allocate the memory
		size = type->Size();
		data = ( byte* )Mem_Alloc( size );
	}
	
	// init object memory
	ClearObject();
	
	return true;
#else
	return false;
#endif
}


/*
============
idScriptObject::ClearObject

Resets the memory for the script object without changing its type.
============
*/
void idScriptObject::ClearObject( void )
{
	/*
	size_t size;
	
	if( type != &type_object )
	{
		// init object memory
		size = type->Size();
		memset( data, 0, size );
	}
	*/
}

/*
============
idScriptObject::HasObject
============
*/
bool idScriptObject::HasObject( void ) const
{
	// FIXME
	//return ( type != &type_object );
	return false;
}

/*
============
idScriptObject::GetTypeDef
============
*/
/*
idTypeDef* idScriptObject::GetTypeDef( void ) const
{
	return type;
}
*/

/*
============
idScriptObject::GetTypeName
============
*/
const char* idScriptObject::GetTypeName( void ) const
{
	// FIXME
	return "<FIXME>";

	//return type->Name();
}

/*
============
idScriptObject::GetConstructor
============
*/
const function_t* idScriptObject::GetConstructor( void ) const
{
	const function_t* func;
	
	func = GetFunction( "init" );
	return func;
}

/*
============
idScriptObject::GetDestructor
============
*/
const function_t* idScriptObject::GetDestructor( void ) const
{
	const function_t* func;
	
	func = GetFunction( "destroy" );
	return func;
}

/*
============
idScriptObject::GetFunction
============
*/
const function_t* idScriptObject::GetFunction( const char* name ) const
{
	const function_t* func;
	
	/*
	FIXME
	if( type == &type_object )
	{
		return NULL;
	}
	*/
	
	func = gameLocal.program.FindFunction( name/*, type*/ );
	return func;
}

/*
============
idScriptObject::GetVariable
============
*/
/*
byte* idScriptObject::GetVariable( const char* name, etype_t etype ) const
{
	int				i;
	int				pos;
	const idTypeDef*	t;
	const idTypeDef*	parm;
	
	if( type == &type_object )
	{
		return NULL;
	}
	
	t = type;
	do
	{
		if( t->SuperClass() != &type_object )
		{
			pos = t->SuperClass()->Size();
		}
		else
		{
			pos = 0;
		}
		for( i = 0; i < t->NumParameters(); i++ )
		{
			parm = t->GetParmType( i );
			if( !strcmp( t->GetParmName( i ), name ) )
			{
				if( etype != parm->FieldType()->Type() )
				{
					return NULL;
				}
				return &data[ pos ];
			}
			
			if( parm->FieldType()->Inherits( &type_object ) )
			{
				pos += type_object.Size();
			}
			else
			{
				pos += parm->FieldType()->Size();
			}
		}
		t = t->SuperClass();
	}
	while( t && ( t != &type_object ) );
	
	return NULL;
}
*/

/***********************************************************************

  tyLuaProgram

***********************************************************************/

/*
============
tyLuaProgram::AllocType
============
*/
/*
idTypeDef* tyLuaProgram::AllocType( idTypeDef& type )
{
	idTypeDef* newtype;
	
	newtype	= new idTypeDef( type );
	types.Append( newtype );
	
	return newtype;
}
*/

/*
============
tyLuaProgram::AllocType
============
*/
/*
idTypeDef* tyLuaProgram::AllocType( etype_t etype, idVarDef* edef, const char* ename, int esize, idTypeDef* aux )
{
	idTypeDef* newtype;
	
	newtype	= new idTypeDef( etype, edef, ename, esize, aux );
	types.Append( newtype );
	
	return newtype;
}
*/

/*
============
tyLuaProgram::GetType

Returns a preexisting complex type that matches the parm, or allocates
a new one and copies it out.
============
*/
/*
idTypeDef* tyLuaProgram::GetType( idTypeDef& type, bool allocate )
{
	int i;
	
	//FIXME: linear search == slow
	for( i = types.Num() - 1; i >= 0; i-- )
	{
		if( types[ i ]->MatchesType( type ) && !strcmp( types[ i ]->Name(), type.Name() ) )
		{
			return types[ i ];
		}
	}
	
	if( !allocate )
	{
		return NULL;
	}
	
	// allocate a new one
	return AllocType( type );
}
*/

/*
============
tyLuaProgram::FindType

Returns a preexisting complex type that matches the name, or returns NULL if not found
============
*/
/*
idTypeDef* tyLuaProgram::FindType( const char* name )
{
	idTypeDef*	check;
	int			i;
	
	for( i = types.Num() - 1; i >= 0; i-- )
	{
		check = types[ i ];
		if( !strcmp( check->Name(), name ) )
		{
			return check;
		}
	}
	
	return NULL;
}
*/

/*
============
tyLuaProgram::GetDefList
============
*/
/*
idVarDef* tyLuaProgram::GetDefList( const char* name ) const
{
	int i, hash;
	
	hash = varDefNameHash.GenerateKey( name, true );
	for( i = varDefNameHash.First( hash ); i != -1; i = varDefNameHash.Next( i ) )
	{
		if( idStr::Cmp( varDefNames[i]->Name(), name ) == 0 )
		{
			return varDefNames[i]->GetDefs();
		}
	}
	return NULL;
}
*/

/*
============
tyLuaProgram::AddDefToNameList
============
*/
/*
void tyLuaProgram::AddDefToNameList( idVarDef* def, const char* name )
{
	int i, hash;
	
	hash = varDefNameHash.GenerateKey( name, true );
	for( i = varDefNameHash.First( hash ); i != -1; i = varDefNameHash.Next( i ) )
	{
		if( idStr::Cmp( varDefNames[i]->Name(), name ) == 0 )
		{
			break;
		}
	}
	if( i == -1 )
	{
		i = varDefNames.Append( new idVarDefName( name ) );
		varDefNameHash.Add( hash, i );
	}
	varDefNames[i]->AddDef( def );
}
*/

// Techyon RB: moved from AllocDef
/*
byte* tyLuaProgram::ReserveDefMemory( int size )
{
	byte* mem = &variables[ numVariables ];
	numVariables += size;
	if( numVariables > sizeof( variables ) )
	{
#if defined(USE_EXCEPTIONS)
		throw idCompileError( va( "Exceeded global memory size (%zd bytes)", sizeof( variables ) ) );
#else
		gameLocal.Error( "Exceeded global memory size (%zd bytes)", sizeof( variables ) );
#endif
	}
	
	memset( mem, 0, size );
	
	return mem;
}
*/
// Techyon END

// Techyon RB: moved from AllocDef
/*
idVarDef* tyLuaProgram::AllocVarDef( idTypeDef* type, const char* name, idVarDef* scope )
{
	// allocate a new def
	idVarDef* def = new idVarDef( type );
	def->scope		= scope;
	def->numUsers	= 1;
	def->num		= varDefs.Append( def );
	
	// add the def to the list with defs with this name and set the name pointer
	AddDefToNameList( def, name );
	
	return def;
}
*/
// Techyon END


/*
============
tyLuaProgram::AllocDef
============
*/
/*
idVarDef* tyLuaProgram::AllocDef( idTypeDef* type, const char* name, idVarDef* scope, bool constant )
{
	idVarDef*	def;
	idStr		element;
	idVarDef*	def_x;
	idVarDef*	def_y;
	idVarDef*	def_z;
	
	// allocate a new def
	def = AllocVarDef( type, name, scope );
	
	if( ( type->Type() == ev_vector ) || ( ( type->Type() == ev_field ) && ( type->FieldType()->Type() == ev_vector ) ) )
	{
		//
		// vector
		//
		if( !strcmp( name, RESULT_STRING ) )
		{
			// <RESULT> vector defs don't need the _x, _y and _z components
			assert( scope->Type() == ev_function );
			def->value.stackOffset	= scope->value.functionPtr->locals;
			def->initialized		= idVarDef::stackVariable;
			scope->value.functionPtr->locals += type->Size();
		}
		else if( scope->TypeDef()->Inherits( &type_object ) )
		{
			idTypeDef	newtype( ev_field, NULL, "float field", 0, &type_float );
			
			// Techyon RB: changed local type to ftype
			idTypeDef*	ftype = GetType( newtype, true );
			
			// set the value to the variable's position in the object
			def->value.ptrOffset = scope->TypeDef()->Size();
			
			// make automatic defs for the vectors elements
			// origin can be accessed as origin_x, origin_y, and origin_z
			sprintf( element, "%s_x", def->Name() );
			def_x = AllocDef( ftype, element, scope, constant );
			
			sprintf( element, "%s_y", def->Name() );
			def_y = AllocDef( ftype, element, scope, constant );
			def_y->value.ptrOffset = def_x->value.ptrOffset + sizeof( float );
			
			sprintf( element, "%s_z", def->Name() );
			def_z = AllocDef( ftype, element, scope, constant );
			def_z->value.ptrOffset = def_y->value.ptrOffset + sizeof( float );
			// Techyon END
		}
		else
		{
			// Techyon RB: from dhewm3
			idTypeDef	newtype( ev_float, &def_float, "vector float", 0, NULL );
			idTypeDef*	ftype = GetType( newtype, true );
			
			// make automatic defs for the vectors elements
			// origin can be accessed as origin_x, origin_y, and origin_z
			sprintf( element, "%s_x", def->Name() );
			def_x = AllocVarDef( ftype, element, scope );
			
			sprintf( element, "%s_y", def->Name() );
			def_y = AllocVarDef( ftype, element, scope );
			
			sprintf( element, "%s_z", def->Name() );
			def_z = AllocVarDef( ftype, element, scope );
			
			// get the memory for the full vector and point the _x, _y and _z
			// defs at the vector member offsets
			if( scope->Type() == ev_function )
			{
				// vector on stack
				def->value.stackOffset	= scope->value.functionPtr->locals;
				def->initialized		= idVarDef::stackVariable;
				scope->value.functionPtr->locals += type->Size();
				
				def_x->value.stackOffset = def->value.stackOffset;
				def_y->value.stackOffset = def_x->value.stackOffset + sizeof( float );
				def_z->value.stackOffset = def_y->value.stackOffset + sizeof( float );
			}
			else
			{
				// global vector
				def->value.bytePtr		= ReserveDefMemory( type->Size() );
				def_x->value.bytePtr	= def->value.bytePtr;
				def_y->value.bytePtr	= def_x->value.bytePtr + sizeof( float );
				def_z->value.bytePtr	= def_y->value.bytePtr + sizeof( float );
			}
			
			def_x->initialized = def->initialized;
			def_y->initialized = def->initialized;
			def_z->initialized = def->initialized;
			// Techyon END
		}
	}
	else if( scope->TypeDef()->Inherits( &type_object ) )
	{
		//
		// object variable
		//
		// set the value to the variable's position in the object
		def->value.ptrOffset = scope->TypeDef()->Size();
	}
	else if( scope->Type() == ev_function )
	{
		//
		// stack variable
		//
		// since we don't know how many local variables there are,
		// we have to have them go backwards on the stack
		def->value.stackOffset	= scope->value.functionPtr->locals;
		def->initialized		= idVarDef::stackVariable;
		
		if( type->Inherits( &type_object ) )
		{
			// objects only have their entity number on the stack, not the entire object
			scope->value.functionPtr->locals += type_object.Size();
		}
		else
		{
			scope->value.functionPtr->locals += type->Size();
		}
	}
	else
	{
		//
		// global variable
		//
		// Techyon BEGIN
		def->value.bytePtr = ReserveDefMemory( def->TypeDef()->Size() );
		// Techyon END
		
		//memset( def->value.bytePtr, 0, def->TypeDef()->Size() );
	}
	
	return def;
}
*/

/*
============
tyLuaProgram::GetDef

If type is NULL, it will match any type
============
*/
/*
idVarDef* tyLuaProgram::GetDef( const idTypeDef* type, const char* name, const idVarDef* scope ) const
{
	idVarDef*		def;
	idVarDef*		bestDef;
	int				bestDepth;
	int				depth;
	
	bestDepth = 0;
	bestDef = NULL;
	for( def = GetDefList( name ); def != NULL; def = def->Next() )
	{
		if( def->scope->Type() == ev_namespace )
		{
			depth = def->DepthOfScope( scope );
			if( !depth )
			{
				// not in the same namespace
				continue;
			}
		}
		else if( def->scope != scope )
		{
			// in a different function
			continue;
		}
		else
		{
			depth = 1;
		}
		
		if( !bestDef || ( depth < bestDepth ) )
		{
			bestDepth = depth;
			bestDef = def;
		}
	}
	
	// see if the name is already in use for another type
	if( bestDef && type && ( bestDef->TypeDef() != type ) )
	{
#if defined(USE_EXCEPTIONS)
		throw idCompileError( va( "Type mismatch on redeclaration of %s", name ) );
#else
		gameLocal.Error( "Type mismatch on redeclaration of %s", name );
#endif
	}
	
	return bestDef;
}
*/

/*
============
tyLuaProgram::FreeDef
============
*/
/*
void tyLuaProgram::FreeDef( idVarDef* def, const idVarDef* scope )
{
	idVarDef* e;
	int i;
	
	if( def->Type() == ev_vector )
	{
		idStr name;
		
		sprintf( name, "%s_x", def->Name() );
		e = GetDef( NULL, name, scope );
		if( e )
		{
			FreeDef( e, scope );
		}
		
		sprintf( name, "%s_y", def->Name() );
		e = GetDef( NULL, name, scope );
		if( e )
		{
			FreeDef( e, scope );
		}
		
		sprintf( name, "%s_z", def->Name() );
		e = GetDef( NULL, name, scope );
		if( e )
		{
			FreeDef( e, scope );
		}
	}
	
	varDefs.RemoveIndex( def->num );
	for( i = def->num; i < varDefs.Num(); i++ )
	{
		varDefs[ i ]->num = i;
	}
	
	delete def;
}
*/

/*
============
tyLuaProgram::FindFreeResultDef
============
*/
/*
idVarDef* tyLuaProgram::FindFreeResultDef( idTypeDef* type, const char* name, idVarDef* scope, const idVarDef* a, const idVarDef* b )
{
	idVarDef* def;
	
	for( def = GetDefList( name ); def != NULL; def = def->Next() )
	{
		if( def == a || def == b )
		{
			continue;
		}
		if( def->TypeDef() != type )
		{
			continue;
		}
		if( def->scope != scope )
		{
			continue;
		}
		if( def->numUsers <= 1 )
		{
			continue;
		}
		return def;
	}
	
	return AllocDef( type, name, scope, false );
}
*/

/*
================
tyLuaProgram::FindFunction

Searches for the specified function in the currently loaded script.  A full namespace should be
specified if not in the global namespace.

Returns 0 if function not found.
Returns >0 if function found.
================
*/
function_t* tyLuaProgram::FindFunction( const char* name ) const
{
	lua_getglobal( luaState, name );
	if(	!lua_isfunction( luaState, -1 ) )
	{
		lua_pop( luaState, 1 );
		gameLocal.Printf( "FindFunction: %s should be a function\n", name );
        return NULL;
    }

	lua_pop( luaState, 1 );

#if 0
	int			start;
	int			pos;
	idVarDef*	namespaceDef;
	idVarDef*	def;
	
	assert( name );
	
	idStr fullname = name;
	start = 0;
	namespaceDef = &def_namespace;
	do
	{
		pos = fullname.Find( "::", true, start );
		if( pos < 0 )
		{
			break;
		}
		
		idStr namespaceName = fullname.Mid( start, pos - start );
		def = GetDef( NULL, namespaceName, namespaceDef );
		if( !def )
		{
			// couldn't find namespace
			return NULL;
		}
		namespaceDef = def;
		
		// skip past the ::
		start = pos + 2;
	}
	while( def->Type() == ev_namespace );
	
	idStr funcName = fullname.Right( fullname.Length() - start );
	def = GetDef( NULL, funcName, namespaceDef );
	if( !def )
	{
		// couldn't find function
		return NULL;
	}
	
	if( ( def->Type() == ev_function ) && ( def->value.functionPtr->eventdef == NULL ) )
	{
		return def->value.functionPtr;
	}
#endif
	
	// is not a function, or is an eventdef
	return NULL;
}

/*
================
tyLuaProgram::FindFunction

Searches for the specified object function in the currently loaded script.

Returns 0 if function not found.
Returns >0 if function found.
================
*/
/*
function_t* tyLuaProgram::FindFunction( const char* name, const idTypeDef* type ) const
{
	const idVarDef*	tdef;
	const idVarDef*	def;
	
	// look for the function
	def = NULL;
	for( tdef = type->def; tdef != &def_object; tdef = tdef->TypeDef()->SuperClass()->def )
	{
		def = GetDef( NULL, name, tdef );
		if( def )
		{
			return def->value.functionPtr;
		}
	}
	
	return NULL;
}
*/

/*
================
tyLuaProgram::AllocFunction
================
*/
/*
function_t& tyLuaProgram::AllocFunction( idVarDef* def )
{
	if( functions.Num() >= functions.Max() )
	{
#if defined(USE_EXCEPTIONS)
		throw idCompileError( va( "Exceeded maximum allowed number of functions (%d)", functions.Max() ) );
#else
		gameLocal.Error( "Exceeded maximum allowed number of functions (%d)", functions.Max() );
#endif
	}
	
	// fill in the dfunction
	function_t& func	= *functions.Alloc();
	func.eventdef		= NULL;
	func.def			= def;
	func.type			= def->TypeDef();
	func.firstStatement	= 0;
	func.numStatements	= 0;
	func.parmTotal		= 0;
	func.locals			= 0;
	func.filenum		= filenum;
	func.parmSize.SetGranularity( 1 );
	func.SetName( def->GlobalName() );
	
	def->SetFunction( &func );
	
	return func;
}
*/

/*
================
tyLuaProgram::SetEntity
================
*/
void tyLuaProgram::SetEntity( const char* name, idEntity* ent )
{
	/*
	FIXME

	idVarDef*	def;
	idStr		defName( "$" );
	
	defName += name;
	
	def = GetDef( &type_entity, defName, &def_namespace );
	if( def && ( def->initialized != idVarDef::stackVariable ) )
	{
		// 0 is reserved for NULL entity
		if( !ent )
		{
			*def->value.entityNumberPtr = 0;
		}
		else
		{
			*def->value.entityNumberPtr = ent->entityNumber + 1;
		}
	}
	*/
}

/*
================
tyLuaProgram::AllocStatement
================
*/
/*
statement_t* tyLuaProgram::AllocStatement( void )
{
	if( statements.Num() >= statements.Max() )
	{
#if defined(USE_EXCEPTIONS)
		throw idCompileError( va( "Exceeded maximum allowed number of statements (%d)", statements.Max() ) );
#else
		gameLocal.Error( "Exceeded maximum allowed number of statements (%d)", statements.Max() );
#endif
	}
	return statements.Alloc();
}
*/

/*
==============
tyLuaProgram::BeginCompilation

called before compiling a batch of files, clears the pr struct
==============
*/
void tyLuaProgram::BeginCompilation( void )
{
	FreeData();
	
#if defined(USE_EXCEPTIONS)
	try
#endif
	{
		// TODO improve
		luaState = luaL_newstate();

		luaL_openlibs( luaState );

		// register Lua standard libs
		//luaopen_base( luaState );
		//luaopen_string( luaState );

		// TODO register custom libs
		//luaopen_sys( luaState );
		//luaopen_entity( luaState );
		//luaopen_dmath( luaState );
		//luaopen_vector( luaState );
	}
#if defined(USE_EXCEPTIONS)
	catch( idCompileError& err )
	{
		gameLocal.Error( "%s", err.error );
	}
#endif
}

/*
==============
tyLuaProgram::DisassembleStatement
==============
*/
void tyLuaProgram::DisassembleStatement( idFile* file, int instructionPointer ) const
{
#if 0
	// Techyon RB: added const
	const opcode_t*		op;
	// Techyon END
	const statement_t*	statement;
	
	statement = &statements[ instructionPointer ];
	op = &idCompiler::opcodes[ statement->op ];
	file->Printf( "%20s(%d):\t%6d: %15s\t", fileList[ statement->file ].c_str(), statement->linenumber, instructionPointer, op->opname );
	
	if( statement->a )
	{
		file->Printf( "\ta: " );
		statement->a->PrintInfo( file, instructionPointer );
	}
	
	if( statement->b )
	{
		file->Printf( "\tb: " );
		statement->b->PrintInfo( file, instructionPointer );
	}
	
	if( statement->c )
	{
		file->Printf( "\tc: " );
		statement->c->PrintInfo( file, instructionPointer );
	}
	
	file->Printf( "\n" );
#endif
}

/*
==============
tyLuaProgram::Disassemble
==============
*/
void tyLuaProgram::Disassemble( void ) const
{
#if 0
	int					i;
	int					instructionPointer;
	const function_t*	func;
	idFile*				file;
	
	file = fileSystem->OpenFileByMode( "script/disasm.txt", FS_WRITE );
	
	for( i = 0; i < functions.Num(); i++ )
	{
		func = &functions[ i ];
		if( func->eventdef )
		{
			// skip eventdefs
			continue;
		}
		
		file->Printf( "\nfunction %s() %d stack used, %d parms, %d locals {\n", func->Name(), func->locals, func->parmTotal, func->locals - func->parmTotal );
		
		for( instructionPointer = 0; instructionPointer < func->numStatements; instructionPointer++ )
		{
			DisassembleStatement( file, func->firstStatement + instructionPointer );
		}
		
		file->Printf( "}\n" );
	}
	
	fileSystem->CloseFile( file );
#endif
}

/*
==============
tyLuaProgram::FinishCompilation

Called after all files are compiled to check for errors
==============
*/
void tyLuaProgram::FinishCompilation( void )
{
	int	i;
	
	top_functions	= functions.Num();
//	top_statements	= statements.Num();
//	top_types		= types.Num();
//	top_defs		= varDefs.Num();
	top_files		= fileList.Num();
	
	variableDefaults.Clear();
	variableDefaults.SetNum( numVariables );
	
	for( i = 0; i < numVariables; i++ )
	{
		variableDefaults[ i ] = variables[ i ];
	}
}

/*
==============
tyLuaProgram::CompileStats

called after all files are compiled to report memory usage.
==============
*/
void tyLuaProgram::CompileStats( void )
{
	int	memused;
	int	memallocated;
	int	numdefs;
	int	stringspace;
	int funcMem;
	int	i;
	
	gameLocal.Printf( "---------- Compile stats ----------\n" );
	gameLocal.DPrintf( "Files loaded:\n" );
	
	stringspace = 0;
	for( i = 0; i < fileList.Num(); i++ )
	{
		gameLocal.DPrintf( "   %s\n", fileList[ i ].c_str() );
		stringspace += fileList[ i ].Allocated();
	}
	stringspace += fileList.Size();
	
//	numdefs = varDefs.Num();
	memused = 0; //varDefs.Num() * sizeof( idVarDef );
//	memused += types.Num() * sizeof( idTypeDef );
	memused += stringspace;
	
//	for( i = 0; i < types.Num(); i++ )
//	{
//		memused += types[ i ]->Allocated();
//	}
	
	funcMem = functions.MemoryUsed();
	for( i = 0; i < functions.Num(); i++ )
	{
		funcMem += functions[ i ].Allocated();
	}
	
	memallocated = funcMem + memused + sizeof( tyLuaProgram );
	
//	memused += statements.MemoryUsed();
	memused += functions.MemoryUsed();	// name and filename of functions are shared, so no need to include them
	memused += sizeof( variables );
	
	gameLocal.Printf( "\nMemory usage:\n" );
	gameLocal.Printf( "     Strings: %d, %d bytes\n", fileList.Num(), stringspace );
//	gameLocal.Printf( "  Statements: %d, %d bytes\n", statements.Num(), statements.MemoryUsed() );
	gameLocal.Printf( "   Functions: %d, %d bytes\n", functions.Num(), funcMem );
	gameLocal.Printf( "   Variables: %d bytes\n", numVariables );
	gameLocal.Printf( "    Mem used: %d bytes\n", memused );
	gameLocal.Printf( " Static data: %d bytes\n", sizeof( tyLuaProgram ) );
	gameLocal.Printf( "   Allocated: %d bytes\n", memallocated );
	gameLocal.Printf( " Thread size: %d bytes\n\n", sizeof( idThread ) );
}

/*
================
tyLuaProgram::CompileText
================
*/
/*
bool tyLuaProgram::CompileText( const char* source, const char* text, bool console )
{
	idCompiler	compiler;
	int			i;
	idVarDef*	def;
	idStr		ospath;
	
	// use a full os path for GetFilenum since it calls OSPathToRelativePath to convert filenames from the parser
	ospath = fileSystem->RelativePathToOSPath( source );
	filenum = GetFilenum( ospath );
	
#if defined(USE_EXCEPTIONS)
	try
#endif
	{
		compiler.CompileFile( text, filename, console );
		
		// check to make sure all functions prototyped have code
		for( i = 0; i < varDefs.Num(); i++ )
		{
			def = varDefs[ i ];
			if( ( def->Type() == ev_function ) && ( ( def->scope->Type() == ev_namespace ) || def->scope->TypeDef()->Inherits( &type_object ) ) )
			{
				if( !def->value.functionPtr->eventdef && !def->value.functionPtr->firstStatement )
				{
#if defined(USE_EXCEPTIONS)
					throw idCompileError( va( "function %s was not defined\n", def->GlobalName() ) );
#else
					gameLocal.Error( "function %s was not defined\n", def->GlobalName() );
#endif
				}
			}
		}
	}
#if defined(USE_EXCEPTIONS)
	catch( idCompileError& err )
	{
		if( console )
		{
			gameLocal.Printf( "%s\n", err.error );
			return false;
		}
		else
		{
			gameLocal.Error( "%s\n", err.error );
		}
	};
#endif
	
	if( !console )
	{
		CompileStats();
	}
	
	return true;
}
*/

/*
================
tyLuaProgram::CompileFunction
================
*/
/*
const function_t* tyLuaProgram::CompileFunction( const char* functionName, const char* text )
{
	bool result;
	
	result = CompileText( functionName, text, false );
	
	if( g_disasm.GetBool() )
	{
		Disassemble();
	}
	
	if( !result )
	{
		gameLocal.Error( "Compile failed." );
	}
	
	return FindFunction( functionName );
}
*/

/*
================
tyLuaProgram::CompileFile
================
*/
void tyLuaProgram::CompileFile( const char* filename )
{
	char* src;
	bool result;
	
	if( fileSystem->ReadFile( filename, ( void** )&src, NULL ) < 0 )
	{
		gameLocal.Error( "Couldn't load %s\n", filename );
	}
	
	//result = CompileText( filename, src, false );
	result = luaL_loadbuffer( luaState, src, strlen(src), filename );
	
	fileSystem->FreeFile( src );
	
	if( g_disasm.GetBool() )
	{
		Disassemble();
	}
	
	if( result )
	{
		gameLocal.Error( "Compile failed in file %s.", filename );
	}

	if( lua_pcall( luaState, 0, 0, 0 ) )
	{
		gameLocal.Error( "Cannot pcall: %s", lua_tostring( luaState, -1 ) );
	}
}

/*
================
tyLuaProgram::FreeData
================
*/
void tyLuaProgram::FreeData( void )
{
	int i;

	if( luaState )
	{
		lua_close( luaState );
		luaState = NULL;
	}
	
	// free the defs
//	varDefs.DeleteContents( true );
//	varDefNames.DeleteContents( true );
//	varDefNameHash.Free();
	
//	returnDef		= NULL;
//	returnStringDef = NULL;
//	sysDef			= NULL;
	
	// free any special types we've created
//	types.DeleteContents( true );
	
	filenum = 0;
	
	numVariables = 0;
	memset( variables, 0, sizeof( variables ) );
	
	// clear all the strings in the functions so that it doesn't look like we're leaking memory.
	for( i = 0; i < functions.Num(); i++ )
	{
		functions[ i ].Clear();
	}
	
	filename.Clear();
	fileList.Clear();
//	statements.Clear();
	functions.Clear();
	
	top_functions	= 0;
	top_statements	= 0;
	top_types		= 0;
	top_defs		= 0;
	top_files		= 0;
	
	filename = "";
}

/*
================
tyLuaProgram::Startup
================
*/
void tyLuaProgram::Startup( const char* defaultScript )
{
	gameLocal.Printf( "Initializing Lua scripts\n" );
	
	// make sure all data is freed up
	idThread::Restart();
	
	// get ready for loading scripts
	BeginCompilation();
	
	// load the default script
	if( defaultScript && *defaultScript )
	{
		CompileFile( defaultScript );
	}
	
	FinishCompilation();
}

/*
================
tyLuaProgram::Save
================
*/
void tyLuaProgram::Save( idSaveGame* savefile ) const
{
	int i;
	int currentFileNum = top_files;
	
	savefile->WriteInt( ( fileList.Num() - currentFileNum ) );
	while( currentFileNum < fileList.Num() )
	{
		savefile->WriteString( fileList[ currentFileNum ] );
		currentFileNum++;
	}
	
	for( i = 0; i < variableDefaults.Num(); i++ )
	{
		if( variables[i] != variableDefaults[i] )
		{
			savefile->WriteInt( i );
			savefile->WriteByte( variables[i] );
		}
	}
	// Mark the end of the diff with default variables with -1
	savefile->WriteInt( -1 );
	
	savefile->WriteInt( numVariables );
	for( i = variableDefaults.Num(); i < numVariables; i++ )
	{
		savefile->WriteByte( variables[i] );
	}
	
	int checksum = CalculateChecksum();
	savefile->WriteInt( checksum );
}

/*
================
tyLuaProgram::Restore
================
*/
bool tyLuaProgram::Restore( idRestoreGame* savefile )
{
	int i, num, index;
	bool result = true;
	idStr scriptname;
	
	savefile->ReadInt( num );
	for( i = 0; i < num; i++ )
	{
		savefile->ReadString( scriptname );
		CompileFile( scriptname );
	}
	
	savefile->ReadInt( index );
	while( index >= 0 )
	{
		savefile->ReadByte( variables[index] );
		savefile->ReadInt( index );
	}
	
	savefile->ReadInt( num );
	for( i = variableDefaults.Num(); i < num; i++ )
	{
		savefile->ReadByte( variables[i] );
	}
	
	int saved_checksum, checksum;
	
	savefile->ReadInt( saved_checksum );
	checksum = CalculateChecksum();
	
	if( saved_checksum != checksum )
	{
		result = false;
	}
	
	return result;
}

/*
================
tyLuaProgram::CalculateChecksum
================
*/
int tyLuaProgram::CalculateChecksum( void ) const
{
	// TODO
#if 1
	return 0;
#else
	int i, result;
	
	typedef struct
	{
		unsigned short	op;
		int				a;
		int				b;
		int				c;
		unsigned short	linenumber;
		unsigned short	file;
	} statementBlock_t;
	
	statementBlock_t*	statementList = new statementBlock_t[ statements.Num() ];
	
	memset( statementList, 0, ( sizeof( statementBlock_t ) * statements.Num() ) );
	
	// Copy info into new list, using the variable numbers instead of a pointer to the variable
	for( i = 0; i < statements.Num(); i++ )
	{
		statementList[i].op = statements[i].op;
		
		if( statements[i].a )
		{
			statementList[i].a = statements[i].a->num;
		}
		else
		{
			statementList[i].a = -1;
		}
		if( statements[i].b )
		{
			statementList[i].b = statements[i].b->num;
		}
		else
		{
			statementList[i].b = -1;
		}
		if( statements[i].c )
		{
			statementList[i].c = statements[i].c->num;
		}
		else
		{
			statementList[i].c = -1;
		}
		
		statementList[i].linenumber = statements[i].linenumber;
		statementList[i].file = statements[i].file;
	}
	
	result = MD4_BlockChecksum( statementList, ( sizeof( statementBlock_t ) * statements.Num() ) );
	
	delete [] statementList;
	
	return result;
#endif
}

/*
==============
tyLuaProgram::Restart

Restores all variables to their initial value
==============
*/
void tyLuaProgram::Restart( void )
{
	int i;
	
	idThread::Restart();
	
	//
	// since there may have been a script loaded by the map or the user may
	// have typed "script" from the console, free up any types and vardefs that
	// have been allocated after the initial startup
	//
//	for( i = top_types; i < types.Num(); i++ )
//	{
//		delete types[ i ];
//	}
//	types.SetNum( top_types, false );
	
//	for( i = top_defs; i < varDefs.Num(); i++ )
//	{
//		delete varDefs[ i ];
//	}
//	varDefs.SetNum( top_defs, false );
	
	for( i = top_functions; i < functions.Num(); i++ )
	{
		functions[ i ].Clear();
	}
	functions.SetNum( top_functions	);
	
//	statements.SetNum( top_statements );
	fileList.SetNum( top_files, false );
	filename.Clear();
	
	// reset the variables to their default values
	numVariables = variableDefaults.Num();
	for( i = 0; i < numVariables; i++ )
	{
		variables[ i ] = variableDefaults[ i ];
	}
}

/*
================
tyLuaProgram::GetFilenum
================
*/
int tyLuaProgram::GetFilenum( const char* name )
{
	if( filename == name )
	{
		return filenum;
	}
	
	idStr strippedName;
	strippedName = fileSystem->OSPathToRelativePath( name );
	if( !strippedName.Length() )
	{
		// not off the base path so just use the full path
		filenum = fileList.AddUnique( name );
	}
	else
	{
		filenum = fileList.AddUnique( strippedName );
	}
	
	// save the unstripped name so that we don't have to strip the incoming name every time we call GetFilenum
	filename = name;
	
	return filenum;
}

/*
================
tyLuaProgram::tyLuaProgram
================
*/
tyLuaProgram::tyLuaProgram()
{
	luaState = NULL;

	FreeData();
}

/*
================
tyLuaProgram::~tyLuaProgram
================
*/
tyLuaProgram::~tyLuaProgram()
{
	FreeData();
}

/*
================
tyLuaProgram::ReturnEntity
================
*/
void tyLuaProgram::ReturnEntity( idEntity* ent )
{
	/*
	if( ent )
	{
		*returnDef->value.entityNumberPtr = ent->entityNumber + 1;
	}
	else
	{
		*returnDef->value.entityNumberPtr = 0;
	}
	*/
}


#endif // #if defined(USE_LUA)