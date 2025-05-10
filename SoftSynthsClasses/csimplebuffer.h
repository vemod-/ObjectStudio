#ifndef CSIMPLEBUFFER_H
#define CSIMPLEBUFFER_H

#include "cfloatbuffer.h"
#include <QtEndian>

class CSimpleBuffer : public CFloatBuffer
{
public:
    inline CSimpleBuffer(const ulong64 s)
    {
        m_Shadow=false;
        m_Data=nullptr;
        init(s);
    }
    inline ~CSimpleBuffer() { deleteData(); }
    inline void initZero(const ulong64 s)
    {
        init(s);
        zero();
    }
    inline void fitSize(const ulong64 s)
    {
        if (m_Data != nullptr)
        {
            if (s==m_Capacity)
            {
                m_Size=s;
            }
            else
            {
                float* t=new float[s];
                if (m_Size > 0) copyFloatBuffer(t,m_Data,m_Size);
                deleteData();
                m_Shadow=false;
                m_Data=t;
                m_Capacity=s;
                m_CurrentPointer=m_Data;
            }
        }
        else
        {
            init(s);
        }
    }
    inline void resize(const ulong64 s)
    {
        if (s <= m_Capacity)
        {
            m_Size=s;
        }
        else
        {
            fitSize(s);
        }
    }
    inline void append(const float* b, const ulong64 s, const float f=1.f, const ulong64 sizestep=0xFFFF)
    {
        ulong64 cp=m_Capacity;
        const ulong64 sz=m_Size;
        while (s+sz > cp) cp+=sizestep;
        if (cp > m_Capacity) resize(cp);
        (b) ? copyFloatBuffer(m_Data+sz,b,f,s) : zeroFloatBuffer(m_Data+sz,s);
        m_Size=sz+s;
    }
    inline void squeeze()
    {
        if (m_Size < m_Capacity) fitSize(m_Size);
    }
    inline void init(const ulong64 s)
    {
        deleteData();
        m_Shadow=false;
        m_Size=s;
        m_Capacity=s;
        m_Data=new float[m_Size];
        m_CurrentPointer=m_Data;
    }

    inline CSimpleBuffer(float* d=nullptr, const ulong64 s=0)
    {
        m_Shadow=true;
        m_Data=nullptr;
        fromRawData(d,s);
    }
    inline CSimpleBuffer(CSimpleBuffer& b)
    {
        m_Shadow=true;
        m_Data=nullptr;
        fromRawData(b.data(),b.size());
    }
    inline void makeNull() { fromRawData(nullptr,0); }
    inline void fromRawData(float* d, const ulong64 s)
    {
        deleteData();
        m_Shadow=true;
        m_Data=d;
        m_Size=s;
        m_Capacity=s;
        m_CurrentPointer=m_Data;
    }
    inline bool isValid() const { return ((m_Data != nullptr) && (m_Size != 0)); }

    inline void zero() { zeroFloatBuffer(m_Data,m_Capacity); }
    inline float* dataPointer(const ulong64 p) const { return m_Data+p; }
    inline void set(const float v) { *m_CurrentPointer++=v; }
    inline void fill(const float* b, const ulong64 l)
    {
        (!b) ? zero() : copyFloatBuffer(m_Data,b,qMin<ulong>(m_Capacity,l));
    }
    inline void copy(const CSimpleBuffer& b,const ulong64 src,const ulong64 l=0,const ulong64 dest=0)
    {
        const ulong len=qMin<ulong>((l==0) ? b.size() : l,m_Capacity);
        copyFloatBuffer(dataPointer(dest),b.dataPointer(src),len);
    }
    inline void setShort(const short v) { *m_CurrentPointer++=v*MAXSHORTMULTIPLY_F; }
    inline void setInt(const int v) { *m_CurrentPointer++=v*MAXINTMULTIPLY_F; }
    inline void reset() { m_CurrentPointer=m_Data; }
    inline ulong64 chunkSize() const { return m_Size*sizeof(float); }
    inline bool isEmpty() const { return (m_Size==0); }
protected:
    ulong64 m_Capacity;
    float* m_CurrentPointer;
    bool m_Shadow;
    inline void deleteData() { if (!m_Shadow) if (m_Data != nullptr) delete [] m_Data; }
};

class CChannelBuffer : public CFloatBuffer
{
public:
    inline CChannelBuffer(const ulong64 s,const uint c=1)
    {
        m_Shadow=false;
        m_Data=nullptr;
        m_ChannelPointers=nullptr;
        init(s,c);
    }
    inline ~CChannelBuffer()
    {
        deleteData();
        delete[] m_ChannelPointers;
    }
    inline void init(const ulong64 s, const uint c)
    {
        deleteData();
        m_Shadow=false;
        m_Size=s;
        m_Channels=c;
        m_Capacity=(s==0) ? 0xFFFF : s;
        m_Data=new float[m_Capacity*m_Channels];
        m_CurrentPointer=m_Data;
        setChannelPointers();
    }
    inline void initZero(const ulong64 s, const uint c)
    {
        init(s,c);
        zero();
    }
    inline void resize(const ulong64 s)
    {
        if (s <= m_Capacity)
            m_Size=s;
        else
            fitSize(s);
    }
    inline void fitSize(const ulong64 s)
    {
        if (s==m_Capacity) {
            m_Size=s;
            return;
        }
        if (m_Data != nullptr) {
            float* t=new float[s*m_Channels];
            if (m_Size > 0)
            {
                for (uint c=0;c<m_Channels;c++) copyFloatBuffer(t+(s*c),channelPointer(c),m_Size);
            }
            deleteData();
            m_Shadow=false;
            m_Data=t;
            m_Capacity=s;
            m_CurrentPointer=m_Data;
            setChannelPointers();
        }
        else {
            init(s,m_Channels);
        }
    }
    inline void append(const float* b, const ulong64 s, const ulong64 sizestep=0xFFFF)
    {
        ulong64 cp=m_Capacity;
        const ulong64 sz=m_Size;
        while (s+sz > cp) cp+=sizestep;
        if (cp > m_Capacity) resize(cp);
        for (uint c=0;c<m_Channels;c++) {
            (b) ? copyFloatBuffer(channelPointer(c)+sz,b+(s*c),s) : zeroFloatBuffer(channelPointer(c)+sz,s);
        }
        m_Size=sz+s;
    }
    inline void squeeze()
    {
        if (m_Size < m_Capacity) fitSize(m_Size);
    }
    inline void fromShortInterleaved(const std::vector<short>& b, const uint c)
    {
        init(b.size()/c,c);
        const short* t=b.data();
        for (uint ch=0;ch<c;ch++) {
            for (ulong64 i=ch;i<b.size();i+=c) setShort(*(t+i));
        }
    }
    inline CChannelBuffer(float* d=nullptr, const uint c=1, const ulong64 s=0)
    {
        m_Shadow=true;
        m_ChannelPointers=nullptr;
        fromRawData(d,c,s);
    }
    inline CChannelBuffer(CChannelBuffer& b)
    {
        m_Shadow=true;
        m_ChannelPointers=nullptr;
        fromRawData(b.data(),b.channels(),b.size());
    }
    inline void makeNull() { fromRawData(nullptr,1,0); }
    inline void fromRawData(float* d, const uint c, const ulong64 s)
    {
        deleteData();
        m_Shadow=true;
        m_Data=d;
        m_Size=s;
        m_Capacity=s;
        m_Channels=c;
        m_CurrentPointer=d;
        setChannelPointers();
    }
    inline bool isValid() const { return ((m_Data != nullptr) && (m_Size != 0)); }

    inline void zero() { zeroFloatBuffer(m_Data,m_Capacity*m_Channels); }
    inline void zeroChannel(const uint c) { zeroFloatBuffer(channelPointer(c),m_Size); }
    inline float* channelPointer(const uint c) const { return m_Data+(m_Capacity*c); }//{ return m_ChannelPointers[c]; }
    inline float* dataPointer(const ulong64 p,const uint c) const { return channelPointer(c)+p; }
    inline void set(const float v) { *m_CurrentPointer++=v; }
    inline void setAt(const ulong64 p, const uint c, const float v) { *dataPointer(p,c)=v; }
    inline void zeroAt(const ulong64 p, const uint c) { *dataPointer(p,c)=0; }
    inline void addAt(const ulong64 p, const uint c, const float v) { *dataPointer(p,c)+=v; }
    inline void setAtX(const ulong64 p, const float v) { for (uint c=0;c<m_Channels;c++) *dataPointer(p,c)=v; }
    inline void zeroAtX(const ulong64 p) { for (uint c=0;c<m_Channels;c++) *dataPointer(p,c)=0; }
    inline void addAtX(const ulong64 p, const float v) { for (uint c=0;c<m_Channels;c++) *dataPointer(p,c)=v; }
    inline void fill(const float* b,const ulong64 l)
    {
        (!b) ? zero() : copyFloatBuffer(m_Data,b,qMin<ulong>(m_Capacity*m_Channels,l));
    }
    inline void setX(const float* b,const uint c,const ulong64 l)
    {
        copyFloatBuffer(channelPointer(c),b,qMin<ulong>(l,m_Capacity));
    }
    inline void setX(const ulong64 dest,const CChannelBuffer& b,const ulong64 src)
    {
        for (uint c=0;c<m_Channels;c++) *dataPointer(dest,c)=*b.dataPointer(src,c);
    }
    inline void setX(const ulong64 dest,const CChannelBuffer& b,const ulong64 src,const float f)
    {
        if (isZero(f))
            zeroAtX(dest);
        else if (isOne(f))
            for (uint c=0;c<m_Channels;c++) *dataPointer(dest,c)=*b.dataPointer(src,c);
        else
            for (uint c=0;c<m_Channels;c++) *dataPointer(dest,c)=*b.dataPointer(src,c)*f;
    }
    inline void addX(const ulong64 dest,const CChannelBuffer& b,const ulong64 src)
    {
        for (uint c=0;c<m_Channels;c++) *dataPointer(dest,c)+=*b.dataPointer(src,c);
    }
    inline void addX(const ulong64 dest,const CChannelBuffer& b,const ulong64 src,const float f)
    {
        if (isOne(f))
            for (uint c=0;c<m_Channels;c++) *dataPointer(dest,c)+=*b.dataPointer(src,c);
        else if (f>0)
            for (uint c=0;c<m_Channels;c++) *dataPointer(dest,c)+=*b.dataPointer(src,c)*f;
    }
    inline void copy(const CChannelBuffer& b,const ulong64 src,const ulong64 l=0,const ulong64 dest=0)
    {
        const ulong len=qMin<ulong>((l==0) ? b.size() : l,m_Capacity);
        for (uint c=0;c<m_Channels;c++) copyFloatBuffer(dataPointer(dest,c),b.dataPointer(src,c),len);
    }
    inline void setShort(const short v) { *m_CurrentPointer++=v*MAXSHORTMULTIPLY_F; }
    inline void setInt(const int v) { *m_CurrentPointer++=v*MAXINTMULTIPLY_F; }
    inline void reset(const uint c=0) { m_CurrentPointer=channelPointer(c); }
    inline float at(const ulong64 p,const uint c) const { return *(channelPointer(c)+p); }
    inline short shortAt(const ulong64 p,const uint c) const { return short(*(channelPointer(c)+p)*SHRT_MAX); }
    inline int intAt(const ulong64 p,const uint c) const { return int(*(channelPointer(c)+p)*INT_MAX); }
    inline const std::vector<short> toShortInterleaved() const
    {
        std::vector<short> b(dataSize());
        short* t=b.data();
        for (ulong64 i=0;i<size();i++) {
            for (uint c=0;c<channels();c++) *t++=shortAt(i,c);
        }
        return b;
    }
    inline const std::vector<int> toIntInterleaved() const
    {
        std::vector<int> b(dataSize());
        int* t=b.data();
        for (ulong64 i=0;i<size();i++) {
            for (uint c=0;c<channels();c++) *t++=qToBigEndian<int>(intAt(i,c));
        }
        return b;
    }
    inline ulong64 dataSize() const { return m_Size*m_Channels; }
    inline ulong64 chunkSize() const { return dataSize()*sizeof(float); }
    inline uint channels() const { return m_Channels; }
    inline bool isEmpty() const { return (m_Size==0); }
    inline float** channelPointers() const
    {
        return (m_Channels==0) ? nullptr : m_ChannelPointers;
    }
    inline void normalize()
    {
        float* channelPeaks = new float[m_Channels];
        for (uint c=0;c<m_Channels;c++) channelPeaks[c] = peakFloatBuffer(channelPointer(c), m_Size);
        float max = 0;
        for (uint c=0;c<m_Channels;c++) max = qMax<float>(max, channelPeaks[c]);
        delete [] channelPeaks;
        float factor = 1.f / max;
        if (isZero(factor)) return;
        for (uint c=0;c<m_Channels;c++) multiplyFloatBuffer(channelPointer(c), factor, m_Size);
    }
protected:
    inline void deleteData() { if (!m_Shadow) if (m_Data != nullptr) delete [] m_Data; }
    float** m_ChannelPointers;
    ulong64 m_Capacity;
    uint m_Channels;
    float* m_CurrentPointer;
    bool m_Shadow;
    inline void setChannelPointers()
    {
        if (m_ChannelPointers) delete[] m_ChannelPointers;
        m_ChannelPointers=new float*[m_Channels];
        for (uint c=0;c<m_Channels;c++) m_ChannelPointers[c]=m_Data+(m_Capacity*c);
    }
};

#endif // CSIMPLEBUFFER_H
