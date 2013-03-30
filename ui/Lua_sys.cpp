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

#include "Lua_local.h"


// from lua baselib
static int sys_print( lua_State* L )
{
	idStr	text;
	
	// number of arguments
	int n = lua_gettop( L );
	int i;
	lua_getglobal( L, "tostring" );
	for( i = 1; i <= n; i++ )
	{
		const char* s;
		size_t l;
		lua_pushvalue( L, -1 ); // function to be called
		lua_pushvalue( L, i ); // value to print
		lua_call( L, 1, 1 );
		
		// get result
		s = lua_tolstring( L, -1, &l );
		if( s == NULL )
			return luaL_error( L, LUA_QL( "tostring" ) " must return a string to " LUA_QL( "print" ) );
			
		if( i > 1 )
		{
			//luai_writestring( "\t", 1 );
			text += "\t";
		}
		//luai_writestring( s, l );
		text += s;
		
		// pop result
		lua_pop( L, 1 );
	}
	
	//luai_writeline();
	common->Printf( "%s", text.c_str() );
	return 0;
}

static const luaL_Reg sys_funcs[] =
{
	{"print", sys_print},
	{NULL, NULL}
};

extern "C"
{

	int luaopen_sys( lua_State* L )
	{
		luaL_newlib( L, sys_funcs );
		return 1;
	}
	
}
