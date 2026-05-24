#ifndef CSCOPE_H
#define CSCOPE_H

#include "idevice.h"

#define devicejacks monoin,modulationin
#define devicecategory Monitor

class CScope : public IDevice
{
public:
    CScope();
    ~CScope();
    void init(const int Index, QWidget* MainWindow);
    void tick();
private:
    enum JackNames
    {devicejacks};
    enum ParameterNames
    {pnVolume,pnScopeRate,pnFrequency,pnDetectPitch,pnScopeMode};
    void Reset();
    void inline updateDeviceParameter(const CParameter* p = nullptr);
};

#endif // CSCOPE_H
