#include <QtGui/QApplication>
#include <QLocale>
#include <QTranslator>
#include <QTextCodec>
#include <QDebug>
#include <Qthread>


#include "MainWindow.h"

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../../game/game.h"

static QCoreApplication*		s_qtMain = NULL;
static MainWindow*				s_qtRadiant = NULL;
static bool						didTranslator = false;

idRenderWorld*                          rw;
idSoundWorld*                           sw;


#if defined(USE_QTRADIANT_THREAD)

class RadiantThread : public QThread
{
protected:
	void run()
	{
		s_qtMain->processEvents();
	}
};

static RadiantThread*           s_qtRadiantThread = NULL;

void    QtRadiantInit()
{
	// make sure the renderer is initialized
	if( !renderSystem->IsOpenGLRunning() )
	{
		common->Printf( "no OpenGL running\n" );
		return;
	}
	
	if( s_qtRadiant && s_qtRadiant->isVisible() )
	{
		common->Printf( "QtRadiant already running\n" );
		return;
	}
	
	if( rw == NULL )
	{
		rw = renderSystem->AllocRenderWorld();
		rw->InitFromMap( NULL );
	}
	if( sw == NULL )
	{
		sw = soundSystem->AllocSoundWorld( rw );
	}
	
	if( s_qtMain == NULL )
	{
		int argc = 0;
		s_qtMain = new QApplication( argc, NULL );
	}
	
	if( didTranslator == false )
	{
		QString locale = QLocale::system().name();
		
		QTranslator translator;
		translator.load( QString( ":/translations/" ) + locale );
		QTextCodec::setCodecForTr( QTextCodec::codecForName( "utf8" ) );
		s_qtMain->installTranslator( &translator );
		didTranslator = true;
	}
	
	// thread should be deleted in QtRadiantShutdown
	s_qtRadiantThread = new RadiantThread();
	
	s_qtRadiantThread->start();
	
	if( s_qtRadiant == NULL )
	{
		s_qtRadiant = new MainWindow();
	}
	s_qtRadiant->show();
	
	com_editors |= EDITOR_QTRADIANT;
	
	common->Printf( "Launching QtRadiant\n" );
}
void    QtRadiantShutdown()
{
	if( s_qtRadiantThread != NULL )
	{
		s_qtRadiantThread->terminate();
		delete s_qtRadiantThread;
		s_qtRadiantThread = NULL;
	}
	com_editors &= ~EDITOR_QTRADIANT;
	
	common->Printf( "Closing QtRadiant\n" );
	s_qtMain->closeAllWindows();
	if( s_qtRadiant != NULL )
	{
		if( s_qtRadiant->isVisible() == false )
		{
			delete s_qtRadiant;
			s_qtRadiant = NULL;
		}
		else
		{
			s_qtRadiant->close();
			delete s_qtRadiant;
			s_qtRadiant = NULL;
		}
	}
}

void    QtRadiantRun()
{
	/*      if(s_qtRadiantThread == NULL)
	        {
	                s_qtRadiantThread = new RadiantThread();
	        }
	        s_qtRadiantThread->start();*/
}

void	QtRadiantPrint( const char* text )
{
	if( s_qtRadiant != NULL && s_qtRadiant->isVisible() )
	{
		s_qtRadiant->GetInspectorConsoleDock()->LogMessage( text );
	}
}

#else

void	QtRadiantInit()
{
	// make sure the renderer is initialized
	if( !renderSystem->IsOpenGLRunning() )
	{
		common->Printf( "no OpenGL running\n" );
		return;
	}

	if( s_qtRadiant ) // && s_qtRadiant->isVisible())
	{
		common->Printf( "QtRadiant already running\n" );
		return;
	}

	if( rw == NULL )
	{
		rw = renderSystem->AllocRenderWorld();
		rw->InitFromMap( NULL );
	}

	if( sw == NULL )
	{
		sw = soundSystem->AllocSoundWorld( rw );
	}

	s_qtMain = QApplication::instance();
	if( s_qtMain == NULL )
	{
		int argc = 0;
		s_qtMain = new QApplication( argc, NULL );
	}

	if( didTranslator == false )
	{
		QString locale = QLocale::system().name();

		QTranslator translator;
		translator.load( QString( ":/translations/" ) + locale );
		QTextCodec::setCodecForTr( QTextCodec::codecForName( "utf8" ) );
		s_qtMain->installTranslator( &translator );
		didTranslator = true;
	}

	if( s_qtRadiant == NULL )
	{
		s_qtRadiant = new MainWindow();
	}
	s_qtRadiant->show();

	com_editors |= EDITOR_QTRADIANT;

	common->Printf( "Launching QtRadiant\n" );
}

void	QtRadiantShutdown()
{
	if( s_qtRadiant != NULL )
	{
		delete s_qtRadiant;
		s_qtRadiant = NULL;

		com_editors &= ~EDITOR_QTRADIANT;
	}
}

void	QtRadiantRun()
{
	if( s_qtMain != NULL && s_qtRadiant != NULL )
	{
		//while(s_qtMain->hasPendingEvents())
		{
			//s_qtMain->processEvents();
		}
	}
}

void	QtRadiantPrint( const char* text )
{
	if( s_qtRadiant != NULL ) // && s_qtRadiant->isVisible())
	{
		s_qtRadiant->logMessage( text );
	}
}


#endif // #if defined(USE_QTRADIANT_THREAD)


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