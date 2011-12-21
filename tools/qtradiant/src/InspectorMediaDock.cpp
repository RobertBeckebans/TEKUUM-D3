#include "InspectorMediaDock.h"
#include "ui_InspectorMediaDock.h"

InspectorMediaDock::InspectorMediaDock(QWidget *parent) :
QWidget(parent),
ui(new Ui::InspectorMediaDock)
{
    ui->setupUi(this);
}

InspectorMediaDock::~InspectorMediaDock()
{
    delete ui;
}

void InspectorMediaDock::changeEvent(QEvent *e)
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
