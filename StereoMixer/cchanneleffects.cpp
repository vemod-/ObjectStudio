#include "cchanneleffects.h"
#include "ui_cchanneleffects.h"
#include "effectlabel.h"

CChannelEffects::CChannelEffects(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CChannelEffects)
{
    ui->setupUi(this);
    ui->label_3->setEffect(EffectLabel::Raised);
    ui->label_3->setShadowColor(QColor(255,255,255,200));
    ui->label_3->setTextColor(QColor(0,0,0,200));
    connect(ui->Bypass,&QAbstractButton::toggled,this,&CChannelEffects::setBypass);
    connect(ui->Mute,&QAbstractButton::toggled,this,&CChannelEffects::setMute);
    connect(ui->Pan,&QAbstractSlider::valueChanged,this,&CChannelEffects::setPan);
    connect(ui->Pan,&QAbstractSlider::valueChanged,this,&CChannelEffects::triggerPanValueChanged);
    connect(ui->Solo,&QAbstractButton::toggled,this,&CChannelEffects::soloTriggered);
}

CChannelEffects::~CChannelEffects()
{
    delete ui;
}

void CChannelEffects::init(CStereoMixerChannel* ch)
{
    m_Ch = ch;
    auto lo=dynamic_cast<QGridLayout*>(ui->EffectLayout->layout());
    for (int i = Effect.size(); i < int(m_Ch->sendCount); i++)
    {
        auto d=new QSynthKnob(this);
        Effect.append(d);
        d->setMaximumSize(32,32);
        d->setKnobStyle(QSynthKnob::SimpleStyle);
        d->setMaximum(100);
        d->setValue(100);
        d->setNotchesVisible(true);
        d->setNotchStyle(QSynthKnob::dBNotch);
        auto l=new EffectLabel(this);
        l->setEffect(EffectLabel::Raised);
        l->setShadowColor(QColor(255,255,255,200));
        l->setTextColor(QColor(0,0,0,200));
        l->setMaximumHeight(13);
        setFontSizeScr(l,9);
        l->setText("AUX "+QString::number(i+1));
        l->setAlignment(Qt::AlignCenter);
        //l->setStyleSheet("background:transparent;");
        //QHBoxLayout* hlo=new QHBoxLayout();
        //hlo->setMargin(0);
        //hlo->setSpacing(0);
        //lo->addLayout(hlo);
        if (i & 1)
        {
            lo->addWidget(d,(i*2)+1,0,3,1,Qt::AlignHCenter);
            lo->addWidget(l,(i*2)+2,1,1,1,Qt::AlignHCenter);
        }
        else
        {
            lo->addWidget(l,(i*2)+2,0,1,1,Qt::AlignHCenter);
            lo->addWidget(d,(i*2)+1,1,3,1,Qt::AlignHCenter);
        }
        connect(d, &QSynthKnob::valueChanged, [=]() {setEffect(i);});
        setEffect(i,100);
    }
    setBypass(false);
    setMute(false);
    setPan(100);
    setSolo(false);
}

void CChannelEffects::setEffect(int e, int v)
{
    if (e>=Effect.size()) return;
    QSynthKnob* k = Effect[e];
    k->blockSignals(true);
    k->setValue(v);
    k->blockSignals(false);
    m_Ch->Effect[e]=v*0.01f;
}

void CChannelEffects::setEffect(int e)
{
    if (e>=Effect.size()) return;
    m_Ch->Effect[e]=Effect[e]->value()*0.01f;
}

void CChannelEffects::setBypass(bool v)
{
    ui->Bypass->blockSignals(true);
    ui->Bypass->setChecked(v);
    ui->Bypass->blockSignals(false);
    m_Ch->EffectMute=v;
}

void CChannelEffects::setMute(bool v)
{
    ui->Mute->blockSignals(true);
    ui->Mute->setChecked(v);
    ui->Mute->blockSignals(false);
    m_Ch->Mute=v;

}

void CChannelEffects::setPan(int v)
{
    ui->Pan->blockSignals(true);
    ui->Pan->setValue(v);
    ui->Pan->blockSignals(false);
    if (v<=100)
    {
        m_Ch->PanL=1;
        m_Ch->PanR=v*0.01f;
    }
    else
    {
        m_Ch->PanR=1;
        m_Ch->PanL=(100-(v-100))*0.01f;
    }
}

void CChannelEffects::setSolo(bool v)
{
    ui->Solo->blockSignals(true);
    ui->Solo->setChecked(v);
    ui->Solo->blockSignals(false);
}

bool CChannelEffects::isSolo()
{
    return ui->Solo->isChecked();
}

void CChannelEffects::unserialize(const QDomLiteElement* xml)
{
    if (!xml) return;
    setBypass(xml->attributeValueBool("Bypass"));
    setPan(xml->attributeValueInt("Pan"));
    setMute(xml->attributeValueBool("Mute"));
    setSolo(xml->attributeValueBool("Solo"));
    for (int i=0;i<Effect.size();i++) setEffect(i,xml->attributeValueInt("Effect"+QString::number(i+1),100));
}

void CChannelEffects::serialize(QDomLiteElement* xml) const
{
    xml->setAttribute("Bypass",ui->Bypass->isChecked());
    xml->setAttribute("Pan",ui->Pan->value());
    xml->setAttribute("Mute",ui->Mute->isChecked());
    xml->setAttribute("Solo",ui->Solo->isChecked());
    for (int i=0;i<Effect.size();i++) xml->setAttribute("Effect"+QString::number(i+1),Effect[i]->value());
}
