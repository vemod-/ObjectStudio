#ifndef CPREAMP_H
#define CPREAMP_H

#include "idevice.h"
#include "filters.h"
#include "c3bandfilter.h"
#include "campsimulator.h"

class CPreamp : public IDevice
{
public:
    CPreamp();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
private:
    enum JackNames
    {jnIn,jnOut};
    enum ParameterNames
    {pnGain,pnBass,pnMid,pnTreble,pnPresence,pnSimulation,pnVolume};
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    float gain;
    float volume;
    C3BandFilter f3;
    filter f;
    CAmpSimulator amp;
};

#endif // CPREAMP_H
