#include "cexciter.h"

CExciter::CExciter() : InVolFactor(0),OutVolFactor(0),EffFactor(0),k2(0)
{
}

void CExciter::init(const int Index, QWidget* MainWindow)
{
    m_Name="Exciter";
    IDevice::init(Index,MainWindow);
    addJackWaveOut(jnOut);
    addJackWaveOut(jnEffOut,"Effect Out");
    addJackWaveIn();
    addParameterSelect("Type","Soft§Clipping");
    addParameterVolume("Gain");
    addParameterPercent("Amount",100);
    addParameterCutOff("Cutoff Frequency",presets.HalfRate*0.425);
    addParameterPercent("Effect",50);
    updateDeviceParameter();
}

void CExciter::process()
{
    const CMonoBuffer* InBuffer = FetchAMono(jnIn);
    CMonoBuffer* OutBuffer=MonoBuffer(jnOut);
    CMonoBuffer* EffectBuffer=MonoBuffer(jnEffOut);
    if (!InBuffer->isValid())
    {
        OutBuffer->zeroBuffer();
        EffectBuffer->zeroBuffer();
        return;
    }
    if (m_Parameters[pnType]->Value==0)
    {
        for (uint i=0;i<m_BufferSize;i++)
        {
            float x=RBJFilter.filter(InBuffer->at(i)*InVolFactor);
            x=soft(x,m_Parameters[pnAmount]->Value)*EffFactor;
            EffectBuffer->setAt(i,x);
            OutBuffer->setAt(i,x+(InBuffer->at(i)*OutVolFactor));
        }
    }
    else
    {
        for (uint i=0;i<m_BufferSize;i++)
        {
            float x=RBJFilter.filter(InBuffer->at(i)*InVolFactor);
            x=clip(x*k2,1,-1)*EffFactor;
            EffectBuffer->setAt(i,x);
            OutBuffer->setAt(i,x+(InBuffer->at(i)*OutVolFactor));
        }
    }
}

void CExciter::updateDeviceParameter(const CParameter* /*p*/)
{
    const float Amount=m_Parameters[pnAmount]->scaleValue(0.999999f);
    k2=0.9999f+Amount;
    InVolFactor=m_Parameters[pnInVolume]->PercentValue;
    EffFactor=m_Parameters[pnOutVolume]->scaleValue(0.002f);
    OutVolFactor=1.0f-powf((EffFactor*0.01f)/0.002f,2);
    RBJFilter.calc_filter_coeffs(1,m_Parameters[pnCutOffFrequency]->Value,presets.SampleRate,1,1,false);
}

float inline CExciter::clip(float x, float a, float b)
{
    const float x1 = fabsf(x-a);
    const float x2 = fabsf(x-b);
    x = x1 + (a+b);
    x -= x2;
    x *= 0.5;
    return x;
}

float inline CExciter::soft(float x,int amount)
{
    return x*(fabsf(x) + amount)/(powf(x,2) + (amount-1)*fabsf(x) + 1);
}
