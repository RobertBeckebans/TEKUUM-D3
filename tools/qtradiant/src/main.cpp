#include <QtGui/QApplication>
#include <QLocale>
#include <QTranslator>
#include <QTextCodec>
#include <QDebug>


#include "MainWindow.h"

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../../game/game.h"

#ifdef ID_DEBUG_MEMORY
#undef new
#undef DEBUG_NEW
#define DEBUG_NEW new
#endif

static QApplication*		s_qtMain = NULL;
static MainWindow*			s_qtRadiant = NULL;

void	QtRadiantInit()
{
	int argc = 0;
	s_qtMain = new QApplication(argc, NULL);

	s_qtRadiant = new MainWindow();
	s_qtRadiant->show();

	com_editors |= EDITOR_QTRADIANT;

	//s_qtMain->exec();
}

void	QtRadiantShutdown()
{
	if(s_qtRadiant != NULL)
	{
		delete s_qtRadiant;
		s_qtRadiant = NULL;

		com_editors &= ~EDITOR_QTRADIANT;
	}
}

void	QtRadiantRun()
{
	if(s_qtMain != NULL && s_qtRadiant != NULL)
	{
		//while(s_qtMain->hasPendingEvents())
		{
			//s_qtMain->processEvents();
		}
	}
}

/*
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QString locale = QLocale::system().name();

	QTranslator translator;
	translator.load(QString(":/translations/") + locale);
	QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
	a.installTranslator(&translator);

	MainWindow w;
	w.show();
	return a.exec();
}
*/