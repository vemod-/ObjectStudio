#ifndef CAUDIOBUFFER_H
#define CAUDIOBUFFER_H

#include "ijackbase.h"
#include "cfloatbuffer.h"
#include "cpresets.h"
;
#pragma pack(push,1)

class CAudioBuffer : public CFloatBuffer, protected IPresetRef
{
protected:
    IJackBase::AttachModes m_AttachMode;
    ulong64 m_WaveBufferSize;
    bool m_Shadow;
    float* m_DataR;
    inline CAudioBuffer(float* b, IJackBase::AttachModes a) : m_AttachMode(a)
    {
        m_Size=presets.ModulationRate;
        m_Shadow=true;
        m_WaveBufferSize=(m_AttachMode==IJackBase::Stereo) ? m_Size*2 : m_Size;
        m_Data=b;
        m_DataR=(m_Data != nullptr) ? m_Data+m_Size : nullptr;
    }
    inline bool isValid() const { return (m_Data != nullptr); }
    inline void fromRawData(void* d, IJackBase::AttachModes a)
    {
        deleteData();
        m_Shadow=true;
        m_AttachMode=a;
        m_Size=presets.ModulationRate;
        m_WaveBufferSize=(m_AttachMode==IJackBase::Stereo) ? m_Size*2 : m_Size;
        m_Data=static_cast<float*>(d);
        m_DataR=(m_Data != nullptr) ? m_Data+m_Size : nullptr;
    }
    inline void deleteData() { if (!m_Shadow) if (m_Data) delete [] m_Data; }
public:
    inline CAudioBuffer(IJackBase::AttachModes a) : m_AttachMode(a)
    {
        m_Size=presets.ModulationRate;
        m_Shadow=false;
        m_WaveBufferSize=(m_AttachMode==IJackBase::Stereo) ? m_Size*2 : m_Size;
        m_Data=new float[m_WaveBufferSize];
        m_DataR=m_Data+m_Size;
        zeroBuffer();
    }
    virtual ~CAudioBuffer();
    inline float* zeroBuffer() {
        zeroFloatBuffer(m_Data,m_WaveBufferSize);
        return m_Data;
    }
    inline float* writeBuffer(const float* b,bool zero=false) {
        if (!b) return (zero) ? zeroBuffer() : nullptr;
        copyFloatBuffer(m_Data,b,m_WaveBufferSize);
        return m_Data;
    }
    inline float* writeBuffer(const float *b, const float factor,bool zero=false) {
        if (!b) return (zero) ? zeroBuffer() : nullptr;
        copyFloatBuffer(m_Data,b,factor,m_WaveBufferSize);
        return m_Data;
    }
    inline float* writeBuffer(const float* b, const IJackBase::AttachModes a,bool zero=false) {
        if (!b) return (zero) ? zeroBuffer() : nullptr;
        if (a==m_AttachMode) return writeBuffer(b);
        if (m_AttachMode==IJackBase::Wave) {
            copyAddMultiplyFloatBuffer(m_Data,b,b+m_Size,M_SQRT1_2_F,m_Size);
        }
        else {
            copyFloatBuffer(m_Data,b,m_Size);
            copyFloatBuffer(m_DataR,b,m_Size);
        }
        return m_Data;
    }
    inline float* writeBuffer(const CAudioBuffer* b, const float factor=1) {
        if (!b) return zeroBuffer();
        if ((!b->isValid()) || (isZero(factor))) {
            return zeroBuffer();
        }
        if (isOne(factor)) return writeBuffer(b->data(),b->attachmode());
        if (b->attachmode()==m_AttachMode) return writeBuffer(b->data(),factor);
        if (m_AttachMode==IJackBase::Wave) {
            copyAddMultiplyFloatBuffer(m_Data,b->data(),b->data()+m_Size,M_SQRT1_2_F*factor,m_Size);
        }
        else {
            copyFloatBuffer(m_Data,b->data(),factor,m_Size);
            copyFloatBuffer(m_DataR,m_Data,m_Size);
            //copyFloatBuffer(m_DataR,b->data(),factor,m_Size);
        }
        return m_Data;
    }
    inline float* writeBuffer(const CAudioBuffer* b, const IJackBase::AttachModes a,bool zero=false) {
        return (!b) ? zeroBuffer() : writeBuffer(b->data(), a, zero);
    }
    inline float* addBuffer(const float *b, const float factor=1) {
        if (b) addFloatBuffer(m_Data,b,factor,m_WaveBufferSize);
        return m_Data;
    }
    inline float* addBuffer(const float* b, const IJackBase::AttachModes a) {
        if (b)
        {
            if (a==m_AttachMode) return addBuffer(b);
            if (m_AttachMode==IJackBase::Wave) {
                addAddFloatBuffer(m_Data,b,b+m_Size,M_SQRT1_2_F,m_Size);
            }
            else {
                addFloatBuffer(m_Data,b,m_Size);
                addFloatBuffer(m_DataR,b,m_Size);
            }
        }
        return m_Data;
    }
    inline float* addBuffer(const CAudioBuffer* b, const float factor=1) {
        if (!b) return zeroBuffer();
        if (b->isValid() && (!isZero(factor)))
        {
            if (isOne(factor)) return addBuffer(b->data(),b->attachmode());
            if (b->attachmode()==m_AttachMode) return addBuffer(b->data(),factor);
            if (m_AttachMode==IJackBase::Wave) {
                addAddFloatBuffer(m_Data,b->data(),b->data()+m_Size,M_SQRT1_2_F*factor,m_Size);
            }
            else {
                addFloatBuffer(m_Data,b->data(),factor,m_Size);
                addFloatBuffer(m_DataR,b->data(),factor,m_Size);
            }
        }
        return m_Data;
    }
    inline void updateBuffer(const float *b, const IJackBase::AttachModes a, bool overwrite) {
        if (b)
        {
            (overwrite) ? writeBuffer(b,a) : addBuffer(b,a);
        }
    }
    inline void updateBuffer(const CAudioBuffer *b, const IJackBase::AttachModes a, bool overwrite) {
        if (b) updateBuffer(b->data(), a, overwrite);
    }
    inline float* multiplyBuffer(const float factor) {
        multiplyFloatBuffer(m_Data,factor,m_WaveBufferSize);
        return m_Data;
    }
    inline void peakBuffer(float* p, const float factor=1, const float max=2) const {
        *p=peakFloatBuffer(m_Data,m_WaveBufferSize,*p,factor,max);
    }
    inline void operator *= (const float factor) { multiplyBuffer(factor); }
    inline void operator /= (const float divisor) { multiplyBuffer(1/divisor); }
    inline void operator += (const CAudioBuffer* b) { if (b) addBuffer(b->m_Data,b->m_AttachMode); }
    inline void operator += (const CAudioBuffer& b) { addBuffer(b.m_Data,b.m_AttachMode); }
    inline void operator += (const float* b) { addBuffer(b); }
    inline void operator = (CAudioBuffer& b) { writeBuffer(b.m_Data,b.m_AttachMode); }
    inline void operator = (const float* b) { writeBuffer(b); }
    inline IJackBase::AttachModes attachmode() const { return m_AttachMode; }
};

class CMonoBuffer : public CAudioBuffer
{
public:
    inline CMonoBuffer() : CAudioBuffer(IJackBase::Wave){}
    inline CMonoBuffer(float*b) : CAudioBuffer(b,IJackBase::Wave){}
    virtual ~CMonoBuffer();
    inline float* fromStereo(const float* b) { return CAudioBuffer::writeBuffer(b,IJackBase::Stereo); }
    inline float* addStereo(const float* b) { return CAudioBuffer::addBuffer(b,IJackBase::Stereo); }
    inline void fromRawData(void* d)
    {
        deleteData();
        m_Shadow=true;
        m_Data=static_cast<float*>(d);
    }
    inline void makeNull()
    {
        fromRawData(nullptr);
    }
    inline bool isValid() const { return CAudioBuffer::isValid(); }
};

class CStereoBuffer : public CAudioBuffer
{
public:
    inline CStereoBuffer(float*b) : CAudioBuffer(b,IJackBase::Stereo) {
        leftBuffer=new CMonoBuffer(m_Data);
        rightBuffer=new CMonoBuffer(m_DataR);
    }
    inline CStereoBuffer() : CAudioBuffer(IJackBase::Stereo) {
        leftBuffer=new CMonoBuffer(m_Data);
        rightBuffer=new CMonoBuffer(m_DataR);
    }
    virtual ~CStereoBuffer();
    inline void zeroLeftBuffer() { zeroFloatBuffer(m_Data,m_Size); }
    inline void zeroRightBuffer() { zeroFloatBuffer(m_DataR,m_Size); }
    inline float* writeStereoBuffer(const float* b, const float factorL=1, const float factorR=1) {
        if (!b) return nullptr;
        writeLeftBuffer(b,factorL);
        writeRightBuffer(b+m_Size,factorR);
        return m_Data;
    }
    inline float* writeStereoBuffer(const CStereoBuffer* b, const float factorL=1, const float factorR=1) {
        if (!b) return zeroBuffer();
        if (!b->isValid()) return zeroBuffer();
        writeLeftBuffer(b->data(),factorL);
        writeRightBuffer(b->dataR(),factorR);
        return m_Data;
    }
    inline void writeLeftBuffer(const float* b, const float factor=1) {
        (b) ? copyFloatBuffer(m_Data,b,factor,m_Size) : zeroLeftBuffer();
    }
    inline void writeRightBuffer(const float* b, const float factor=1) {
        (b) ? copyFloatBuffer(m_DataR,b,factor,m_Size) : zeroRightBuffer();
    }
    inline float* fromDualMono(const float* L, const float* R, const float factor=1) {
        writeLeftBuffer(L,factor);
        writeRightBuffer(R,factor);
        return m_Data;
    }
    inline float* fromDualMono(const CMonoBuffer* bL, const CMonoBuffer* bR, const float factor=1) {
        writeLeftBuffer(bL->data(),factor);
        writeRightBuffer(bR->data(),factor);
        return m_Data;
    }
    inline float* fromMono(const float* b) { return CAudioBuffer::writeBuffer(b,IJackBase::Wave); }
    inline float* fromMono(const float* b, const float factorL, const float factorR) {
        if (!b) return nullptr;
        writeLeftBuffer(b,factorL);
        writeRightBuffer(b,factorR);
        return m_Data;
    }
    inline float* addStereoBuffer(const float *b, const float factorL=1, const float factorR=1) {
        addLeftBuffer(b, factorL);
        if (b) addRightBuffer(b+m_Size, factorR);
        return m_Data;
    }
    inline float* addStereoBuffer(const CStereoBuffer* b, const float factorL=1, const float factorR=1) {
        if (!b) return m_Data;
        if (!b->isValid()) return m_Data;
        addLeftBuffer(b->data(), factorL);
        addRightBuffer(b->dataR(), factorR);
        return m_Data;
    }
    inline void addLeftBuffer(const float* b, float factor=1) { if (b) addFloatBuffer(m_Data,b,factor,m_Size); }
    inline void addRightBuffer(const float* b, const float factor=1) { if (b) addFloatBuffer(m_DataR,b,factor,m_Size); }
    inline float* addDualMono(const float* L, const float* R) {
        addLeftBuffer(L);
        addRightBuffer(R);
        return m_Data;
    }
    inline float* addDualMono(const CMonoBuffer* bL, CMonoBuffer* bR) {
        addLeftBuffer(bL->data());
        addRightBuffer(bR->data());
        return m_Data;
    }
    inline float* addMono(const float* b) { return CAudioBuffer::addBuffer(b,IJackBase::Wave); }
    inline float* addMono(const float* b,const float factorL=1, const float factorR=1) {
        addLeftBuffer(b,factorL);
        addRightBuffer(b,factorR);
        return m_Data;
    }
    inline float* multiplyStereoBuffer(const float factorL,const float factorR) {
        multiplyFloatBuffer(m_Data,factorL,m_Size);
        multiplyFloatBuffer(m_DataR,factorR,m_Size);
        return m_Data;
    }
    inline void peakStereoBuffer(float* pLeft, float* pRight, const float factorL=1, const float factorR=1 , const float max=2) const {
        *pLeft = peakFloatBuffer(m_Data,m_Size,*pLeft,factorL,max);
        *pRight = peakFloatBuffer(m_DataR,m_Size,*pRight,factorR,max);
    }
    inline float atR(const ulong64 p) const { return m_DataR[p]; }
    inline void zeroAt(const ulong64 p) {
        m_Data[p]=0;
        m_DataR[p]=0;
    }
    inline void setAt(const ulong64 p,const float v) {
        m_Data[p]=v;
        m_DataR[p]=v;
    }
    inline void addAt(const ulong64 p,const float v) {
        m_Data[p]+=v;
        m_DataR[p]+=v;
    }
    inline void setAtL(const ulong64 p,const float v) {
        m_Data[p]=v;
    }
    inline void addAtL(const ulong64 p,const float v) {
        m_Data[p]+=v;
    }
    inline void setAtR(const ulong64 p,const float v) {
        m_DataR[p]=v;
    }
    inline void addAtR(const ulong64 p,const float v) {
        m_DataR[p]+=v;
    }
    inline void setAt(const ulong64 p,const float vL, const float vR) {
        m_Data[p]=vL;
        m_DataR[p]=vR;
    }
    inline void addAt(const ulong64 p,const float vL,const float vR) {
        m_Data[p]+=vL;
        m_DataR[p]+=vR;
    }
    inline void interleaveAt(const ulong64 p, float*& i, const float factor = 1)
    {
        *i++=m_Data[p]*factor;
        *i++=m_DataR[p]*factor;
    }
    inline void setAtFromInterleaved(const ulong64 p, float*& i)
    {
        m_Data[p]=*i++;
        m_DataR[p]=*i++;
    }
    inline void addAtInterleaved(const ulong64 p, float*& i)
    {
        m_Data[p]+=*i++;
        m_DataR[p]+=*i++;
    }
    inline float* dataR() const { return m_DataR; }
    inline int channels() const { return 2; }
    inline void fromRawData(void* d)
    {
        deleteData();
        m_Shadow=true;
        m_Data=static_cast<float*>(d);
        m_DataR=(m_Data != nullptr) ? m_Data+m_Size : nullptr;
        leftBuffer->fromRawData(m_Data);
        rightBuffer->fromRawData(m_DataR);
    }
    inline void makeNull()
    {
        fromRawData(nullptr);
    }
    inline bool isValid() const { return (m_Data != nullptr); }
    CMonoBuffer* leftBuffer;
    CMonoBuffer* rightBuffer;
};

#pragma pack(pop)

#endif // CAUDIOBUFFER_H
