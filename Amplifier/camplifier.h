#ifndef CAMPLIFIER_H
#define CAMPLIFIER_H

#include "idevice.h"
#include "cvoltagemodulator.h"

class CAmplifier : public IDevice
{
public:
    CAmplifier();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
private:
    enum JackNames
    {jnIn,jnOut,jnModulation};
    enum ParameterNames
    {pnModulation};
    CVoltageModulator Modulator;
    void inline updateDeviceParameter(const CParameter* p = nullptr);
};

#endif // CAMPLIFIER_H
