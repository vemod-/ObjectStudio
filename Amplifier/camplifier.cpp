#include "camplifier.h"

CAmplifier::CAmplifier()
{
}

void CAmplifier::Init(const int Index, void *MainWindow)
{
    m_Name=devicename;
    IDevice::Init(Index,MainWindow);
    AddJackWaveIn();
    AddJackWaveOut(jnOut);
    AddJack("Modulation In",(IJack::AttachModes)(IJack::Amplitude | IJack::Pitch),IJack::In,0);
    AddParameterPercent("Modulation",100);
    LastMod=0;
    CurrentMod=0;
    CalcParams();
}

float* CAmplifier::GetNextA(const int ProcIndex)
{
    float Mod= Fetch(jnModulation);
    if (!Mod) return NULL;
    if (Mod != LastMod)
    {
        LastMod = Mod;
        CurrentMod = (float)Mod * ModFactor;
    }
    return AudioBuffers[ProcIndex]->WriteBuffer(FetchA(jnIn),CurrentMod);
}

void inline CAmplifier::CalcParams()
{
    ModFactor = (float)m_ParameterValues[pnModulation] * 0.01;
}
