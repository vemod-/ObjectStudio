#ifndef CADSR_H
#define CADSR_H

//#include <QtCore>
#include "cpresets.h"
#include "qdomlite.h"

class CADSR : protected IPresetRef
{
public:
    enum EnvelopeStates
    {
        esSilent,
        esPlaying,
        esReleasing
    };
    struct ADSRParams
    {
        ulong64 Delay;
        ulong64 Attack;
        ulong64 Hold;
        ulong64 Decay;
        int Sustain;
        ulong64 Release;
    };
    CADSR();
    float GetVol(const float Trigger);
    float GetVol();
    void serialize(QDomLiteElement* xml) const;
    void unserialize(const QDomLiteElement* xml);
    void clear();
    void Start(float Vel=1.f);
    void Finish();
    ADSRParams AP;
    int Mode;
    EnvelopeStates State;
    void calcParams();
    ldouble Buffer2mSec(const ldouble buffer);
    ulong64 mSec2Buffer(const ulong64 mSec);
private:
    ulong64 DelayLen;
    ulong64 AttackLen;
    ulong64 HoldLen;
    ulong64 DecayLen;
    ulong64 ReleaseLen;
    double SustainVol;
    ulong64 ReleaseTime;
    double LastGet;
    float LastTrigger;
    ulong64 TimeCount;
    double CurrentVolume;
    double VolumeFactor;
    double VelocityFactor;
    double inline VolConv(const double Percent);
    double attackCoef;
    double decayCoef;
    double releaseCoef;
    double targetRatioA;
    double targetRatioDR;
    double attackBase;
    double decayBase;
    double releaseBase;
    double calcCoef(double rate, double targetRatio);
};

#endif // CADSR_H
