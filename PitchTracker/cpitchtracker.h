#ifndef CPITCHTRACKER_H
#define CPITCHTRACKER_H

#include "idevice.h"
//#include "cpitchtrackerclass.h"
#include "YinPitchDetector.h"

#define BufferCount 2

#define devicejacks monoin,frequencyout,midifrequencyout,midiout,diffmodulationout,corrmodulationout
#define devicecategory MIDIGenerator | SynthModule

class CPitchTracker : public IDevice
{
private:
    enum JackNames
    {devicejacks};
    enum ParameterNames
    {pnThreshold,pnTune,pnMaxFreq,pnRate,pnGlide,pnSlack};
    int LastNote;
    double tuneFactor;
    CMIDIBuffer MIDIBuffer;
    CYIN PD;
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    //CFFTTracker m_FFTTracker;
    //CBinaryAutoCorrelation m_BAC;
public:
    CPitchTracker();
    void init(const int Index, QWidget* MainWindow);
    float getNext(const int ProcIndex);
    void process();
    CMIDIBuffer* getNextP(int /*ProcIndex*/);
};

#endif // CPITCHTRACKER_H
