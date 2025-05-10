#ifndef CLFO_H
#define CLFO_H

#include "idevice.h"
#include "cwavebank.h"

class CLFO : public IDevice
{
private:
    enum JackNames
    {jnOutPitch,jnOutAmplitude};
    enum ParameterNames
    {pnFrequency,pnWaveForm,pnRectify,pnLevel,pnBias};
    float FreqValue;
    float VolumeFactor;
    float Rectify;
    float RectifyFactor;
    float Bias;
    float ReturnValue;
    CWaveBank WaveBank;
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    float inline Rect(float v);
public:
    CLFO();
    void init(const int Index, QWidget* MainWindow);
    void tick();
    float getNext(const int ProcIndex);
};

#endif // CLFO_H
