#ifndef CSCOPE_H
#define CSCOPE_H

#include "idevice.h"

class CScope : public IDevice
{
public:
    CScope();
    ~CScope();
    void init(const int Index, QWidget* MainWindow);
    void tick();
private:
    enum JackNames
    {jnIn,jnModulationIn};
    enum ParameterNames
    {pnVolume,pnScopeRate,pnFrequency,pnDetectPitch,pnScopeMode};
    void Reset();
    void inline updateDeviceParameter(const CParameter* p = nullptr);
};

#endif // CSCOPE_H
