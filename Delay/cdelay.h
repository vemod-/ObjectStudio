#ifndef CDELAY_H
#define CDELAY_H

#include "idevice.h"
#include "cwavebank.h"
#include "cringbuffer.h"

class CDelay : public IDevice
{
private:
    enum JackNames
    {jnOut,jnEffectOut,jnIn};
    enum ParameterNames
    {pnFrequency,pnAmplitude,pnDelay,pnRegen,pnMix};
    CRingBuffer ring;
    int ReadPosition;
    float CleanMix;
    float EffectMix;
    float RegenCleanMix;
    float RegenEffectMix;
    int DelayRate;
    float CurrentMod;
    CWaveBank WaveBank;
    void process();
    void inline updateDeviceParameter(const CParameter* p = nullptr);
public:
    CDelay();
    ~CDelay() {}
    void init(const int Index, QWidget* MainWindow);
    void tick();
};

#endif // CDELAY_H
