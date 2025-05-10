#include "clfo.h"


CLFO::CLFO():FreqValue(0)
{

}

void CLFO::init(const int Index, QWidget* MainWindow) {
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackModulationOut(jnOutPitch,"Out");
    addParameterRate("Frequency",400);
    addParameterSelect("WaveForm","Sine§Square§Triangle§Sawtooth§Ramp§Noise§S&H Noise");
    addParameterRectify();
    addParameterLevel();
    addParameterBias();
    updateDeviceParameter();
}

float CLFO::getNext(const int /*ProcIndex*/) {
    return Rect(ReturnValue)+Bias;
}

float CLFO::Rect(float v)
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

void CLFO::tick() {
    ReturnValue = WaveBank.getNextFreq(FreqValue,CWaveBank::WaveForms(m_Parameters[pnWaveForm]->Value))*VolumeFactor;
    IDevice::tick();
}

void CLFO::updateDeviceParameter(const CParameter* /*p*/) {
    Rectify=m_Parameters[pnRectify]->PercentValue;
    if (Rectify<0) RectifyFactor=(-Rectify-0.5f)*-2;
    else if (Rectify>0) RectifyFactor=(Rectify-0.5f)*-2;
    else RectifyFactor=1;
    VolumeFactor=m_Parameters[pnLevel]->PercentValue;
    Bias=m_Parameters[pnBias]->PercentValue;
    FreqValue=m_Parameters[pnFrequency]->PercentValue*m_BufferSize;
}
