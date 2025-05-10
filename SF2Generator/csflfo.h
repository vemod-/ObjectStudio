#ifndef CSFLFO_H
#define CSFLFO_H

#include "softsynthsdefines.h"
#include "cpresets.h"
#include "cwavebank.h"

class CSFLFO : protected IPresetRef
{
private:
    enum SFLFOValues
    {
        evSilent,
        evDelay,
        evAttack
    };
    int mDelay;
    long Counter;
    CWaveBank LFO;
    SFLFOValues CurrentAction;
    float FreqValue;
public:
    void Init(const int Freq, const int Delay, const int /*KeyNum*/)
    {
        mDelay=timecent2msec(Delay)*presets.ModulationsPermSec;
        FreqValue=8.176f*cent2Factorf(Freq)*presets.ModulationRate;
        Counter=0;
        CurrentAction=evSilent;
        LFO.reset(presets.SampleRate/4);
    }
    float inline GetNext()
    {
        if (CurrentAction==evSilent) return 0;
        if (CurrentAction==evDelay)
        {
            if (Counter++ < mDelay) return 0;
            CurrentAction=evAttack;
        }
        return LFO.getNextFreq(FreqValue,CWaveBank::Triangle);
    }
    void Start()
    {
        Counter=0;
        CurrentAction=evDelay;
        LFO.reset(presets.SampleRate/4);
    }
};

#endif // CSFLFO_H
