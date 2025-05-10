#ifndef CFLOATBUFFER_H
#define CFLOATBUFFER_H

#include "softsynthsdefines.h"
;
#pragma pack(push,1)

class CFloatBuffer
{
protected:
    float* m_Data;
    ulong64 m_Size;
public:
    inline CFloatBuffer(float* d=nullptr,ulong64 s=0) : m_Data(d),m_Size(s) { }
    inline float at(const ulong64 p) const { return m_Data[p]; }
    inline short shortAt(const ulong64 p) const { return short(*(m_Data+p)*SHRT_MAX); }
    inline int intAt(const ulong64 p) const { return int(*(m_Data+p)*INT_MAX); }
    inline void zeroAt(const ulong64 p) { m_Data[p]=0; }
    inline void setAt(const ulong64 p,const float v) { m_Data[p]=v; }
    inline void addAt(const ulong64 p,const float v) { m_Data[p]+=v; }
    inline float& operator[] (const ulong64 nIndex) { return m_Data[nIndex]; }
    inline float* data() const { return m_Data; }
    inline ulong64 size() const { return m_Size; }
    inline int channels() const { return 1; }

    static void inline zeroFloatBuffer(float* b, const ulong64 s) { zeroMemory(b,s*sizeof(float)); }
    static void inline addFloatBuffer(float* dest, const float* source, const ulong64 s) { for (ulong64 i=0;i<s;i++) dest[i]+=source[i]; }
    static void inline addFloatBuffer(float* dest, const float* source, const float factor, const ulong64 s) {
        if (isOne(factor)) addFloatBuffer(dest,source,s);
        else if (!isZero(factor)) for (ulong64 i=0;i<s;i++) dest[i]+=source[i]*factor;
    }
    static void inline addAddFloatBuffer(float* dest, const float* b1, const float* b2, const float factor, const ulong64 s) {
        if (isOne(factor)) { for (ulong64 i=0;i<s;i++) dest[i]+=b1[i]+b2[i]; }
        else if (!isZero(factor)) { for (ulong64 i=0;i<s;i++) dest[i]+=(b1[i]+b2[i])*factor; }
    }
    static void inline copyFloatBuffer(float* dest, const float* source, const ulong64 s) { copyMemory(dest,source,s*sizeof(float)); }
    static void inline copyFloatBuffer(float* dest, const float* source, const float factor, const ulong64 s) {
        if (isZero(factor)) zeroFloatBuffer(dest,s);
        else if (isOne(factor)) copyFloatBuffer(dest,source,s);
        else { for (ulong64 i=0;i<s;i++) dest[i]=source[i]*factor; }
    }
    static void inline multiplyFloatBuffer(float* b, const float factor, const ulong64 s) {
        if (isZero(factor)) zeroFloatBuffer(b,s);
        else if (!isOne(factor)) { for (ulong64 i=0;i<s;i++) b[i]*=factor; }
    }
    static void inline copyAddMultiplyFloatBuffer(float* dest, const float* b1, const  float* b2, const float factor, const ulong64 s) {
        if (isZero(factor)) zeroFloatBuffer(dest,s);
        else if (isOne(factor)) { for (ulong64 i=0;i<s;i++) dest[i]=b1[i]+b2[i]; }
        else { for (ulong64 i=0;i<s;i++) dest[i]=(b1[i]+b2[i])*factor; }
    }
    static float inline peakFloatBuffer(float* b, const ulong64 s, float p=0, const float factor=1, const float max=2) {
        if (isZero(factor)) return qMin<float>(p,max);
        if (p>max) return max;
        float tempPeak=0;
        for (ulong64 i=0;i<s;i++) tempPeak=qMax<float>(qAbs<float>(b[i]),tempPeak);
        return qBound<float>(p,tempPeak*factor,max);
    }
};

#pragma pack(pop)

#endif // CFLOATBUFFER_H
