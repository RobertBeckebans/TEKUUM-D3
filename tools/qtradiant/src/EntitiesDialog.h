#ifndef ENTITIESDIALOG_H
#define ENTITIESDIALOG_H

#include <QDialog>

namespace Ui {
	class EntitiesDialog;
}

class EntitiesDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit EntitiesDialog(QWidget *parent = 0);
	~EntitiesDialog();
	
protected:
	void changeEvent(QEvent *e);
	
private slots:
	void on_buttonBox_rejected();

private:
	Ui::EntitiesDialog *ui;
};

#endif // ENTITIESDIALOG_H
