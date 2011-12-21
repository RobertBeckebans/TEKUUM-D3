#ifndef INSPECTORTEXTURESDOCK_H
#define INSPECTORTEXTURESDOCK_H

#include <QWidget>

namespace Ui {
class InspectorTexturesDock;
}

class InspectorTexturesDock : public QWidget
{
    Q_OBJECT
    
public:
    explicit InspectorTexturesDock(QWidget *parent = 0);
    ~InspectorTexturesDock();
    
protected:
    void changeEvent(QEvent *e);
    
private:
    Ui::InspectorTexturesDock *ui;
};

#endif // INSPECTORTEXTURESDOCK_H
