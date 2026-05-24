#ifndef CSAMPLEANDHOLD_H
#define CSAMPLEANDHOLD_H

#include "idevice.h"

#define devicejacks monoin,modulationout
#define devicecategory SynthModule

class CSampleAndHold : public IDevice
{
public:
    CSampleAndHold();
    void init(const int Index, QWidget* MainWindow);
    float getNext(const int ProcIndex);
private:
    enum JackNames
    {devicejacks};
    enum ParameterNames
    {pnSampleRate};
    float ReturnValue;
    double m_SampleRate;
    double m_Counter;
    void inline updateDeviceParameter(const CParameter* p = nullptr);
};

#endif // CSAMPLEANDHOLD_H
