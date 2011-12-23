#ifndef GAMEMAINWINDOW_H
#define GAMEMAINWINDOW_H

#include <QWidget> 

class GameMainWindow : public QWidget
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