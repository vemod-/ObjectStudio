#ifndef CTUNER_H
#define CTUNER_H

#include "idevice.h"

#define devicejacks monoout,monoin
#define devicecategory Monitor

class CTuner : public IDevice
{
public:
    CTuner();
    ~CTuner();
    void init(const int Index, QWidget* MainWindow);
    void tick();
    CAudioBuffer* getNextA(const int ProcIndex);
private:
    enum JackNames
    {devicejacks};
    enum ParameterNames
    {pnTune,pnSilent,pnMaxFreq,pnRate};
    void inline updateDeviceParameter(const CParameter* p = nullptr);
};

#endif // CTUNER_H
