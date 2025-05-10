#ifndef CFASTCIRCULARBUFFER_H
#define CFASTCIRCULARBUFFER_H

#include "csimplebuffer.h"

class CFastCircularBuffer : protected CSimpleBuffer
{
public:
    CFastCircularBuffer(ulong maxSize) : CSimpleBuffer(maxSize),m_maxGrowth(maxSize + 100) { zero(); }
    CFastCircularBuffer(ulong maxSize, ulong stepSize) : CSimpleBuffer(maxSize),m_maxGrowth(maxSize + stepSize),m_stepSize(stepSize) { zero(); }
    void reset() {
        m_readPtr=0;
        resize(m_latency);
        if (m_latency) zeroFloatBuffer(m_Data,m_latency);
    }
    void clear(const uint latency = 0) {
        m_latency = latency;
        reset();
    }
    void setStepSize(ulong stepSize) {
        reset();
        m_stepSize = stepSize;
    }
    void setMaxGrowth(ulong maxGrowth) { m_maxGrowth = maxGrowth; }
    inline bool isAvail(ulong size) const { return (m_Size >= size + m_readPtr); }
    inline float* read(ulong size) {
        float* r = m_Data + m_readPtr;
        m_readPtr += qMin<ulong>(m_stepSize, size);
        return r;
    }
    void write(float *d, uint s) {
        if (m_Size >= m_maxGrowth)
        {
            const ulong skipSize = m_Size - m_readPtr;
            if (skipSize) copyFloatBuffer(m_Data, m_Data + m_readPtr, skipSize);
            resize(skipSize);
            m_readPtr = 0;
        }
        append(d,s);
    }
private:
    ulong m_maxGrowth;
    ulong m_stepSize = 100;
    ulong m_readPtr = 0;
    ulong m_latency = 0;
};

#endif // CFASTCIRCULARBUFFER_H
