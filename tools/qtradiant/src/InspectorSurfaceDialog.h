#ifndef INSPECTORSURFACEDIALOG_H
#define INSPECTORSURFACEDIALOG_H

#include <QDialog>

namespace Ui {
	class InspectorSurfaceDialog;
}

class InspectorSurfaceDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit InspectorSurfaceDialog(QWidget *parent = 0);
	~InspectorSurfaceDialog();
	
protected:
	void changeEvent(QEvent *e);
	
private:
	Ui::InspectorSurfaceDialog *ui;
};

#endif // INSPECTORSURFACEDIALOG_H
