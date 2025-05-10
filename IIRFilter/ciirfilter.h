#ifndef CIIRFILTER_H
#define CIIRFILTER_H

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
    {jnOut,jnIn,jnModulation};
    enum ParameterNames
    {pnType,pnProtoType,pnLoFreq,pnHiFreq,pnOrder,pnModulation};
    float ModFactor;
    float CurrentMod;
    float LastMod;
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    IIRFilterClass iirFilter;
};

#endif // CIIRFILTER_H
