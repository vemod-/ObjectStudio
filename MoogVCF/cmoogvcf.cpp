#include "cmoogvcf.h"


CMoogVCF::CMoogVCF()
{
    InVolumeFactor=0;
    OutVolumeFactor=0;
    LastResonance=0;
    f=0;
    fa=0;
    fb=0;
    In1=0;
    In2=0;
    In3=0;
    In4=4;
    Out1=0;
    Out2=0;
    Out3=0;
    Out4=0;
}

void CMoogVCF::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackWaveOut(jnOut);
    addJackWaveIn();
    addJackModulationIn();
    addParameterVolume("Gain");
    makeParameterGroup(2,"Cutoff",Qt::green);
    addParameter(CParameter::Percent,"Cutoff Modulation","%",0,200,0,"",0);
    addParameterCutOff();
    addParameterPercent("Response Time",50);
    addParameterPercent("Resonance");
    addParameterVolume();
    Modulator.init(m_Jacks[jnModulation],m_Parameters[pnCutOffModulation]);
    CalcExpResonance(0,0);
    updateDeviceParameter();
}

CAudioBuffer* CMoogVCF::getNextA(const int ProcIndex)
{
    const CMonoBuffer* InBuffer = FetchAMono(jnIn);
    if (!InBuffer->isValid()) return nullptr;
    const float CurrentFreq = qBound<float>(20,Modulator.execFreq(m_Parameters[pnCutOffFrequency]->Value),presets.MaxCutoff);
    bool Recalc=Modulator.changed();
    if (m_Parameters[pnResonance]->Value != LastResonance)
    {
        LastResonance=m_Parameters[pnResonance]->Value;
        Recalc=true;
    }
    if (Recalc) CalcExpResonance(CurrentFreq,LastResonance);
    CAudioBuffer* Buffer=m_AudioBuffers[ProcIndex];
    for (uint i=0;i<m_BufferSize;i++)
    {
        const float Signal=((InBuffer->at(i) * InVolumeFactor) - (Out4 * fb)) * fa;
        Out1 = Signal + 0.3f * In1 + (1 - f) * Out1; // Pole 1
        In1 = Signal;
        Out2 = Out1 + 0.3f * In2 + (1 - f) * Out2; // Pole 2
        In2 = Out1;
        Out3 = Out2 + 0.3f * In3 + (1 - f) * Out3; // Pole 3
        In3 = Out2;
        Out4 = Out3 + 0.3f * In4 + (1 - f) * Out4; // Pole 4
        In4 = Out3;
        Buffer->setAt(i,Out4 * OutVolumeFactor);
    }
    return Buffer;
}

void inline CMoogVCF::updateDeviceParameter(const CParameter* /*p*/)
{
    InVolumeFactor=m_Parameters[pnInVolume]->PercentValue;
    OutVolumeFactor=m_Parameters[pnOutVolume]->PercentValue;
    Modulator.setGlide(m_Parameters[pnResponse]->Value);
    Modulator.setDefaultFreq(m_Parameters[pnCutOffFrequency]->Value);
}

void inline CMoogVCF::CalcExpResonance(float CutOff,float Resonance)
{
    const float fc=CutOff/presets.MaxCutoff;
    const float res=Resonance/25;
    f = fc * 1.16f;
    fb = res * (1.0f - 0.15f * f * f);
    fa=0.35013f * (f*f)*(f*f);
}

void CMoogVCF::play(const bool FromStart)
{
    Modulator.setDefaultFreq(m_Parameters[pnCutOffFrequency]->Value);
    IDevice::play(FromStart);
}
