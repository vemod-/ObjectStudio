#include "cringmodulator.h"

CRingModulator::CRingModulator():ModulationFactor(0)
{

}

void CRingModulator::init(const int Index, QWidget* MainWindow) {
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackWaveOut(jnOut);
    addJackWaveIn();
    addJackWaveIn("Modulation");
    addParameterPercent();
    updateDeviceParameter();
}

CAudioBuffer *CRingModulator::getNextA(const int ProcIndex) {
    const CMonoBuffer* InBuffer = FetchAMono(jnIn);
    const CMonoBuffer* InModulationBuffer = FetchAMono(jnModulation);
    CMonoBuffer* OutBuffer=MonoBuffer(jnOut);
    if (!InBuffer->isValid()) return nullptr;
    if (!InModulationBuffer->isValid()) return nullptr;
    for (uint i=0;i<m_BufferSize;i++)
    {
        const float Modulation=(InModulationBuffer->at(i)*ModulationFactor)*InBuffer->at(i);
        OutBuffer->setAt(i,InBuffer->at(i)*CleanFactor + Modulation);//Modulation;
    }
    return m_AudioBuffers[ProcIndex];
}

void CRingModulator::updateDeviceParameter(const CParameter* /*p*/) {
    ModulationFactor=m_Parameters[pnModulation]->PercentValue;
    CleanFactor=1-ModulationFactor;
}
