#include "cmixerwidget.h"
#include "ui_cmixerwidget.h"
#include "cchanneleffects.h"

CMixerWidget::CMixerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CMixerWidget)
{
    ui->setupUi(this);
    timercounter=0;
    connect(&peakTimer,&QTimer::timeout,this,&CMixerWidget::peak);
    master=new CMasterWidget(this);
    master->hide();
    lo=new QGridLayout(this);
    lo->setSpacing(1);
    lo->setContentsMargins(1,1,1,1);
    setLayout(lo);
}

CMixerWidget::~CMixerWidget()
{
    peakTimer.stop();
    peakTimer.disconnect();
    delete ui;
}

void CMixerWidget::peak()
{
    if (!peakTimer.isActive()) return;
    timercounter++;
    master->checkPeak();
//#if defined(MIDIFILE2WAVE) || defined(MIDIFILE2WAVE_LIBRARY)
    if (timercounter>=channels.size())
    {
        master->checkEffects();
        for (CHANNELWIDGET* c : std::as_const(channels)) c->checkAll();//for (int i=0;i<channels.size();i++) channels[i]->checkAll();
        timercounter=0;
        return;
    }
//#endif
    /*
#ifdef WAVERECORDER_LIBRARY
    if (timercounter>=channels.size())
    {
        master->checkEffects();
        timercounter=0;
    }
#endif
*/
    for (CHANNELWIDGET* c : std::as_const(channels)) c->checkPeak();
}

void CMixerWidget::setSoloChannel(int channel)
{
    auto c = channels[channel]->findChild<CChannelEffects*>("ChannelEffects");
    if (c->isSolo())
    {
        for (int i=0;i<channels.size();i++)
        {
            if (i != channel) channels[i]->soloButton(false);
        }
        master->setSoloChannel(channel);
    }
    else
    {
        master->setSoloChannel(-1);
    }
}

CHANNELWIDGET* CMixerWidget::appendChannel(int index)
{
    if (index==-1) index=channels.size();
    auto ch=new CHANNELWIDGET(this);
    connect(ch, &CHANNELWIDGET::soloTriggered, [=]() {setSoloChannel(index);});
    lo->addWidget(ch,0,index);
    channels.append(ch);
    return ch;
}

void CMixerWidget::removeChannel(int index)
{
    if (index==-1) index=channels.size()-1;
    lo->removeWidget(channels[index]);
    delete channels.takeAt(index);
}

void CMixerWidget::hideMaster()
{
    master->hide();
    lo->removeWidget(master);
}

void CMixerWidget::showMaster(CStereoMixer *mx, QList<CDeviceContainer*>* effects)
{
    master->init(mx,effects);
    master->show();
    lo->addWidget(master,0,channels.size());
}

void CMixerWidget::stop()
{
    peakTimer.stop();
}

void CMixerWidget::start()
{
    peakTimer.start(40+channels.size());
}

void CMixerWidget::clear()
{
    for (CHANNELWIDGET* c : std::as_const(channels)) lo->removeWidget(c);//for (int i=0;i<channels.size();i++) lo->removeWidget(channels[i]);
    qDeleteAll(channels);
    channels.clear();
    master->clear();
    lo->removeWidget(master);
}

void CMixerWidget::resetPeak()
{
    master->resetPeak();
    for (CHANNELWIDGET* c : std::as_const(channels)) c->resetPeak();//for (int i=0;i<channels.size();i++) channels[i]->resetPeak();
}

void CMixerWidget::unserialize(const QDomLiteElement* xml)
{
    if (!xml) return;
    for (int i=0;i<channels.size();i++)
    {
        if (const QDomLiteElement* ch=xml->elementByTag("Channel"+QString::number(i))) channels[i]->unserialize(ch->elementByTag("Channel"));
    }
    master->unserialize(xml->elementByTag("Master"));
}

void CMixerWidget::serialize(QDomLiteElement* xml) const
{
    for (int i=0;i<channels.size();i++)
    {
        channels[i]->serialize(xml->appendChild("Channel"+QString::number(i))->appendChild("Channel"));
    }
    master->serialize(xml->appendChild("Master"));
}
