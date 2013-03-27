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
