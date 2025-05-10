#ifndef CSEQUENSER_H
#define CSEQUENSER_H

#include "idevice.h"
/*
#ifdef BeatPoly
#undef BeatPoly
#endif
#define BeatPoly 1
*/
#include "sequenserclasses.h"
#include "cmseccounter.h"

class CSequenser : public IDevice
{
public:
    CSequenser();
    ~CSequenser();
    QList<PatternType*> Patterns;
    QList<PatternListType*> PatternsInList;
    void play(const bool FromStart);
    void pause();
    ulong milliSeconds() const;
    ulong ticks() const;
    ulong64 samples() const;
    void skip(const ulong64 samples);
    void init(const int Index, QWidget* MainWindow);
    CMIDIBuffer* getNextP(const int ProcIndex);
    void tick();
private:
    enum JackNames
    {jnMIDIOut};
    enum ParameterNames
    {pnTempo,pnMIDIChannel,pnHumanize};
    int Counter;
    int BeatCount;
    int PatternIndex;
    int PatternRepeatCount;
    int CurrentPitch;
    int CurrentLength;
    int PatternLength;
    int NextBeat;
    int NextStop;
    CTickCounter mSecCount;
    //bool Playing;
    CMIDIBuffer* MIDIBuffer;
    ulong m_MilliSeconds;
    ulong m_Ticks;
    void Reset();
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    void CalcDuration();
};

#endif // CSEQUENSER_H
