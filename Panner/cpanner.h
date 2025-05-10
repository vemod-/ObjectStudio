#ifndef CPANNER_H
#define CPANNER_H

#include "idevice.h"
#include "cvoltagemodulator.h"

class CPanner : public IDevice
{
private:
    enum JackNames
    {jnIn,jnOut,jnOutLeft,jnOutRight,jnModulation};
    enum ParameterNames
    {pnPan,pnModulation};
    float LeftModFactor;
    float RightModFactor;
    float LeftFactor;
    float RightFactor;
    CMonoBuffer* InSignal;
    CVoltageModulator Modulator;
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    void process();
public:
    CPanner();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
};

#endif // CPANNER_H
