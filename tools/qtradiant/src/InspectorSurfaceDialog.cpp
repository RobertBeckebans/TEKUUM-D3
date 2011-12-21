#include "InspectorSurfaceDialog.h"
#include "ui_InspectorSurfaceDialog.h"

InspectorSurfaceDialog::InspectorSurfaceDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::InspectorSurfaceDialog)
{
	ui->setupUi(this);
}

InspectorSurfaceDialog::~InspectorSurfaceDialog()
{
	delete ui;
}

void InspectorSurfaceDialog::changeEvent(QEvent *e)
{
	QDialog::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}
