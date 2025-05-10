#ifndef CSPECTRUM_H
#define CSPECTRUM_H

#include "idevice.h"

class CSpectrum : public IDevice
{
public:
    CSpectrum();
    void init(const int Index, QWidget* MainWindow);
    void tick();
private:
    enum JackNames
    {jnIn};
    enum ParameterNames
    {pnVolume,pnSpectrumRate,pnMode,pnWindow,pnScale,pnRange};
    void Reset();
    void inline updateDeviceParameter(const CParameter* p = nullptr);
};

#endif // CSPECTRUM_H
