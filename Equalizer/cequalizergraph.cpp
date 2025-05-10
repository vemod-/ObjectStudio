#include "cequalizergraph.h"
#include "ui_cequalizergraph.h"

CEqualizerGraph::CEqualizerGraph(QWidget *parent) :
    QCanvas(parent,1),
    ui(new Ui::CEqualizerGraph)
{
    ui->setupUi(this);
    //emit Changed();
}

CEqualizerGraph::~CEqualizerGraph()
{
    delete ui;
}

void CEqualizerGraph::resizeEvent(QResizeEvent* e)
{
    QCanvas::resizeEvent(e);
    emit Changed();
}

void CEqualizerGraph::showEvent(QShowEvent* e) {
    QCanvas::showEvent(e);
    emit Changed();
}
