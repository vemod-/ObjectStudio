#include "cmixerform.h"
#include "ui_cmixerform.h"
#include "idevice.h"

CMixerForm::CMixerForm(IDevice* Device, QWidget *parent) :
    CSoftSynthsForm(Device,true,parent),
    ui(new Ui::CMixerForm)
{
    ui->setupUi(this);
    for (int i=0;i<Mixer::mixerchannels;i++)
    {
        MF=findChildren<CMixerFrame*>();
        MF[i]->init(MIXERCLASS,i);
        connect(MF[i],&CMixerFrame::SoloClicked,this,&CMixerForm::SoloClicked);
    }
    connect(ui->LeftSlider,&QAbstractSlider::valueChanged,this,&CMixerForm::LeftChanged);
    connect(ui->RightSlider,&QAbstractSlider::valueChanged,this,&CMixerForm::RightChanged);
    ui->LeftSlider->setValue(MIXERCLASS->MasterLeft*100);
    ui->RightSlider->setValue(MIXERCLASS->MasterRight*100);
    m_TimerID=startTimer(50);
}

CMixerForm::~CMixerForm()
{
    killTimer(m_TimerID);
    m_TimerID=0;
    delete ui;
}

void CMixerForm::timerEvent(QTimerEvent* /*event*/)
{
    if (!m_TimerID) return;
    //if (!m_Device) return;
    if (isVisible())
    {
        ui->StereoPeak->setValues(MIXERCLASS->PeakL,MIXERCLASS->PeakR);
        MIXERCLASS->PeakL=0;
        MIXERCLASS->PeakR=0;
        for (int i=0;i<Mixer::mixerchannels;i++)
        {
            MF[i]->peak(MIXERCLASS->Channel[i].Peak);
            MIXERCLASS->Channel[i].Peak=0;
        }
    }
}

void CMixerForm::Reset()
{
    ui->StereoPeak->reset();
    for (int i=0;i<Mixer::mixerchannels;i++)
    {
        MF[i]->reset();
    }
}

void CMixerForm::setSender(const QString &s, int Index)
{
    MF[Index]->setSender(s);
}

void CMixerForm::unserializeCustom(const QDomLiteElement* xml)
{
    if (!xml) return;
    if (const QDomLiteElement* Master = xml->elementByTag("Master"))
    {
        ui->RightSlider->setValue(Master->attributeValueInt("VolumeRight"));
        ui->LeftSlider->setValue(Master->attributeValueInt("VolumeLeft"));
        MIXERCLASS->SoloChannel = Master->attributeValueInt("SoloChannel");
        ui->LockButton->setChecked(Master->attributeValueBool("ChannelLock"));
    }
    for (int i=0;i<Mixer::mixerchannels;i++)
    {
        if (const QDomLiteElement* Channel = xml->elementByTag("Channel" + QString::number(i+1))) MF[i]->unserialize(Channel->elementByTag("Custom"));
    }
}

void CMixerForm::serializeCustom(QDomLiteElement* xml) const
{
    QDomLiteElement* Master = xml->appendChild("Master");
    Master->setAttribute("VolumeRight",ui->RightSlider->value());
    Master->setAttribute("VolumeLeft",ui->LeftSlider->value());
    Master->setAttribute("ChannelLock",ui->LockButton->isChecked());
    Master->setAttribute("SoloChannel",MIXERCLASS->SoloChannel);

    for (int i=0;i<Mixer::mixerchannels;i++)
    {
        MF[i]->serialize(xml->appendChild("Channel" + QString::number(i+1))->appendChild("Custom"));
    }
}

void CMixerForm::LeftChanged(int Value)
{
    //if (!m_Device) return;
    MIXERCLASS->MasterLeft=Value*0.01f;

    if (ui->LockButton->isChecked())
    {
        ui->RightSlider->blockSignals(true);
        ui->RightSlider->setValue(Value);
        ui->RightSlider->blockSignals(false);
        MIXERCLASS->MasterRight=Value*0.01f;
    }
    ui->LeftLabel->setText(percent2dBText(ui->LeftSlider->value()));
    ui->RightLabel->setText(percent2dBText(ui->RightSlider->value()));

}

//---------------------------------------------------------------------------

void CMixerForm::RightChanged(int Value)
{
    //if (!m_Device) return;
    MIXERCLASS->MasterRight=Value*0.01f;
    if (ui->LockButton->isChecked())
    {
        ui->LeftSlider->blockSignals(true);
        ui->LeftSlider->setValue(Value);
        ui->LeftSlider->blockSignals(false);
        MIXERCLASS->MasterLeft=Value*0.01f;
    }
    ui->LeftLabel->setText(percent2dBText(ui->LeftSlider->value()));
    ui->RightLabel->setText(percent2dBText(ui->RightSlider->value()));

}

void CMixerForm::SoloClicked(bool Pressed, int Index)
{
    if (Pressed)
    {
        for (int i=0;i<Mixer::mixerchannels;i++)
        {
            if (i != Index) MF[i]->setSolo(false);
        }
        MIXERCLASS->SoloChannel=Index;
    }
    else
    {
        MIXERCLASS->SoloChannel=-1;
    }
}

void CMixerForm::showEvent(QShowEvent *)
{
    ui->StereoPeak->setMargin(ui->LeftSlider->grooveMargin());
}
