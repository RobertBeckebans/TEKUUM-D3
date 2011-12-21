#ifndef INSPECTORCONSOLEDOCK_H
#define INSPECTORCONSOLEDOCK_H

#include <QWidget>

namespace Ui {
class InspectorConsoleDock;
}

class InspectorConsoleDock : public QWidget
{
    Q_OBJECT
    
public:
    explicit InspectorConsoleDock(QWidget *parent = 0);
    ~InspectorConsoleDock();
    
protected:
    void changeEvent(QEvent *e);
    
private:
    Ui::InspectorConsoleDock *ui;
};

#endif // INSPECTORCONSOLEDOCK_H
