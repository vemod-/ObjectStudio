#ifndef CLIMITER_H
#define CLIMITER_H

#include "idevice.h"
#include "cringbuffer.h"

class CLimiter : public IDevice
{
private:
    enum JackNames
    {jnOut,jnIn};
    enum ParameterNames
    {pnLimitVol,pnOutVol};
    uint buflen;
    uint ready_num;
    uint pos;
    float out_vol;
    float limit_vol;
    CRingBuffer ring;
    void inline updateDeviceParameter(const CParameter* p = nullptr);
public:
    CLimiter();
    ~CLimiter() {}
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
};

#endif // CLIMITER_H
