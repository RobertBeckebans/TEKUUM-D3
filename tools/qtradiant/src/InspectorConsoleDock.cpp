#include "InspectorConsoleDock.h"
#include "ui_InspectorConsoleDock.h"

InspectorConsoleDock::InspectorConsoleDock( QWidget* parent ) :
	QWidget( parent ),
	ui( new Ui::InspectorConsoleDock )
{
	ui->setupUi( this );
}

InspectorConsoleDock::~InspectorConsoleDock()
{
	delete ui;
}

void InspectorConsoleDock::changeEvent( QEvent* e )
{
	QWidget::changeEvent( e );
	switch( e->type() )
	{
		case QEvent::LanguageChange:
			ui->retranslateUi( this );
			break;
		default:
			break;
	}
}


void InspectorConsoleDock::logMessage( const char* msg )
{
	QString s( msg );
	//s = s.trimmed();
	ui->consoleOutputText->append( s );
	//ui->consoleOutputText->setVisible(true);
}