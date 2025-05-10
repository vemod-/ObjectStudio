#include "cpolybox.h"
#include "cmacroboxform.h"

CPolyBox::CPolyBox() {}

CPolyBox::~CPolyBox()
{
    qDebug() << "~CPolyBox";
    if (m_Initialized)
    {
        FORMFUNC(CMacroBoxForm)->DesktopComponent->clear();
        qDeleteAll(JacksCreated);
    }
}

void CPolyBox::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackStereoOut(jnOut);
    addJackDualMonoOut(jnOutLeft);
    addJackMIDIIn();
    m_Form=new CMacroBoxForm(this,MainWindow);
    CDesktopComponent* d=FORMFUNC(CMacroBoxForm)->DesktopComponent;
    addTickerDevice(d->deviceList());
    setDeviceParent(d->deviceList());
    d->deviceList()->setPolyphony(CVDevice::CVVoices);

    for (int i = 0; i < 3; i++)
    {
        WaveOut.append(dynamic_cast<CInJack*>(m_Jacks[i]->createInsideJack(i,this)));
        JacksCreated.append(WaveOut[i]);
    }



    for (int i=0;i<CVDevice::CVVoices;i++)
    {
        for (int j = 0; j < 3; j++) d->addJack(WaveOut[j],i);
        JacksCreated.append(d->addJack(new COutJack("Frequency In","This",IJack::Voltage,IJack::Out,this,2+i),i));
        JacksCreated.append(d->addJack(new COutJack("Trigger In","This",IJack::Voltage,IJack::Out,this,2+i+CVDevice::CVVoices),i));
        JacksCreated.append(d->addJack(new COutJack("MIDI In","This",IJack::MIDI,IJack::Out,this,2+i+CVDevice::CVVoices+CVDevice::CVVoices),i));
    }
    addParameterMIDIChannel();
    addParameterTranspose();
    addParameterTune();
    Reset();
}
/*
void CPolyBox::hideForm()
{
    FORMFUNC(CMacroBoxForm)->DesktopComponent->hideForms();
    m_Form->setVisible(false);
}
*/
void CPolyBox::process()
{
    CVDevice.parseMIDI(FetchP(jnMIDIIn));
}

float CPolyBox::getNext(const int ProcIndex)
{
    if (m_Process)
    {
        m_Process=false;
        process();
    }
    if ((ProcIndex>=4) && (ProcIndex<(4+CVDevice::CVVoices)))
    {
        return CVDevice.note(ProcIndex-4).Voltage + (CVDevice.getPitchbend(ProcIndex-4)/1200.0);
    }
    if ((ProcIndex>=4+CVDevice::CVVoices) && (ProcIndex<(4+CVDevice::CVVoices+CVDevice::CVVoices)))
    {
        return CVDevice.note(ProcIndex-(4+CVDevice::CVVoices)).Velocity*CVDevice.Vol(ProcIndex-(4+CVDevice::CVVoices));
    }
    return 0;
}

CMIDIBuffer* CPolyBox::getNextP(int ProcIndex)
{
    if (m_Process)
    {
        m_Process=false;
        process();
    }
    if (ProcIndex>=4+CVDevice::CVVoices+CVDevice::CVVoices)
    {
        MIDIBuffer.clear();
        const int v = ProcIndex-(4+CVDevice::CVVoices+CVDevice::CVVoices);
        const CCVDevice::CVNote& n=CVDevice.note(v);
        if ((prevMIDIkey[v] != n.MIDIKey) || (prevVel[v] != n.MIDIVelocity))
        {
            if (prevMIDIkey[v]) MIDIBuffer.append(0x80,prevMIDIkey[v]);
            if (n.MIDIKey) MIDIBuffer.append(0x90,n.MIDIKey,n.MIDIVelocity);
            prevMIDIkey[v] = n.MIDIKey;
            prevVel[v] = n.MIDIVelocity;
        }
        return &MIDIBuffer;
    }
    return nullptr;
}

CAudioBuffer* CPolyBox::getNextA(const int ProcIndex)
{
    if (ProcIndex < 3) return WaveOut[ProcIndex]->getNextA();
    return nullptr;
}

void inline CPolyBox::updateDeviceParameter(const CParameter* /*p*/)
{
    CVDevice.Tune=m_Parameters[pnTune]->PercentValue;
    CVDevice.setTranspose(m_Parameters[pnTranspose]->Value);
    CVDevice.setChannelMode(m_Parameters[pnMIDIChannel]->Value);
}

void CPolyBox::Reset()
{
    for (int i = 0; i < CVDevice::CVVoices; i++)
    {
        prevMIDIkey[i]=0;
        prevVel[i]=0;
    }
    updateDeviceParameter();
}

void CPolyBox::play(const bool FromStart)
{
    if (FromStart)
    {
        CVDevice.reset();
        updateDeviceParameter();
    }
    IDevice::play(FromStart);
}

void CPolyBox::pause()
{
    CVDevice.allNotesOff();
    updateDeviceParameter();
    IDevice::pause();
}

void CPolyBox::skip(const ulong64 samples)
{
    CVDevice.allNotesOff();
    updateDeviceParameter();
    IDevice::skip(samples);
}
