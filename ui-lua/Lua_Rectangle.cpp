/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 2013 Robert Beckebans

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

#include <lua.hpp>
#include <luawrapper.hpp>
#include <luawrapperutil.hpp>

#include "Lua_local.h"
#include "Rectangle.h"



int idRectangle::Lua_new( lua_State* L )
{
	int args = lua_gettop( L );
	
	idRectangle* rect = new idRectangle(); //LuaWrapper<idWindow>::allocator(L);
	
	if( args == 4 )
	{
		rect->x = luaL_checknumber( L, 1 );
		rect->y = luaL_checknumber( L, 2 );
		rect->w = luaL_checknumber( L, 3 );
		rect->h = luaL_checknumber( L, 4 );
	}
	
	luaW_push<idRectangle>( L, rect );
	luaW_hold<idRectangle>( L, rect );
	//luaW_postconstructor<idWindow>(L, args);
	
	return 1;
}


int idRectangle::Lua_gc( lua_State* L )
{
	idRectangle* rect = luaW_check<idRectangle>( L, 1 );
	//idLib::Printf( "Lua says bye to rectangle = %p\n", rect );
	
	delete rect;
	
	return 0;
}

int idRectangle::Lua_index( lua_State* L )
{
	idRectangle* rect = luaW_check<idRectangle>( L, 1 );
	if( rect )
	{
		if( lua_isstring( L, 2 ) )
		{
			char		buf[MAX_STRING_CHARS];
			
			const char* field = luaL_checkstring( L, 2 );
			
			switch( *field )
			{
				case 'x':
					lua_pushnumber( L, rect->x );
					return 1;
					
				case 'y':
					lua_pushnumber( L, rect->y );
					return 1;
					
				case 'w':
					lua_pushnumber( L, rect->w );
					return 1;
					
				case 'h':
					lua_pushnumber( L, rect->h );
					return 1;
			}
		}
	}
	
	return 0; //return luaW_index<idRectangle>( L );
}

int idRectangle::Lua_newindex( lua_State* L )
{
	idRectangle* rect = luaW_check<idRectangle>( L, 1 );
	if( rect )
	{
		if( lua_isstring( L, 2 ) )
		{
			const char* field = luaL_checkstring( L, 2 );
			float n = luaL_checknumber( L, 3 );
			
			switch( *field )
			{
				case 'x':
					rect->x = n;
					return 0;
					
				case 'y':
					rect->y = n;
					return 0;
					
				case 'w':
					rect->w = n;
					return 0;
					
				case 'h':
					rect->h = n;
					return 0;
			}
		}
	}
	
	return 0; //luaW_newindex<idRectangle>( L );
}

int idRectangle::Lua_tostring( lua_State* L )
{
	char		buf[MAX_STRING_CHARS];
	
	idRectangle* rect = luaW_check<idRectangle>( L, 1 );
	if( rect )
	{
		sprintf( buf, "Rectangle: name='%s' ", rect->String() );
		lua_pushstring( L, buf );
	}
	
	return 1;
}


static const luaL_Reg Rectangle_default[] =
{
	{ "new",			idRectangle::Lua_new },
//	{ "__postctor",		luaU_build<idWindow> },
	{NULL, NULL}
};

static const luaL_Reg Rectangle_meta[] =
{
//	{ "__postctor",		luaU_build<idRectangle> },
//	{ "__gc",			idRectangle::Lua_gc},
	{ "__index",		idRectangle::Lua_index },
	{ "__newindex",		idRectangle::Lua_newindex },
	{ "__tostring",		idRectangle::Lua_tostring },
	
	{NULL, NULL}
};

extern "C"
{

	int luaopen_Rectangle( lua_State* L )
	{
		luaW_register< idRectangle >( L, "Rectangle", Rectangle_default, Rectangle_meta );
		
		return 0;
	}
}
