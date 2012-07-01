#ifndef GAMEMAINWINDOW_H
#define GAMEMAINWINDOW_H

#include "../../libs/glew/include/GL/glew.h"
#include <QWidget>

#include "GameGLWidget.h"

class GameMainWindow : public QWidget
{
private:
	Q_OBJECT
	
	GameMainWindow();
	GameMainWindow( int argc, const char** argv );
	GameMainWindow( const GameMainWindow& );
	GameMainWindow& operator= ( const GameMainWindow& );
	
public:
	//GameMainWindow( int argc, const char** argv );
	
	static void				createInstance( int argc, const char** argv );
	static GameMainWindow*	getInstance();
	
	QGLWidget*				getGLWidget() const
	{
		return glWidget;
	}
	
protected:
	void					keyPressEvent( QKeyEvent* event );
	void					keyReleaseEvent( QKeyEvent* event );
	void					mousePressEvent( QMouseEvent* event );
	void					mouseReleaseEvent( QMouseEvent* event );
	void					mouseMoveEvent( QMouseEvent* event );
	
	//void initializeGL();
	//void paintGL();
	//void resizeGL(int width, int height);
	
private:
	int						QKeyToDKey( QKeyEvent* event );
	
signals:

public slots:
	//void onTimer();
	
private:
	static GameMainWindow*	instance;
	//QTimer *timer;
	GameGLWidget*			glWidget;
};

#endif // #ifndef GAMEMAINWINDOW_H