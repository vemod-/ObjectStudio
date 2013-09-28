#include "ckarlsenfilter.h"


CKarlsenFilter::CKarlsenFilter()
{
}

void CKarlsenFilter::Init(const int Index, void *MainWindow) {
    m_Name=devicename;
    Maxcutoff=m_Presets.SampleRate * 0.425;
    IDevice::Init(Index,MainWindow);
    AddJackWaveOut(jnOut);
    AddJackWaveIn();
    AddJack("Modulation",(IJack::AttachModes)(IJack::Amplitude | IJack::Pitch),IJack::In);
    AddParameterVolume("Gain");
    AddParameterPercent("Cutoff Modulation");
    AddParameter(ParameterType::Numeric,"Cutoff Frequency","Hz",20,Maxcutoff,0,"",Maxcutoff);
    AddParameterPercent("Response Time",50);
    AddParameterPercent("Resonance");
    AddParameterVolume();
    FreqGlider.SetSpeed(5);
    pole1=0;
    pole2=0;
    pole3=0;
    pole4=0;
    ModulationFactor=0;
    InVolumeFactor=0;
    OutVolumeFactor=0;
    LastResonance=0;
    LastCO=0;
    CalcExpResonance(0);
    CalcParams();
}

float *CKarlsenFilter::GetNextA(const int ProcIndex) {
    float* InSignal=FetchA(jnIn);
    if (!InSignal) return NULL;
    bool Recalc=false;
    float CutOff=m_ParameterValues[pnCutOffFrequency];
    if (ModulationFactor) CutOff*= pow(2,Fetch(jnModulation)*ModulationFactor);
    if (m_ParameterValues[pnResonance] != LastResonance)
    {
        LastResonance=m_ParameterValues[pnResonance];
        Recalc=true;
    }
    if (CutOff>Maxcutoff) CutOff=Maxcutoff;
    if (CutOff<20) CutOff=20;
    if (LastCO!=CutOff)
    {
        FreqGlider.SetTargetFreq(CutOff);
        LastCO=CutOff;
        Recalc=true;
    }
    float CurrentFreq=FreqGlider.GetCurrentFreq();
    if (LastCO != CurrentFreq) Recalc=true;
    if (Recalc)
    {
        CalcExpResonance(CurrentFreq);
        MixFactor=1.0-(((float)m_ParameterValues[pnResonance]*0.01)*0.6);
    }
    float volIn=InVolumeFactor*MixFactor;
    float volOut=OutVolumeFactor*MixFactor;
    float* Buffer=AudioBuffers[ProcIndex]->Buffer;
    for (int i=0;i<m_BufferSize;i++)
    {
        float input=*(InSignal+i) * volIn;
        float rez = pole4 * rezamount; if (rez > 1) {rez = 1;}
        input = input - rez;
        pole1 = pole1 + ((-pole1 + input) * cutoffreq);
        pole2 = pole2 + ((-pole2 + pole1) * cutoffreq);
        pole3 = pole3 + ((-pole3 + pole2) * cutoffreq);
        pole4 = pole4 + ((-pole4 + pole3) * cutoffreq);
        Buffer[i]=pole4 * volOut;
    }
    return Buffer;
}

void CKarlsenFilter::CalcParams() {
    ModulationFactor=(float)m_ParameterValues[pnCutOffModulation]* 0.01;
    InVolumeFactor=(float)m_ParameterValues[pnInVolume]*0.01;
    OutVolumeFactor=(float)m_ParameterValues[pnOutVolume]*0.01;
    FreqGlider.SetGlide(m_ParameterValues[pnResponse]);
}

void CKarlsenFilter::CalcExpResonance(float CutOff) {
    rezamount=(float)m_ParameterValues[pnResonance]/5;
    cutoffreq=CutOff/Maxcutoff;
}
