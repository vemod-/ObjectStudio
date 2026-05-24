#ifndef CAUTOTUNE_H
#define CAUTOTUNE_H

#define devicejacks monoout,monoin
#define devicecategory Effect

#include "YinPitchDetector.h"
#include "smbpitchshifter.h"
#include "idevice.h"

class CAutoTune : public IDevice
{
public:
    CAutoTune();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
private:
    enum JackNames
    {devicejacks};
    enum ParameterNames
    {pnTune,pnGlide,pnSlack,pnThreshold,pnMaxFreq,pnRate,pnOversampling};
    CYIN PD;
    smbPitchShifter PS;
    void inline updateDeviceParameter(const CParameter* p = nullptr);
};

#endif // CAUTOTUNE_H
