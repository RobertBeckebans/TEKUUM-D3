#if defined(USE_LUA)

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "Lua_local.h"
#include "../Game_local.h"

extern "C" void luai_writestringerror( const char* fmt, ... )
{
	va_list		argptr;
	char		text[MAX_STRING_CHARS];
	
	va_start( argptr, fmt );
	idStr::vsnPrintf( text, sizeof( text ), fmt, argptr );
	va_end( argptr );
	
	gameLocal.Error( "%s", text );
}

#endif // #if defined(USE_LUA)