#include "precompiled.h"
#pragma hdrstop

#include <lua.hpp>

#include "Lua_local.h"
#include "Window.h"



static int Window_GC( lua_State* L )
{
	//idLib::Printf( "Lua says bye to entity = %p\n", luaget_Window( L ) );
	
	return 0;
}

static int Window_ToString( lua_State* L )
{
	lua_Window*	luaWindow;
	idWindow*	window;
	char		buf[MAX_STRING_CHARS];
	
	luaWindow = luaget_Window( L, 1 );
	window = luaWindow->win;
	
	sprintf( buf, "Window: name=%s \n", window->GetName() );
	
	lua_pushstring( L, buf );
	
	return 1;
}

static const luaL_Reg Window_ctor[] =
{
	{NULL, NULL}
};

static const luaL_Reg Window_meta[] =
{
	{"__gc",			Window_GC},
	{"__tostring",		Window_ToString},
	//{"GetText",			Window_GetText},
	//{"SetText",			Window_SetText},
	
	{NULL, NULL}
};

extern "C"
{

	int luaopen_Window( lua_State* L )
	{
		luaL_newmetatable( L, "windowDef" );
		
		lua_pushstring( L, "__index" );
		lua_pushvalue( L, -2 );		// pushes the metatable
		lua_settable( L, -3 );		// metatable.__index = metatable
		
		luaL_setfuncs( L, Window_meta, 0 );
		//luaL_setfuncs(L, "windowDef", Window_ctor);
		
		return 1;
	}
	
	void luapush_Window( lua_State* L, idWindow* window )
	{
		lua_Window* luaWindow = ( lua_Window* ) lua_newuserdata( L, sizeof( lua_Window ) );
		
		luaL_getmetatable( L, "windowDef" );
		lua_setmetatable( L, -2 );
		
		luaWindow->win = window;
	}
	
	lua_Window*     luaget_Window( lua_State* L, int argNum )
	{
		void* userData = luaL_checkudata( L, argNum, "windowDef" );
		luaL_argcheck( L, userData != NULL, argNum, "`windowDef' expected" );
		
		return ( lua_Window* ) userData;
	}
}
