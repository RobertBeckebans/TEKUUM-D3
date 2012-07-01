#include "CameraWindow.h"
#include "ui_CameraWindow.h"

#include "../../sys/qt/GameMainWindow.h"

CameraWindow::CameraWindow( QWidget* parent ) :
	//QWidget( parent ),
	Radiant3DWidget( parent, GameMainWindow::getInstance()->getGLWidget() )
	//ui( new Ui::CameraWindow )
{
	//ui->setupUi( this );
}

CameraWindow::~CameraWindow()
{
	//delete ui;
}

void CameraWindow::changeEvent( QEvent* e )
{
	QWidget::changeEvent( e );
	switch( e->type() )
	{
		case QEvent::LanguageChange:
			//ui->retranslateUi( this );
			break;
		default:
			break;
	}
}
