#ifndef RADIANT3DWIDGET_H
#define RADIANT3DWIDGET_H

#include <QGLWidget>
#include <QTimer>
#include <GL/gl.h>
// note: glext can be still included on *nix but im not sure GLU is needed ?
#include "glext.h"
#include <GL/GLU.h>

class Radiant3DWidget : public QGLWidget
{
	Q_OBJECT
public:
	explicit Radiant3DWidget(QWidget *parent = 0);
	
signals:

protected:
	void initializeGL();
	void resizeGL(int width, int height);
	void paintGL();
	
public slots:
	void onTimer();
	
private:
	float rotx;
	QTimer *timer;
};

#endif // RADIANT3DWIDGET_H
