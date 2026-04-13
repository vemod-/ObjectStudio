#ifndef SMBPITCHSHIFTER_H
#define SMBPITCHSHIFTER_H

#include "cfft.h"
#include "cspectralwindow.h"
//#include "QMutexLocker"

#define MAX_FRAME_LENGTH 4096
#define MAX_POLYPHONY 8

struct smbVoice {
    double sumPhase[(MAX_FRAME_LENGTH/2)+1];
    int index[(MAX_FRAME_LENGTH/2)+1];
    int maxBin;
    double shiftFactor;
    float velocity;
    double newFactor;
    float newVelocity;
};

class IOBuffer {
public:
    IOBuffer() {
        reset();
    }
    void reset() {
        memset(data, 0, IOBuffer::IOBufferSize * sizeof(float));
        pos = 0;
    }
    float* current() {
        return data + pos;
    }
    void inc(int s) {
        pos += s;
        if (pos >= IOBufferSize) pos = 0;
    }
    bool wrap(int s) {
        if (pos + s > IOBufferSize) {
            wrapSizeA = std::min(IOBuffer::IOBufferSize - pos, s);
            wrapSizeB = s - wrapSizeA;
            return true;
        }
        return false;
    }
    void write(const float* p, int s) {
        if (p) {
            memcpy(data + pos, p, s * sizeof(float));
        }
        else {
            memset(data + pos, 0, s * sizeof(float));
        }
    }
    void read(float* p, int s) {
        if (p) memcpy(p, data + pos, s * sizeof(float));
    }
    void add(float* p, int s, float f) {
        if (p) {
            for (int i = 0; i < s; i++) p[i] += data[pos + i] * f;
        }
    }
    void zero(int s) {
        memset(data + pos, 0, s * sizeof(float));
    }
    float* wrapToTemp() {
        memcpy(tempFrame, data + pos, wrapSizeA * sizeof(float));
        memcpy(tempFrame + wrapSizeA, data, wrapSizeB * sizeof(float));
        return tempFrame;
    }
    float* readWrap(int s) {
        int readPos = pos - s;
        if (readPos < 0) readPos += IOBufferSize;

        float* temp;
        if (readPos + s <= IOBufferSize) {
            temp = data + readPos;
        } else {
            int sizeA = IOBufferSize - readPos;
            int sizeB = s - sizeA;
            memcpy(tempFrame, data + readPos, sizeA * sizeof(float));
            memcpy(tempFrame + sizeA, data, sizeB * sizeof(float));
            temp = tempFrame;
        }
        return temp;
    }
    void wrapAddFromTemp() {
        for (int j = 0; j < wrapSizeA; j++) data[pos + j] += tempFrame[j];
        for (int j = 0; j < wrapSizeB; j++) data[j] += tempFrame[wrapSizeA + j];
    }
    int pos = 0;
    static const int IOBufferSize = MAX_FRAME_LENGTH;
    float data[IOBufferSize];
    static inline float tempFrame[MAX_FRAME_LENGTH];
private:
    int wrapSizeA = 0;
    int wrapSizeB = 0;
};

class smbPitchShifter
{
public:
    smbPitchShifter(double sampleRate, int stepSize, int polyphony = 1);
    void process(const double f, const float *indata, float *outdata, float mix = 0) {
        setShiftFactor(f);
        process(indata,outdata,mix);
    }
    void process(const double* f, float* s, const float *indata, float *outdata) {
        setShiftFactor(f,m_Polyphony);
        setScale(s,m_Polyphony);
        process(indata,outdata);
    }
    inline void setShiftFactor(const double f, int i = 0) {
        m_Voices[i].newFactor = f;
    }
    inline void setScale(const float s) {
        m_Voices[0].newVelocity = s;
    }
    inline void setShiftFactor(const double* f, int poly) {
        for (int i = 0; i < MAX_POLYPHONY; i++)
        {
            smbVoice* v = &m_Voices[i];
            if (i < poly) {
                v->newFactor = f[i];
            }
            else {
                v->newFactor = 0;
            }
        }
    }
    inline void setScale(const float* s, int poly) {
        for (int i = 0; i < MAX_POLYPHONY; i++)
        {
            smbVoice* v = &m_Voices[i];
            if (i < poly) {
                v->newVelocity = s[i];
            }
            else {
                v->newVelocity = 0;
            }
        }
    }
    void setOverSampling(int osamp) {
        osamp = std::min(MAX_FRAME_LENGTH / m_StepSize, osamp);
        m_NewOSamp = osamp;
    }
private:
    void reset();
    void process(const float* indata, float* outdata, float f = 0);
    void calcShiftVars(smbVoice* v) {
        const double invShift = 1.0 / v->shiftFactor;
        for (int k = 0; k <= m_HalfFrameSize; k++) {
            v->index[k] = std::min((int)(k * invShift + 0.5), m_HalfFrameSize);
        }
        v->maxBin = std::min<int>(floor(m_HalfFrameSize * v->shiftFactor),m_HalfFrameSize);
    }
    CSpectralWindow m_win;
    CFFTtwiddleInterleaved<double> m_fft;
    IOBuffer m_InBuffer;
    IOBuffer m_OutBuffer;
    double m_LastPhase[(MAX_FRAME_LENGTH/2)+1];
    double gAnaFreq[(MAX_FRAME_LENGTH/2)+1];
    double gAnaMagn[(MAX_FRAME_LENGTH/2)+1];
    double m_freqPerBinV[(MAX_FRAME_LENGTH/2)+1];
    double m_ExpPhaseDiffV[(MAX_FRAME_LENGTH/2)+1];
    /* set up some handy variables */
    const double PI2 = M_PI * 2;
    double OS_PI2;
    double PI2_OS;
    const double m_SampleRate;
    int m_HalfFrameSize;
    const int m_StepSize;
    const int m_Polyphony = 1;
    double m_Gain = 1;
    double m_FreqPerBin;
    double m_ExpectedPhaseDiff;
    smbVoice m_Voices[MAX_POLYPHONY];
    int m_FrameSize;
    int m_OSamp = 8;
    int m_NewOSamp = 8;
    //QRecursiveMutex mutex;
};

#endif // SMBPITCHSHIFTER_H
