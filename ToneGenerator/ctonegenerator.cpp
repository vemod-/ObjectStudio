#include "ctonegenerator.h"

CToneGenerator::CToneGenerator():WavePosition(0),DetunePosition(0)
{
}

void CToneGenerator::init(const int Index, QWidget* MainWindow) {
    m_Name="ToneGenerator";
    IDevice::init(Index,MainWindow);
    addJackWaveOut(0);
    addJackModulationIn("Frequency");
    addJackModulationIn();
    addJackModulationIn("Pulse Modulation");
    addParameterFrequency();
    addParameterPercent("Glide");
    makeParameterGroup(3,"Tune",Qt::green);
    addParameterPercent();
    addParameterPan("Tuning");
    addParameterPan("Detune");
    addParameterSelect("WaveForm","Sine§Square§Triangle§Sawtooth§Ramp§Noise§S&H Noise");
    startParameterGroup();
    addParameterPan("PulseWave");
    addParameterPercent("PulseWave Modulation");
    endParameterGroup();
    addParameterRectify();
    addParameterVolume();
    Modulator.init(m_Jacks[jnModulation],m_Parameters[pnModulation],m_Parameters[pnTuning]);
    updateDeviceParameter();
}

CAudioBuffer *CToneGenerator::getNextA(const int ProcIndex) {
    const float currentVoltage=Modulator.exec(Fetch(jnFrequency));
    const float CurrentFrequency=voltage2Freqf(currentVoltage);

    const float PulseModIn = (m_Parameters[pnPulseModulation]->Value) ? Fetch(jnPulseModulation) : 0;

    CAudioBuffer* Buffer=m_AudioBuffers[ProcIndex];

    for (uint i=0;i<m_BufferSize;i++)
    {
        WavePosition += CurrentFrequency;
        while (WavePosition >= presets.SampleRate) WavePosition -= presets.SampleRate;
        while (WavePosition < 0) WavePosition += presets.SampleRate;
        if (m_Parameters[pnDetune]->Value==0)
        {
            const float v=WaveBank.getNext(PulseCalc(WavePosition,PulseModIn),CWaveBank::WaveForms(m_Parameters[pnWaveForm]->Value));
            Buffer->setAt(i,Rect(v)*VolumeFactor);
        }
        else
        {
            const float DetuneVoltage = currentVoltage + m_Parameters[pnDetune]->percentValue();
            DetunePosition = DetunePosition + voltage2Freqf(DetuneVoltage);
            while (DetunePosition >= presets.SampleRate) DetunePosition -= presets.SampleRate;
            while (DetunePosition < 0) DetunePosition += presets.SampleRate;
            const float v1=WaveBank.getNext(PulseCalc(WavePosition,PulseModIn),CWaveBank::WaveForms(m_Parameters[pnWaveForm]->Value));
            const float v2=WaveBank.getNext(PulseCalc(DetunePosition,PulseModIn),CWaveBank::WaveForms(m_Parameters[pnWaveForm]->Value));
            Buffer->setAt(i,(Rect(v1)+Rect(v2))*VolumeFactor*0.5);
        }
    }
    return Buffer;
}

float CToneGenerator::Rect(float v)
{
    if (Rectify < 0)
    {
        if (v > 0) return v * RectifyFactor;
    }
    else if (Rectify > 0)
    {
        if (v < 0) return v * RectifyFactor;
    }
    return v;
}

double CToneGenerator::PulseCalc(double Pos, float Modulation) {
    float Mod=PulseFactor;
    if (!isZero(Modulation))
    {
        Mod += Modulation * m_Parameters[pnPulseModulation]->PercentValue;
        Mod=qBound<float>(-0.99f, Mod, 0.99f);
    }
    if (isZero(Mod)) return Pos;
    const double T=(presets.HalfRate*Mod)+presets.HalfRate;
    if (Pos<T) return (presets.HalfRate*Pos)/T;
    return presets.HalfRate+((presets.HalfRate*(Pos-T))/(presets.SampleRate-T));
}

void CToneGenerator::updateDeviceParameter(const CParameter* /*p*/) {
    VolumeFactor=m_Parameters[pnVolume]->PercentValue;
    Modulator.setDefaultFreq(m_Parameters[pnFrequency]->PercentValue);
    Modulator.setGlide(m_Parameters[pnGlide]->Value);
    PulseFactor=m_Parameters[pnPulse]->scaleValue(0.0099f);
    Rectify=m_Parameters[pnRectify]->PercentValue;
    if (Rectify<0) RectifyFactor=(-Rectify-0.5f)*-2;
    else if (Rectify>0) RectifyFactor=(Rectify-0.5f)*-2;
    else RectifyFactor=1;
}

void CToneGenerator::play(const bool FromStart)
{
    Modulator.setDefaultFreq(m_Parameters[pnFrequency]->PercentValue);
    IDevice::play(FromStart);
}
