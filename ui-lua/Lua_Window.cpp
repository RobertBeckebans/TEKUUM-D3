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
#include "UserInterfaceLocal.h"


int idWindow::Lua_new( lua_State* L )
{
	int args = lua_gettop( L );

	idWindow* window = new idWindow( ( idUserInterfaceLocal* ) uiManager->LoadingGUI() ); //LuaWrapper<idWindow>::allocator(L);

	if( args == 1 && lua_isstring( L, 1 ) )
	{
		//char		buf[MAX_STRING_CHARS];

		const char* name = luaL_checkstring( L, 1 );
		window->SetInitialState( name );
	}
	else
	{
		window->SetInitialState( "<unknown>" );
	}

	luaW_push<idWindow>( L, window );
	luaW_hold<idWindow>( L, window );
//	luaW_postconstructor<idWindow>(L, args);

	return 1;
}

int idWindow::Lua_gc( lua_State* L )
{
	idWindow* window = luaW_check<idWindow>( L, 1 );
	idLib::Printf( "Lua says bye to window = %p\n", window );

	// RB: already freed by delete desktop
//	delete window;

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
			else if( idStr::Cmp( field, "rect" ) == 0 )
			{
				sprintf( buf, "%s", window->rect.c_str() );
				lua_pushstring( L, buf );
				return 1;
			}
			/*
			else if( idStr::Cmp( field, "backcolor" ) == 0 )
			{
				luaW_push<idVec4>( L, &backColor );
				return 1;
			}
			*/
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
			else if( idStr::Cmp( field, "forecolor" ) == 0 )
			{
				idVec4* v = luaW_check<idVec4>( L, 3 );
				if( v )
				{
					window->foreColor = *v;
				}

				return 0;
			}
			else if( idStr::Cmp( field, "backcolor" ) == 0 )
			{
				idVec4* v = luaW_check<idVec4>( L, 3 );
				if( v )
				{
					window->backColor = *v;
					//luaW_release<idVec4>( L, v );

					//window->gui->PrintLuaStack();

					//LuaWrapper<idVec4>::identifier( L, v ); // obj key value storage id
					//luaW_wrapperfield<idVec4>( L, LUAW_HOLDS_KEY ); // obj id counts count holds
					//float holds = lua_tonumber( L, -1 );

					//holds = 0;

					//window->gui->PrintLuaStack();
				}

				return 0;
			}
			else if( idStr::Cmp( field, "matcolor" ) == 0 )
			{
				idVec4* v = luaW_check<idVec4>( L, 3 );
				if( v )
				{
					window->matColor = *v;
				}

				return 0;
			}
			else if( idStr::Cmp( field, "hovercolor" ) == 0 )
			{
				idVec4* v = luaW_check<idVec4>( L, 3 );
				if( v )
				{
					window->hoverColor = *v;
				}

				return 0;
			}
			else if( idStr::Cmp( field, "background" ) == 0 )
			{
				const char* text = luaL_checkstring( L, 3 );
				window->backGroundName = text;

				return 0;
			}
			else if( idStr::Cmp( field, "rect" ) == 0 )
			{
				idRectangle* rect = luaW_check<idRectangle>( L, 3 );
				if( rect )
				{
					window->rect = *rect;
				}

				return 0;
			}
			else if( idStr::Cmp( field, "font" ) == 0 )
			{
				const char* fontName = luaL_checkstring( L, 3 );

#if defined(USE_IDFONT)
				font = renderSystem->RegisterFont( fontName );
#else
				window->fontNum = dc->FindFont( fontName );
#endif
			}
			else if( idStr::Cmp( field, "textalign" ) == 0 )
			{
				float num = luaL_checknumber( L, 3 );
				window->textAlign = num;

				return 0;
			}
			else if( idStr::Cmp( field, "textalignx" ) == 0 )
			{
				float num = luaL_checknumber( L, 3 );
				window->textAlignx = num;

				return 0;
			}
			else if( idStr::Cmp( field, "textaligny" ) == 0 )
			{
				float num = luaL_checknumber( L, 3 );
				window->textAligny = num;

				return 0;
			}
			else if( idStr::Cmp( field, "textscale" ) == 0 )
			{
				float num = luaL_checknumber( L, 3 );
				window->textScale = num;

				return 0;
			}
			else if( idStr::Cmp( field, "visible" ) == 0 )
			{
				int b;
				if( lua_isboolean( L, 3 ) )
				{
					b = lua_toboolean( L, 3 );
				}
				else
				{
					b = luaL_checknumber( L, 3 );
				}
				window->visible = ( b != 0 );

				return 0;
			}
			else if( idStr::Cmp( field, "noevents" ) == 0 )
			{
				int b;
				if( lua_isboolean( L, 3 ) )
				{
					b = lua_toboolean( L, 3 );
				}
				else
				{
					b = luaL_checknumber( L, 3 );
				}
				window->noEvents = ( b != 0 );

				return 0;
			}
			else if( idStr::Cmp( field, "menugui" ) == 0 )
			{
				int b;
				if( lua_isboolean( L, 3 ) )
				{
					b = lua_toboolean( L, 3 );
				}
				else
				{
					b = luaL_checknumber( L, 3 );
				}

				if( b != 0 )
				{
					window->flags |= WIN_MENUGUI;
				}

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

int idWindow::Lua_AddChild( lua_State* L )
{
	idWindow* window = luaW_check<idWindow>( L, 1 );
	if( window )
	{
		idWindow* child = luaW_check<idWindow>( L, 2 );
		if( child )
		{
			drawWin_t dwt;

			child->SetupFromState();
			child->SetParent( window );

			dwt.simp = NULL;
			dwt.win = NULL;

			/*
			RB: TODO handle simple window case

			if( win->IsSimple() )
			{
				idSimpleWindow* simple = new idSimpleWindow( win );
				dwt.simp = simple;
				drawWindows.Append( dwt );
				delete win;
			}
			else
			*/
			{
				window->AddChild( child );

				window->SetFocus( child, false );

				dwt.win = child;
				window->drawWindows.Append( dwt );
			}
		}
	}

	return 0;
}

int idWindow::Lua_AddChildren( lua_State* L )
{
	idWindow* window = luaW_check<idWindow>( L, 1 );
	if( window )
	{
		int args = lua_gettop( L );

		// self, child, child, ...
		for( int i = 2; i <= args; i++ )
		{
			idWindow* child = luaW_check<idWindow>( L, i );
			if( child )
			{
				drawWin_t dwt;

				child->SetupFromState();
				child->SetParent( window );

				dwt.simp = NULL;
				dwt.win = NULL;

				/*
				RB: TODO handle simple window case

				if( win->IsSimple() )
				{
					idSimpleWindow* simple = new idSimpleWindow( win );
					dwt.simp = simple;
					drawWindows.Append( dwt );
					delete win;
				}
				else
				*/
				{
					window->AddChild( child );

					//window->SetFocus( child, false );

					dwt.win = child;
					window->drawWindows.Append( dwt );
				}
			}
		}
	}

	return 0;
}

int idWindow::Lua_AddCommand( lua_State* L )
{
	idWindow* window = luaW_check<idWindow>( L, 1 );
	if( window )
	{
		const char* cmd = luaL_checkstring( L, 2 );
		if( cmd != NULL && cmd[0] != '\0' )
		{
			window->AddCommand( cmd );
		}
	}

	return 0;
}

int idWindow::Lua_Open( lua_State* L )
{
	idWindow* window = luaW_check<idWindow>( L, 1 );
	if( window )
	{
		window->Open();
	}

	return 0;
}

int idWindow::Lua_Close( lua_State* L )
{
	idWindow* window = luaW_check<idWindow>( L, 1 );
	if( window )
	{
		window->Close();
	}

	return 0;
}

int idWindow::Lua_SetFocus( lua_State* L )
{
	idWindow* window = luaW_check<idWindow>( L, 1 );
	if( window )
	{
		window->SetFocus( window );
	}

	return 0;
}

static const luaL_Reg Window_default[] =
{
	{ "new",			idWindow::Lua_new },
//	{ "__postctor",		luaU_build<idWindow> },
//	{ "text",			idWindow::Lua_text },
	{NULL, NULL}
};

static const luaL_Reg Window_meta[] =
{
//	{ "__new",			idWindow::Lua_new},
	{ "__postctor",		luaU_build<idWindow> },
	{ "clone",			luaU_clone<idWindow> },
	{ "__gc",			idWindow::Lua_gc},
	{ "__index",		idWindow::Lua_index },
	{ "__newindex",		idWindow::Lua_newindex },
	{ "__tostring",		idWindow::Lua_tostring },
	{ "GetText",		idWindow::Lua_GetText },
	{ "SetText",		idWindow::Lua_SetText },
//	{ "text",			idWindow::Lua_text },
	{ "AddChild",		idWindow::Lua_AddChild },
	{ "AddChildren",	idWindow::Lua_AddChildren },
	{ "AddCommand",		idWindow::Lua_AddCommand },
	{ "Open",			idWindow::Lua_Open },
	{ "Close",			idWindow::Lua_Close },
	{ "SetFocus",		idWindow::Lua_SetFocus },

	{NULL, NULL}
};

extern "C"
{

	int luaopen_Window( lua_State* L )
	{
		luaW_register< idWindow >( L, "Window", Window_default, Window_meta );

		return 0;
	}
}
