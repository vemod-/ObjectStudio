#ifndef CLAYER2_H
#define CLAYER2_H

#include "imp3layer.h"

#pragma pack(push,1)

class CLayer2 : public IMP3LayerIandII
{
public:
    CLayer2(const MPEG_HEADER* h) : IMP3LayerIandII(h)
    {
        frameSlotCount = 144;
        frameSlotSize = 1;
        sbtCount = 36;
#ifdef mono_dispatch_count
#undef mono_dispatch_count
#endif
#define mono_dispatch_count 18
        fillGroupTable(3,5,m_group3_table); //grouped 3 level lookup table 5 bit token
        fillGroupTable(5,7,m_group5_table); //grouped 5 level lookup table 7 bit token
        fillGroupTable(9,10,m_group9_table); //grouped 9 level lookup table 10 bit token
        m_bit_allocation_table_count.resize(4);
    }
    virtual ~CLayer2();
    int decode_start()
    {
        /* ABCD_INDEX = lookqt[mode][sr_index][br_index]  */
        /* -1 = invalid  */
        static const char lookqt[4][3][16] =
        {
            {{1, -1, -1, -1, 2, -1, 2, 0, 0, 0, 1, 1, 1, 1, 1, -1},  /*  44ks stereo */
             {0, -1, -1, -1, 2, -1, 2, 0, 0, 0, 0, 0, 0, 0, 0, -1},  /*  48ks */
             {1, -1, -1, -1, 3, -1, 3, 0, 0, 0, 1, 1, 1, 1, 1, -1}},  /*  32ks */
            {{1, -1, -1, -1, 2, -1, 2, 0, 0, 0, 1, 1, 1, 1, 1, -1},  /*  44ks joint stereo */
             {0, -1, -1, -1, 2, -1, 2, 0, 0, 0, 0, 0, 0, 0, 0, -1},  /*  48ks */
             {1, -1, -1, -1, 3, -1, 3, 0, 0, 0, 1, 1, 1, 1, 1, -1}},  /*  32ks */
            {{1, -1, -1, -1, 2, -1, 2, 0, 0, 0, 1, 1, 1, 1, 1, -1},  /*  44ks dual chan */
             {0, -1, -1, -1, 2, -1, 2, 0, 0, 0, 0, 0, 0, 0, 0, -1},  /*  48ks */
             {1, -1, -1, -1, 3, -1, 3, 0, 0, 0, 1, 1, 1, 1, 1, -1}},  /*  32ks */
            // mono extended beyond legal br index
            //  1,2,2,0,0,0,1,1,1,1,1,1,1,1,1,-1,          /*  44ks single chan */
            //  0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,-1,          /*  48ks */
            //  1,3,3,0,0,0,1,1,1,1,1,1,1,1,1,-1,          /*  32ks */
            // legal mono
            {{1, 2, 2, 0, 0, 0, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1},  /*  44ks single chan */
             {0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, -1, -1},  /*  48ks */
             {1, 3, 3, 0, 0, 0, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1}},  /*  32ks */
        };
        /* bit allocation table look up */
        /* table per mpeg spec tables 3b2a/b/c/d  /e is mpeg2 */
        /* look_bat[abcd_index][4][16]  */
        static const byte look_bit_allocation_table[5][4][16] =
        {
            /* LOOK_BATA */
            {{0, 1, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17},
             {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 17},
             {0, 1, 2, 3, 4, 5, 6, 17, 0, 0, 0, 0, 0, 0, 0, 0},
             {0, 1, 2, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
            /* LOOK_BATB */
            {{0, 1, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17},
             {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 17},
             {0, 1, 2, 3, 4, 5, 6, 17, 0, 0, 0, 0, 0, 0, 0, 0},
             {0, 1, 2, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
            /* LOOK_BATC */
            {{0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16},
             {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
             {0, 1, 2, 4, 5, 6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
            /* LOOK_BATD */
            {{0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16},
             {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
             {0, 1, 2, 4, 5, 6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
            /* LOOK_BATE */
            {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
             {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
             {0, 1, 2, 4, 5, 6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0},
             {0, 1, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
        };
        /* look_nbat[abcd_index]][4] */
        static const byte look_bit_allocation_table_count[5][4] =
        {
            {3, 8, 12, 4},
            {3, 8, 12, 7},
            {2, 0, 6, 0},
            {2, 0, 10, 0},
            {4, 0, 7, 19},
        };
        //qDebug() << CurrentFrameInfo.bitRate << CurrentFrameInfo.bitsPerSample << CurrentFrameInfo.channels << CurrentFrameInfo.dataSize << CurrentFrameInfo.frequency << CurrentFrameInfo.maxInputSize << CurrentFrameInfo.minInputSize << CurrentFrameInfo.outputSize << CurrentFrameInfo.frames << CurrentFrameInfo.skipSize;
        // compute abcd index for bit allo table selection
        const int abcd_index = (CurrentFrameHeader.version == MPEG_HEADER::mpeg_version_1) ? lookqt[CurrentFrameHeader.mode][CurrentFrameHeader.frequency_index][CurrentFrameHeader.bitRate_index] : 4;
        if (abcd_index < 0) return 0;  // fail invalid Layer II bit rate index
        for (int i = 0; i < 4; i++)
        {
            m_bit_allocation_table_count[i] = look_bit_allocation_table_count[abcd_index][i];
            for (int j = 0; j < 16; j++) m_bit_allocation_table[i][j] = look_bit_allocation_table[abcd_index][i][j];
        }
        setCallerLimit();
        for (uint i = 0; i < m_allo.size(); i++) m_allo[i].setCSFactorSize(3);
        m_sf_dispatch.resize(m_max_subBands);
        // set sbt function
        Subband_transform = new CSbt(CurrentFrameHeader.layer,m_option.reduction,m_option.convert,CurrentFrameInfo.channels);
        return 1;
    }
    void decode_reset(){}
private:
    typedef std::vector<std::array<short,3>> group_table;
    group_table m_group3_table;
    group_table m_group5_table;
    group_table m_group9_table;
    int m_bit_allocation_table[4][16];
    std::vector<uint> m_sf_dispatch;
    void fillGroupTable(const int levels, const int bits, group_table& table)
    {
        table.resize(1 << bits);
        for (int i = 0; i < (1 << bits); i++)
        {
            int code = i;
            for (int j = 0; j < 3; j++) {
                table[i][j] = (code % levels) - ((levels-1)/2);
                code /= levels;
            }
        }
    }
    void unpack_bit_allocation()
    {
        static const double look_c_value[mono_dispatch_count] = {
            0, 0.666667, 0.4, 0.285714, 0.222222, 0.133333, 0.0645161, 0.031746, 0.015748,
            0.00784314, 0.00391389, 0.00195503, 0.00097704, 0.0004884, 0.00024417, 0.000122078, 6.1037e-05, 3.0518e-05
        };
        static const int bat_bit_master[mono_dispatch_count] = { 0, 5, 7, 9, 10, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48 };
        static const int bit_count[4] = {4, 4, 3, 2};
        m_bit_skip = 0;
        int nstereo = m_stereo_sb_count;
        uint k = 0;
        for (uint i = 0; i < 4; i++)
        {
            for (uint j = 0; j < m_bit_allocation_table_count[i]; j++, k++)
            {
                CAllocator& a = m_allo[k];
                const uint v = a.init(m_bit_allocation_table[i][m_bitget.bitget(bit_count[i])],look_c_value);
                if (k >= m_subBand_count_limit) m_bit_skip += bat_bit_master[v];
                if (--nstereo < 0)
                {
                    m_allo[++k] = a.flagAsJoint(mono_dispatch_count);
                    j++;
                }
            }
        }
        m_allo[m_subBand_count_limit].flagAsSkipAndTerminate(mono_dispatch_count);
        m_allo[m_max_subBands].flagAsTerminate(mono_dispatch_count);
    }
    void unpack_scaleFactor()  // unpack scale factor // // combine dequant and scale factors //
    {
        for (uint i = 0; i < m_max_subBands; i++) // unpack scale factor selectors
        {
            m_sf_dispatch[i] = (m_allo[i].isAllocated()) ? m_bitget.bitget(2) : 4; // no allo //
        }
        for (uint i = 0; i < m_max_subBands; i++)
        {
            CAllocator& a = m_allo[i];
            switch (m_sf_dispatch[i])
            {
            case 0:   // 3 factors 012 //
                m_bitget.bitget_check(18);
                for (uint j = 0; j < 3; j++) a.allocateScaleFactor(m_bitget.mac_bitget(6),1,j);
                break;
            case 1:   // 2 factors 002 //
                m_bitget.bitget_check(12);
                a.allocateScaleFactor(m_bitget.mac_bitget(6),2);
                a.allocateScaleFactor(m_bitget.mac_bitget(6),1,2);
                break;
            case 2:   // 1 factor 000 //
                m_bitget.bitget_check(6);
                a.allocateScaleFactor(m_bitget.mac_bitget(6),3);
                break;
            case 3:   // 2 factors 022 //
                m_bitget.bitget_check(12);
                a.allocateScaleFactor(m_bitget.mac_bitget(6));
                a.allocateScaleFactor(m_bitget.mac_bitget(6),2,1);
                break;
            case 4:   // no allo // //-- m_cs_factor[2][i] = m_cs_factor[1][i] = m_cs_factor[0][i] = 0.0;  --//
                break;
            case 5:   // all done //
                return;
            } // end switch //
            //i++;
        }
    }
    //-------------------------------------------------------------------------//
    inline void unpack(const uint k, const uint j, const uint i, float* s, const int bit)
    {
        s[k+(j*64)] = m_allo[k].unpack(i,bit);
    }
    inline void zero(const uint k, const uint j, float* s) { s[k+(j*64)] = 0.0f; }
    inline void unpack_n(const int n, float* s, const uint i, const uint k)
    {
        for (uint j = 0; j < 3; j++) unpack(k,j,i,s,getbit(n));
    }
    inline void unpackj_n(const int n, float* s, const uint i, uint& k)
    {
        for (uint j = 0; j < 3; j++)
        {
            const int tmp = (getbit(n));
            unpack(k,j,i,s,tmp);
            unpack(k+1,j,i,s,tmp);
        }
        k++; // skip right chan dispatch //
    }
    inline void zero_n(float* s, const uint k) { for (uint j = 0; j < 3; j++) zero(k,j,s); }
    inline void zeroj_n(float* s, uint& k)
    {
        for (uint j = 0; j < 3; j++)
        {
            zero(k,j,s);
            zero(k+1,j,s);
        }
        k++; // skip right chan dispatch //
    }
    inline void unpack_table(const int n1, float* s, const uint i, const uint k, const group_table& t)
    {
        const uint n = m_bitget.bitget(n1);
        for (uint j = 0; j < 3; j++) unpack(k,j,i,s,t[n][j]);
    }
    inline void unpackj_table(const int n1, float* s, const uint i, uint&k, const group_table& t)
    {
        const uint n = m_bitget.bitget(n1);
        for (uint j = 0; j < 3; j++)
        {
            unpack(k,j,i,s,t[n][j]);
            unpack(k+1,j,i,s,t[n][j]);
        }
        k++; // skip right chan dispatch //
    }

    //-------------------------------------------------------------------------//
    void unpack_samples() // unpack samples //
    {
        float* s = m_sample.data();
        for (uint i = 0; i < 3; i++) // 3 groups of scale factors //
        {
            for (uint j = 0; j < 4; j++)
            {
                for (uint k = 0; k <= m_max_subBands; k++)
                {
                    const uint sd = m_allo[k].sampDispatch();
                    if (sd < mono_dispatch_count) // mono
                    {
                        if (sd == 0) zero_n(s,k); //s[k + 128] = s[k + 64] = s[k] = 0.0F;
                        else if (sd == 1) unpack_table(5,s,i,k,m_group3_table);// 3 levels grouped 5 bits //
                        else if (sd == 2) unpack_table(7,s,i,k,m_group5_table);// 5 levels grouped 7 bits //
                        else if (sd == 3) unpack_n(3,s,i,k);// 7 levels //
                        else if (sd == 4) unpack_table(10,s,i,k,m_group9_table);// 9 levels grouped 10 bits //
                        else unpack_n(sd-1,s,i,k);// (1 << (sd-1)) -1 levels //
                    }
                    else if (sd < mono_dispatch_count * 2) // -- joint ---- //
                    {
                        if (sd == (mono_dispatch_count + 0)) zeroj_n(s,k);
                        else if (sd == (mono_dispatch_count + 1)) unpackj_table(5,s,i,k,m_group3_table);// 3 levels grouped 5 bits //
                        else if (sd == (mono_dispatch_count + 2)) unpackj_table(7,s,i,k,m_group5_table);// 5 levels grouped 7 bits //
                        else if (sd == (mono_dispatch_count + 3)) unpackj_n(3,s,i,k);// 7 levels //
                        else if (sd == (mono_dispatch_count + 4)) unpackj_table(10,s,i,k,m_group9_table);// 9 levels grouped 10 bits //
                        else unpackj_n((sd-1)-mono_dispatch_count,s,i,k);// (1 << (sd-19)) -1 levels //
                    }
                    else // -- end of dispatch -- //
                    {
                        if (sd == ((mono_dispatch_count * 2) + 1)) m_bitget.bitget_skip(m_bit_skip);
                        s += 3 * 64;
                        break;
                    }
                } // end k loop //
            }    // end j loop //
        }    // end i loop //
    }
};

#pragma pack(pop)

#endif // CLAYER2_H
