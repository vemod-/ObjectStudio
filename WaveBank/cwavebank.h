#ifndef CWAVEBANK_H
#define CWAVEBANK_H

#include "csimplebuffer.h"
#include "cpresets.h"

class SingleWaveBankArray : protected IPresetRef
{
public:
    ~SingleWaveBankArray();
    inline static float sine(const uint pos)
    {
        return getInstance()->m_Sine.at(pos);
    }
    inline static float triangle(const uint pos)
    {
        return getInstance()->m_Triangle.at(pos);
    }
    inline static float sawtooth(const uint pos)
    {
        return getInstance()->m_SawTooth.at(pos);
    }
private:
    inline static SingleWaveBankArray* getInstance()
    {
        static SingleWaveBankArray    instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return &instance;
    }
    CSimpleBuffer m_Triangle;
    CSimpleBuffer m_Sine;
    CSimpleBuffer m_SawTooth;
    inline SingleWaveBankArray() {
        m_SawTooth.init(presets.SampleRate);
        m_Sine.init(presets.SampleRate);
        m_Triangle.init(presets.SampleRate);
        const float HalfRateDivPi=presets.HalfRate / PI_F;
        qDebug() << "Wavebank buffers created";
        for (uint P=0; P < presets.SampleRate; P++)
        {
            //Sine
            m_Sine.set(sinf(P / HalfRateDivPi));
            //Triangle
            (P < presets.HalfRate) ? m_Triangle.set(((P * 2.f) / presets.HalfRate) - 1.f) :
                                   m_Triangle.set((((P - presets.HalfRate) * -2.f) / presets.HalfRate) + 1.f);
            //Sawtooth
            m_SawTooth.set((P / (float)presets.HalfRate) - 1.f);
        }
    }                   // Constructor? (the {} brackets) are needed here.
    inline SingleWaveBankArray(SingleWaveBankArray const&);              // Don't Implement
    inline void operator=(SingleWaveBankArray const&); // Don't implement
};

class CWaveBank : protected IPresetRef
{
private:
    float HoldFloat=0;
    bool HoldSet1=false;
    bool HoldSet2=false;
    float wPos=0;
public:
    enum WaveForms
    {Sine,Square,Triangle,Sawtooth,Ramp,Noise,SampleAndHold};
    inline CWaveBank() {}
    ~CWaveBank();
    float inline getNext(const uint Position,const WaveForms WaveForm)
    {
        switch (WaveForm)
        {
        case Sine:
            return SingleWaveBankArray::sine(Position);
        case Square:
            if (Position < presets.HalfRate) return 1;
            return -1;
        case Triangle:
            return SingleWaveBankArray::triangle(Position);
        case Sawtooth:
            return SingleWaveBankArray::sawtooth(Position);
        case Ramp:
            return -SingleWaveBankArray::sawtooth(Position);
        case Noise:
            return (QRandomGenerator::global()->generateDouble()*2)-1;
        case SampleAndHold:
            if (Position < presets.HalfRate)
            {
                if (!HoldSet1)
                {
                    HoldFloat = (QRandomGenerator::global()->generateDouble()*2)-1;
                    HoldSet1 = true;
                    HoldSet2 = false;
                }
            }
            else
            {
                if (!HoldSet2)
                {
                    HoldFloat = (QRandomGenerator::global()->generateDouble()*2)-1;
                    HoldSet2 = true;
                    HoldSet1 = false;
                }
            }
            return HoldFloat;
        }
        return 0;
    }
    float inline getNextFreq(const float Frequency,const WaveForms WaveForm)
    {
        const float retVal=getNext(uint(wPos),WaveForm);
        wPos+=Frequency;
        while (wPos>=presets.SampleRate) wPos-=presets.SampleRate;
        return retVal;
    }
    inline void reset(const uint Position)
    {
        wPos = Position;
    }
};

#endif // CWAVEBANK_H
