#include "cnoisegate.h"

CNoiseGate::CNoiseGate()
{
}

void CNoiseGate::init(const int Index, QWidget* MainWindow) {
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackWaveIn();
    addJackWaveOut(jnOut);
    addJackModulationOut(jnEnvOut,"Envelope Out");
    addParameterPercent("Threshold");
    startParameterGroup();
    addParameterPercent("Response Time");
    addParameterPercent("Decay Time");
    endParameterGroup();
    CurrentVol=0;
    updateDeviceParameter();
}

CAudioBuffer *CNoiseGate::getNextA(const int ProcIndex) {
    if (m_Process)
    {
        m_Process=false;
        process();
    }
    if (isZero(CurrentVol)) return nullptr;//&m_NullBufferMono;
    return m_AudioBuffers[ProcIndex];
}

float CNoiseGate::getNext(int) {
    if (m_Process)
    {
        m_Process=false;
        process();
    }
    return CurrentVol;
}

void CNoiseGate::process() {
    const CMonoBuffer* InBuffer = FetchAMono(jnIn);
    if (!InBuffer->isValid())
    {
        CurrentVol=0;
        return;
    }
    CMonoBuffer* OutBuffer=MonoBuffer(jnOut);
    OutBuffer->writeBuffer(InBuffer);
    float Signal=0;
    OutBuffer->peakBuffer(&Signal);
    CurrentVol = glider.runVoltage(int(Signal > Threshold));
    *OutBuffer *= CurrentVol;
}

void CNoiseGate::updateDeviceParameter(const CParameter* /*p*/) {
    Threshold=m_Parameters[pnThreshold]->scaleValue(0.005f);
    glider.setGlide(m_Parameters[pnResponse]->Value,m_Parameters[pnDecay]->Value);
}
