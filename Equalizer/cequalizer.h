#ifndef CEQUALIZER_H
#define CEQUALIZER_H

#include "idevice.h"
#include "biquad.h"

/* Bandwidth of EQ filters in octaves */
#define BWIDTH        1.0f
#define EQUALIZERFORM FORMFUNC(CEqualizerForm)

class CEqualizer : public IDevice
{
private:
    enum JackNames
    {jnOut,jnIn};
    enum ParameterNames
    {};
    CBiquad filters[8];
    void inline updateDeviceParameter(const CParameter* p = nullptr);
public:
    CEqualizer();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
    float Level[8]={0};
    int Freq[8]={0};
    void SetLevel(const int index, const float Level);
    void SetFreq(const int index, const int Freq);
    void play(const bool FromStart);
};


#endif // CEQUALIZER_H
