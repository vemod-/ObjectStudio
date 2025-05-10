#include "ckarlsenfilter.h"

CKarlsenFilter::CKarlsenFilter()
{
    pole1=0;
    pole2=0;
    pole3=0;
    pole4=0;
    InVolumeFactor=0;
    OutVolumeFactor=0;
    LastResonance=0;
    rezamount=0;
    cutoffreq=0;
    MixFactor=0;
}

void CKarlsenFilter::init(const int Index, QWidget* MainWindow) {
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
    CalcExpResonance(0);
    updateDeviceParameter();
}

CAudioBuffer *CKarlsenFilter::getNextA(const int ProcIndex) {
    const CMonoBuffer* InBuffer = FetchAMono(jnIn);
    if (!InBuffer->isValid()) return nullptr;
    const float CurrentFreq = qBound<float>(20,Modulator.execFreq(m_Parameters[pnCutOffFrequency]->Value),presets.MaxCutoff);
    bool Recalc=Modulator.changed();
    if (m_Parameters[pnResonance]->Value != LastResonance)
    {
        LastResonance=m_Parameters[pnResonance]->Value;
        Recalc=true;
    }
    if (Recalc)
    {
        CalcExpResonance(CurrentFreq);
        MixFactor=1.f-(m_Parameters[pnResonance]->PercentValue*0.6f);
    }
    const float volIn=InVolumeFactor*MixFactor;
    const float volOut=OutVolumeFactor*MixFactor;
    CAudioBuffer* Buffer=m_AudioBuffers[ProcIndex];
    for (uint i=0;i<m_BufferSize;i++)
    {
        const float input = (InBuffer->at(i) * volIn) - qMin<float>(pole4 * rezamount,1);
        pole1 = pole1 + ((-pole1 + input) * cutoffreq);
        pole2 = pole2 + ((-pole2 + pole1) * cutoffreq);
        pole3 = pole3 + ((-pole3 + pole2) * cutoffreq);
        pole4 = pole4 + ((-pole4 + pole3) * cutoffreq);
        Buffer->setAt(i,pole4 * volOut);
    }
    return Buffer;
}

void CKarlsenFilter::updateDeviceParameter(const CParameter* /*p*/) {
    InVolumeFactor=m_Parameters[pnInVolume]->PercentValue;
    OutVolumeFactor=m_Parameters[pnOutVolume]->PercentValue;
    Modulator.setGlide(m_Parameters[pnResponse]->Value);
    Modulator.setDefaultFreq(m_Parameters[pnCutOffFrequency]->Value);
}

void CKarlsenFilter::CalcExpResonance(float CutOff) {
    rezamount=m_Parameters[pnResonance]->scaleValue(0.2f);
    cutoffreq=CutOff/presets.MaxCutoff;
}

void CKarlsenFilter::play(const bool FromStart)
{
    Modulator.setDefaultFreq(m_Parameters[pnCutOffFrequency]->Value);
    IDevice::play(FromStart);
}
