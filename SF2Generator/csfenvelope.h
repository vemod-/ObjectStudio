#ifndef CSFENVELOPE_H
#define CSFENVELOPE_H

#include "softsynthsdefines.h"
#include "cpresets.h"
#include "cadsr.h"

class CSFEnvelope : public CADSR
{
public:
    void Init(const int Delay, const int Attack, const int Hold, const int Decay, const int Sustain, const int Release, const int AutoHold, const int AutoRelease, const int KeyNum)
    {
        AP.Delay=timecent2msec(Delay)*presets.ModulationsPermSec;
        AP.Attack=timecent2msec(Attack)*presets.ModulationsPermSec;
        AP.Hold=ulong64(timecent2msec(Hold)*presets.ModulationsPermSec*voltage2Factorf((AutoHold*(KeyNum-60))/-1200));
        AP.Decay=timecent2msec(Decay)*presets.ModulationsPermSec;
        AP.Sustain=cB2Percentf(Sustain)*100;
        AP.Release=ulong64(timecent2msec(Release)*presets.ModulationsPermSec*voltage2Factorf((AutoRelease*(KeyNum-60))/-1200));
        State=esSilent;
    }
    inline bool isSilent() { return State==esSilent; }
    float inline GetNext(){ return GetVol(); }
};

#endif // CSFENVELOPE_H
