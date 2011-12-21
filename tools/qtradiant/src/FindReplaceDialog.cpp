#include "FindReplaceDialog.h"
#include "ui_FindReplaceDialog.h"

FindReplaceDialog::FindReplaceDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::FindReplaceDialog)
{
	ui->setupUi(this);
}

FindReplaceDialog::~FindReplaceDialog()
{
	delete ui;
}

void FindReplaceDialog::changeEvent(QEvent *e)
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
