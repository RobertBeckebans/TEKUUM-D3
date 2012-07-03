#ifndef __GTKTESTEDITOR_H__
#define __GTKTESTEDITOR_H__

//#if _MSC_VER > 1000
//#pragma once
//#endif // _MSC_VER > 1000

#include <gtkmm/window.h>
#include <gtkmm/table.h>
#include <gtkmm/button.h>

class GtkTestEditor : public Gtk::Window
{
public:
	GtkTestEditor( Gtk::Window* parent = NULL );  // standard constructor
	~GtkTestEditor();
protected:
	// TODO
private:
	Gtk::Table				_table;
	Gtk::Button				_closeButton;
	
private:
	void					OnCloseButtonClicked();
};

#endif // !__GTKTESTEDITOR_H__