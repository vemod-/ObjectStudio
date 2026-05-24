#include "cswitch.h"

CSwitch::CSwitch() {}

void CSwitch::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackMonoIn("In 1");
    addJackMonoIn("In 2");
    addJackModulationIn("Voltage In 1");
    addJackModulationIn("Voltage in 2");
    addJackMonoOut(monoout1,"Out 1");
    addJackMonoOut(monoout2,"Out 2");
    addJackModulationOut(voltageout1,"Voltage Out 1");
    addJackModulationOut(voltageout2,"Voltage Out 2");
    addParameterOffOn("Switch");
    updateDeviceParameter();
}

CAudioBuffer* CSwitch::getNextA(const int ProcIndex)
{
    if (m_Parameters[pnSwitch]->Value == 0) return FetchA((ProcIndex - monoout1) + monoin1);
    return FetchA((1 - (ProcIndex - monoout1)) + monoin1);
}

float CSwitch::getNext(const int ProcIndex)
{
    if (m_Parameters[pnSwitch]->Value == 0) return Fetch((ProcIndex - voltageout1) + voltagein1);
    return Fetch((1 - (ProcIndex - voltageout1)) + voltagein1);
}

void inline CSwitch::updateDeviceParameter(const CParameter* /*p*/)
{
}
