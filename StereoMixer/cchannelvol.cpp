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

int CChannelVol::vol() const
{
    return m_Ch->Level * 100;
}

void CChannelVol::setVol(int v)
{
    m_Ch->Level = v * 0.01f;
    ui->VolLabel->setText(percent2dBText(v));
    ui->VolSlider->blockSignals(true);
    ui->VolSlider->setValue(v);
    ui->VolSlider->blockSignals(false);
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
    if (xml) setVol(xml->attributeValueInt("Volume"));
}

void CChannelVol::serialize(QDomLiteElement* xml) const
{
    xml->setAttribute("Volume",ui->VolSlider->value());
}

