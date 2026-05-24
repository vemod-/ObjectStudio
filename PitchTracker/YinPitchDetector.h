#ifndef YINPITCHDETECTOR_H
#define YINPITCHDETECTOR_H

#include "cfreqglider.h"
#include "vector"
#include "../PitchShifter/smbpitchshifter.h"
#include "../SoftSynthsClasses/softsynthsdefines.h"
#include <QDebug>

class YinPitchDetector
{
public:
    YinPitchDetector(int sampleRate, int bufferSize)
        : m_sampleRate(sampleRate),
        m_bufferSize(bufferSize),
        m_halfSize(bufferSize / 2),
        tauMax(std::min(m_halfSize - 1, sampleRate / 2))
    {
        //m_diff.resize(m_halfSize);
        m_cmnf.resize(m_halfSize);
        tauMin = std::max(2, sampleRate / (sampleRate / 2));
    }

    float process(const float* x)
    {
        difference(x);
        //cumulativeMeanNormalizedDifference();
        int tau = absoluteThreshold();

        if (tau == -1) return 0.0f;

        float betterTau = parabolicInterpolation(tau);

        return m_sampleRate / betterTau;
    }
    void setThreshold(float t) {
        m_threshold = t;
    }
    int bufferSize() {
        return m_bufferSize;
    }
    void setMaxFreq(int f) {
        tauMin = std::max(2, m_sampleRate / f);
    }
private:
    /*
    void difference(const float* x)
    {
        const float* x1 = x;
        for (int tau = 0; tau < tauMin; tau++) m_diff[tau] = 0;
        for (int tau = tauMin; tau < tauMax; tau++) {
            float sum = 0.0f;
            const float* x2 = x + tau;
            for (int i = 0; i < m_halfSize; i++) {
                const float d = x1[i] - x2[i];
                sum += d * d;
            }
            m_diff[tau] = sum;
        }
    }
    void cumulativeMeanNormalizedDifference()
    {
        float runningSum = 0.0f;
        for (int tau = 0; tau < tauMin; tau++) m_cmnf[tau] = 1.0f; // ignorera
        for (int tau = tauMin; tau < tauMax; tau++) {
            runningSum += m_diff[tau];
            m_cmnf[tau] = m_diff[tau] * tau / runningSum;
        }
    }
*/
    void difference(const float* x)
    {
        const float* x1 = x;
        float runningSum = 0.0f;
        for (int tau = 0; tau < tauMin; tau++) {
            //m_diff[tau] = 0;
            m_cmnf[tau] = 1.0f;
        }
        for (int tau = tauMin; tau < tauMax; tau++) {
            float sum = 0.0f;
            const float* x2 = x + tau;
            for (int i = 0; i < m_halfSize; i++) {
                const float d = x1[i] - x2[i];
                sum += d * d;
            }
            runningSum += sum;
            m_cmnf[tau] = sum * tau / runningSum;
        }
    }
    int absoluteThreshold()
    {
        for (int tau = tauMin; tau < tauMax; tau++) {
            if (m_cmnf[tau] < m_threshold) {
                while (tau + 1 < m_halfSize && m_cmnf[tau + 1] < m_cmnf[tau]) tau++;
                return tau;
            }
        }
        return -1;
    }
    float parabolicInterpolation(int tau)
    {
        if (tau <= 0 || tau >= m_halfSize - 1) return (float)tau;
        tau = std::clamp(tau, tauMin + 1, tauMax - 2);
        const float s0 = m_cmnf[tau - 1];
        const float s1 = m_cmnf[tau];
        const float s2 = m_cmnf[tau + 1];

        const float denom = (2.0f * s1 - s2 - s0);
        if (denom == 0.0f) return (float)tau;

        return tau + (s2 - s0) / (2.0f * denom);
    }
    const int m_sampleRate;
    const int m_bufferSize;
    const int m_halfSize;

    //std::vector<float> m_diff;
    std::vector<float> m_cmnf;

    float m_threshold = 0.1f; // viktig parameter!

    std::atomic<int> tauMin;
    int tauMax;
};

class CYIN {
public:
    struct PitchRecord {
        float Pitch;
        int MidiKey;
        float MidiCents;
    };
    CYIN(int sampleRate) : m_SampleRate(sampleRate),
        m_LoYin(sampleRate, MAX_FRAME_LENGTH / 4),
        m_HiYin(sampleRate, MAX_FRAME_LENGTH / 4),
        maxPitch(sampleRate / 6),
        breakPitch(20 + (sampleRate / (m_HiYin.bufferSize() / 2))),
        minPitchLo(sampleRate / (m_LoYin.bufferSize() * 4 / 2))
    {
        memset(&m_CurrentPitchRecord,0,sizeof(PitchRecord));
        glider.setSpeed(10);
    }
    bool ProcessBuffer(float* buffer, int size) {
        m_HiBuffer.write(buffer, size);
        m_HiBuffer.inc(size);
        for (int i = 0; i < size; i += loFactor) {
            //float x = buffer[i];
            //for (int j = 1; j < loFactor; j++) x += buffer[i + j];
            //x /= loFactor;
            m_LoBuffer.data[m_LoBuffer.pos++] = buffer[i];
        }
        if (m_LoBuffer.pos >= IOBuffer::IOBufferSize) m_LoBuffer.pos = 0;

        m_HopCounter += size;

        if (m_HopCounter >= m_HopRate)
        {
            m_HopCounter = 0;
            float hiPitch = m_HiYin.process(m_HiBuffer.readWrap(m_HiYin.bufferSize()));
            float newPitch = (hiPitch < breakPitch) ? m_LoYin.process(m_LoBuffer.readWrap(m_LoYin.bufferSize())) / loFactor : hiPitch;
            if (newPitch < minPitchLo || newPitch > maxPitch) newPitch = 0;// m_CurrentPitchRecord.Pitch; // ignorera
            m_CurrentPitchRecord.Pitch = newPitch;
            if (m_CurrentPitchRecord.Pitch > 0) {
                float m_CurrentCent = freq2Centf(m_CurrentPitchRecord.Pitch, m_Tune);
                int key = int(m_CurrentCent * 0.01f + 0.5f);
                if (key >= 0 && key <= 127) {
                    m_CurrentPitchRecord.MidiKey = key;
                    m_CurrentPitchRecord.MidiCents = m_CurrentCent - key * 100.0f;
                }
                else {
                    m_CurrentPitchRecord.MidiKey = 0;
                    m_CurrentPitchRecord.MidiCents = 0;
                }
            }
            else {
                m_CurrentPitchRecord.MidiKey = 0;
                m_CurrentPitchRecord.MidiCents = 0;
            }
            return true;
        }
        return false;
    }
    const PitchRecord& CurrentPitchRecord() {
        return m_CurrentPitchRecord;
    }
    void setTune(float pitch) {
        m_Tune = pitch;
    }
    float tune() {
        return m_Tune;
    }
    void setDetectLevelThreshold(float t) {
        m_LoYin.setThreshold(t);
        m_HiYin.setThreshold(t);
    }
    void setPitchRecordsPerSecond(int r) {
        m_HopRate = m_SampleRate / r;
    }
    void setMaxDetectFrequency(int f) {
        m_HiYin.setMaxFreq(f);
        m_LoYin.setMaxFreq(f);
    }
    void setDetectSlack(int c) {
        m_Slack = c;
    }
    double correctionCents()
    {
        double c = -m_CurrentPitchRecord.MidiCents;
        if (fabs(c) < m_Slack) c = 0.0;
        //if (fabs(c) < 10.0f) m_glideFactor *= 2.0f;
        c = std::clamp<double>(c, -100, 100);
        return glider.runCent(c);
    }
    double correctionFactor() {
        return cent2Factor(correctionCents());
    }
    void setGlide(int glide) {
        glider.setGlide(glide);
    }
private:
    const int m_SampleRate;
    IOBuffer m_LoBuffer;
    YinPitchDetector m_LoYin;
    IOBuffer m_HiBuffer;
    YinPitchDetector m_HiYin;
    const int maxPitch;
    const int breakPitch;
    const int minPitchLo;
    const int loFactor = 4;
    PitchRecord m_CurrentPitchRecord;
    int m_HopRate = 512;
    int m_HopCounter = 0;
    float m_Tune = 440;
    CFreqGlider glider;
    int m_Slack = 0;
};

#endif // YINPITCHDETECTOR_H
