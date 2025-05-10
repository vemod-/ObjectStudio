#ifndef CAUTOTUNE_H
#define CAUTOTUNE_H

#include "cpitchdetect.h"
#include "smbpitchshifter.h"
#include "cfreqglider.h"
#include "idevice.h"

class CAutoTune : public IDevice
{
public:
    CAutoTune();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
private:
    enum JackNames
    {jnOut,jnIn};
    enum ParameterNames
    {pnTune,pnGlide,pnMaxFreq,pnRate,pnOversampling};
    CPitchDetect PD;
    smbPitchShifter PS;
    CFreqGlider glider;
    int m_lastMIDICent = 0;
    void inline updateDeviceParameter(const CParameter* p = nullptr);
};

#endif // CAUTOTUNE_H
