#include "cmixerframe.h"
#include "ui_cmixerframe.h"

CMixerFrame::CMixerFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CMixerFrame)
{
    ui->setupUi(this);
    Mixer=nullptr;
    Index=0;
    connect(ui->VolSlider,&QAbstractSlider::valueChanged,this,&CMixerFrame::VolChanged);
    connect(ui->PanDial,&QAbstractSlider::valueChanged,this,&CMixerFrame::PanChanged);
    connect(ui->EffectDial,&QAbstractSlider::valueChanged,this,&CMixerFrame::EffectChanged);
    connect(ui->BypassButton,&QAbstractButton::clicked,this,&CMixerFrame::BypassButtonClicked);
    connect(ui->SoloButton,&QAbstractButton::clicked,this,&CMixerFrame::SoloButtonClicked);
    connect(ui->MuteButton,&QAbstractButton::clicked,this,&CMixerFrame::MuteButtonClicked);
    ui->VolLabel->setText(percent2dBText(ui->VolSlider->value()));
}

CMixerFrame::~CMixerFrame()
{
    delete ui;
}

void CMixerFrame::init(CMixer* MixerClass,int ChannelIndex)
{
    Mixer=MixerClass;
    Index=ChannelIndex;
    ui->VolSlider->setValue(Mixer->Channel[Index].Level*100);
    setSender(QString());
}

void CMixerFrame::peak(float Value)
{
    ui->Peak->setValue(Value);
}

void CMixerFrame::reset()
{
    ui->Peak->reset();
}

void CMixerFrame::setSolo(bool Value)
{
    ui->SoloButton->setChecked(Value);
}

void CMixerFrame::setSender(const QString& s)
{
    if (s.isEmpty())
    {
        setFontSizeScr(ui->IndexLabel,13);
        ui->IndexLabel->setText(QString::number(Index+1));
    }
    else
    {
        setFontSizeScr(ui->IndexLabel,9);
        ui->IndexLabel->setText(s);
    }
}

void CMixerFrame::PanChanged(int Value)
{
    if (!Mixer) return;
    float PanValue=Value*0.01f;
    if (PanValue<0)
    {
        Mixer->Channel[Index].PanL=1;
        Mixer->Channel[Index].PanR=1+PanValue;
    }
    else
    {
        Mixer->Channel[Index].PanR=1;
        Mixer->Channel[Index].PanL=1-PanValue;
    }
}
//---------------------------------------------------------------------------

void CMixerFrame::EffectChanged(int Value)
{
    if (!Mixer) return;
    Mixer->Channel[Index].Effect=Value*0.01f;
}
//---------------------------------------------------------------------------

void CMixerFrame::serialize(QDomLiteElement* xml) const
{
    xml->setAttribute("Volume",ui->VolSlider->value());
    xml->setAttribute("Pan",ui->PanDial->value());
    xml->setAttribute("Send",ui->EffectDial->value());
    xml->setAttribute("Mute",Mixer->Channel[Index].Mute);
    xml->setAttribute("EffectMute",Mixer->Channel[Index].EffectMute);
}

void CMixerFrame::unserialize(const QDomLiteElement* xml)
{
    if (!xml) return;
    ui->VolSlider->setValue(xml->attributeValueInt("Volume"));
    Mixer->Channel[Index].Level=ui->VolSlider->value()*0.01f;
    ui->PanDial->setValue(xml->attributeValueInt("Pan"));
    ui->EffectDial->setValue(xml->attributeValueInt("Send"));
    Mixer->Channel[Index].Mute=xml->attributeValueBool("Mute");
    Mixer->Channel[Index].EffectMute=xml->attributeValueBool("EffectMute");

    ui->SoloButton->setChecked(Mixer->SoloChannel==Index);
    ui->MuteButton->setChecked(Mixer->Channel[Index].Mute);
    ui->BypassButton->setChecked(Mixer->Channel[Index].EffectMute);
}


void CMixerFrame::VolChanged(int Value)
{
    ui->VolLabel->setText(percent2dBText(Value));
    if (!Mixer) return;
    Mixer->Channel[Index].Level=Value*0.01f;
}
//---------------------------------------------------------------------------

void CMixerFrame::MuteButtonClicked(bool Value)
{
    if (!Mixer) return;
    Mixer->Channel[Index].Mute=Value;
}
//---------------------------------------------------------------------------

void CMixerFrame::SoloButtonClicked(bool Value)
{
    if (!Mixer) return;
    emit SoloClicked(Value,Index);
}
//---------------------------------------------------------------------------

void CMixerFrame::BypassButtonClicked(bool Value)
{
    if (!Mixer) return;
    Mixer->Channel[Index].EffectMute=Value;
}

void CMixerFrame::showEvent(QShowEvent *)
{
    ui->Peak->setMargin(ui->VolSlider->grooveMargin());
    ui->frame->setMargin(ui->VolSlider->grooveMargin());
}
