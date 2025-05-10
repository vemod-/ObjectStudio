#ifndef CKARLSENFILTER_H
#define CKARLSENFILTER_H

#include "idevice.h"
#include "cvoltagemodulator.h"

class CKarlsenFilter : public IDevice
{
public:
    CKarlsenFilter();
    void init(const int Index, QWidget* MainWindow) override;
    CAudioBuffer* getNextA(const int ProcIndex) override;
    void play(const bool FromStart) override;
private:
    enum JackNames
    {jnOut,jnIn,jnModulation};
    enum ParameterNames
    {pnInVolume,pnCutOffModulation,pnCutOffFrequency,pnResponse,pnResonance,pnOutVolume};

    int LastResonance;
    float rezamount;
    float cutoffreq;
    float pole1;
    float pole2;
    float pole3;
    float pole4;
    float InVolumeFactor;
    float OutVolumeFactor;
    float MixFactor;
    CVoltageModulator Modulator;
    void inline updateDeviceParameter(const CParameter* p = nullptr) override;
    void inline CalcExpResonance(float CutOff);
};

#endif // CKARLSENFILTER_H
