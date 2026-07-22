#include "cpreamp.h"
//#include <math.h>



CPreamp::CPreamp()
{
}

void CPreamp::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackMonoIn();
    addJackMonoOut(monoout);
    addParameter(CParameterVars::dB, "Gain", "dB", 100, 1000, 1, "", 100);
    startParameterGroup("EQ");
    addParameterVolume("Bass");
    addParameterVolume("Mid");
    addParameterVolume("Treble");
    addParameter(CParameterVars::dB, "Presence", "dB", 100, 200, 1, "", 100);
    endParameterGroup();
    addParameterPercent("Tube");
    addParameterVolume("Output");
    f3.init(880,5000,presets.SampleRate);
    updateDeviceParameter();
}


CAudioBuffer* CPreamp::getNextA(const int ProcIndex)
{

    const CMonoBuffer* InBuffer = FetchAMono(monoin);
    CMonoBuffer* OutBuffer=MonoBuffer(monoout);
    if (!InBuffer->isValid()) return nullptr;
    for (uint i=0;i<m_BufferSize;i++)
    {
        float r=f3.apply(InBuffer->at(i)*gain);
        r=f.applyfilter(r);
        r=amp.process(r);
        OutBuffer->setAt(i,r*volume);
    }
    return m_AudioBuffers[ProcIndex];
}

void inline CPreamp::updateDeviceParameter(const CParameter* /*p*/)
{
    gain = m_Parameters[pnGain]->PercentValue;
    volume = m_Parameters[pnVolume]->PercentValue;
    f3.lg = m_Parameters[pnBass]->PercentValue;
    f3.mg = m_Parameters[pnMid]->PercentValue;
    f3.hg = m_Parameters[pnTreble]->PercentValue;
    amp.setAmount(m_Parameters[pnSimulation]->PercentValue);
    f.setfilter_presence(5000,(m_Parameters[pnPresence]->Value-100)*0.1f,4000,presets.SampleRate);
}
