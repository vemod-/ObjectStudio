#include "ciirfilter.h"

CIIRFilter::CIIRFilter() : iirFilter(presets.SampleRate)
{
    LastMod=0;
    CurrentMod=0;
    ModFactor=0;
}

void CIIRFilter::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackWaveOut(jnOut);
    addJackWaveIn();
    addJackModulationIn("Modulation In");
    startParameterGroup("IIR",Qt::blue);
    addParameterSelect("Type","LP§HP§BP",0);
    addParameterSelect("Prototype","Butterworth§Chebyshev",0);
    endParameterGroup();
    startParameterGroup();
    addParameterCutOff("Low Freq",20);
    addParameterCutOff("High Freq",presets.MaxCutoff);
    endParameterGroup();
    addParameter(CParameter::Numeric,"Order","",1,16,0,"",5);
    addParameter(CParameter::Percent,"Modulation","%",0,200,0,"",0);
    updateDeviceParameter();
}

CAudioBuffer* CIIRFilter::getNextA(const int ProcIndex)
{
    const float Mod=Fetch(jnModulation);
    const CMonoBuffer* InBuffer = FetchAMono(jnIn);
    if (!InBuffer->isValid()) return nullptr;
    //if (isZero(Mod)) return nullptr;//&m_NullBufferMono;
    if (!closeEnough(Mod,LastMod))
    {
        LastMod = Mod;
        CurrentMod = Mod * ModFactor;
    }
    iirFilter.FilterBuffer(InBuffer->data(),0,m_AudioBuffers[ProcIndex]->data(),0,presets.ModulationRate);
    return m_AudioBuffers[ProcIndex];
}

void inline CIIRFilter::updateDeviceParameter(const CParameter* /*p*/)
{
    ModFactor = m_Parameters[pnModulation]->PercentValue;
    iirFilter.setFreqHigh(m_Parameters[pnHiFreq]->Value);
    iirFilter.setFreqLow(m_Parameters[pnLoFreq]->Value);
    iirFilter.setType(IIRFilterType(m_Parameters[pnType]->Value + 1));
    iirFilter.setProto(IIRProtoType(m_Parameters[pnProtoType]->Value + 1));
    iirFilter.setOrder(m_Parameters[pnOrder]->Value);
}
