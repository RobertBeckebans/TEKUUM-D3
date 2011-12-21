#include "qttest.h"
#include <QtGui/QApplication>
#include <qlabel.h>

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../../game/game.h"

//#include "QtTestEditor.h"

#ifdef ID_DEBUG_MEMORY
#undef new
#undef DEBUG_NEW
#define DEBUG_NEW new
#endif

static QApplication*		s_qtMain = NULL;
static QtTest*				s_qtTestEditor = NULL;

void	QtTestEditorInit()
{
	int argc = 0;
	s_qtMain = new QApplication(argc, NULL);

	s_qtTestEditor = new QtTest();
	s_qtTestEditor->show();

	com_editors |= EDITOR_QTTEST;
}
void	QtTestEditorShutdown()
{
	// TODO
}

void	QtTestEditorRun()
{
	//s_qtMain->processEvents();
}

