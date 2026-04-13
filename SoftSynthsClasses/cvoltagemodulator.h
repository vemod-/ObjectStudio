#ifndef CVOLTAGEMODULATOR_H
#define CVOLTAGEMODULATOR_H

#include "cfreqglider.h"
#include "ijack.h"
#include "cparameter.h"

class CVoltageModulator
{
public:
    enum TuneType
    {
        Coarse,
        Fine,
        Freq,
        Cents
    };
    CVoltageModulator(){}
    void init(IJack* ModulationJack = nullptr, CParameter* ModulationParameter = nullptr, CParameter* TuneParameter = nullptr, const TuneType T = Coarse)
    {
        ModulationIn = static_cast<CInJack*>(ModulationJack);
        Modulation = ModulationParameter;
        Tuning = TuneParameter;
        TuningType = T;
    }
    void setGlide(const int g)
    {
        GlideValue = g;
        FreqGlider.setGlide(GlideValue);
        //float t = (((100.0 - g) * 0.99) + 1.0) * 0.01;
        //m_glideFactor = powf(t, 2.0f); // eller 2.0f
        //m_glideFactor = sqrtf(t);
    }
    float exec(const float InVoltage=0)
    {
        Changed = false;
        if (InVoltage > 0)
        {
            if (!closeEnough(InVoltage,LastInVoltage))
            {
                DefaultVoltage=InVoltage;
            }
            LastInVoltage=InVoltage;
        }
        else if (DefaultVoltage > 0)
        {
            LastInVoltage=DefaultVoltage;
        }

        float ReturnVoltage = FreqGlider.runVoltage(LastInVoltage);
        if ((Modulation != nullptr) & (ModulationIn != nullptr)) if (Modulation->Value) ReturnVoltage += ModulationIn->getNext()*Modulation->percentValue();
        if (Tuning)
        {
            if (TuningType == Coarse) { if (Tuning->Value) ReturnVoltage += Tuning->percentValue(); }
            else if (TuningType == Fine) { if (Tuning->Value) ReturnVoltage += Tuning->percentValue()*0.01f; }
            else if (TuningType == Freq) { ReturnVoltage += tune2voltagef(Tuning->percentValue()); }
            else if (TuningType == Cents) { ReturnVoltage += Tuning->Value / 1200.0; }
        }
        //m_LastReturnVoltage = FreqGlider.runVoltage(ReturnVoltage);
        //m_LastReturnVoltage = (1.0 - m_glideFactor) * m_LastReturnVoltage + m_glideFactor * ReturnVoltage;

        if (!closeEnough(ReturnVoltage,PrevVoltage))
        {
            PrevVoltage = ReturnVoltage;
            Changed = true;
        }
        return ReturnVoltage;
    }
    float execFreq(float freq=0)
    {
        return voltage2Freqf(exec(freq2voltagef(freq)));
    }
    long execCent(long c=0)
    {
        return long(exec(c/1200.f)*1200.f);
    }
    void setDefaultVoltage(const float v)
    {
        DefaultVoltage = v;
    }
    void setDefaultFreq(const float f)
    {
        DefaultVoltage = freq2voltagef(f);
    }
    bool changed()
    {
        return Changed;
    }
private:
    CInJack* ModulationIn=nullptr;
    CParameter* Modulation=nullptr;
    CParameter* Tuning=nullptr;
    float PrevVoltage = 0;
    float DefaultVoltage = 0;
    float LastInVoltage=0;
    CFreqGlider FreqGlider;
    int GlideValue=0;
    bool Changed=false;
    TuneType TuningType=Coarse;
    //double m_glideFactor = 1;
    //float m_LastReturnVoltage = 0;
};


#endif // CVOLTAGEMODULATOR_H
