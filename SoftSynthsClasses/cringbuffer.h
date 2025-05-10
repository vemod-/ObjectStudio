#ifndef CRINGBUFFER_H
#define CRINGBUFFER_H

#include "cfloatbuffer.h"

class CRingBuffer : public CFloatBuffer
{
public:
    inline CRingBuffer(ulong64 size=0)
    {
        m_Data=nullptr;
        reserve(size);
    }
    inline ~CRingBuffer() { delete [] m_Data; }
    inline void reserve(const ulong64 size)
    {
        if (m_Data) delete [] m_Data;
        m_Size=size;
        m_Data=new float[m_Size];
        zeroFloatBuffer(m_Data,m_Size);
    }
    inline void limit(int& n) const {
        while (n < 0) n += m_Size;
        while (uint(n) >= m_Size) n -= m_Size;
    }
    inline void limit(uint& n) const {
        while (n >= m_Size) n -= m_Size;
    }
    inline float read_buffer(int& pos) const {
        limit(pos);
        return m_Data[pos];
    }
    inline float read_buffer(uint& pos) const {
        limit(pos);
        return m_Data[pos];
    }
    inline float read_buffer(uint pos) const {
        return m_Data[pos];
    }
    inline float read_buffer(uint pos, uint n) const {
        while (n + pos >= m_Size)  n -= m_Size;
        return m_Data[n + pos];
    }
    inline void write_buffer(float insample, uint pos) {
        m_Data[pos] = insample;
    }
    inline void write_buffer(float insample, uint pos, int n) {
        while ((n + int(pos)) < 0) n += m_Size;
        write_buffer(insample,pos,uint(n));
    }
    inline void write_buffer(float insample, uint pos, uint n) {
        while (n + pos >= m_Size) n -= m_Size;
        m_Data[n + pos] = insample;
    }
    inline float push_buffer(float insample, uint &pos) {
        const float outsample = m_Data[pos];
        m_Data[pos++] = insample;
        if (pos >= m_Size) pos = 0;
        return outsample;
    }
};

#endif // CRINGBUFFER_H
