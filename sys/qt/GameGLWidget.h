#ifndef GAMEGLWIDGET_H
#define GAMEGLWIDGET_H

#include "../../libs/glew/include/GL/glew.h"
#include <QGLWidget>

class GameGLWidget : public QGLWidget
{
	Q_OBJECT

public:
	GameGLWidget();

protected:
    void keyPressEvent(QKeyEvent *event);
	void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
	
	void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

private:

signals:

public slots:
	void onTimer();

private:
	QTimer *timer;
};

#endif // #ifndef GAMEGLWIDGET_H