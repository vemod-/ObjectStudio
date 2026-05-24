#ifndef CIIRFILTER_H
#define CIIRFILTER_H

#define devicejacks monoout,monoin,modulationin
#define devicecategory SynthModule | Effect

#include "idevice.h"
#include "../PitchTracker/ciirfilters.h"

#define IIRFilterClass CIIRFilters

class CIIRFilter : public IDevice
{

public:
    CIIRFilter();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
private:
    enum JackNames
    {devicejacks};
    enum ParameterNames
    {pnType,pnProtoType,pnLoFreq,pnHiFreq,pnOrder,pnModulation};
    float ModFactor;
    float CurrentMod;
    float LastMod;
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    IIRFilterClass iirFilter;
};

#endif // CIIRFILTER_H
