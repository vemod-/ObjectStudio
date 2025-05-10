#include "csignalsplitter.h"


CSignalSplitter::CSignalSplitter()
{
}

void CSignalSplitter::init(const int Index, QWidget *MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackWaveOut(jnOut1,"Out 1");
    addJackWaveOut(jnOut2,"Out 2");
    addJackWaveIn();
    addJackModulationIn();
    addParameterSelect("Type","Frequency§Volume");
    addParameter(CParameter::Numeric,"Split Frequency","Hz",20,presets.MaxCutoff/2,0,"",440);
    addParameterPercent("Split Volume",50);
    addParameterPercent();
    addParameterPercent("X-fade",10);
    addParameterPercent("Response Time",10);
    modulator.init(m_Jacks[jnModulation],m_Parameters[pnModulation]);
    updateDeviceParameter();
}

void CSignalSplitter::updateDeviceParameter(const CParameter* /*p*/)
{
    modulator.setGlide(m_Parameters[pnResponse]->Value);
    modulator.setDefaultFreq(m_Parameters[pnSplitFreq]->Value);
    hipass.hpSetParams(m_freq, m_Parameters[pnSlope]->PercentValue+1, presets.SampleRate);
    lopass.lpSetParams(m_freq, m_Parameters[pnSlope]->PercentValue+1, presets.SampleRate);

}

void CSignalSplitter::play(const bool FromStart)
{
    modulator.setDefaultFreq(m_Parameters[pnSplitFreq]->Value);
    IDevice::play(FromStart);
}

void CSignalSplitter::process()
{
    const CMonoBuffer* InBuffer = FetchAMono(jnIn);
    CMonoBuffer* OutBuffer1=MonoBuffer(jnOut1);
    CMonoBuffer* OutBuffer2=MonoBuffer(jnOut2);
    if (!InBuffer->isValid()) return;
    if (m_Parameters[pnType]->Value == 0)
    {
        m_freq = modulator.execFreq();
        if (modulator.changed())
        {
            hipass.hpSetParams(m_freq, m_Parameters[pnSlope]->PercentValue+1, presets.SampleRate);
            lopass.lpSetParams(m_freq, m_Parameters[pnSlope]->PercentValue+1, presets.SampleRate);
        }
        for (uint i = 0; i < presets.ModulationRate; i++)
        {
            const float v = InBuffer->at(i);
            OutBuffer1->setAt(i,hipass.run(v));
            OutBuffer2->setAt(i,lopass.run(v));
        }
    }
    if (m_Parameters[pnType]->Value == 1)
    {
        InBuffer->peakBuffer(&m_level);
        if (++m_peakCount > 10)
        {
            m_peakCount=0;
            m_splitLevel=m_level;
            m_level=0;
        }
        const float level=modulator.exec(m_splitLevel);

        const float lowerzero = qBound<float>(0,m_Parameters[pnSplitVolume]->PercentValue-m_Parameters[pnSlope]->PercentValue*0.5f,1);
        const float lowertop = qBound<float>(0,m_Parameters[pnSplitVolume]->PercentValue+m_Parameters[pnSlope]->PercentValue*0.5f,1);

        float vol = 1;
        if (level<lowerzero) vol = 0;
        else if (level<lowertop)
        {
            float diff=lowertop-lowerzero;
            float Val=level-(lowerzero);
            vol=(vol*Val)/diff;
        }
        vol = lin2expf(vol);

        OutBuffer1->writeBuffer(InBuffer,vol);
        OutBuffer2->writeBuffer(InBuffer,1-vol);
    }
}
