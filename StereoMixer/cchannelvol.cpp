#include "cchannelvol.h"
#include "ui_cchannelvol.h"
#include "softsynthsdefines.h"

CChannelVol::CChannelVol(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CChannelVol)
{
    ui->setupUi(this);

    ui->VolLabel->setText("0.00 dB");
    connect(ui->VolSlider,&QAbstractSlider::valueChanged,this,&CChannelVol::setVol);
    connect(ui->VolSlider,&QAbstractSlider::valueChanged,this,&CChannelVol::volChanged);
    ui->VolSlider->setValue(100);
}

CChannelVol::~CChannelVol()
{
    delete ui;
}

int CChannelVol::vol()
{
    return ui->VolSlider->value();
}

void CChannelVol::setVol(int v)
{
    ui->VolLabel->setText(percent2dBText(v));
    ui->VolSlider->setValue(v);
}

void CChannelVol::peak(float l, float r)
{
    ui->PeakLeft->setValue(l);
    ui->PeakRight->setValue(r);
}

void CChannelVol::resetPeak()
{
    ui->PeakLeft->reset();
    ui->PeakRight->reset();
}

void CChannelVol::showEvent(QShowEvent *)
{
    ui->frame->setMargin(ui->VolSlider->grooveMargin());
    ui->PeakLeft->setMargin(ui->VolSlider->grooveMargin());
    ui->PeakRight->setMargin(ui->VolSlider->grooveMargin());
}

void CChannelVol::unserialize(const QDomLiteElement* xml)
{
    if (xml) ui->VolSlider->setValue(xml->attributeValueInt("Volume"));
}

void CChannelVol::serialize(QDomLiteElement* xml) const
{
    xml->setAttribute("Volume",ui->VolSlider->value());
}

