#ifndef CSIGNALSPLITTER_H
#define CSIGNALSPLITTER_H

#include "idevice.h"
#include "biquad.h"
#include "cvoltagemodulator.h"

#define devicejacks monoout1,monoout2,monoin,modulationin
#define devicecategory SynthModule | Effect

class CSignalSplitter : public IDevice
{

public:
    CSignalSplitter();
    void init(const int Index, QWidget* MainWindow) override;
    void play(const bool FromStart) override;
private:
    enum JackNames
    {devicejacks};
    enum ParameterNames
    {pnType,pnSplitFreq,pnSplitVolume,pnModulation,pnSlope,pnResponse};
    CBiquad lopass;
    CBiquad hipass;
    void inline updateDeviceParameter(const CParameter* p = nullptr) override;
    void process() override;
    CVoltageModulator modulator;
    float m_splitLevel=0;
    float m_level;
    int m_peakCount=0;
    int m_freq=0;
};

#endif // CSIGNALSPLITTER_H
