#ifndef CSFFILTER_H
#define CSFFILTER_H

#include "softsynthsdefines.h"
#include "cpresets.h"

class CSFFilter : protected IPresetRef
{
private:
    float FiltCoefTab0;
    float FiltCoefTab1;
    float FiltCoefTab2;
    float FiltCoefTab3;
    float FiltCoefTab4;
    float ly1;
    float ly2;
    float lx1;
    float lx2;
    float m_ExpResonance;
    int LastCO;
    float MixFactor;
    int CurrentCutOff;
    int CurrentHiQ;
    float CurrentAmount;
    void inline CalcExpResonance()
    {
        m_ExpResonance=expf(CurrentHiQ*0.0625f);
    }
public:
    void Init(const int CutOff, const int HiQ)
    {
        CurrentCutOff=int(8.176*cent2Factor(CutOff));
        CurrentHiQ=int(10*filterQadip(HiQ))+1;
        //Debug(QString::number(CurrentCutOff) + " " + QString::number(CurrentHiQ));
        CurrentAmount=1;
        FiltCoefTab0=0;
        FiltCoefTab1=0;
        FiltCoefTab2=0;
        FiltCoefTab3=0;
        FiltCoefTab4=0;
        ly1=0;
        ly2=0;
        lx1=0;
        lx2=0;
        m_ExpResonance=0;
        MixFactor=0;
        LastCO=0;
        SetAmount(1);
    }
    float inline GetNext(const float Signal)
    {
        const float Temp_y=(FiltCoefTab0 * Signal) + (FiltCoefTab1 * lx1) + (FiltCoefTab2 * lx2) + (FiltCoefTab3 * ly1) + (FiltCoefTab4 * ly2);
        ly2=ly1;
        ly1=Temp_y;
        lx2=lx1;
        lx1=Signal;
        return Temp_y * MixFactor;
    }
    void SetAmount(const float Amount)
    {
        CurrentAmount=Amount;
        const int CutOff=qBound<int>(20,int(CurrentCutOff * CurrentAmount),presets.MaxCutoff);
        if (LastCO != CutOff)
        {
            LastCO=CutOff;
            const float Omega=(PI_F * LastCO) / presets.HalfRate;
            const float cs=cosf(Omega);
            const float Alpha=sinf(Omega) / CurrentHiQ;
            const float a0=1+Alpha;
            FiltCoefTab1=(1-cs)/a0;
            FiltCoefTab0=FiltCoefTab1*0.5f;
            FiltCoefTab2=FiltCoefTab0;
            FiltCoefTab3=-(-2*cs)/a0;
            FiltCoefTab4=-(1-Alpha)/a0;
            MixFactor=(float(presets.MaxCutoff) / float(LastCO)) * 0.004f;
            MixFactor=1.f / ((CurrentHiQ*MixFactor)+(1-MixFactor));
        }
    }
};

#endif // CSFFILTER_H
