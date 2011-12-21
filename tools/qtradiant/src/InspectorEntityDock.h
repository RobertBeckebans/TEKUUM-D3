#ifndef INSPECTORENTITYDOCK_H
#define INSPECTORENTITYDOCK_H

#include <QWidget>

namespace Ui {
class InspectorEntityDock;
}

class InspectorEntityDock : public QWidget
{
    Q_OBJECT
    
public:
    explicit InspectorEntityDock(QWidget *parent = 0);
    ~InspectorEntityDock();
    
protected:
    void changeEvent(QEvent *e);
    
private:
    Ui::InspectorEntityDock *ui;
};

#endif // INSPECTORENTITYDOCK_H
