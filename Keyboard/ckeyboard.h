#ifndef CKEYBOARD_H
#define CKEYBOARD_H

#include "idevice.h"
#include "cmidibuffer.h"

class CKeyboard : public IDevice
{

public:
    CKeyboard();
    void init(const int Index, QWidget* MainWindow);
    CMIDIBuffer* getNextP(const int ProcIndex);
    float getNext(const int ProcIndex);
    void tick();
    CMIDIBuffer MIDIBuffer;
    QList<int> notesOn;
    QList<int> notesOff;
    QList<int> notesDown;
    int pitchBend;
    int pbOld;
    float mod1;
    int modOld1;
    float mod2;
    int modOld2;
    enum ParameterNames
    {pnMIDIChannel,pnTune,pnPitchBendRange,pnMod1Mode,pnMod2Mode};
private:
    enum JackNames
    {jnMIDI,jnFrequency,jnTrigger,jnModulationOut1,jnModulationOut2,jnModulationIn1,jnModulationIn2};
    void inline updateDeviceParameter(const CParameter* p = nullptr);
};

#endif // CKEYBOARD_H
