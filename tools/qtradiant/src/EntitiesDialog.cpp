#include "EntitiesDialog.h"
#include "ui_EntitiesDialog.h"

EntitiesDialog::EntitiesDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::EntitiesDialog)
{
	ui->setupUi(this);
}

EntitiesDialog::~EntitiesDialog()
{
	delete ui;
}

void EntitiesDialog::changeEvent(QEvent *e)
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

void EntitiesDialog::on_buttonBox_rejected()
{
	hide();
}
