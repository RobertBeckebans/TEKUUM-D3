#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QDockWidget>
#include <QMessageBox>
#include <QFileDialog>

#include "InspectorConsoleDock.h"
#include "InspectorEntityDock.h"
#include "InspectorMediaDock.h"
#include "InspectorTexturesDock.h"
#include "CameraWindow.h"

namespace Ui
{
class MainWindow;
class InspectorConsoleDock;
class InspectorEntityDock;
class InspectorMediaDock;
class InspectorTexturesDock;
class CameraWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow( QWidget* parent = 0 );
	~MainWindow();

private slots:
	void on_actionAboutQt_triggered();

	void on_actionAbout_triggered();

	void on_actionMapInfo_triggered();

	void on_actionEntityInfo_triggered();

	void on_actionFindReplace_triggered();

	void on_actionOpen_triggered();

	void on_actionSaveAs_triggered();

	void on_actionSave_triggered();

	void on_actionExit_triggered();

private:
	QList<QDockWidget*> m_docks;
	Ui::MainWindow* ui;

	/*Ui::InspectorConsoleDock  *m_inspectorUi;
	Ui::InspectorEntityDock   *m_entityUi;
	Ui::InspectorMediaDock    *m_mediaUi;
	Ui::InspectorTexturesDock *m_texturesUi;
	Ui::CameraWindow          *m_cameraUi;*/

	QDockWidget*				inspectorConsoleDock;
	QDockWidget*				inspectorEntityDock;
	QDockWidget*				inspectorMediaDock;
	QDockWidget*				inspectorTexturesDock;
	QDockWidget*				cameraWindowDock;
#if defined(USE_QT_WINDOWING)
	QDockWidget*				gameWindowDock;
#endif

	QLabel*						statusCoords;

public:

	/*InspectorConsoleDock*		GetInspectorConsoleDock() { return inspectorConsoleDock; }
	QDockWidget*				GetInspectorEntityDock() { return inspectorEntityDock; }
	QDockWidget*				GetInspectorMediaDock() { return inspectorMediaDock; }
	QDockWidget*				GetInspectorTexturesDock() { return inspectorTexturesDock; }
	QDockWidget*				GetCameraWindow() { return cameraWindow; }*/

	void						logMessage( const char* msg );
};

#endif // MAINWINDOW_H
