#ifndef RADIANT3DWIDGET_H
#define RADIANT3DWIDGET_H

#include "../../../libs/glew/include/GL/glew.h"
#include <QGLWidget>
#include <QTimer>
//#include <GL/gl.h>
//#include <GL/glu.h>
//#include "glext.h"



class Radiant3DWidget : public QGLWidget
{
	Q_OBJECT
public:
#if defined(USE_QT_WINDOWING)
	explicit Radiant3DWidget( QWidget* parent = 0, const QGLWidget* shareWidget = 0 );
#else
	explicit Radiant3DWidget( QWidget* parent = 0 );
#endif

signals:

protected:
	void initializeGL();
	void resizeGL( int width, int height );
	void paintGL();

public slots:
	void onTimer();

private:
	float rotx;
	QTimer* timer;
};

#endif // RADIANT3DWIDGET_H
