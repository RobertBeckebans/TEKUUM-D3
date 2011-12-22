#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ui_InspectorConsoleDock.h"
#include "ui_InspectorEntityDock.h"
#include "ui_InspectorMediaDock.h"
#include "ui_InspectorTexturesDock.h"
#include "ui_CameraWindow.h"

#include "EntitiesDialog.h"
#include "MapInfoDialog.h"
#include "FindReplaceDialog.h"

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../../game/game.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	m_inspectorUi(new Ui::InspectorConsoleDock()),
	m_entityUi   (new Ui::InspectorEntityDock()),
	m_mediaUi    (new Ui::InspectorMediaDock()),
	m_texturesUi (new Ui::InspectorTexturesDock()),
	m_cameraUi   (new Ui::CameraWindow())
{
	ui->setupUi(this);

	// setup docks
	inspectorConsoleDock  = new QDockWidget(tr("Console"), this);
	//inspectorConsoleDock  = new InspectorConsoleDock(this);
	inspectorEntityDock   = new QDockWidget(tr("Entity"), this);
	inspectorMediaDock    = new QDockWidget(tr("Media"), this);
	inspectorTexturesDock = new QDockWidget(tr("Textures"), this);
	cameraWindow          = new QDockWidget(tr("Cam"), this);

	inspectorConsoleDock->setObjectName("inspectorConsoleDock");
	inspectorEntityDock->setObjectName("inspectorEntityDock");
	inspectorMediaDock->setObjectName("inspectorMediaDock");
	inspectorTexturesDock->setObjectName("inspectorTexturesDock");
	cameraWindow->setObjectName("cameraWindow");

	//inspectorConsoleDock->setWidget(new QWidget(inspectorConsoleDock));
	inspectorConsoleDock->setWidget(new InspectorConsoleDock(inspectorConsoleDock));
	inspectorEntityDock->setWidget(new QWidget(inspectorEntityDock));
	inspectorMediaDock->setWidget(new QWidget(inspectorMediaDock));
	inspectorTexturesDock->setWidget(new QWidget(inspectorTexturesDock));
	cameraWindow->setWidget(new QWidget(cameraWindow));

	//m_inspectorUi->setupUi(inspectorConsoleDock->widget());
	m_entityUi->setupUi(inspectorEntityDock->widget());
	m_mediaUi->setupUi(inspectorMediaDock->widget());
	m_texturesUi->setupUi(inspectorTexturesDock->widget());
	m_cameraUi->setupUi(cameraWindow->widget());

	addDockWidget(Qt::LeftDockWidgetArea, cameraWindow);
	addDockWidget(Qt::LeftDockWidgetArea, inspectorConsoleDock);
	addDockWidget(Qt::LeftDockWidgetArea, inspectorEntityDock);
	addDockWidget(Qt::LeftDockWidgetArea, inspectorMediaDock);
	addDockWidget(Qt::LeftDockWidgetArea, inspectorTexturesDock);

	m_docks.append(cameraWindow);
	m_docks.append(inspectorConsoleDock);
	m_docks.append(inspectorEntityDock);
	m_docks.append(inspectorMediaDock);
	m_docks.append(inspectorTexturesDock);

	tabifyDockWidget(inspectorConsoleDock, inspectorEntityDock);
	tabifyDockWidget(inspectorEntityDock, inspectorMediaDock);
	tabifyDockWidget(inspectorMediaDock, inspectorTexturesDock);

	// setup icons from theme (on linux) or use resource icon
	ui->actionNewMap->setIcon(QIcon::fromTheme("document-new",     QIcon(":/icons/new")));
	ui->actionOpen->  setIcon(QIcon::fromTheme("document-open",    QIcon(":/icons/open")));
	ui->actionSave->  setIcon(QIcon::fromTheme("document-save",    QIcon(":/icons/save")));
	ui->actionExit->  setIcon(QIcon::fromTheme("application-exit", QIcon(":/icons/exit")));

	ui->actionCut->        setIcon(QIcon::fromTheme("edit-cut",           QIcon(":/icons/cut")));
	ui->actionCopy->       setIcon(QIcon::fromTheme("edit-copy",          QIcon(":/icons/copy")));
	ui->actionPaste->      setIcon(QIcon::fromTheme("edit-paste",         QIcon(":/icons/paste")));
	ui->actionDelete->     setIcon(QIcon::fromTheme("edit-delete",        QIcon(":/icons/delete")));
	ui->actionPreferences->setIcon(QIcon::fromTheme("preferences-system", QIcon(":/icons/preferences")));

	// setup statusbar widgets
	statusCoords = new QLabel("x: 0.0\ty: 0.0\tz: 0.0");
	ui->statusBar->addPermanentWidget(statusCoords);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_actionAboutQt_triggered()
{
	QApplication::aboutQt();
}

void MainWindow::on_actionAbout_triggered()
{
	QMessageBox::about(this, "About QtRadiant",
					   "Developed by:<br/>" \
					   " - Ensiform<br/>" \
					   " - morsik<br/>" \
					   " - Tr3B<br/><br/>" \
					   "<a href=\"http://p.yusukekamiyamane.com/\">Fugue Icons</a> by Yusuke Kamiyamane"
					   );
}

void MainWindow::on_actionMapInfo_triggered()
{
	MapInfoDialog *mapWindow = new MapInfoDialog(this);
	mapWindow->show();
}

void MainWindow::on_actionEntityInfo_triggered()
{
	EntitiesDialog *entitiesWindow = new EntitiesDialog(this);
	entitiesWindow->show();
}

void MainWindow::on_actionFindReplace_triggered()
{
	FindReplaceDialog *findReplaceWindow = new FindReplaceDialog(this);
	findReplaceWindow->show();
}

void MainWindow::on_actionOpen_triggered()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Map..."), ".", tr("Doom3 Maps (*.map)"));
}

void MainWindow::on_actionSave_triggered()
{
	on_actionSaveAs_triggered();
}

void MainWindow::on_actionSaveAs_triggered()
{
	QFileDialog::getSaveFileName(this, tr("Save Map..."), ".", tr("Doom3 Maps (*.map)"));
}

void MainWindow::on_actionExit_triggered()
{
#if defined(USE_QTRADIANT_THREAD)
	QtRadiantShutdown();
#else
	com_editors &= ~EDITOR_QTRADIANT;
	close();
#endif
}


void MainWindow::logMessage(const char* msg)
{
	InspectorConsoleDock* console = qobject_cast<InspectorConsoleDock*>(inspectorConsoleDock->widget());

	if(console != NULL)
		console->logMessage(msg);
}