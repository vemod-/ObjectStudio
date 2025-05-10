#ifndef CDRUMMACHINE_H
#define CDRUMMACHINE_H

#include "idevice.h"
#include "cwavegenerator.h"
#include "cmseccounter.h"

#define DRUMMACHINEFORM FORMFUNC(CDrumMachineForm)

namespace DrumMachine
{
const int SoundCount=7;
}

#include "sequenserclasses.h"

class CWaveGeneratorX : public CWaveGenerator
{
public:
    QString Name;
    float Volume;
    void trigger(const int vol)
    {
        if (vol>0)
        {
            reset();
            Volume=vol*0.01f;
        }
    }
};

class CDrumMachine : public IDevice
{
private:
    enum JackNames
    {jnOut,jnMIDIOut};
    enum ParameterNames
    {pnTempo,pnVolume,pnHumanize};
    float VolumeFactor;
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    int Counter;
    int BeatCount;
    int PatternIndex;
    int PatternRepeatCount;
    int PatternLength;
    ulong m_MilliSeconds;
    ulong m_Ticks;
    CTickCounter mSecCount;
    //bool Playing;
    void CalcDuration();
    void Reset();
    void inline AddSound(const QString& Path,const QString& Name,CWaveGeneratorX& WG)
    {
        if (WG.load(":/sounds/"+Path))
        {
            WG.Name=Name;
            WG.Volume=0;
        }
    }
    byte MIDINumbers[7] = {36,38,42,46,49,50,48};
    CMIDIBuffer MIDIBuffer;
public:
    ~CDrumMachine();
    void init(const int index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
    CMIDIBuffer* getNextP(const int ProcIndex);
    void tick();
    CWaveGeneratorX WG[DrumMachine::SoundCount];
    QList<PatternType*> Patterns;
    QList<PatternListType*> PatternsInList;
    void play(const bool FromStart);
    void pause();
    ulong milliSeconds() const;
    ulong64 samples() const;
    ulong ticks() const;
    void skip(const ulong64 samples);
};

#endif // CDRUMMACHINE_H
