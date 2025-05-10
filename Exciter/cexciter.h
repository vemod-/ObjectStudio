#ifndef CEXCITER_H
#define CEXCITER_H

#include "idevice.h"
#include "cfxrbjfilter.h"

class CExciter : public IDevice
{
private:
    enum JackNames
    {jnOut,jnEffOut,jnIn};
    enum ParameterNames
    {pnType,pnInVolume,pnAmount,pnCutOffFrequency,pnOutVolume};
    CFxRbjFilter RBJFilter;
    float InVolFactor;
    float OutVolFactor;
    float EffFactor;
    float k2;
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    float inline clip(float x, float a, float b);
    float inline soft(float x, int amount);
    void process();
public:
    CExciter();
    void init(const int Index, QWidget* MainWindow);
};


#endif // CEXCITER_H
