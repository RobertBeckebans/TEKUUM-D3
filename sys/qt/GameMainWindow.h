#ifndef GAMEMAINWINDOW_H
#define GAMEMAINWINDOW_H

#include "../../libs/glew/include/GL/glew.h"
#include <QGLWidget>

class GameMainWindow : public QGLWidget
{
	Q_OBJECT

public:
	GameMainWindow();

protected:
    void keyPressEvent(QKeyEvent *event);

signals:

public slots:

private:
};

#endif // #ifndef GAMEMAINWINDOW_H