/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 2012-2013 Robert Beckebans

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

#ifndef __LUA_LOCAL_H__
#define __LUA_LOCAL_H__

#include <lua.hpp>
#include <luawrapper.hpp>
extern "C"
{
#include "../libs/luasocket/src/luasocket.h"
}

class idWindow;
class idUserInterface;


#define lua_regconstint( L, name )	\
	lua_pushstring( L, #name ); \
	lua_pushinteger( L, name ); \
	lua_settable( L, -3 )

#define lua_regglobalconstint( L, name )	\
	lua_getglobal( L, "_G" ); \
	lua_pushinteger( L, name ); \
	lua_setfield( L, -2, #name ); \
	lua_pop( L, 1 );

extern "C"
{
//	int			luaopen_sys( lua_State* L );

	int			luareg_globals( lua_State* L );

	int			luaopen_Window( lua_State* L );
	int			luaopen_Rectangle( lua_State* L );
	int			luaopen_Vec3( lua_State* L );
	int			luaopen_Vec4( lua_State* L );
}



#endif /* !__LUA_LOCAL_H__ */