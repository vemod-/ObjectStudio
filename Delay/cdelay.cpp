#include "cdelay.h"

void CDelay::init(const int Index, QWidget* MainWindow) {
    m_Name=devicename;
    ring.reserve(presets.DoubleRate);
    IDevice::init(Index,MainWindow);
    addJackWaveOut(jnOut);
    addJackWaveOut(jnEffectOut,"EffectOut");
    addJackWaveIn();
    startParameterGroup("LFO",Qt::green);
    addParameterRate("Modulation Frequency");
    addParameterPercent("Modulation Amplitude");
    endParameterGroup();
    startParameterGroup();
    addParameterTime("Delay Time",10);
    addParameterPercent("Delay Regeneration");
    endParameterGroup();
    addParameterPercent("Effect",100);
    updateDeviceParameter();
}

void CDelay::tick() {
    CurrentMod = (m_Parameters[pnAmplitude]->Value) ?
        WaveBank.getNextFreq(m_Parameters[pnFrequency]->PercentValue*m_BufferSize,CWaveBank::Triangle)*(float)m_Parameters[pnAmplitude]->Value : 0;
    IDevice::tick();
}

void CDelay::updateDeviceParameter(const CParameter* /*p*/) {
    CurrentMod=0;
    CleanMix=m_Parameters[pnMix]->DryValue;
    EffectMix=m_Parameters[pnMix]->scaleValue(0.012f);
    RegenCleanMix=(200-m_Parameters[pnRegen]->Value)* 0.002f;
    RegenEffectMix=m_Parameters[pnRegen]->scaleValue(0.008f);
    DelayRate=m_Parameters[pnDelay]->mSec2samplesValue();
}

CDelay::CDelay():ReadPosition(0),CleanMix(0),EffectMix(0), RegenCleanMix(0),RegenEffectMix(0),DelayRate(0),CurrentMod(0){}

void CDelay::process() {
    const CMonoBuffer* InBuffer = FetchAMono(jnIn);
    CMonoBuffer* OutBuffer=MonoBuffer(jnOut);
    CMonoBuffer* EffectBuffer=MonoBuffer(jnEffectOut);
    for (uint i=0;i<m_BufferSize;i++)
    {
        EffectBuffer->setAt(i,ring.read_buffer(ReadPosition));
        float fTemp=EffectBuffer->at(i)*RegenEffectMix;
        if (!InBuffer->isValid())
        {
            OutBuffer->setAt(i,fTemp*EffectMix);
        }
        else
        {
            fTemp+=(InBuffer->at(i)*RegenCleanMix);
            OutBuffer->setAt(i,(fTemp*EffectMix)+(InBuffer->at(i)*CleanMix));
        }
        ring.write_buffer(fTemp,ReadPosition++,int(DelayRate+CurrentMod));
    }
}
