#ifndef CSWITCH_H
#define CSWITCH_H

#include "idevice.h"

#define devicejacks monoin1,monoin2,voltagein1,voltagein2,monoout1,monoout2,voltageout1,voltageout2
#define devicecategory SynthModule

class CSwitch : public IDevice
{
public:
    CSwitch();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
    float getNext(const int ProcIndex);
private:
    enum JackNames
    {devicejacks};
    enum ParameterNames
    {pnSwitch};
    void inline updateDeviceParameter(const CParameter* p = nullptr);
};

#endif // CSWITCH_H
