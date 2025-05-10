#ifndef L3MSIS_H
#define L3MSIS_H

#include "l3sf.h"
#include "l3hybrid.h"
#include "l3huff.h"
#include "l3alias.h"
#include "math.h"
#include <algorithm>

#pragma pack(push,1)

class SIDE_INFO {
public:
    SIDE_INFO(const int version) { m_version = version; }
    uint main_data_begin;
    uint scfsi[2]; /* 4 bit flags [ch] */
    static uint getSideSize(const int version, const int channels)
    {
        if (version == 1)
        {
            return (channels == 1) ? 17 : 32;
        }
        else
        {
            return (channels == 1) ? 9 : 17;
        }
    }
    void getSideInfo(const int channels, CBitget& bitget)
    {
        if (m_version == 1)
        {
            main_data_begin = bitget.bitget(9);
            private_bits = (channels == 1) ? bitget.bitget(5) : bitget.bitget(3);
            for (int ch = 0; ch < channels; ch ++) scfsi[ch] = bitget.bitget(4);
        }
        else
        {
            main_data_begin = bitget.bitget(8);
            private_bits = (channels == 1) ? bitget.bitget(1) : bitget.bitget(2);
            scfsi[0] = scfsi[1] = 0;
        }
    }
private:
    int m_version = 0;
    uint private_bits;
};

class CMsis
{
public:
    CMsis(const int version)
    {
        m_version = version;
        (version == 1) ? msis_init_v1() : msis_init_v2();
    }
    int midSideStereoMode = 0;
    int intensityStereoMode = 0;
    IS_SF_INFO intensityStereoScalefactorInfo;
    void setStereoModes(const int mid_side, const int intensity)
    {
        midSideStereoMode = mid_side;
        intensityStereoMode = intensity;
    }
    void midSideStereoProcess(float* x_left, float* x_right, const int n)  /* sum-difference stereo */
    {
        /*-- note: sqrt(2) done scaling by dequant ---*/
        for (int i = 0; i < n; i++)
        {
            const float xl = x_left[i] + x_right[i];
            const float xr = x_left[i] - x_right[i];
            x_left[i] = xl;
            x_right[i] = xr;
        }
    }
    void intensityStereoProcess(float* x_left, float* x_right, const int gr, CCritBands* bands)
    {
        if (m_version == 1)
        {
            intensityStereoProcess_v1(x_left,x_right,gr,bands);
        }
        else
        {
            intensityStereoProcess_v2(x_left,x_right,gr,bands);
        }
    }
private:
    int m_version = 0;
    void intensityStereoProcess_v1(float* x_left, float* x_right, const int gr, CCritBands* bands)
    {
        SCALE_FACTOR* sf_right = bands->scaleFactor(gr,1);
        CB_INFO* cb_info_right = &bands->cbInfo(gr,1)->info;
        int cb_right = cb_info_right->cbmax; /* start at end of right */
        int cb_index = bands->bandIndex(cb_info_right->cbtype,cb_right);
        cb_right++;
        int sample_countdown = bands->sampleCount(gr,0) - cb_index;  /* process to len of left */

        if (cb_info_right->cbtype == CB_INFO::long_cb)
        {
            /* long_blocks: */
            for (int cb = cb_right; cb < MAX_BANDS_LONG; cb++)
            {
                const int isf = sf_right->long_block[cb];
                const double fl = lr_v1[midSideStereoMode][isf][0];
                const double fr = lr_v1[midSideStereoMode][isf][1];
                for (int j = 0; j < bands->bandCount(CB_INFO::long_cb,cb); j++, cb_index++)
                {
                    if (--sample_countdown < 0) return;
                    x_right[cb_index] = fr * x_left[cb_index];
                    x_left[cb_index] = fl * x_left[cb_index];
                }
            }
        }
        else
        {
            //short_blocks:
            for (int cb = cb_right; cb < MAX_BANDS_SHORT; cb++)
            {
                double fls[3], frs[3];
                for (int w = 0; w < 3; w++)
                {
                    const int isf = sf_right->short_block[w][cb];
                    fls[w] = lr_v1[midSideStereoMode][isf][0];
                    frs[w] = lr_v1[midSideStereoMode][isf][1];
                }
                for (int j = 0; j < bands->bandCount(CB_INFO::short_cb,cb); j++)
                {
                    sample_countdown -= 3;
                    if (sample_countdown < 0) return;
                    for (int k = 0; k < 3; k++,cb_index++)
                    {
                        x_right[cb_index] = frs[k] * x_left[cb_index];
                        x_left[cb_index] = fls[k] * x_left[cb_index];
                    }
                }
            }
        }
    }
    void intensityStereoProcess_v2(float* x_left, float* x_right, const int gr, CCritBands* bands)
    {
        SCALE_FACTOR* sf = bands->scaleFactor(gr,1);
        CB_INFO* cb_info_left = &bands->cbInfo(gr,0)->info; // left ch
        CB_INFO* cb_info_right = &bands->cbInfo(gr,1)->info; // right ch
        int il[MAX_BANDS_LONG];
        double (*lr)[2];
        lr = lr_v2[intensityStereoScalefactorInfo.intensityScale][midSideStereoMode];

        if (cb_info_right->cbtype == CB_INFO::long_cb)
        {
            /* long_blocks: */
            const int cb_right = cb_info_right->cbmax; /* start at end of right */
            int cb_index = bands->bandIndex(CB_INFO::long_cb,cb_right);
            int sample_countdown = bands->sampleCount(gr,0) - cb_index;  /* process to len of left */
            /* gen sf info */
            intensityStereoScalefactorInfo.getSubBlocks(il);
            for (int cb = cb_right + 1; cb < MAX_BANDS_LONG; cb++)
            {
                const int isf = il[cb] + sf->long_block[cb];
                const double fl = lr[isf][0];
                const double fr = lr[isf][1];
                for (int j = 0; j < bands->bandCount(CB_INFO::long_cb,cb); j++, cb_index++)
                {
                    if (--sample_countdown < 0) return;
                    x_right[cb_index] = fr * x_left[cb_index];
                    x_left[cb_index] = fl * x_left[cb_index];
                }
            }
        }
        else
        {
            //short_blocks:
            /* gen sf info */
            intensityStereoScalefactorInfo.getSubBlocks(il);
            for (int w = 0; w < 3; w++)
            {
                const int cb_right = cb_info_right->cbmax_short[w]; /* start at end of right */
                int cb_index = bands->bandIndex(CB_INFO::short_cb,cb_right) + w;
                const int cb_left = cb_info_left->cbmax_short[w]; /* process to end of left */

                for (int cb = cb_right + 1; cb <= cb_left; cb++)
                {
                    const int isf = il[cb] + sf->short_block[w][cb];
                    const double fl = lr[isf][0];
                    const double fr = lr[isf][1];
                    for (int j = 0; j < bands->bandCount(CB_INFO::short_cb,cb); j++)
                    {
                        x_right[cb_index] = fr * x_left[cb_index];
                        x_left[cb_index] = fl * x_left[cb_index];
                        cb_index += 3;
                    }
                }
            }
        }
    }
    /* intensity stereo */
    /* if ms mode quant pre-scales all values by 1.0/sqrt(2.0) ms_mode in table
       compensates   */
    /* [ms_mode 0/1][sf][left/right]  */
    double lr_v1[2][8][2] = {{{0}}};
    /* lr2[intensity_scale][ms_mode][sflen_offset+sf][left/right] */
    double lr_v2[2][2][64][2] = {{{{0}}}};
    void msis_init_v1()
    {
        double t = M_PI / 12.0;
        for (int i = 0; i < 7; i++)
        {
            const double s = sin(i * t);
            const double c = cos(i * t);
            /* ms_mode = 0 */
            lr_v1[0][i][0] = s / (s + c);
            lr_v1[0][i][1] = c / (s + c);
            /* ms_mode = 1 */
            lr_v1[1][i][0] = M_SQRT2 * (s / (s + c));
            lr_v1[1][i][1] = M_SQRT2 * (c / (s + c));
        }
        /* sf = 7 */
        /* ms_mode = 0 */
        lr_v1[0][7][0] = 1.0;
        lr_v1[0][7][1] = 0.0;
        /* ms_mode = 1, in is bands is routine does ms processing */
        lr_v1[1][7][0] = 1.0;
        lr_v1[1][7][1] = 1.0;
    }
    /*===============================================================*/
    void msis_init_v2()
    {
        double ms_factor[2] = {0,M_SQRT2};
        /* intensity stereo MPEG2 */
        /* lr2[intensity_scale][ms_mode][sflen_offset+sf][left/right] */

        for (int intensity_scale = 0; intensity_scale < 2; intensity_scale++)
        {
            const double t = pow(2.0, -0.25 * (1 + intensity_scale));
            for (int ms_mode = 0; ms_mode < 2; ms_mode++)
            {

                int n = 1;
                int k = 0;
                for (int sflen = 0; sflen < 6; sflen++)
                {
                    for (int sf = 0; sf < (n - 1); sf++, k++)
                    {
                        if (sf == 0)
                        {
                            lr_v2[intensity_scale][ms_mode][k][0] = ms_factor[ms_mode] * 1.0;
                            lr_v2[intensity_scale][ms_mode][k][1] = ms_factor[ms_mode] * 1.0;
                        }
                        else if ((sf & 1))
                        {
                            lr_v2[intensity_scale][ms_mode][k][0] = ms_factor[ms_mode] * pow(t, (sf + 1) / 2.0);
                            lr_v2[intensity_scale][ms_mode][k][1] = ms_factor[ms_mode] * 1.0;
                        }
                        else
                        {
                            lr_v2[intensity_scale][ms_mode][k][0] = ms_factor[ms_mode] * 1.0;
                            lr_v2[intensity_scale][ms_mode][k][1] = ms_factor[ms_mode] * pow(t, sf / 2.0);
                        }
                    }

                    /* illegal is_pos used to do ms processing */
                    if (ms_mode == 0)
                    {   /* ms_mode = 0 */
                        lr_v2[intensity_scale][ms_mode][k][0] = 1.0;
                        lr_v2[intensity_scale][ms_mode][k][1] = 0.0;
                    }
                    else
                    {
                        /* ms_mode = 1, in is bands is routine does ms processing */
                        lr_v2[intensity_scale][ms_mode][k][0] = 1.0;
                        lr_v2[intensity_scale][ms_mode][k][1] = 1.0;
                    }
                    k++;
                    n += n;
                }
            }
        }
    }
};

class CSide
{
public:
    CSide(const int version)
    {
        sideInfo = new SIDE_INFO(version);
        m_version = version;
    }
    ~CSide() { delete sideInfo; }
    SIDE_INFO* sideInfo = nullptr;
    void getSideInfo(const int gr, const int channels, CBitget& bitget, CCritBands* bands, CMsis* stereo_mode)
    {
        sideInfo->getSideInfo(channels,bitget);
        if (m_version == 1)
        {
            for (int gr = 0; gr < 2; gr ++)
            {
                for (int ch = 0; ch < channels; ch ++)
                {
                    bands->granuleInfo(gr,ch)->getSideInfo(stereo_mode->midSideStereoMode,bitget);
                }
            }
        }
        else
        {
            for (int ch = 0; ch < channels; ch ++)
            {
                bands->granuleInfo(gr,ch)->getSideInfo(stereo_mode->midSideStereoMode,bitget);
            }
        }
    }
private:
    int m_version = 0;
};

class CLayer3Process
{
public:
    CLayer3Process(const int version, const int frequency_index, const int limit)
    {
        m_version = version;
        m_side = new CSide(version);
        m_msis = new CMsis(version);
        m_bands = new CCritBands(version,frequency_index,limit);
    }
    ~CLayer3Process()
    {
        delete m_msis;
        delete m_side;
        delete m_bands;
    }
    void midSideStereoProcess(const int gr)  /* sum-difference stereo */
    {
        int m0;
        if (m_msis->intensityStereoMode == 0) // process to longer of left/right
        {
            m0 = qMax<int>(sampleCount(gr,0),sampleCount(gr,1));
        }
        else // process to last cb in right
        {
            m0 = bandIndex(cbInfo(gr,1)->cbtype,cbInfo(gr,1)->cbmax);
        }
        m_msis->midSideStereoProcess(sample(gr,0),sample(gr,1),m0);
    }
    void intensityStereoProcess(const int gr) { m_msis->intensityStereoProcess(sample(gr,0),sample(gr,1),gr,m_bands); }
    void dequant(const int gr, const int ch) { m_bands->dequant(gr,ch); }
    inline GR_INFO* granuleInfo(const int gr, const int ch) { return m_bands->granuleInfo(gr,ch); }
    inline int sampleCount(const int gr, const int ch) { return m_bands->sampleCount(gr,ch); }
    inline int setSampleCount(const int gr, const int ch, const int v) { return m_bands->setSampleCount(gr,ch,v); }
    inline int getTotalBandCount(const int gr, int ch) { return m_bands->bandlimitClip(sampleCount(gr,ch)); }
    inline int getLongBandCount(const int gr, const int ch)
    {
        int c = getTotalBandCount(gr,ch);
        if (granuleInfo(gr,ch)->block_type == GR_INFO::block_type_2) // long bands
        {
            c = (granuleInfo(gr,ch)->mixedBlockFlag) ? bandIndex(CB_INFO::long_cb,m_bands->mixedBandCount - 1) : 0;
        }
        return m_bands->bandlimitClip(c);
    }
    int hybrid(const int nlong, const int ntot, const int gr, const int ch)
    {
        const int igr_prev = gr ^ 1;
        return setSampleCount(gr,ch,m_hybrid.hybrid(sample(gr,ch), sample(igr_prev,ch), granuleInfo(gr,ch)->block_type, nlong, ntot, sampleCount(igr_prev,ch),m_bands->m_band_limit));
    }
    int hybrid_sum(const int nlong, const int ntot, const int gr)
    {
        return m_hybrid.hybrid_sum(sample(gr,1), sample(gr,0), granuleInfo(gr,1)->block_type, nlong, ntot);
    }
    void freq_invert(const int sampleCount) { m_hybrid.freq_invert(sampleCount); }
    void sum_f_bands(const int igr, const int n) { m_hybrid.sum_f_bands(sample(igr,0),sample(igr,1),n); }
    uint main_data_begin() { return m_side->sideInfo->main_data_begin; }
    void huff(const int gr, const int ch, const int bit0, CBitget& bitget)
    {
        int* samples = reinterpret_cast<int*>(sample(gr,ch)); // sampleS(gr,ch);
        GR_INFO* gi = granuleInfo(gr,ch);
        const int big_values_len = m_bands->bandlimitClip(gi->big_values * 2);
        const int region0_start = m_bands->clip(bandIndex(CB_INFO::long_cb,gi->region0_count),big_values_len);
        const int region1_start = m_bands->clip(bandIndex(CB_INFO::long_cb,gi->region0_count + gi->region1_count + 1),big_values_len);
        m_huff.huffman(samples, region0_start, gi->table_select[0],bitget);
        m_huff.huffman(samples + region0_start, region1_start - region0_start, gi->table_select[1],bitget);
        m_huff.huffman(samples + region1_start, big_values_len - region1_start, gi->table_select[2],bitget);
        const int qbits = gi->part2_3_length - (bitget.bitget_bits_used() - bit0);
        int rzero_start = big_values_len + m_huff.huffman_quad(samples + big_values_len, m_bands->m_band_limit - big_values_len, qbits, gi->count1table_select,bitget);
        setSampleCount(gr,ch,rzero_start);
        // limit n4 or allow deqaunt to sf band 22
        rzero_start = (gi->block_type == GR_INFO::block_type_2) ? m_bands->bandlimitClip(rzero_start, CB_INFO::short_cb) : m_bands->bandlimitClip(rzero_start, CB_INFO::long_cb);
        if (rzero_start < m_bands->sampleSize()) zeroMemory(samples + rzero_start, sizeof(float) * (m_bands->sampleSize() - rzero_start));
        if (bitget.bitget_overrun()) zeroMemory(samples, sizeof(float) * m_bands->sampleSize());
    }
    void antialias(const int gr, const int ch)
    {
        if (cbInfo(gr,ch)->long_block_cb_end)
        {// have no long blocks
            int n1 = (granuleInfo(gr,ch)->mixedBlockFlag) ? 1 : (sampleCount(gr,ch) + 7) / IMDCTPOINTS_LONG; //must be 17!
            //int n1 = (granuleInfo(gr,ch)->mixedBlockFlag) ? 1 : CHybrid::bandCountRoundUp(sampleCount(gr,ch));
            n1 = m_bands->clip(n1,31);
            m_alias.antialias(sample(gr,ch), n1);
            n1 = 18 * n1 + 8;  // update number of samples
            if (sampleCount(gr,ch) < n1) setSampleCount(gr,ch,n1);
        }
    }
    void setStereoModes(const int mid_side, const int intensity) { m_msis->setStereoModes(mid_side,intensity); }
    int intensityStereoMode() { return m_msis->intensityStereoMode; }
    int midSideStereoMode() { return  m_msis->midSideStereoMode; }
    //Side
    void getSideInfo(const int gr, const int channels, CBitget& bitget)
    {
        m_side->getSideInfo(gr, channels, bitget, m_bands, m_msis);
    }
    void getScaleFactor(const int gr, const int ch, CBitget& bitget)
    {
        if (m_version == 1)
        {
            m_bands->scaleFactor(gr,ch)->getScaleFactor_v1(gr, granuleInfo(gr,ch), m_side->sideInfo->scfsi[ch],bitget,m_bands->scaleFactor(gr ^ 1,ch));
        }
        else
        {
            m_msis->intensityStereoScalefactorInfo = m_bands->scaleFactor(gr,ch)->getScaleFactor_v2(ch, granuleInfo(gr,ch), m_msis->intensityStereoMode,bitget);
        }
    }
    float* hybridBuffer() { return &m_hybrid.hybridBuffer[0][0]; }
private:
    int m_version = 0;
    inline float* sample(const int gr, const int ch) { return m_bands->sample(gr,ch); }
    inline int bandIndex(const CB_INFO::cbTypes ls, const uint cb) { return  m_bands->bandIndex(ls,cb); }
    inline CB_INFO* cbInfo(const int gr, const int ch) { return &m_bands->cbInfo(gr,ch)->info; }
    CCritBands* m_bands = nullptr;
    CSide* m_side = nullptr;
    CMsis* m_msis = nullptr;
    CHybrid m_hybrid;
    CHuff m_huff;
    CAlias m_alias;
};

#pragma pack(pop)

#endif // L3MSIS_H
