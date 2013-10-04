#include "cnoisegate.h"


CNoiseGate::CNoiseGate()
{
}

void CNoiseGate::Init(const int Index, void *MainWindow) {
    m_Name=devicename;
    IDevice::Init(Index,MainWindow);
    AddJackWaveIn();
    AddJackWaveOut(jnOut);
    AddJack("Envelope Out",IJack::Amplitude,IJack::Out,jnEnvOut);
    AddParameterPercent("Threshold");
    AddParameterPercent("Response Time");
    AddParameterPercent("Decay Time");
    CurrentVol=0;
    TargetVol=0;
    LastGlideVol=0;
    CalcParams();
}

float *CNoiseGate::GetNextA(const int ProcIndex) {
    if (m_Process)
    {
        m_Process=false;
        Process();
    }
    if (CurrentVol==0) return NULL;
    return AudioBuffers[ProcIndex]->Buffer;
}

float CNoiseGate::GetNext(int) {
    if (m_Process)
    {
        m_Process=false;
        Process();
    }
    return CurrentVol*0.01;
}

void CNoiseGate::Process() {
    float GFactor;
    float* InSignal=FetchA(jnIn);
    if (!InSignal)
    {
        CurrentVol=0;
        return;
    }
    float* Buffer=AudioBuffers[jnOut]->Buffer;
    for (int i=0;i<m_BufferSize;i++)
    {
        int Signal=*(InSignal+i);
        if (fabs(Signal)>Threshold)
        {
            TargetVol=100;
        }
        else
        {
            TargetVol=0;
        }

        if (LastGlideVol>TargetVol)
        {
            GFactor=DecayFactor;
        }
        else
        {
            GFactor=-GlideFactor;
        }
        if (abs(LastGlideVol - TargetVol)>abs(GFactor))
        {
            CurrentVol=LastGlideVol-GFactor;
        }
        else
        {
            CurrentVol=TargetVol;
        }
        LastGlideVol=CurrentVol;

        Buffer[i]=Signal;
    }
    AudioBuffers[jnOut]->Multiply(CurrentVol*0.01);
}

void CNoiseGate::CalcParams() {
    Threshold=m_ParameterValues[pnThreshold]*0.01;
    GlideFactor=((101-m_ParameterValues[pnResponse])*m_Presets.ResponseFactor)/(m_BufferSize/10.0);
    DecayFactor=((101-m_ParameterValues[pnDecay])*m_Presets.ResponseFactor)/(m_BufferSize*80.0);
}
