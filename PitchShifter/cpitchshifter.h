#ifndef CPITCHSHIFTER_H
#define CPITCHSHIFTER_H

#include "idevice.h"
#include "smbpitchshifter.h"
#include "cvoltagemodulator.h"

class CPitchShifter : public IDevice
{
private:
    enum JackNames
    {jnIn,jnModulation,jnOut};
    enum ParameterNames
    {pnShift,pnOverSampling,pnFrameSize,pnModulation,pnTune,pnMix};
    int PitchShift;
    //float ModFactor;
    //float Tune;
    float Wet;
    float Dry;
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    smbPitchShifter smb;
    CVoltageModulator Modulator;
public:
    CPitchShifter();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
};

#endif // CPITCHSHIFTER_H
