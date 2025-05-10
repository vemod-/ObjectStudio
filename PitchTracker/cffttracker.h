#ifndef CFFTTRACKER_H
#define CFFTTRACKER_H

#include "cspectralwindow.h"
#include "cfft.h"
#include "cfastcircularbuffer.h"

class CFFTTracker
{
public:
    CFFTTracker(long sampleRate, ulong fftFrameSize=8192);
    void process(float* inbuffer, uint samplecount);
    int midiPitch;
    int midiCents;
private:
    CSpectralWindow m_win;
    CFFTtwiddleInterleaved<double> m_fft;
    CFastCircularBuffer InFIFO;
    long m_SampleRate;
    ulong m_FrameSize;
    float a[2], b[3], mem1[4], mem2[4];
    std::vector<float> filterBuffer;
    std::vector<double> peaks;
    double prevIndex;
};

#endif // CFFTTRACKER_H
