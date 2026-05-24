#ifndef CUNIFILTER_H
#define CUNIFILTER_H

#include "idevice.h"
#include "cfxrbjfilter.h"

#define devicejacks monoout,monoin,modulationin
#define devicecategory Effect | SynthModule

class CUnifilter : public IDevice
{
private:
    enum JackNames
    {devicejacks};
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
