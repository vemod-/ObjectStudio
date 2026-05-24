#ifndef CVOCODER_H
#define CVOCODER_H

#include "idevice.h"
#include "ccvdevice.h"
#include "YinPitchDetector.h"
#include "smbpitchshifter.h"

#define devicejacks monoout,monoin,midiin
#define devicecategory Instrument

class CVocoder : public IDevice
{

public:
    CVocoder();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
    void play(const bool FromStart);
    void pause();
private:
    enum JackNames
    {devicejacks};
    enum ParameterNames
    {pnMIDIChannel,pnTranspose,pnTune,pnAutotune,pnGlide,pnSlack,pnThreshold,pnOversampling,pnEffect};
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    CCVDevice CVDevice;
    CYIN PD;
    smbPitchShifter PS;
    double m_shiftFactor[8];
    float m_scale[8];
    CMonoBuffer* inBuffer = nullptr;
    int m_lastKey = 0;
};

#endif // CVOCODER_H
