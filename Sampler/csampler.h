#ifndef CSAMPLER_H
#define CSAMPLER_H

#include "idevice.h"
#include "csamplerdevice.h"
#include "cvoltagemodulator.h"

class CSampler : public IDevice
{
private:
    enum JackNames
    {jnOut,jnMIDIIn,jnModulation};
    enum ParameterNames
    {pnMIDIChannel,pnTranspose,pnTune,pnModulation};
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    CSamplerDevice SamplerDevice;
    //float LastMod;
    //float CurrentMod;
    float VolumeFactor;
    void process();
    CVoltageModulator Modulator;
public:
    CSampler();
    void play(const bool FromStart);
    void pause();
    void init(const int Index, QWidget* MainWindow);
};

#endif // CSAMPLER_H
