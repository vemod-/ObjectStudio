#include "cstereomixerform.h"
#include "ui_cstereomixerform.h"
#include <QHBoxLayout>

CStereoMixerForm::CStereoMixerForm(IDevice *Device, QWidget *parent) :
    CSoftSynthsForm(Device, true, parent),
    ui(new Ui::CStereoMixerForm)
{
    ui->setupUi(this);

    m_Mx=new CMixerWidget(this);
    auto ly=new QHBoxLayout(this);
    ly->setContentsMargins(0,0,0,0);
    ly->setSpacing(0);
    ly->addWidget(m_Mx);
    for (int i=Effects.size();i<3;i++)
    {
        Effects.append(dynamic_cast<CDeviceContainer*>(deviceList.addDevice(new CDeviceContainer("Effect"),i+1,parent)));
    }
    auto m = DEVICEFUNC(CStereoMixer);
    for (int i=0;i<3;i++)
    {
        deviceList.addJack(m->SendJacks[i]);
        deviceList.connect(m->SendJacks[i]->jackID(),"Effect "+ QString::number(i+1) +" In");
        deviceList.addJack(m->ReturnJacks[i]);
        deviceList.connect(m->ReturnJacks[i]->jackID(),"Effect "+ QString::number(i+1) +" Out");
    }
    for (uint i=0;i<m->channelCount();i++)
    {
        CSF2ChannelWidget* ch=m_Mx->appendChannel();
        ch->init(m->channels[i],QString::number(i+1));
        ch->setVisible(true);
    }
    m_Mx->showMaster(m,&Effects);
    //setFixedSize(m_Mx->sizeHint());
    m_Mx->adjustSize();
    m_Mx->start();
    m_Mx->show();
}

CStereoMixerForm::~CStereoMixerForm()
{
    delete ui;
}

void CStereoMixerForm::setSender(const QString& s, const int index)
{
    m_Mx->channels[index]->setSender(s);
}

void CStereoMixerForm::unserializeCustom(const QDomLiteElement* xml)
{
    if (!xml) return;
    //qDebug() << "CStereoMixerForm unserializeCustom" << xml->toString();
    for (CDeviceContainer* d : std::as_const(Effects)) d->ClearDevice();
    m_Mx->unserialize(xml);
    for (int i=0;i<3;i++)
    {
        if (const QDomLiteElement* e=xml->elementByTag("Effect"+QString::number(i)))
        {
            Effects[i]->unserializeCustom(e->elementByTag("Custom"));
        }
    }
}

void CStereoMixerForm::serializeCustom(QDomLiteElement* xml) const
{
    m_Mx->serialize(xml);
    for (int i=0;i<Effects.size();i++)
    {
        Effects[i]->serializeCustom(xml->appendChild("Effect"+QString::number(i))->appendChild("Custom"));
    }
}
