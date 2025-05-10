#ifndef CTONEGENERATOR_H
#define CTONEGENERATOR_H

#include "idevice.h"
#include "cwavebank.h"
#include "cvoltagemodulator.h"

class CToneGenerator : public IDevice
{
private:
    enum JackNames
    {jnOut,jnFrequency,jnModulation,jnPulseModulation};
    enum ParameterNames
    {pnFrequency,pnGlide,pnModulation,pnTuning,pnDetune,pnWaveForm,pnPulse,pnPulseModulation,pnRectify,pnVolume};
    //float voltageValue;
    double WavePosition;
    double DetunePosition;
    //float lastVoltageIn;
    float VolumeFactor;
    float PulseFactor;
    float Rectify;
    float RectifyFactor;
    CWaveBank WaveBank;
    CVoltageModulator Modulator;
    //CFreqGlider FreqGlider;
    double inline PulseCalc(double Pos,float Modulation);
    float inline Rect(float v);
    void inline updateDeviceParameter(const CParameter* p = nullptr) override;
public:
    CToneGenerator();
    void init(const int index, QWidget* MainWindow) override;
    CAudioBuffer* getNextA(const int ProcIndex) override;
    void play(const bool FromStart) override;
};

#endif // CTONEGENERATOR_H
