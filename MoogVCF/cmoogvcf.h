#ifndef CMOOGVCF_H
#define CMOOGVCF_H

#include "idevice.h"
#include "cvoltagemodulator.h"

class  CMoogVCF : public IDevice
{
public:
    CMoogVCF();
    void init(const int Index, QWidget* MainWindow) override;
    CAudioBuffer* getNextA(const int ProcIndex) override;
    void play(const bool FromStart) override;
private:
    enum JackNames
    {jnOut,jnIn,jnModulation};
    enum ParameterNames
    {pnInVolume,pnCutOffModulation,pnCutOffFrequency,pnResponse,pnResonance,pnOutVolume};
    int LastResonance;
    float InVolumeFactor;
    float OutVolumeFactor;
    void inline updateDeviceParameter(const CParameter* p = nullptr) override;
    void inline CalcExpResonance(float CutOff,float Resonance);
    //float fc;
    //float res;
    float f;
    float fb;
    float fa;
    float In1;
    float In2;
    float In3;
    float In4;
    float Out1;
    float Out2;
    float Out3;
    float Out4;
    //CFreqGlider FreqGlider;
    CVoltageModulator Modulator;
};

#endif // CMOOGVCF_H
