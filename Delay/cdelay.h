#ifndef CDELAY_H
#define CDELAY_H

#define devicejacks monoout,effectmonoout,monoin
#define devicecategory Effect

#include "idevice.h"
#include "cwavebank.h"
#include "cringbuffer.h"
#include "biquad.h"

class CDelay : public IDevice
{
private:
    enum JackNames
    {devicejacks};
    enum ParameterNames
    {pnFrequency,pnAmplitude,pnDelay,pnRegen,pnRegenEQ,pnMix};
    CRingBuffer ring;
    int ReadPosition;
    float CleanMix;
    float EffectMix;
    float RegenCleanMix;
    float RegenEffectMix;
    int DelayRate;
    float CurrentMod;
    CWaveBank WaveBank;
    CBiquad hs;
    void process();
    void inline updateDeviceParameter(const CParameter* p = nullptr);
public:
    CDelay();
    ~CDelay() {}
    void init(const int Index, QWidget* MainWindow);
    void tick();
};

#endif // CDELAY_H
