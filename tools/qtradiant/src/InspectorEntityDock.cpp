#include "InspectorEntityDock.h"
#include "ui_InspectorEntityDock.h"

InspectorEntityDock::InspectorEntityDock(QWidget *parent) :
QWidget(parent),
ui(new Ui::InspectorEntityDock)
{
    ui->setupUi(this);
}

InspectorEntityDock::~InspectorEntityDock()
{
    delete ui;
}

void InspectorEntityDock::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
