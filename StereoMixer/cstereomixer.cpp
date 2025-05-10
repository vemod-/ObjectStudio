#include "cstereomixer.h"
#include "cstereomixerform.h"

#undef devicename
#define devicename "StereoMixer"

CStereoMixerChannel::CStereoMixerChannel(const int sends)
{
    sendCount=uint(sends);
    Effect=new float[sendCount];
    for (uint i=0;i<sendCount;i++) Effect[i]=1;
    Level=1;
    PanL=1;
    PanR=1;
    Mute=false;
    EffectMute=false;
    PeakL=0;
    PeakR=0;
    f3L.init(880,500,presets.SampleRate);
    f3R.init(880,500,presets.SampleRate);
    hpL.hpSetParams(80,1,presets.SampleRate);
    hpR.hpSetParams(80,1,presets.SampleRate);
    f3L.hg=1;
    f3L.lg=1;
    f3L.mg=1;
    f3R.hg=1;
    f3R.lg=1;
    f3R.mg=1;
    EQ=false;
    Gain=1;
    LoCut=false;
}

CStereoMixerChannel::~CStereoMixerChannel()
{
    delete [] Effect;
}

void CStereoMixerChannel::mixChannel(CStereoBuffer& Signal, CStereoBuffer *Out, std::vector<CStereoBuffer*>& Send, const bool First, CStereoBuffer& WorkBuffer)
{
    if (First)
    {
        if (!isZero(Level))
        {
            Out->writeStereoBuffer(Signal.data(),Level*PanL*Gain,Level*PanR*Gain);
            if (LoCut)
            {
                for (uint i = 0; i < Signal.size(); i++)
                {
                    Out->setAtL(i,hpL.run(Out->at(i)));
                    Out->setAtR(i,hpR.run(Out->atR(i)));
                }
            }
            if (EQ)
            {
                for (uint i = 0; i < Signal.size(); i++)
                {
                    Out->setAtL(i,f3L.apply(Out->at(i)));
                    Out->setAtR(i,f3R.apply(Out->atR(i)));
                }
            }
            EffectRack.mixerChannelProc(Out);
            if (!EffectMute)
            {
                for (uint j = 0; j < sendCount; j++) Send[j]->writeBuffer(Out,Effect[j]);
            }
            else
            {
                for (uint j = 0; j < sendCount; j++) Send[j]->zeroBuffer();
            }
            Out->peakStereoBuffer(&PeakL,&PeakR);
        }
        else
        {
            Out->zeroBuffer();
            for (uint j = 0; j < sendCount; j++) Send[j]->zeroBuffer();
            PeakL=0;
            PeakR=0;
        }
    }
    else
    {
        if (!isZero(Level))
        {
            WorkBuffer.writeStereoBuffer(Signal.data(),Level*PanL*Gain,Level*PanR*Gain);
            if (LoCut)
            {
                for (uint i = 0; i < Signal.size(); i++)
                {
                    WorkBuffer.setAtL(i,hpL.run(WorkBuffer.at(i)));
                    WorkBuffer.setAtR(i,hpR.run(WorkBuffer.atR(i)));
                }
            }
            if (EQ)
            {
                for (uint i = 0; i < Signal.size(); i++)
                {
                    WorkBuffer.setAtL(i,f3L.apply(WorkBuffer.at(i)));
                    WorkBuffer.setAtR(i,f3R.apply(WorkBuffer.atR(i)));
                }
            }
            EffectRack.mixerChannelProc(&WorkBuffer);
            *Out += &WorkBuffer;
            if (!EffectMute) for (uint j = 0; j < sendCount; j++) Send[j]->addBuffer(&WorkBuffer,Effect[j]);
            WorkBuffer.peakStereoBuffer(&PeakL,&PeakR);
        }
        else
        {
            PeakL=0;
            PeakR=0;
        }
    }
}

CStereoMixer::CStereoMixer(const uint channelCount, const uint sendCount)
{
    m_Disabled=false;
    m_ChannelCount=channelCount;
    m_SendCount=sendCount;
    Signal=new CStereoBuffer*[m_ChannelCount];
    Sends=new float[m_SendCount];
    for (uint i = 0; i < m_SendCount; i++) Sends[i]=1;
    channels=new CStereoMixerChannel*[m_ChannelCount];
    for (uint i = 0; i < m_ChannelCount; i++) channels[i]=new CStereoMixerChannel(int(m_SendCount));
    jnIn=jnSend+int(m_SendCount);
    jnReturn=jnIn+int(m_ChannelCount);
}

CStereoMixer::~CStereoMixer()
{
    QMutexLocker locker(&mutex);
    m_Disabled=true;
    if (m_Form) {
        removerEffectRacksFromDeviceList(&FORMFUNC(CStereoMixerForm)->deviceList);
        FORMFUNC(CStereoMixerForm)->deviceList.clear();
    }
    for (uint i = 0; i < m_ChannelCount; i++) delete channels[i];
    delete [] channels;
    delete [] Sends;
    delete [] Signal;
}

void CStereoMixer::process()
{
    //CStereoBuffer* OutBuffer=StereoBuffer(jnOut);
    //CStereoBuffer** SendBuffers=reinterpret_cast<CStereoBuffer**>(&m_AudioBuffers[jnSend]);
    for (uint i = 0; i < m_ChannelCount; i++) Signal[i]=dynamic_cast<CStereoBuffer*>(m_InJacks[i]->getNextA());//FetchAStereo(jnIn+i);

    OrigChannel.clear();
    for (uint i =0; i < m_ChannelCount; i++)
    {
        if (Signal[i]->isValid())
        {
            if (SoloChannel>-1)
            {
                if (SoloChannel==int(i)) OrigChannel.push_back(i);
            }
            else if (!channels[i]->Mute)
            {
                OrigChannel.push_back(i);
            }
        }
    }
    if (OrigChannel.empty())
    {
        OutBuffer->zeroBuffer();
        for (uint i=0;i<m_SendCount;i++) SendBuffers[i]->zeroBuffer();
    }
    else
    {
        for (uint InChannel=0;InChannel<OrigChannel.size();InChannel++)
        {
            const uint ch=OrigChannel[InChannel];
            channels[ch]->mixChannel(*Signal[ch],OutBuffer,SendBuffers,(InChannel==0),WorkBuffer);
        }
    }
    for (uint j=0;j<m_SendCount;j++)
    {
        OutBuffer->addBuffer(ReturnJacks[j]->getNextA(),Sends[j]);
    }
    OutBuffer->multiplyStereoBuffer(MasterLeft*MixFactor,MasterRight*MixFactor);
    OutBuffer->peakStereoBuffer(&PeakL,&PeakR);
}

void CStereoMixer::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackStereoOut(jnOut);
    OutBuffer=StereoBuffer(jnOut);
    for (uint i=0;i<m_SendCount;i++)
    {
        COutJack* OJ = (MainWindow) ? new COutJack("Send " + QString::number(i+1),m_DeviceID,IJackBase::Stereo,IJack::Out,this,int(i)+jnSend) :
                                      addJackStereoOut(jnSend+int(i),"Send " + QString::number(i+1));
        SendBuffers.push_back(dynamic_cast<CStereoBuffer*>(OJ->audioBuffer));
        SendJacks.push_back(OJ);
    }
    for (uint i=0;i<m_ChannelCount;i++)
    {
        addJackStereoIn("In " + QString::number(i+1));
    }
    for (uint i=0;i<m_SendCount;i++)
    {
        CInJack* IJ = (MainWindow) ? new CInJack("Return " + QString::number(i+1),m_DeviceID,IJackBase::Stereo,IJack::In,this) :
                                     addJackStereoIn("Return " + QString::number(i+1));
        ReturnJacks.push_back(IJ);
    }
    PeakL=0;
    PeakR=0;
    SoloChannel=-1;
    MasterLeft=1;
    MasterRight=1;
    MixFactor=mixFactorf(int(m_ChannelCount+m_SendCount));
    if (MainWindow) {
        m_Form=new CStereoMixerForm(this,MainWindow);
        addTickerDevice(&FORMFUNC(CStereoMixerForm)->deviceList);
        addEffectRacksToDeviceList(&FORMFUNC(CStereoMixerForm)->deviceList,MainWindow);
    }
}

void CStereoMixer::addEffectRacksToDeviceList(CDeviceList* dl, QWidget* mainWindow) {
    for (uint i=0;i<m_ChannelCount;i++) {
        channels[i]->EffectRack.init(i+1,mainWindow);
        dl->addDevice(&channels[i]->EffectRack,i+1,mainWindow);
    }
}

void CStereoMixer::removerEffectRacksFromDeviceList(CDeviceList* dl) {
    for (uint i=0;i<m_ChannelCount;i++) {
        dl->removeDevice(&channels[i]->EffectRack);
    }
}

CAudioBuffer* CStereoMixer::getNextA(const int ProcIndex)
{
    if (!m_Disabled)
    {
        if (m_Process)
        {
            m_Process=false;
            process();
        }
        if (ProcIndex==jnOut) return m_AudioBuffers[jnOut];
        return SendBuffers[uint(ProcIndex-jnSend)];
        //return m_AudioBuffers[ProcIndex];
    }
    return nullptr;
}

void CStereoMixer::play(bool FromStart)
{
    PeakL=0;
    PeakR=0;
    for (uint i=0;i<m_ChannelCount;i++)
    {
        CStereoMixerChannel* ch=channels[i];
        ch->PeakL=0;
        ch->PeakR=0;
    }
    if (m_Form) FORMFUNC(CStereoMixerForm)->m_Mx->resetPeak();
    IDevice::play(FromStart);
}

void CStereoMixer::connectionChanged()
{
    if (m_Form)
    {
        auto f = FORMFUNC(CStereoMixerForm);
        for (uint i =0; i < channelCount(); i++)
        {
            CInJack* j = m_InJacks[i];
            (j->outJackCount()) ? f->setSender(j->outJack(0)->jackID(),int(i)) : f->setSender(QString(),int(i));
        }
    }
}


