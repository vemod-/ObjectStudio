#ifndef CFREQGLIDER_H
#define CFREQGLIDER_H

#include "cpresets.h"

class CFreqGlider : protected IPresetRef
{
private:
    double m_LastGlideVoltage=0;
    double m_AttackAdd=0;
    double m_DecayAdd=0;
    int m_CurrentGlide=0;
    double m_TargetVoltage=0;
    int m_CurrentSpeed=1;
    inline double calcAdd(int Glide)
    {
        const double factor=(pow(double(100-Glide)*0.01,4)*0.95)+0.05;
        return factor*(presets.ModulationTime/(m_CurrentSpeed*40));
    }
    inline void calc()
    {
        m_AttackAdd=calcAdd(m_CurrentGlide);
        m_DecayAdd=m_AttackAdd;
    }
    inline void exec()
    {
        if (closeEnough(m_LastGlideVoltage,m_TargetVoltage)) return;
        double f = m_TargetVoltage;
        if (m_CurrentGlide)
        {
            if (m_LastGlideVoltage - m_DecayAdd >= m_TargetVoltage)
            {
                f = m_LastGlideVoltage - m_DecayAdd;
            }
            else if (m_LastGlideVoltage + m_AttackAdd <= m_TargetVoltage)
            {
                f = m_LastGlideVoltage + m_AttackAdd;
            }
            else
            {
                f = m_TargetVoltage;
            }
        }
        m_LastGlideVoltage = f;
    }
public:
    CFreqGlider()
    {
        calc();
    }
    inline void setGlide(const int Glide)
    {
        m_CurrentGlide=Glide;
        calc();
    }
    inline void setGlide(const int Attack, const int Decay)
    {
        m_CurrentGlide=Attack+Decay;
        m_AttackAdd = calcAdd(Attack);
        m_DecayAdd = calcAdd(Decay);

    }
    inline void setTargetFreq(const float Freq)
    {
        m_TargetVoltage = freq2voltage(Freq);
        //calc();
    }
    inline void setTargetVoltage(const float Volt)
    {
        m_TargetVoltage = Volt;
        //calc();
    }
    inline void setTargetCent(const long MIDICents)
    {
        m_TargetVoltage = MIDICents/1200.0;
        //calc();
    }
    inline float currentFreq()
    {
        exec();
        return voltage2Freq(m_LastGlideVoltage);
    }
    inline long currentCent()
    {
        exec();
        return m_LastGlideVoltage*1200;
    }
    inline float currentVoltage()
    {
        exec();
        return m_LastGlideVoltage;
    }
    inline float runVoltage(const float v)
    {
        if (m_CurrentGlide==0) return v;
        setTargetVoltage(v);
        return currentVoltage();
    }
    inline float runFreq(const float f)
    {
        if (m_CurrentGlide==0) return f;
        setTargetFreq(f);
        return currentFreq();
    }
    inline long runCent(const long c)
    {
        if (m_CurrentGlide==0) return c;
        setTargetCent(c);
        return currentCent();
    }
    inline void setSpeed(const int Speed)
    {
        m_CurrentSpeed=Speed;
        calc();
    }
};

#endif // CFREQGLIDER_H
