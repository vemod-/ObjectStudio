#ifndef CWAVEBANK_H
#define CWAVEBANK_H

#include <QtCore>

class SingleWaveBankArray
{
    public:
        static SingleWaveBankArray* getInstance()
        {
            static SingleWaveBankArray    instance; // Guaranteed to be destroyed.
                                  // Instantiated on first use.
            return &instance;
        }
        QVarLengthArray<float> Triangle;
        QVarLengthArray<float> Sine;
        QVarLengthArray<float> SawTooth;
    private:
        SingleWaveBankArray() {}                   // Constructor? (the {} brackets) are needed here.
        SingleWaveBankArray(SingleWaveBankArray const&);              // Don't Implement
        void operator=(SingleWaveBankArray const&); // Don't implement
};

class CWaveBank
{
private:
    float HoldFloat;
    short HoldInt;
    bool HoldSet1;
    bool HoldSet2;
    static float HalfRate;
    static unsigned int SampleRate;
    SingleWaveBankArray* Buffers;
    void FillBuffers();
    float wPos;
    static float RAND_MAX_DIV;
public:
    enum WaveForms
    {Sine,Square,Triangle,Sawtooth,Noise,SampleAndHold};
    CWaveBank();
    const float GetNext(const unsigned int& Position,const WaveForms& WaveForm);
    const float GetNextFreq(const float& Frequency,const WaveForms& WaveForm);
};

#endif // CWAVEBANK_H
