#ifndef CPITCHSHIFTER_H
#define CPITCHSHIFTER_H

#include "idevice.h"
#include "smbpitchshifter.h"
#include "cvoltagemodulator.h"

#define devicejacks monoin,modulationin,monoout
#define devicecategory Effect | SynthModule

class CPitchShifter : public IDevice
{
private:
    enum JackNames
    {devicejacks};
    enum ParameterNames
    {pnShift,pnOverSampling,pnModulation,pnTune,pnMix};
    //float ModFactor;
    //float Tune;
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    void play(bool FromStart) {
        smb.reset();
        IDevice::play(FromStart);
    }
    smbPitchShifter smb;
    CMonoBuffer returnBuffer;
    CVoltageModulator Modulator;
public:
    CPitchShifter();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
};

#endif // CPITCHSHIFTER_H
