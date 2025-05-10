#ifndef CPITCHTRACKERCLASS_H
#define CPITCHTRACKERCLASS_H

#include <QVector>

class CPitchTrackerClass
{
public:
    QVector<float> coeffs;
    float CurrentFreq;
    int CurrentVel;
    int CurrentNote;
    float CurrentMIDIFreq;
    float CurrentDiff;
    float Threshold;
    float InTune;
    float OutTune;
    CPitchTrackerClass(int BufferSz,int SampleRate);
    ~CPitchTrackerClass();
    void process();
private:
    QVector<float> coeffs1;
    QVector<float> coeffs2;
    QVector<float> coeffs3;
    QVector<float> Product;
    int note_num[5];
    int note_num3[5];
    int m_BufferSize;
    int m_SampleRate;
};

#endif // CPITCHTRACKERCLASS_H
