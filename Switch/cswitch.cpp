#include "cswitch.h"

CSwitch::CSwitch() {}

void CSwitch::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackWaveIn("In 1");
    addJackWaveIn("In 2");
    addJackModulationIn("Voltage In 1");
    addJackModulationIn("Voltage in 2");
    addJackWaveOut(jnOut1,"Out 1");
    addJackWaveOut(jnOut2,"Out 2");
    addJackModulationOut(jnVoltageOut1,"Voltage Out 1");
    addJackModulationOut(jnVoltageOut2,"Voltage Out 2");
    addParameterOffOn("Switch");
    updateDeviceParameter();
}

CAudioBuffer* CSwitch::getNextA(const int ProcIndex)
{
    if (m_Parameters[pnSwitch]->Value == 0) return FetchA((ProcIndex - jnOut1) + jnIn1);
    return FetchA((1 - (ProcIndex - jnOut1)) + jnIn1);
}

float CSwitch::getNext(const int ProcIndex)
{
    if (m_Parameters[pnSwitch]->Value == 0) return Fetch((ProcIndex - jnVoltageOut1) + jnVoltageIn1);
    return Fetch((1 - (ProcIndex - jnVoltageOut1)) + jnVoltageIn1);
}

void inline CSwitch::updateDeviceParameter(const CParameter* /*p*/)
{
}
