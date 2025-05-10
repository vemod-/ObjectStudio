#ifndef CUNIFILTER_H
#define CUNIFILTER_H

#include "idevice.h"
#include "cfxrbjfilter.h"

class CUnifilter : public IDevice
{
private:
    enum JackNames
    {jnOut,jnIn,jnModulation};
    enum ParameterNames
    {pnFilterType,pnInVolume,pnCutOffModulation,pnCutOffFrequency,pnResonance,pnOutVolume};
    CFxRbjFilter RBJFilter;
    float InVolFactor;
    float OutVolFactor;
    void inline updateDeviceParameter(const CParameter* p = nullptr);
public:
    CUnifilter();
    ~CUnifilter();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
};

#endif // CUNIFILTER_H
