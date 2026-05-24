#ifndef CPANNER_H
#define CPANNER_H

#include "idevice.h"
#include "cvoltagemodulator.h"

#define devicejacks stereoin,stereoout,leftmonoout,rightmonoout,modulationin
#define devicecategory Effect | SynthModule

class CPanner : public IDevice
{
private:
    enum JackNames
    {devicejacks};
    enum ParameterNames
    {pnPan,pnModulation};
    float LeftModFactor;
    float RightModFactor;
    float LeftFactor;
    float RightFactor;
    CStereoBuffer* InSignal;
    CVoltageModulator Modulator;
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    void process();
public:
    CPanner();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
};

#endif // CPANNER_H
