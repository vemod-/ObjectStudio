#ifndef CKARLSENFILTER_H
#define CKARLSENFILTER_H

#define devicejacks monoout,monoin,modulationin
#define devicecategory Effect

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
    {devicejacks};
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
