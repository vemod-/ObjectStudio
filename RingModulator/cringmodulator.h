#ifndef CRINGMODULATOR_H
#define CRINGMODULATOR_H

#include "idevice.h"

#define devicejacks monoout,monoin,modmonoin
#define devicecategory Effect | SynthModule

class CRingModulator : public IDevice
{
private:
    enum JackNames
    {devicejacks};
    enum ParameterNames
    {pnModulation};
    float ModulationFactor;
    float CleanFactor;
    void inline updateDeviceParameter(const CParameter* p = nullptr);
public:
    CRingModulator();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
};

#endif // CRINGMODULATOR_H
