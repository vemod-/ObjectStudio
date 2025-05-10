#include "cmixer.h"
#include "cmixerform.h"

CMixer::CMixer()
{
    PeakL=0;
    PeakR=0;
    prevChannel=0;
    SoloChannel=-1;
    MasterLeft=1;
    MasterRight=1;
    MixFactor=mixFactorf(Mixer::mixerchannels);
}

void CMixer::process()
{
    CStereoBuffer* OutBuffer=StereoBuffer(jnOut);
    CStereoBuffer* SendBuffer=StereoBuffer(jnSend);
    for (int i =0; i < Mixer::mixerchannels; i++) Signal[i]=FetchAMono(i+jnIn);

    OrigChannel.clear();
    for (int i =0; i < Mixer::mixerchannels; i++)
    {
        if (Signal[i]->isValid())
        {
            if (SoloChannel>-1)
            {
                if (SoloChannel==i) OrigChannel.push_back(i);
            }
            else if (!Channel[i].Mute)
            {
                OrigChannel.push_back(i);
            }
        }
    }
    if (OrigChannel.empty())
    {
        OutBuffer->zeroBuffer();
        SendBuffer->zeroBuffer();
    }
    else
    {
        for (uint InChannel=0;InChannel<OrigChannel.size();InChannel++)
        {
            int ch=OrigChannel[InChannel];
            Channel[ch].mixChannel(*Signal[ch],OutBuffer,SendBuffer,(InChannel==0));
        }
    }
    *SendBuffer *= MixFactor;
    OutBuffer->addStereoBuffer(FetchA(jnReturn)->data());
    OutBuffer->multiplyStereoBuffer(MasterLeft*MixFactor,MasterRight*MixFactor);
    OutBuffer->peakStereoBuffer(&PeakL,&PeakR);
}

void CMixer::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);

    addJackStereoIn("Return");
    addJackStereoOut(jnOut);
    addJackStereoOut(jnSend,"Send");

    for (int i = 0; i < Mixer::mixerchannels; i++)
    {
        addJackWaveIn("In " + QString::number(i+1));
    }
    for (CMixerChannel& c : Channel) c.Peak=0;
    m_Form=new CMixerForm(this,MainWindow);
}
/*
void CMixer::GetPeak(float* P,float* L,float* R)
{
    for (int i=0;i<Mixer::mixerchannels;i++)
    {
        P[i]=Channel[i].Peak;
        Channel[i].Peak=0;
    }
    *L=PeakL;
    *R=PeakR;
    PeakL=0;
    PeakR=0;
}
*/
void CMixer::play(bool FromStart)
{
    PeakL=0;
    PeakR=0;
    for (CMixerChannel& c : Channel) c.Peak=0;
    dynamic_cast<CMixerForm*>(m_Form)->Reset();
    IDevice::play(FromStart);
}

void CMixer::connectionChanged()
{
    auto f = dynamic_cast<CMixerForm*>(m_Form);
    for (int i =0; i < Mixer::mixerchannels; i++)
    {
        auto j = dynamic_cast<CInJack*>(m_Jacks[i+jnIn]);
        (j->outJackCount()) ? f->setSender(j->outJack(0)->jackID(),i) : f->setSender(QString(),i);
    }
}
