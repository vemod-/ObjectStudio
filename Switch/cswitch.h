#ifndef CSWITCH_H
#define CSWITCH_H

#include "idevice.h"

class CSwitch : public IDevice
{
public:
    CSwitch();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
    float getNext(const int ProcIndex);
private:
    enum JackNames
    {jnIn1,jnIn2,jnVoltageIn1,jnVoltageIn2,jnOut1,jnOut2,jnVoltageOut1,jnVoltageOut2};
    enum ParameterNames
    {pnSwitch};
    void inline updateDeviceParameter(const CParameter* p = nullptr);
};

#endif // CSWITCH_H
