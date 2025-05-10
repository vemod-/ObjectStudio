#ifndef SMBPITCHSHIFTER_H
#define SMBPITCHSHIFTER_H

#include "cfastcircularbuffer.h"
#include "cfft.h"
#include "cspectralwindow.h"
#include "QMutexLocker"

#define MAX_FRAME_LENGTH 4096
#define MAX_POLYPHONY 8

class smbPitchShifter
{
public:
    smbPitchShifter(double sampleRate, long fftFrameSize=2048, long osamp=8);
    float* process(long numSampsToProcess, float* indata);
    float* process(double f, long numSampsToProcess, float *indata)
    {
        QMutexLocker locker(&mutex);
        return process(f,1.f,numSampsToProcess,indata);
    }
    float* process(double f, float s, long numSampsToProcess, float *indata)
    {
        QMutexLocker locker(&mutex);
        setShiftFactor(f);
        setScale(s);
        return process(numSampsToProcess,indata);
    }
    float* process(double* f, long numSampsToProcess, float *indata)
    {
        QMutexLocker locker(&mutex);
        float s[MAX_POLYPHONY] = {1.f};
        return process(f,s,numSampsToProcess,indata);
    }
    float* process(double* f, float* s, long numSampsToProcess, float *indata)
    {
        QMutexLocker locker(&mutex);
        setShiftFactor(f,m_Polyphony);
        setScale(s,m_Polyphony);
        return process(numSampsToProcess,indata);
    }
    void process(long numSampsToProcess, float *indata, float *outdata)
    {
        float* b = process(numSampsToProcess, indata);
        if (!b) {
            memset(outdata, 0, numSampsToProcess*sizeof(float));
            return;
        }
        memcpy(outdata, b , numSampsToProcess*sizeof(float));
    }
    void process(double f, long numSampsToProcess, float *indata, float *outdata)
    {
        QMutexLocker locker(&mutex);
        process(f,1.f,numSampsToProcess,indata,outdata);
    }
    void process(double f, float s, long numSampsToProcess, float *indata, float *outdata)
    {
        QMutexLocker locker(&mutex);
        setShiftFactor(f);
        setScale(s);
        process(numSampsToProcess,indata,outdata);
    }
    void process(double* f, long numSampsToProcess, float *indata, float *outdata)
    {
        QMutexLocker locker(&mutex);
        float s[MAX_POLYPHONY] = {1.f};
        process(f,s,numSampsToProcess,indata,outdata);
    }
    void process(double* f, float* s, long numSampsToProcess, float *indata, float *outdata)
    {
        QMutexLocker locker(&mutex);
        setShiftFactor(f,m_Polyphony);
        setScale(s,m_Polyphony);
        process(numSampsToProcess,indata,outdata);
    }
    /*
        Routine smbPitchShift(). See top of file for explanation
        Purpose: doing pitch shifting while maintaining duration using the Short
        Time Fourier Transform.
        Author: (c)1999-2002 Stephan M. Bernsee <smb@dspdimension.com>
*/
    inline void setShiftFactor(double f)
    {
        setShiftFactor(&f,1);
    }
    inline void setScale(float s)
    {
        setScale(&s,1);
    }
    inline void setShiftFactor(double* f, uint poly)
    {
        for (uint i = 0; i < MAX_POLYPHONY; i++)
        {
            if (i < poly)
            {
                m_shiftFactor[i]=f[i];
            }
            else
            {
                m_shiftFactor[i]=0;
            }
        }
    }
    inline void setScale(float* s, uint poly)
    {
        for (uint i = 0; i < MAX_POLYPHONY; i++)
        {
            if (i < poly)
            {
                m_Scale[i]=s[i];
            }
            else
            {
                m_Scale[i]=0;
            }
        }
    }
    double* shiftFactor()
    {
        return m_shiftFactor;
    }
    uint polyphony()
    {
        return m_Polyphony;
    }
    void setPolyphony(uint v)
    {
        QMutexLocker locker(&mutex);
        v = qBound<uint>(1,v,8);
        if (m_Polyphony != v)
        {
            m_Polyphony=v;
            reset();
        }
    }
    void setFrameSize(long frameSize)
    {
        QMutexLocker locker(&mutex);
        if (frameSize > MAX_FRAME_LENGTH) frameSize = MAX_FRAME_LENGTH;
        if (m_FrameSize != frameSize)
        {
            m_FrameSize=frameSize;
            reset();
        }
    }
    void setOverSampling(long osamp)
    {
        QMutexLocker locker(&mutex);
        if (osamp != m_OSamp)
        {
            m_OSamp=osamp;
            reset();
        }
    }
private:
    uint m_Polyphony = 1;
    double m_PolyFactor = 1;
    void reset();
    CFastCircularBuffer InFIFO;
    CFastCircularBuffer OutFIFO;
    float m_OutputAccum[2*MAX_FRAME_LENGTH];
    CSpectralWindow m_win;
    CFFTtwiddleInterleaved<double> m_fft;
    double m_LastPhase[MAX_FRAME_LENGTH/2+1];
    double m_SumPhase[MAX_POLYPHONY][(MAX_FRAME_LENGTH/2+1)];
    double gAnaFreq[(MAX_FRAME_LENGTH/2+1)];
    double gAnaMagn[(MAX_FRAME_LENGTH/2+1)];
    double gSynFreq[(MAX_FRAME_LENGTH/2+1)];
    double gSynMagn[(MAX_FRAME_LENGTH/2+1)];
    double m_freqPerBinV[(MAX_FRAME_LENGTH/2)+1];
    double m_ExpPhaseDiffV[(MAX_FRAME_LENGTH/2)+1];
    /* set up some handy variables */
    const double PI2 = M_PI * 2;
    double OS_PI2;
    double PI2_OS;
    long m_HalfFrameSize;
    long m_StepSize;
    double m_FreqPerBin;
    double m_ExpectedPhaseDiff;
    long m_InFifoLatency;

    double m_SampleRate;
    double m_shiftFactor[MAX_POLYPHONY];
    float m_Scale[MAX_POLYPHONY];
    long m_FrameSize;
    long m_OSamp;
    QRecursiveMutex mutex;
};
















class smbPitchShifterOld
{
public:
    smbPitchShifterOld(double sampleRate,long fftFrameSize=2048, long osamp=8);
    void process(double f, long numSampsToProcess, const float *indata, float *outdata)
    {
        m_shiftFactor=f;
        process(numSampsToProcess,indata,outdata);
    }
    void process(long numSampsToProcess, const float *indata, float *outdata);
    /*
        Routine smbPitchShift(). See top of file for explanation
        Purpose: doing pitch shifting while maintaining duration using the Short
        Time Fourier Transform.
        Author: (c)1999-2002 Stephan M. Bernsee <smb@dspdimension.com>
*/
    void setShiftFactor(double f)
    {
        m_shiftFactor=f;
    }
    double shiftFactor()
    {
        return m_shiftFactor;
    }
    void setFrameSize(long frameSize)
    {
        m_FrameSize=frameSize;
        reset();
    }
    void setOverSampling(long osamp)
    {
        m_OSamp=osamp;
        reset();
    }
private:
    void smbFft(double *fftBuffer, long fftFrameSize, long sign);
    void reset();
    long fftFrameSize2;
    long stepSize;
    double freqPerBin;
    double expct;
    long inFifoLatency;
    double smbAtan2(double x, double y);
    float gInFIFO[MAX_FRAME_LENGTH];
    float gOutFIFO[MAX_FRAME_LENGTH];
    double gFFTworksp[2*MAX_FRAME_LENGTH];
    double gLastPhase[MAX_FRAME_LENGTH/2+1];
    double gSumPhase[MAX_FRAME_LENGTH/2+1];
    double gOutputAccum[2*MAX_FRAME_LENGTH];
    double gAnaFreq[MAX_FRAME_LENGTH];
    double gAnaMagn[MAX_FRAME_LENGTH];
    double gSynFreq[MAX_FRAME_LENGTH];
    double gSynMagn[MAX_FRAME_LENGTH];
    double m_WindowFactor[MAX_FRAME_LENGTH];
    long gRover;

    double m_SampleRate;
    double m_shiftFactor;
    long m_FrameSize;
    long m_OSamp;
};

#endif // SMBPITCHSHIFTER_H
