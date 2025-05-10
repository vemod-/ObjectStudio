#ifndef CLAYER1_H
#define CLAYER1_H

#include "imp3layer.h"

#pragma pack(push,1)

class CLayer1 : public IMP3LayerIandII
{
public:
    CLayer1(const MPEG_HEADER* h) : IMP3LayerIandII(h)
    {
        frameSlotCount = 12;
        frameSlotSize = 4;
        sbtCount = 12;
#ifdef mono_dispatch_count
#undef mono_dispatch_count
#endif
#define mono_dispatch_count 15
        m_bit_allocation_table_count.resize(1);
    }
    virtual ~CLayer1();
    int decode_start()
    {
        //qDebug() << CurrentFrameInfo.bitRate << CurrentFrameInfo.bitsPerSample << CurrentFrameInfo.channels << CurrentFrameInfo.dataSize << CurrentFrameInfo.frequency << CurrentFrameInfo.maxInputSize << CurrentFrameInfo.minInputSize << CurrentFrameInfo.outputSize << CurrentFrameInfo.frames << CurrentFrameInfo.skipSize;
        m_bit_allocation_table_count[0] = 32; /*- caller limit -*/
        setCallerLimit();
        for (uint i = 0; i < m_allo.size(); i++) m_allo[i].setCSFactorSize(1);
        /* set sbt function */
        Subband_transform = new CSbt(CurrentFrameHeader.layer,m_option.reduction,m_option.convert,CurrentFrameInfo.channels);
        return 1;
    }
    void decode_reset(){}
private:
    void unpack_bit_allocation()
    {
        static const double look_c_value[mono_dispatch_count] = {
            0, 0.666667, 0.285714, 0.133333, 0.0645161, 0.031746, 0.015748, 0.00784314,
            0.00391389, 0.00195503, 0.00097704, 0.0004884, 0.00024417, 0.000122078, 6.1037e-05
        };
        static const int bat_bit_master[mono_dispatch_count] = { 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }; // , 16
        m_bit_skip = 0;
        int nstereo = m_stereo_sb_count;
        for (uint k = 0; k < m_bit_allocation_table_count[0]; k++)
        {
            CAllocator& a = m_allo[k];
            const uint v = a.init(m_bitget.bitget(4),look_c_value);
            if (k >= m_subBand_count_limit) m_bit_skip += bat_bit_master[v];
            if (--nstereo < 0) m_allo[++k] = a.flagAsJoint(mono_dispatch_count);
        }
        m_allo[m_subBand_count_limit].flagAsSkipAndTerminate(mono_dispatch_count);
        m_allo[m_max_subBands].flagAsTerminate(mono_dispatch_count);
    }
    void unpack_scaleFactor() // unpack scale factor // combine dequant and scale factors
    {
        for (uint i = 0; i < m_bit_allocation_table_count[0]; i++)
        {
            CAllocator& a = m_allo[i];
            if (a.isAllocated())
            {
                a.allocateScaleFactor(m_bitget.bitget(6));
            }
        }
    }
    inline void unpack(const uint k, float* s, const int bit) { s[k] = m_allo[k].unpack(0,bit); }
    inline void zero(const uint k, float* s) { s[k] = 0.0f; }
    inline void unpack_n(const int n, float* s, const uint k) { unpack(k,s,getbit(n)); }
    inline void unpackj_n(const int n, float* s, uint& k)
    {
        const int tmp = getbit(n);
        unpack(k,s,tmp);
        unpack(k+1,s,tmp);
        k++; // skip right chan dispatch //
    }
    inline void zero_n(float* s, const uint k) { zero(k,s); }
    inline void zeroj_n(float* s, uint& k)
    {
        zero(k,s);
        zero(k+1,s);
        k++; // skip right chan dispatch //
    }
    void unpack_samples()
    {
        float* s = m_sample.data();
        for (uint j = 0; j < 12; j++)
        {
            for (uint k = 0; k <= m_max_subBands + 1; k++)
            {
                const uint sd = m_allo[k].sampDispatch();
                if (sd < mono_dispatch_count) // mono
                {
                    if (sd == 0) zero_n(s,k);
                    else unpack_n(sd+1,s,k); // (1 << (sd+1)) -1 levels //
                }
                else if (sd < (mono_dispatch_count * 2)) // -- joint ---- //
                {
                    if (sd ==(mono_dispatch_count + 0)) zeroj_n(s,k);
                    else unpackj_n((sd+1)-mono_dispatch_count,s,k); // (1 << (sd-14)) -1 levels //
                }
                else // -- end of dispatch -- //
                {
                    if (sd == (mono_dispatch_count * 2) + 1) m_bitget.bitget_skip(m_bit_skip);
                    s += 64;
                    break;
                }
            }
        } // end j loop //
    }
};

#pragma pack(pop)

#endif // CLAYER1_H
