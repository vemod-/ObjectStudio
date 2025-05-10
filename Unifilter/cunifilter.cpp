#include "cunifilter.h"

CUnifilter::CUnifilter()
{
}

void CUnifilter::init(const int Index, QWidget* MainWindow) {
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackWaveOut(jnOut);
    addJackWaveIn();
    addJackModulationIn();
    addParameterSelect("Filter Type","Low pass§Hi pass§Band pass 1§Band pass 2§Notch§All pass§Peaking§Low shelf§Hi shelf");
    addParameterVolume("Gain");
    makeParameterGroup(2,"Cutoff",Qt::green);
    addParameter(CParameter::Percent,"Cutoff Modulation","%",0,200,0,"",0);
    addParameterCutOff();
    //AddParameter(Numeric,"Response Time","%",0,100,0,"",50);
    addParameterPercent("Resonance");
    addParameterVolume();
    updateDeviceParameter();
}

CAudioBuffer *CUnifilter::getNextA(const int ProcIndex) {
    const CMonoBuffer* InBuffer = FetchAMono(jnIn);
    if (!InBuffer->isValid()) return nullptr;
    CAudioBuffer* Buffer=m_AudioBuffers[ProcIndex];
    for (uint i=0;i<m_BufferSize;i++)
    {
        Buffer->setAt(i,RBJFilter.filter(InBuffer->at(i))*OutVolFactor);
    }
    return Buffer;
}

void CUnifilter::updateDeviceParameter(const CParameter* /*p*/) {
    InVolFactor=m_Parameters[pnInVolume]->PercentValue;
    OutVolFactor=m_Parameters[pnOutVolume]->PercentValue;
    RBJFilter.calc_filter_coeffs(m_Parameters[pnFilterType]->Value,m_Parameters[pnCutOffFrequency]->Value,presets.SampleRate,(m_Parameters[pnResonance]->scaleValue(0.2f))+1,lin2dBf(InVolFactor),false);
}

CUnifilter::~CUnifilter() {
}
