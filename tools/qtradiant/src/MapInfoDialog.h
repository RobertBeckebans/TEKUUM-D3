#ifndef MAPINFODIALOG_H
#define MAPINFODIALOG_H

#include <QDialog>

namespace Ui {
	class MapInfoDialog;
}

class MapInfoDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit MapInfoDialog(QWidget *parent = 0);
	~MapInfoDialog();
	
protected:
	void changeEvent(QEvent *e);
	
private slots:
	void on_buttonBox_rejected();

private:
	Ui::MapInfoDialog *ui;
};

#endif // MAPINFODIALOG_H
