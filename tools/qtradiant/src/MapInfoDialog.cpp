#include "MapInfoDialog.h"
#include "ui_MapInfoDialog.h"

MapInfoDialog::MapInfoDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::MapInfoDialog)
{
	ui->setupUi(this);
}

MapInfoDialog::~MapInfoDialog()
{
	delete ui;
}

void MapInfoDialog::changeEvent(QEvent *e)
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

void MapInfoDialog::on_buttonBox_rejected()
{
	hide();
}
