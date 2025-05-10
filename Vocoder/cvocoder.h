#ifndef CVOCODER_H
#define CVOCODER_H

#include "idevice.h"
#include "ccvdevice.h"
#include "cpitchdetect.h"
#include "smbpitchshifter.h"
#include "cfreqglider.h"

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
    {jnOut,jnIn,jnMIDIIn};
    enum ParameterNames
    {pnMIDIChannel,pnTranspose,pnTune,pnAutotune,pnGlide,pnOversampling,pnEffect};
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    CCVDevice CVDevice;
    CPitchDetect PD;
    smbPitchShifter PS;
    double m_shiftFactor[8];
    float m_scale[8];
    CMonoBuffer* inBuffer = nullptr;
    int m_lastKey = 0;
    int m_lastMIDICent = 0;
    CFreqGlider glider;
};

#endif // CVOCODER_H
