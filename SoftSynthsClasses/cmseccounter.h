#ifndef CMSECCOUNTER_H
#define CMSECCOUNTER_H

#include "cpresets.h"
#include "softsynthsdefines.h"

class CSampleCounter : protected IPresetRef
{
public:
    inline CSampleCounter() { reset(); }
    inline void reset() {
        m_SampleCount = 0;
    }
    inline ulong64 currentSample() const {
        return m_SampleCount;
    }
    inline void skipBuffer() {
        skip(presets.ModulationRate);
    }
    inline void skip(const ulong64 samples) {
        m_SampleCount += samples;
    }
    inline ulong currentmSec() const {
        return m_SampleCount / presets.SamplesPermSec;
    }
    inline ulong currentBuffer() const {
        return m_SampleCount / presets.ModulationRate;
    }
private:
    ulong64 m_SampleCount = 0;

};

class CTickCounter : protected IPresetRef
{
public:
    inline CTickCounter() { reset(); }
    inline void reset(const int ticksPQ=240)
    {
        m_CurrentTick=0;
        m_CurrentmSec=0;
        m_mSecSampleCount=0;
        m_TickSampleCount=0;
        m_TempoAdjust=1;
        setTempo(500000,ticksPQ);
    }
    inline void addMilliSecond() { addSamples(presets.SamplesPermSec); }
    inline void addBuffer() { addSamples(presets.ModulationRate); }
    inline void eatTick()
    {
        eatmSec(m_SamplesPerTick);
        m_CurrentTick++;
        m_TickSampleCount -= m_SamplesPerTick;
    }
    inline void skipTicks(const ulong ticks)
    {
        eatmSec(ticks * m_SamplesPerTick);
        m_CurrentTick+=ticks;
    }
    inline void skipBuffer() {
        skipSamples(presets.ModulationRate);
    }
    inline bool skipSlack(ulong& ticks, ulong64 totalSamples) {
        const ldouble mSecs = remainingmSecs(ticks);
        const ldouble totalSkipmSecs = totalSamples / presets.SamplesPermSec;
        const bool Exsess = (mSecs > totalSkipmSecs);
        if (Exsess) ticks -= qMin<ulong>(ticks,ticksFrommSecs(mSecs - totalSkipmSecs));
        skipTicks(ticks);
        return Exsess;
    }
    inline ldouble remainingmSecs(ulong ticks) {
        return mSecsFromTicks(ticks) + m_CurrentmSec;
    }
    inline ulong currentmSec() const { return m_CurrentmSec; }
    inline ulong currentTick() const { return m_CurrentTick; }
    inline ulong64 currentSample() const { return m_CurrentmSec * presets.SamplesPermSec; }
    inline void setTempoAdjust(const double tempoAdjust)
    {
        if (closeEnough(tempoAdjust, m_TempoAdjust)) return;
        m_TempoAdjust = tempoAdjust;
        setTempo(m_uSPQ,m_TicksPQ);
    }
    double tempoAdjust() const { return m_TempoAdjust; }
    inline void setTempo(const double uSPQ, const double ticksPQ=240)
    {
        m_uSPQ=uSPQ;
        m_TicksPQ=ticksPQ;
        double uSPerTick=(uSPQ / m_TempoAdjust)/ticksPQ;
        m_SamplesPerTick=uSPerTick / presets.uSPerSample;
        m_mSecsPerTick=uSPerTick * 0.001;
    }
    inline bool moreTicks() const { return (m_TickSampleCount >= m_SamplesPerTick); }
private:
    inline ldouble mSecsFromTicks(const ldouble ticks) const { return m_mSecsPerTick * ticks; }
    inline ldouble ticksFrommSecs(const ldouble mSecs) const { return mSecs / m_mSecsPerTick; }
    inline void addSamples(const ldouble samples) {
        m_TickSampleCount += samples;
    }
    inline void skipSamples(const ulong64 samples) {
        addSamples(samples);
        while (moreTicks()) eatTick();
    }
    void inline eatmSec(const ldouble samples)
    {
        m_mSecSampleCount += samples;
        while (m_mSecSampleCount >= presets.SamplesPermSec)
        {
            m_mSecSampleCount -= presets.SamplesPermSec;
            m_CurrentmSec++;
        }
    }
    ldouble m_mSecSampleCount;
    ulong m_CurrentmSec;
    ulong m_CurrentTick;
    double m_SamplesPerTick;
    double m_mSecsPerTick;
    ldouble m_TickSampleCount;
    double m_uSPQ;
    double m_TicksPQ;
    double m_TempoAdjust;
};

#endif // CMSECCOUNTER_H
