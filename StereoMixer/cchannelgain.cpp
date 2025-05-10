#include "cchannelgain.h"
#include "ui_cchannelgain.h"

CChannelGain::CChannelGain(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CChannelGain)
{
    ui->setupUi(this);
    ui->label->setEffect(EffectLabel::Raised);
    ui->label->setShadowColor(QColor(255,255,255,200));
    ui->label->setTextColor(QColor(0,0,0,200));
    ui->label_2->setEffect(EffectLabel::Raised);
    ui->label_2->setShadowColor(QColor(255,255,255,200));
    ui->label_2->setTextColor(QColor(0,0,0,200));
    ui->Gain->setKnobStyle(QSynthKnob::SynthStyle);
    ui->Gain->setNotchesVisible(true);
    ui->Gain->setNotchStyle(QSynthKnob::dBNotch);
    connect(ui->Gain,&QAbstractSlider::valueChanged,this,&CChannelGain::setGain);
    connect(ui->LoCut,&QAbstractButton::toggled,this,&CChannelGain::setLoCut);
}

CChannelGain::~CChannelGain()
{
    delete ui;
}

void CChannelGain::serialize(QDomLiteElement* xml) const
{
    xml->setAttribute("Gain",ui->Gain->value());
    xml->setAttribute("LoCut",ui->LoCut->isChecked());
}

void CChannelGain::unserialize(const QDomLiteElement* xml)
{
    if (!xml) return;
    setGain(xml->attributeValueInt("Gain",100));
    setLoCut(xml->attributeValueBool("LoCut"));
}

void CChannelGain::init(CStereoMixerChannel* ch)
{
    m_Ch=ch;
    setGain(ch->Gain*100);
    setLoCut(ch->LoCut);
}

void CChannelGain::setGain(int v)
{
    if (v<100) v=100;
    ui->Gain->blockSignals(true);
    ui->Gain->setValue(v);
    ui->Gain->blockSignals(false);
    m_Ch->Gain=v*0.01;
}

void CChannelGain::setLoCut(bool v)
{
    ui->LoCut->blockSignals(true);
    ui->LoCut->setChecked(v);
    ui->LoCut->blockSignals(false);
    m_Ch->LoCut=v;
}
