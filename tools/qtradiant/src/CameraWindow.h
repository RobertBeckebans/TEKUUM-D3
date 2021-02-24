#ifndef CAMERAWINDOW_H
#define CAMERAWINDOW_H

#include <QtWidgets/QWidget>
#include "Radiant3DWidget.h"

namespace Ui
{
class CameraWindow;
}

#if defined(USE_QT_WINDOWING)
class CameraWindow : public Radiant3DWidget
#else
class CameraWindow : public QWidget
#endif
{
	Q_OBJECT

public:
	explicit CameraWindow( QWidget* parent = 0 );
	~CameraWindow();

protected:
	void changeEvent( QEvent* e );

private:
	Ui::CameraWindow* ui;
};

#endif // CAMERAWINDOW_H
