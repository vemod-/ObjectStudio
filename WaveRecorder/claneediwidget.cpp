#include "claneediwidget.h"
#include "ui_claneediwidget.h"

CLaneEdiWidget::CLaneEdiWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CLaneEdiWidget)
{
    ui->setupUi(this);
    ui->ScrollBar->setVisible(false);

    connect(ui->ZoomInButton,&QAbstractButton::clicked,ui->LaneEdit,&CLaneEditControl::zoomIn);
    connect(ui->ZoomOutButton,&QAbstractButton::clicked,ui->LaneEdit,&CLaneEditControl::zoomOut);
    connect(ui->ZoomMinButton,&QAbstractButton::clicked,ui->LaneEdit,&CLaneEditControl::zoomMin);
    connect(ui->ZoomMaxButton,&QAbstractButton::clicked,ui->LaneEdit,&CLaneEditControl::zoomMax);

    ui->zoomLayout->replaceWidget(ui->ScrollBar,ui->LaneEdit->horizontalScrollBar());
}

CLaneEdiWidget::~CLaneEdiWidget()
{
    delete ui;
}
