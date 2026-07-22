#include "cnoisegate.h"

CNoiseGate::CNoiseGate()
{
}

void CNoiseGate::init(const int Index, QWidget* MainWindow) {
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackMonoIn();
    addJackMonoIn("Trigger Signal In");
    addJackMonoOut(monoout);
    addJackModulationOut(modulationout,"Envelope Out");
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
    CMonoBuffer* InBuffer = FetchAMono(monoin);
    CMonoBuffer* TriggerSignalBuffer = FetchAMono(extmonoin);
    if (!TriggerSignalBuffer->isValid()) TriggerSignalBuffer = InBuffer;
    if (!InBuffer->isValid())
    {
        CurrentVol=0;
        return;
    }
    CMonoBuffer* OutBuffer = MonoBuffer(monoout);
    float Signal = 0;
    TriggerSignalBuffer->peakBuffer(&Signal);
    CurrentVol = glider.runVoltage(int(Signal > Threshold));
    OutBuffer->writeBuffer(InBuffer,CurrentVol);
}

void CNoiseGate::updateDeviceParameter(const CParameter* /*p*/) {
    Threshold=m_Parameters[pnThreshold]->scaleValue(0.005f);
    glider.setGlide(m_Parameters[pnResponse]->Value,m_Parameters[pnDecay]->Value);
}
