#ifndef CPOLYBOX_H
#define CPOLYBOX_H

#include "idevice.h"
#include "ccvdevice.h"

class CPolyBox : public IDevice
{
public:
    CPolyBox();
    ~CPolyBox();
    void play(const bool FromStart);
    void pause();
    void skip(const ulong64 samples);
    void init(const int Index, QWidget* MainWindow);
    float getNext(const int ProcIndex);
    CMIDIBuffer* getNextP(const int ProcIndex);
    CAudioBuffer* getNextA(const int ProcIndex);
    void process();
private:
    enum JackNames
    {jnOut,jnOutLeft,jnOutRight,jnMIDIIn};
    enum ParameterNames
    {pnMIDIChannel,pnTranspose,pnTune};
    void Reset();
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    QList<IJack*> JacksCreated;
    QList<CInJack*> WaveOut;
    CCVDevice CVDevice;
    CMIDIBuffer MIDIBuffer;
    int prevMIDIkey[CVDevice::CVVoices];
    int prevVel[CVDevice::CVVoices];
};

#endif // CPOLYBOX_H
