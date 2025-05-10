#ifndef CMIDI2CV_H
#define CMIDI2CV_H

#include "idevice.h"
#include "ccvdevice.h"

class CMIDI2CV : public IDevice
{
private:
    enum JackNames
    {jnFrequency,jnVelocity=CVDevice::CVVoices,jnIn=(CVDevice::CVVoices*2)};
    enum ParameterNames
    {pnMIDIChannel,pnTranspose,pnTune};
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    CCVDevice CVDevice;
public:
    CMIDI2CV();
    void init(const int Index, QWidget* MainWindow);
    float getNext(const int ProcIndex);
    void play(const bool FromStart);
    void pause();
};

#endif // CMIDI2CV_H
