#ifndef CENVELOPE_H
#define CENVELOPE_H

#include "idevice.h"
#include "cadsr.h"

#define ENVELOPEFORM FORMFUNC(CEnvelopeForm)

class CEnvelope : public IDevice
{
public:
    enum ParameterNames
    {pnDelayTime,pnAttackTime,pnHoldTime,pnDecayTime,pnSustainLevel,pnReleaseTime,pnVolume,pnMode};
    CEnvelope();
    void init(const int Index, QWidget* MainWindow);
    float getNext(const int ProcIndex);
private:
    enum JackNames
    {jnTriggerIn,jnOut};
    CADSR ADSR;
    float VolumeFactor;
    void inline updateDeviceParameter(const CParameter* p = nullptr);
};

#endif // CENVELOPE_H
