#ifndef CFILTER_H
#define CFILTER_H

#include "idevice.h"
#include "cvoltagemodulator.h"

class CFilter : public IDevice
{
private:
    enum JackNames
    {jnOut,jnIn,jnModulation};
    enum ParameterNames
    {pnInVolume,pnCutOffModulation,pnCutOffFrequency,pnResponse,pnResonance,pnOutVolume};
    float FiltCoefTab0;
    float FiltCoefTab1;
    float FiltCoefTab2;
    float FiltCoefTab3;
    float FiltCoefTab4;
    float ly1;
    float ly2;
    float lx1;
    float lx2;
    float m_ExpResonance;
    int LastResonance;
    float MixFactor;
    float InVolumeFactor;
    CVoltageModulator Modulator;
    void inline updateDeviceParameter(const CParameter* p = nullptr) override;
    void inline CalcExpResonance();
public:
    CFilter();
    void init(const int Index, QWidget* MainWindow) override;
    CAudioBuffer* getNextA(const int ProcIndex) override;
    void play(const bool FromStart) override;
};

#endif // CFILTER_H
