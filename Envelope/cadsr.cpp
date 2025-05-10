#include "cadsr.h"

CADSR::CADSR()
{
    clear();
}

float CADSR::GetVol(const float Trigger)
{
    if (!closeEnough(Trigger,LastTrigger))
    {
        if (isZero(Trigger))
        {
            Finish();
        }
        else
        {
            if (Mode==1)
            {
                if (isZero(LastTrigger)) Start(Trigger);
            }
            else
            {
                Start();
            }
        }
        LastTrigger = Trigger;
    }
    return GetVol();
}

float CADSR::GetVol()
{
    if (State==esPlaying)
    {
        if (TimeCount < DelayLen) CurrentVolume = 0;
        else if (TimeCount < AttackLen+DelayLen) CurrentVolume = attackBase + CurrentVolume * attackCoef;
        else if (TimeCount <= AttackLen+DelayLen+HoldLen) CurrentVolume = 1;
        else if (TimeCount > AttackLen+DelayLen+HoldLen)
        {
            if (TimeCount <= AttackLen+DelayLen+HoldLen+DecayLen) CurrentVolume = decayBase + CurrentVolume * decayCoef;
        }
        if (TimeCount > AttackLen+DelayLen+HoldLen+DecayLen) CurrentVolume = SustainVol;
    }
    else if (State==esReleasing)
    {
        if (TimeCount < ReleaseTime + ReleaseLen)
        {
            CurrentVolume = releaseBase + CurrentVolume * releaseCoef;
        }
        else
        {
            CurrentVolume = 0;
            State=esSilent;
        }
    }
    LastGet = CurrentVolume;
    TimeCount++;
    return CurrentVolume * VolumeFactor;
}

void CADSR::calcParams()
{
    VolumeFactor = VelocityFactor;
    DelayLen = mSec2Buffer(AP.Delay);
    AttackLen = mSec2Buffer(AP.Attack);
    HoldLen = mSec2Buffer(AP.Hold);
    DecayLen = mSec2Buffer(AP.Decay);
    ReleaseLen = mSec2Buffer(AP.Release);
    SustainVol = VolConv(AP.Sustain);
    targetRatioA=0.3;
    targetRatioDR=0.0001;
    attackCoef = calcCoef(AttackLen, targetRatioA);
    attackBase = (1 + targetRatioA) * (1 - attackCoef);
    decayCoef = calcCoef(DecayLen, targetRatioDR);
    decayBase = (SustainVol - targetRatioDR) * (1 - decayCoef);
    releaseCoef = calcCoef(ReleaseLen, targetRatioDR);
    releaseBase = -targetRatioDR * (1 - releaseCoef);
}

ldouble CADSR::Buffer2mSec(const ldouble buffer)
{
    return buffer*presets.ModulationTime;
}

ulong64 CADSR::mSec2Buffer(const ulong64 mSec)
{
    return mSec * presets.ModulationsPermSec;
}

double CADSR::VolConv(const double Percent)
{
    return Percent * 0.01;
}

void CADSR::Start(float Vel)
{
    State=esPlaying;
    VelocityFactor=Vel;
    calcParams();
    TimeCount = 0;
}

void CADSR::Finish()
{
    ReleaseTime = TimeCount;
    State=esReleasing;
    if (DecayLen) SustainVol = LastGet;
    CurrentVolume = SustainVol;
}

double CADSR::calcCoef(double rate, double targetRatio) {
    return (rate <= 0) ? 0 : exp(-log((1 + targetRatio) / targetRatio) / rate);
}

//---------------------------------------------------------------------------
void CADSR::serialize(QDomLiteElement* xml) const
{
    xml->setAttribute("Delay",AP.Delay);
    xml->setAttribute("Attack",AP.Attack);
    xml->setAttribute("Hold",AP.Hold);
    xml->setAttribute("Decay",AP.Decay);
    xml->setAttribute("Sustain",AP.Sustain);
    xml->setAttribute("Release",AP.Release);
}

void CADSR::unserialize(const QDomLiteElement* xml)
{
    if (!xml) return;
    AP.Delay=xml->attributeValueULongLong("Delay",0);
    AP.Attack=xml->attributeValueULongLong("Attack",0);
    AP.Hold=xml->attributeValueULongLong("Hold",0);
    AP.Decay=xml->attributeValueULongLong("Decay",0);
    AP.Sustain=xml->attributeValueInt("Sustain",100);
    AP.Release=xml->attributeValueULongLong("Release",0);
    calcParams();
}

void CADSR::clear()
{
    AP.Delay=0;
    AP.Attack=0;
    AP.Hold=0;
    AP.Decay=0;
    AP.Sustain=100;
    AP.Release=0;
    LastTrigger=0;
    LastGet=0;
    State=esSilent;
    TimeCount=0;
    CurrentVolume=0;
    Mode=0;
    calcParams();
}
