#include "InspectorTexturesDock.h"
#include "ui_InspectorTexturesDock.h"

InspectorTexturesDock::InspectorTexturesDock(QWidget *parent) :
QWidget(parent),
ui(new Ui::InspectorTexturesDock)
{
    ui->setupUi(this);
}

InspectorTexturesDock::~InspectorTexturesDock()
{
    delete ui;
}

void InspectorTexturesDock::changeEvent(QEvent *e)
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
