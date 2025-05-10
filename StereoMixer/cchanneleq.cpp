#include "cchanneleq.h"
#include "ui_cchanneleq.h"
#include "effectlabel.h"

CChannelEQ::CChannelEQ(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CChannelEQ)
{
    ui->setupUi(this);
    auto lo=dynamic_cast<QGridLayout*>(ui->gridLayout_2);
    ui->LoLabel->setEffect(EffectLabel::Raised);
    ui->LoLabel->setShadowColor(QColor(255,255,255,200));
    ui->LoLabel->setTextColor(QColor(0,0,0,200));
    ui->HiLabel->setEffect(EffectLabel::Raised);
    ui->HiLabel->setShadowColor(QColor(255,255,255,200));
    ui->HiLabel->setTextColor(QColor(0,0,0,200));
    ui->MidLabel->setEffect(EffectLabel::Raised);
    ui->MidLabel->setShadowColor(QColor(255,255,255,200));
    ui->MidLabel->setTextColor(QColor(0,0,0,200));

    ui->Lo->setKnobStyle(QSynthKnob::SimpleStyle);
    ui->Lo->setNotchesVisible(true);
    ui->Lo->setNotchStyle(QSynthKnob::dBNotch);

    ui->Hi->setKnobStyle(QSynthKnob::SimpleStyle);
    ui->Hi->setNotchesVisible(true);
    ui->Hi->setNotchStyle(QSynthKnob::dBNotch);

    ui->Mid->setKnobStyle(QSynthKnob::SimpleStyle);
    ui->Mid->setNotchesVisible(true);
    ui->Mid->setNotchStyle(QSynthKnob::dBNotch);

    connect(ui->Hi,&QAbstractSlider::valueChanged,this,&CChannelEQ::setHi);
    connect(ui->Lo,&QAbstractSlider::valueChanged,this,&CChannelEQ::setLo);
    connect(ui->Mid,&QAbstractSlider::valueChanged,this,&CChannelEQ::setMid);
    connect(ui->EQ,&QAbstractButton::toggled,this,&CChannelEQ::setEQ);

    lo->removeWidget(ui->Hi);
    lo->removeWidget(ui->Mid);
    lo->removeWidget(ui->Lo);
    lo->removeWidget(ui->HiLabel);
    lo->removeWidget(ui->MidLabel);
    lo->removeWidget(ui->LoLabel);

    lo->addWidget(ui->Hi,1,0,3,1,Qt::AlignHCenter);
    lo->addWidget(ui->HiLabel,2,1,1,1,Qt::AlignHCenter);
    lo->addWidget(ui->MidLabel,4,0,1,1,Qt::AlignHCenter);
    lo->addWidget(ui->Mid,3,1,3,1,Qt::AlignHCenter);
    lo->addWidget(ui->Lo,5,0,3,1,Qt::AlignHCenter);
    lo->addWidget(ui->LoLabel,6,1,1,1,Qt::AlignHCenter);
}

CChannelEQ::~CChannelEQ()
{
    delete ui;
}

void CChannelEQ::serialize(QDomLiteElement* xml) const
{
    xml->setAttribute("EQActive",ui->EQ->isChecked());
    xml->setAttribute("HiGain",ui->Hi->value());
    xml->setAttribute("MidGain",ui->Mid->value());
    xml->setAttribute("LoGain",ui->Lo->value());

}

void CChannelEQ::unserialize(const QDomLiteElement* xml)
{
    if (!xml) return;
    setEQ(xml->attributeValueBool("EQActive"));
    setHi(xml->attributeValueInt("HiGain",100));
    setMid(xml->attributeValueInt("MidGain",100));
    setLo(xml->attributeValueInt("LoGain",100));
}

void CChannelEQ::init(CStereoMixerChannel* ch)
{
    m_Ch=ch;
    setHi(ch->f3L.hg*100);
    setLo(ch->f3L.lg*100);
    setMid(ch->f3L.mg*100);
    setEQ(ch->EQ);
}

void CChannelEQ::setHi(int v)
{
    ui->Hi->blockSignals(true);
    ui->Hi->setValue(v);
    ui->Hi->blockSignals(false);
    m_Ch->f3L.hg=v*0.01;
    m_Ch->f3R.hg=v*0.01;
}

void CChannelEQ::setLo(int v)
{
    ui->Lo->blockSignals(true);
    ui->Lo->setValue(v);
    ui->Lo->blockSignals(false);
    m_Ch->f3L.lg=v*0.01;
    m_Ch->f3R.lg=v*0.01;
}

void CChannelEQ::setMid(int v)
{
    ui->Mid->blockSignals(true);
    ui->Mid->setValue(v);
    ui->Mid->blockSignals(false);
    m_Ch->f3L.mg=v*0.01;
    m_Ch->f3R.mg=v*0.01;
}

void CChannelEQ::setEQ(bool v)
{
    ui->EQ->blockSignals(true);
    ui->EQ->setChecked(v);
    ui->EQ->blockSignals(false);
    m_Ch->EQ=v;
    m_Ch->EQ=v;
}
