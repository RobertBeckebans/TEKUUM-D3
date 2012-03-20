#ifndef GAMEGLWIDGET_H
#define GAMEGLWIDGET_H

#include "../../libs/glew/include/GL/glew.h"
#include <QGLWidget>

class GameGLWidget : public QGLWidget
{
	Q_OBJECT

public:
	GameGLWidget(int argc, const char **argv);

protected:
    void keyPressEvent(QKeyEvent *event);
	void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
	
	void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

private:
	static bool	initializedDoom3Engine;
	int				argc;
	const char**	argv;
	void	initDoom3Engine(int argc, const char **argv);

signals:

public slots:
	void onTimer();

private:
	QTimer *timer;
};

#endif // #ifndef GAMEGLWIDGET_H