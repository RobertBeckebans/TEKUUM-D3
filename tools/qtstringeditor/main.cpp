#include "QtStringEditor.h"

#include <QtWidgets/QApplication>
#include <qlabel.h>

#include "precompiled.h"
#pragma hdrstop

#include "../../game/game.h"

#ifdef ID_DEBUG_MEMORY
	#undef new
	#undef DEBUG_NEW
	#define DEBUG_NEW new
#endif

static QCoreApplication*	s_qtMain = NULL;
static QtStringEditor*		s_qtStringEditor = NULL;

void	QtStringEditorInit()
{
	s_qtMain = QApplication::instance();
	if( s_qtMain == NULL )
	{
		int argc = 0;
		s_qtMain = new QApplication( argc, NULL );
	}

	s_qtStringEditor = new QtStringEditor();
	s_qtStringEditor->show();

	com_editors |= EDITOR_QTSTRING;
}
void	QtStringEditorShutdown()
{
	// TODO
}

void	QtStringEditorRun()
{
	//s_qtMain->processEvents();
}

