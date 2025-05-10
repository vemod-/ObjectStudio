#include "cfilter.h"

CFilter::CFilter()
{
    FiltCoefTab0=0;
    FiltCoefTab1=0;
    FiltCoefTab2=0;
    FiltCoefTab3=0;
    FiltCoefTab4=0;
    ly1=0;
    ly2=0;
    lx1=0;
    lx2=0;
    m_ExpResonance=0;
    MixFactor=0;
    InVolumeFactor=0;
    LastResonance=0;
    //LastVoltage=0;
}

void CFilter::init(const int Index, QWidget* MainWindow) {
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackWaveOut(jnOut);
    addJackWaveIn();
    addJackModulationIn();
    addParameterVolume("Gain");
    makeParameterGroup(2,"Cutoff",Qt::green);
    addParameter(CParameter::Percent,"Cutoff Modulation","%",0,200,0,"",0);
    addParameterCutOff();
    addParameterPercent("Response Time",0);
    addParameterPercent("Resonance");
    addParameterVolume();
    Modulator.init(m_Jacks[jnModulation],m_Parameters[pnCutOffModulation]);
    CalcExpResonance();
    updateDeviceParameter();
}

CAudioBuffer *CFilter::getNextA(const int ProcIndex) {
    const CMonoBuffer* InBuffer = FetchAMono(jnIn);
    if (!InBuffer->isValid()) return nullptr;
    const float CurrentFreq = qBound<float>(20,Modulator.execFreq(m_Parameters[pnCutOffFrequency]->Value),presets.MaxCutoff);
    bool Recalc=Modulator.changed();
    if (m_Parameters[pnResonance]->Value != LastResonance)
    {
        CalcExpResonance();
        LastResonance=m_Parameters[pnResonance]->Value;
        Recalc=true;
    }
    if (Recalc)
    {
        const float Omega=(PI_F * CurrentFreq) / presets.HalfRate;
        const float sn=sinf(Omega);
        const float cs=cosf(Omega);
        const float Alpha=sn / m_ExpResonance;
        const float b1= 1-cs;
        const float b0= b1*0.5f;
        const float b2= b0;
        const float a0=1+Alpha;
        const float a1=-2*cs;
        const float a2=1-Alpha;
        FiltCoefTab0=b0/a0;
        FiltCoefTab1=b1/a0;
        FiltCoefTab2=b2/a0;
        FiltCoefTab3=-a1/a0;
        FiltCoefTab4=-a2/a0;
        MixFactor=(presets.MaxCutoff / CurrentFreq) * 0.004f;
        MixFactor=0.01f / ((m_ExpResonance*MixFactor)+(1-MixFactor));
        MixFactor*=m_Parameters[pnOutVolume]->Value;
    }
    CMonoBuffer* OutBuffer=MonoBuffer(ProcIndex);
    for (uint i=0;i<m_BufferSize;i++)
    {
        const float Signal=InBuffer->at(i) * InVolumeFactor;
        const float Temp_y=(FiltCoefTab0 * Signal) + (FiltCoefTab1 * lx1) + (FiltCoefTab2 * lx2) + (FiltCoefTab3 * ly1) + (FiltCoefTab4 * ly2);
        ly2=ly1;
        ly1=Temp_y;
        lx2=lx1;
        lx1=Signal;
        OutBuffer->setAt(i,Temp_y*MixFactor);
    }
    return OutBuffer;
}

void CFilter::CalcExpResonance()
{
    m_ExpResonance=expf(m_Parameters[pnResonance]->scaleValue(0.0625f));
}

void CFilter::updateDeviceParameter(const CParameter* /*p*/) {
    InVolumeFactor=m_Parameters[pnInVolume]->PercentValue;
    Modulator.setDefaultFreq(m_Parameters[pnCutOffFrequency]->Value);
    Modulator.setGlide(m_Parameters[pnResponse]->Value);
}

void CFilter::play(const bool FromStart)
{
    Modulator.setDefaultFreq(m_Parameters[pnCutOffFrequency]->Value);
    IDevice::play(FromStart);
}
