#include "cwavebank.h"
#include "softsynthsclasses.h"

float CWaveBank::HalfRate=CPresets::Presets.HalfRate;
unsigned int CWaveBank::SampleRate=CPresets::Presets.SampleRate;
float CWaveBank::RAND_MAX_DIV=1.0/(float)RAND_MAX;

void CWaveBank::FillBuffers()
{
    SampleRate=CPresets::Presets.SampleRate;
    HalfRate=CPresets::Presets.HalfRate;
    RAND_MAX_DIV=1.0/(float)RAND_MAX;
    Buffers->SawTooth.resize(CPresets::Presets.SampleRate);
    Buffers->Sine.resize(CPresets::Presets.SampleRate);
    Buffers->Triangle.resize(CPresets::Presets.SampleRate);
    float HalfRateDivPi=(float)HalfRate / DoublePi;
    qDebug() << "Wavebank buffers created";
    float P=0;
    for (unsigned int Position=0;Position<SampleRate;Position++)
    {
        //Sine
        Buffers->Sine[Position]=sin(P / HalfRateDivPi);

        //Triangle
        if (Position<HalfRate)
        {
            Buffers->Triangle[Position]=((P * 2.0)/HalfRate)-1.0;
        }
        else
        {
            Buffers->Triangle[Position]=(((P - HalfRate) * -2.0) / HalfRate) + 1.0;
        }
        //Sawtooth
        Buffers->SawTooth[Position]=(P/HalfRate)-1.0;
        P++;
    }
}

//---------------------------------------------------------------------------

CWaveBank::CWaveBank()
{
    Buffers=SingleWaveBankArray::getInstance();
    if (Buffers->SawTooth.size()==0) FillBuffers();
    wPos=0;
}

float CWaveBank::GetNextFreq(const float &Frequency, const WaveForms &WaveForm)
{
    float retVal=GetNext(wPos,WaveForm);
    wPos+=Frequency;
    while (wPos>=SampleRate) wPos-=SampleRate;
    return retVal;
}

float CWaveBank::GetNext(const unsigned int& Position,const WaveForms& WaveForm)
{
    switch (WaveForm)
    {
    case Sine:
        return Buffers->Sine[Position];
    case Square:
        if (Position < HalfRate) return 1;
        return -1;
    case Triangle:
        return Buffers->Triangle[Position];
    case Sawtooth:
        return Buffers->SawTooth[Position];
    case Noise:
        return ((qrand()*2)-RAND_MAX)*RAND_MAX_DIV;
    case SampleAndHold:
        if (Position < HalfRate)
        {
            if (!HoldSet1)
            {
                HoldFloat = ((qrand()*2)-RAND_MAX)*RAND_MAX_DIV;
                HoldSet1 = true;
                HoldSet2 = false;
            }
        }
        else
        {
            if (!HoldSet2)
            {
                HoldFloat = ((qrand()*2)-RAND_MAX)*RAND_MAX_DIV;
                HoldSet2 = true;
                HoldSet1 = false;
            }
        }
        return HoldFloat;
    }
    return 0;
}
