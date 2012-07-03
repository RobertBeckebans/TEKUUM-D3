#include "CameraWindow.h"
#include "ui_CameraWindow.h"

#include "../../sys/qt/GameMainWindow.h"

CameraWindow::CameraWindow( QWidget* parent ) :
#if defined(USE_QT_WINDOWING)
	Radiant3DWidget( parent, GameMainWindow::getInstance()->getGLWidget() )
#else
	QWidget( parent )//,
#endif
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
