#ifndef INSPECTORMEDIADOCK_H
#define INSPECTORMEDIADOCK_H

#include <QWidget>

namespace Ui {
class InspectorMediaDock;
}

class InspectorMediaDock : public QWidget
{
    Q_OBJECT
    
public:
    explicit InspectorMediaDock(QWidget *parent = 0);
    ~InspectorMediaDock();
    
protected:
    void changeEvent(QEvent *e);
    
private:
    Ui::InspectorMediaDock *ui;
};

#endif // INSPECTORMEDIADOCK_H
