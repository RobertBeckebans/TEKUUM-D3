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
#include "Window.h"


int idWindow::Lua_new( lua_State* L )
{
	int args = lua_gettop( L );
	
	idWindow* window = new idWindow(); //LuaWrapper<idWindow>::allocator(L);
	
	if( args == 1 && lua_isstring( L, 1 ) )
	{
		char		buf[MAX_STRING_CHARS];
		
		const char* name = luaL_checkstring( L, 1 );
		window->SetInitialState( name );
	}
	
	luaW_push<idWindow>( L, window );
	luaW_hold<idWindow>( L, window );
	//luaW_postconstructor<idWindow>(L, args);
	
	return 1;
}

int idWindow::Lua_gc( lua_State* L )
{
	idLib::Printf( "Lua says bye to windows = %p\n", luaW_check<idWindow>( L, 1 ) );
	
	return 0;
}

int idWindow::Lua_index( lua_State* L )
{
	idWindow* window = luaW_check<idWindow>( L, 1 );
	if( window )
	{
		if( lua_isstring( L, 2 ) )
		{
			char		buf[MAX_STRING_CHARS];
			
			const char* field = luaL_checkstring( L, 2 );
			
			// TODO more fields
			if( idStr::Cmp( field, "name" ) == 0 )
			{
				sprintf( buf, "%s", window->name.c_str() );
				lua_pushstring( L, buf );
				return 1;
			}
			else if( idStr::Cmp( field, "text" ) == 0 )
			{
				sprintf( buf, "%s", window->text.c_str() );
				lua_pushstring( L, buf );
				return 1;
			}
		}
	}
	
	return luaW_index<idWindow>( L );
}

int idWindow::Lua_newindex( lua_State* L )
{
	idWindow* window = luaW_check<idWindow>( L, 1 );
	if( window )
	{
		if( lua_isstring( L, 2 ) )
		{
			const char* field = luaL_checkstring( L, 2 );
			
			// TODO more fields
			if( idStr::Cmp( field, "text" ) == 0 )
			{
				const char* text = luaL_checkstring( L, 3 );
				window->text = text;
				
				return 0;
			}
			else if( idStr::Cmp( field, "background" ) == 0 )
			{
				const char* text = luaL_checkstring( L, 3 );
				window->backGroundName = text;
				
				return 0;
			}
		}
	}
	
	return luaW_newindex<idWindow>( L );
}

int idWindow::Lua_tostring( lua_State* L )
{
	char		buf[MAX_STRING_CHARS];
	
	idWindow* window = luaW_check<idWindow>( L, 1 );
	if( window )
	{
		sprintf( buf, "Window: name='%s' ", window->name.c_str() );
		lua_pushstring( L, buf );
	}
	
	return 1;
}

int idWindow::Lua_GetText( lua_State* L )
{
	char		buf[MAX_STRING_CHARS];
	
	idWindow* window = luaW_check<idWindow>( L, 1 );
	if( window )
	{
		sprintf( buf, "%s", window->text.c_str() );
		lua_pushstring( L, buf );
		return 1;
	}
	
	return 0;
}

int idWindow::Lua_SetText( lua_State* L )
{
	idWindow* window = luaW_check<idWindow>( L, 1 );
	if( window )
	{
		const char* text = luaL_checkstring( L, 2 );
		window->text = text;
	}
	
	return 0;
}

int idWindow::Lua_text( lua_State* L )
{
	char		buf[MAX_STRING_CHARS];
	
	idWindow* window = luaW_check<idWindow>( L, 1 );
	if( window )
	{
		if( lua_isstring( L, 2 ) )
		{
			const char* text = luaL_checkstring( L, 2 );
			window->text = text;
			return 0;
		}
		else
		{
			sprintf( buf, "%s", window->text.c_str() );
			lua_pushstring( L, buf );
			return 1;
		}
	}
	
	return 0;
}

static const luaL_Reg windowDef_default[] =
{
//	{ "new",			idWindow::Lua_new },
//	{ "__postctor",		luaU_build<idWindow> },
	{ "text",			idWindow::Lua_text },
	{NULL, NULL}
};

static const luaL_Reg windowDef_meta[] =
{
//	{ "__new",			idWindow::Lua_new},
	{ "__postctor",		luaU_build<idWindow> },
	{ "__gc",			idWindow::Lua_gc},
	{ "__index",		idWindow::Lua_index },
	{ "__newindex",		idWindow::Lua_newindex },
	{ "__tostring",		idWindow::Lua_tostring },
	{ "GetText",		idWindow::Lua_GetText },
	{ "SetText",		idWindow::Lua_SetText },
	
	{ "text",			idWindow::Lua_text },
	
	{NULL, NULL}
};

extern "C"
{

	int luaopen_Window( lua_State* L )
	{
		luaW_register< idWindow >( L, "windowDef", windowDef_default, windowDef_meta );
		
		return 0;
	}
}
