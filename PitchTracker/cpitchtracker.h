#ifndef CPITCHTRACKER_H
#define CPITCHTRACKER_H

#include "idevice.h"
//#include "cpitchtrackerclass.h"
#include "cpitchdetect.h"
#include "cffttracker.h"
#include "bcf2.h"

#define BufferCount 2

class CPitchTracker : public IDevice
{
private:
    enum JackNames
    {jnIn,jnFrequencyOut,jnMIDIFreqOut,jnMIDIOut,jnDiffOut};
    enum ParameterNames
    {pnThreshold,pnTune,pnMaxFreq,pnRate,pnOverlap};
    int LastNote;
    double tuneFactor;
    CMIDIBuffer MIDIBuffer;
    CPitchDetect PD;
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
