#ifndef GAMEMAINWINDOW_H
#define GAMEMAINWINDOW_H

#include "../../libs/glew/include/GL/glew.h"
#include <QWidget>

#include "GameGLWidget.h"

class GameMainWindow : public QWidget
{
	Q_OBJECT

public:
	GameMainWindow(int argc, const char **argv);

protected:
    void keyPressEvent(QKeyEvent *event);
	void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
	
	//void initializeGL();
    //void paintGL();
    //void resizeGL(int width, int height);

private:
	void initDoom3Engine(int argc, const char **argv);

signals:

public slots:
	//void onTimer();	

private:
	//QTimer *timer;
	GameGLWidget *glWidget;
};

#endif // #ifndef GAMEMAINWINDOW_H