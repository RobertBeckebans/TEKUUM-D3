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


static int Vec4_new( lua_State* L )
{
	int args = lua_gettop( L );

	idVec4* v = LuaWrapper<idVec4>::allocator( L );//new idVec4();

	if( args == 4 )
	{
		v->x = luaL_checknumber( L, 1 );
		v->y = luaL_checknumber( L, 2 );
		v->z = luaL_checknumber( L, 3 );
		v->w = luaL_checknumber( L, 4 );
	}
	else
	{
		v->Zero();
	}

	luaW_push<idVec4>( L, v );
	luaW_hold<idVec4>( L, v );
	//luaW_postconstructor<idWindow>(L, args);

	return 1;
}

static int Vec4_gc( lua_State* L )
{
	idVec4* v = luaW_check<idVec4>( L, 1 );
	//idLib::Printf( "Lua says bye to Vec4 = %p\n", v );

	delete v;

	return 0;
}

static int Vec4_index( lua_State* L )
{
	idVec4* v = luaW_check<idVec4>( L, 1 );
	if( v )
	{
		if( lua_isstring( L, 2 ) )
		{
			const char* field = luaL_checkstring( L, 2 );

			switch( *field )
			{
				case 'r':
				case 'x':
					lua_pushnumber( L, v->x );
					return 1;

				case 'g':
				case 'y':
					lua_pushnumber( L, v->y );
					return 1;

				case 'b':
				case 'z':
					lua_pushnumber( L, v->z );
					return 1;

				case 'a':
				case 'w':
					lua_pushnumber( L, v->w );
					return 1;
			}
		}
	}

	return 0;
}

static int Vec4_newindex( lua_State* L )
{
	idVec4* v = luaW_check<idVec4>( L, 1 );
	if( v )
	{
		if( lua_isstring( L, 2 ) )
		{
			const char* field = luaL_checkstring( L, 2 );
			float n = luaL_checknumber( L, 3 );

			switch( *field )
			{
				case 'r':
				case 'x':
					v->x = n;
					break;

				case 'g':
				case 'y':
					v->y = n;
					break;

				case 'b':
				case 'z':
					v->z = n;
					break;

				case 'a':
				case 'w':
					v->w = n;
					break;
			}
		}
	}

	return 0;
}

static int Vec4_tostring( lua_State* L )
{
	char		buf[MAX_STRING_CHARS];

	idVec4* v = luaW_check<idVec4>( L, 1 );
	if( v )
	{
		sprintf( buf, "Vec4: '%s' ", v->ToString() );
		lua_pushstring( L, buf );
	}

	return 1;
}


static const luaL_Reg Vec4_default[] =
{
	{ "new",			Vec4_new },
//	{ "__postctor",		luaU_build<idWindow> },
	{NULL, NULL}
};

static const luaL_Reg Vec4_meta[] =
{
//	{ "__postctor",		luaU_build<idRectangle> },
//	{ "__gc",			Vec4_gc },
	{ "__index",		Vec4_index },
	{ "__newindex",		Vec4_newindex },
	{ "__tostring",		Vec4_tostring },

	{NULL, NULL}
};

extern "C"
{

	int luaopen_Vec4( lua_State* L )
	{
		luaW_register< idVec4 >( L, "Vec4", Vec4_default, Vec4_meta );

		return 0;
	}
}
