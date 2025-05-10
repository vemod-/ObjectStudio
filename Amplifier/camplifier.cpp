#include "camplifier.h"

CAmplifier::CAmplifier() {}

void CAmplifier::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackWaveIn();
    addJackWaveOut(jnOut);
    addJackModulationIn("Modulation In");
    addParameterPercent("Modulation",100);
    Modulator.init(m_Jacks[jnModulation],m_Parameters[pnModulation]);
    updateDeviceParameter();
}

CAudioBuffer* CAmplifier::getNextA(const int ProcIndex)
{
    if (m_Parameters[pnModulation]->Value==0) return nullptr;
    const float Mod = Modulator.exec();
    if (isZero(Mod)) return nullptr;
    m_AudioBuffers[ProcIndex]->writeBuffer(FetchA(jnIn),Mod);
    return m_AudioBuffers[ProcIndex];
}

void inline CAmplifier::updateDeviceParameter(const CParameter* /*p*/)
{
}
