#ifndef CSYNCBUFFER_H
#define CSYNCBUFFER_H

#include "csimplebuffer.h"

class CSyncBuffer : protected CSimpleBuffer
{
public:
    CSyncBuffer(uint maxSize) : CSimpleBuffer(maxSize),m_readSize(100),m_readCount(0) { zero(); }
    void setReadSize(double s) {
        m_readSize = qMax<double>(s,1);
    }
    inline bool isAvail() const { return (m_readCount > m_readSize); }
    inline float* data() const { return m_Data; }
    void write(const float *d, uint s) {
        if (m_readCount > m_readSize)
        {
            double intp;
            const double frac=modf(m_readCount,&intp);
            const ulong iSize=ulong(intp);
            const ulong skipSize=m_Size - iSize;
            if (skipSize) copyFloatBuffer(m_Data,m_Data+iSize,skipSize);
            resize(skipSize);
            m_readCount = frac;
        }
        append(d,s);
        while (m_readCount + m_readSize < m_Size) m_readCount += m_readSize;
    }
private:
    double m_readSize;
    double m_readCount;
};

#endif // CSYNCBUFFER_H
