#ifndef CNOISEGATE_H
#define CNOISEGATE_H

#include "idevice.h"
#include "cfreqglider.h"

class CNoiseGate : public IDevice
{
public:
    CNoiseGate();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
    float getNext(int /*ProcIndex*/);
private:
    enum JackNames
    {jnIn,jnOut,jnEnvOut};
    enum ParameterNames
    {pnThreshold,pnResponse,pnDecay};
    float Threshold;
    float CurrentVol;
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    void process();
    CFreqGlider glider;
};

#endif // CNOISEGATE_H
