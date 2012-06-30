#ifndef QTTEST_H
#define QTTEST_H

#include <QtGui/QMainWindow>
#include "ui_qttest.h"

class QtTest : public QMainWindow
{
	Q_OBJECT
	
public:
	//QtTest(QWidget *parent = 0, Qt::WFlags flags = 0);
	//~QtTest();
	
private:
	Ui::QtTestClass ui;
};

#endif // QTTEST1_H
