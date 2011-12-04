#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/messagedialog.h>

#include <sigc++/sigc++.h>

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../../game/game.h"

#include "GtkTestEditor.h"

#ifdef ID_DEBUG_MEMORY
#undef new
#undef DEBUG_NEW
#define DEBUG_NEW new
#endif

static Gtk::Main*	  s_gtkMain = NULL;
static GtkTestEditor* s_gtkTestEditor = NULL;



GtkTestEditor::GtkTestEditor(Gtk::Window * parent)
	:
	Gtk::Window(),
	_table(3, 2, true),
	_closeButton("Gtk::Button")
{
	//set_default_size(800, 600);

	//_table.attach(_closeButton, 0, 1, 0, 1);

	//_closeButton.signal_clicked().connect(sigc::mem_fun(*this, &GtkTestEditor::OnCloseButtonClicked());
	_closeButton.signal_clicked().connect(sigc::mem_fun(*this, &GtkTestEditor::OnCloseButtonClicked));

	//add(_table);
	add(_closeButton);

	show_all_children();

	com_editors |= EDITOR_GTKTEST;
}

GtkTestEditor::~GtkTestEditor()
{
	com_editors &= ~EDITOR_GTKTEST;
}

void GtkTestEditor::OnCloseButtonClicked()
{
	Gtk::MessageDialog dia(*this, "GtkTestEditor::OnCloseButtonClicked() :)");
    dia.run();

	GtkTestEditorShutdown();
}




void	GtkTestEditorInit()
{
	s_gtkMain = new Gtk::Main(0, NULL);
	
	s_gtkTestEditor = new GtkTestEditor();
	s_gtkTestEditor->show();
	
    //main_obj.run(window_obj);
	//main_obj.events_pending()

	//while( mainObj.

	/*
	while( ::PeekMessage(msg, NULL, NULL, NULL, PM_NOREMOVE) ) {
		// pump message
		if ( !AfxGetApp()->PumpMessage() ) {
		}
	}
	*/
}
void	GtkTestEditorShutdown()
{
	delete s_gtkTestEditor;
	s_gtkTestEditor = NULL;
}

void	GtkTestEditorRun()
{
	if( s_gtkTestEditor != NULL)
	{
		while( s_gtkMain->events_pending() )
		{
			s_gtkMain->iteration();
			//s_gtkTestEditor->show();
		}
	}
}