#ifndef L3SF_H
#define L3SF_H

#include "bstream.h"
#include <string.h>
#include "math.h"
#include "array"
#include "vector"

#pragma pack(push,1)

/*--- iSample**(4/3) lookup, -32<=i<=31 ---*/
#define ISMAX 32
#define MAX_BANDS_LONG 21
#define MAX_BANDS_SHORT 12

class GR_INFO
{
public:
    enum blockTypes {
        block_type_0,
        long_block_type_1,
        block_type_2,
        long_block_type_3
    };
    uint part2_3_length = 0;
    uint big_values = 0;
    uint global_gain = 0;
    uint scalefac_compress = 0;
    uint window_switching_flag = 0;
    blockTypes block_type = block_type_0;
    uint mixedBlockFlag = 0;
    uint table_select[3] = {0};
    uint subblock_gain[3] = {0};
    uint region0_count = 0;
    uint region1_count = 0;
    uint preflag = 0;
    uint scalefac_scale = 0;
    uint count1table_select = 0;
    GR_INFO(const int version) { m_version = version; }
    void getSideInfo(const int mid_side_stereo_mode, CBitget& bitget)
    {
        part2_3_length = bitget.bitget(12);
        big_values = bitget.bitget(9);
        global_gain = bitget.bitget(8);
        //global_gain += gain_adjust;
        if (mid_side_stereo_mode) global_gain -= 2;
        (m_version == 1) ? getSideInfo_v1(bitget) : getSideInfo_v2(bitget);
        scalefac_scale = bitget.bitget(1);
        count1table_select = bitget.bitget(1);
    }
private:
    int m_version = 0;
    void getSideInfo_v1(CBitget& bitget)
    {
        scalefac_compress = bitget.bitget(4);
        window_switching_flag = bitget.bitget(1);
        if (window_switching_flag)
        {
            winFlag(bitget);
            /* region count set in terms of long block cb's/bands */
            /* r1 set so r0+r1+1 = 21 (lookup produces 576 bands ) */
            /* if(window_switching_flag) always 36 samples in region0 */
            setRegionCount(8-1, 20-(8-1)); /* 36 samples */
        }
        else
        {
            noWinFlag(bitget);
        }
        preflag = bitget.bitget(1);
    }
    void getSideInfo_v2(CBitget& bitget)
    {
        scalefac_compress = bitget.bitget(9);
        window_switching_flag = bitget.bitget(1);
        if (window_switching_flag)
        {
            winFlag(bitget);
            /* region count set in terms of long block cb's/bands  */
            /* r1 set so r0+r1+1 = 21 (lookup produces 576 bands ) */
            /* bt=1 or 3       54 samples */
            /* bt=2 mixed=0    36 samples */
            /* bt=2 mixed=1    54 (8 long sf) samples? or maybe 36 */
            /* region0 discussion says 54 but this would mix long */
            /* and short in region0 if scale factors switch */
            /* at band 36 (6 long scale factors) */
            (block_type == block_type_2) ? setRegionCount(6-1,20-(6-1)) : setRegionCount(8-1,20-(8-1)); /* 36 samples */ /* 54 samples */
        }
        else
        {
            noWinFlag(bitget);
        }
        preflag = 0;
    }
    void winFlag(CBitget& bitget)
    {
        block_type = blockTypes(bitget.bitget(2));
        mixedBlockFlag = bitget.bitget(1);
        table_select[0] = bitget.bitget(5);
        table_select[1] = bitget.bitget(5);
        subblock_gain[0] = bitget.bitget(3);
        subblock_gain[1] = bitget.bitget(3);
        subblock_gain[2] = bitget.bitget(3);
    }
    void noWinFlag(CBitget& bitget)
    {
        mixedBlockFlag = 0;
        block_type = block_type_0;
        table_select[0] = bitget.bitget(5);
        table_select[1] = bitget.bitget(5);
        table_select[2] = bitget.bitget(5);
        region0_count = bitget.bitget(4);
        region1_count = bitget.bitget(3);
    }
    void setRegionCount(const int r0, const int r1)
    {
        region0_count = r0;
        region1_count = r1;
    }
};

class CCritBands;
class CCritBandChannel;

class CB_INFO
{
public:
    enum cbTypes
    {
        long_cb,
        short_cb
    };
    CB_INFO() { }
    cbTypes cbtype = long_cb; /* long=0 short=1 */
    int cbmax = 0; /* max crit band */
    GR_INFO::blockTypes lb_type = GR_INFO::block_type_0; /* long block type 0 1 3 */
    int short_block_cb_start = 0; /* short band start index 0 3 12 (12=no shorts */
    int long_block_cb_end = 0; /* number long cb's 0 8 21 */
    int cbmax_short[3] = {0}; /* cbmax by individual short blocks */
    void init(GR_INFO* gr_info, const int mixed_band_count)
    {
        long_block_cb_end = MAX_BANDS_LONG + 1;   /* long block cb end */
        short_block_cb_start = 12;   /* short block cb start */
        /* ncbl_mixed = 8 or 6  mpeg1 or 2 */
        if (gr_info->block_type == GR_INFO::block_type_2)
        {
            long_block_cb_end = 0;
            short_block_cb_start = 0;
            if (gr_info->mixedBlockFlag)
            {
                long_block_cb_end = mixed_band_count;
                short_block_cb_start = 3;
            }
        }
        lb_type = gr_info->block_type; // fill in cb_info -- //
        //if (gr_info.block_type == 2) ?????
        //this->lb_type;
        //this->cbs0 = cbs0;
        //this->ncbl = ncbl;
    }
    void init_cbmax_short(const int v)
    {
        for (int i = 0; i < 3; i++) cbmax_short[i] = v;
    }
    void init_cbmax_tot(const cbTypes type)
    {
        if (cbmax_short[1] > cbmax_short[0]) cbmax_short[0] = cbmax_short[1];
        if (cbmax_short[2] > cbmax_short[0]) cbmax_short[0] = cbmax_short[2];

        cbmax = cbmax_short[0];
        cbtype = type;
    }
};

class CCritBandInfo{
public:
#define GLOBAL_GAIN_SCALE (4*15)
    CCritBandInfo()
    {
        /* 8 bit plus 2 lookup x = pow(2.0, 0.25*(global_gain-210)) */
        /* extra 2 for ms scaling by 1/sqrt(2) */
        /* extra 4 for cvt to mono scaling by 1/2 */
        for (int i = 0; i < 256 + 2 + 4; i++) look_global[i] = pow(2.0, 0.25 * ((i - (2 + 4)) - 210 + GLOBAL_GAIN_SCALE));

        /* x = pow(2.0, -0.5*(1+scalefact_scale)*scalefac + preemp) */
        for (int scalefact_scale = 0; scalefact_scale < 2; scalefact_scale++)
        {
            for (int preemp = 0; preemp < 4; preemp++)
            {
                for (int scalefac = 0; scalefac < 32; scalefac++)
                {
                    look_scale[scalefact_scale][preemp][scalefac] = pow(2.0, -0.5 * (1 + scalefact_scale) * (scalefac + preemp));
                }
            }
        }
        /*--- iSample**(4/3) lookup, -32<=i<=31 ---*/
        for (int i = 0; i < 2 * ISMAX; i++)
        {
            const double tmp = i - ISMAX;
            look_pow[i] = tmp * pow(fabs(tmp), (1.0 / 3.0));
        }
        /*-- pow(2.0, -0.25*8.0*subblock_gain)  3 bits --*/
        for (int i = 0; i < 8; i++) look_subblock[i] = pow(2.0, 0.25 * -8.0 * i);
        // quant_init_sf_band(sr_index);   replaced by code in sup.c
    }
    CB_INFO info;
    void dequant(CCritBandChannel* channel, CCritBands* bands);
private:
    inline bool reorderBand(const int *in, float* out, const int n, int& i, const double xs)
    {
        bool non_zero = false;
        for (int j = 0; j < n; j++, i++)
        {
            if (in[i] == 0)
            {
                out[j] = 0.0F;
            }
            else
            {
                non_zero = true;
                if ((in[i] >= (-ISMAX)) && (in[i] < ISMAX))
                {
                    out[j] = xs * look_pow[ISMAX + in[i]];
                }
                else
                {
                    const double tmp = in[i];
                    out[j] = xs * tmp * pow(fabs(tmp), (1.0 / 3.0));
                }
            }
        }
        return non_zero;
    }
    /* 8 bit plus 2 lookup x = pow(2.0, 0.25*(global_gain-210)) */
    /* two extra slots to do 1/sqrt(2) scaling for ms */
    /* 4 extra slots to do 1/2 scaling for cvt to mono */
    double look_global[256 + 2 + 4];
    /*-------- scaling lookup
   x = pow(2.0, -0.5*(1+scalefact_scale)*scalefac + preemp)
   look_scale[scalefact_scale][preemp][scalefac]
   -----------------------*/
    double look_scale[2][4][ISMAX] = {{{0}}};
    double look_pow[2 * ISMAX] = {0};
    /*-- pow(2.0, -0.25*8.0*subblock_gain) --*/
    double look_subblock[8] = {0};
    /*-- reorder buffer ---*/
    float re_buf[192][3] = {{0}};
};

struct IS_SF_SUBBLOCK
{
    int nr;
    int slen;
};

class IS_SF_INFO{
public:
    IS_SF_INFO() { }
    int intensityScale = 0;
    inline void getSubBlocks(int* il)
    {
        int k = 0;
        for (int r = 0; r < 3; r++)
        {
            const int tmp = (1 << sub_block[r].slen) - 1;
            for (int j = 0; j < sub_block[r].nr; j++, k++) il[k] = tmp;
        }
    }
    inline void fillSubBlocks(const int* nr, const int* slen)
    {
        for (uint i = 0; i < 3; i++)
        {
            sub_block[i].nr = nr[i];
            sub_block[i].slen = slen[i];
        }
    }
private:
    IS_SF_SUBBLOCK sub_block[3] = {{0,0}};
};

class SCALE_FACTOR {
public:
    SCALE_FACTOR() {}
    int long_block[MAX_BANDS_LONG + 1] = {0};   /* [cb] */
    int short_block[3][MAX_BANDS_SHORT + 1] = {{0}};  /* [window][cb] */
    void getScaleFactor_v1(const int gr, GR_INFO* grdat, const uint scfsi, CBitget& bitget,SCALE_FACTOR* prev);
    IS_SF_INFO getScaleFactor_v2(const int ch, GR_INFO* grdat, int const intensity_stereo_mode, CBitget& bitget);
private:
    inline void bitget_short(const int sfb, const int slen, CBitget& bitget)
    {
        short_block[0][sfb] = bitget.bitget(slen);
        short_block[1][sfb] = bitget.bitget(slen);
        short_block[2][sfb] = bitget.bitget(slen);
    }
    inline void zero_short(const int sfb)
    {
        short_block[0][sfb] = 0;
        short_block[1][sfb] = 0;
        short_block[2][sfb] = 0;
    }
    inline void bitget_short_block(int& sfb, const int slen, const int end, CBitget& bitget)
    {
        while (sfb < end) bitget_short(sfb++,slen, bitget);
    }
    inline void zero_short_block(int& sfb, const int end)
    {
        while (sfb < end) zero_short(sfb++);
    }
    inline void bitget_long_block(int& sfb, const int slen, const int end, CBitget& bitget)
    {
        while (sfb < end) long_block[sfb++] = bitget.bitget(slen);
    }
    inline void zero_long_block(int& sfb, const int end)
    {
        while (sfb < end) long_block[sfb++] = 0;
    }
    inline void prev_granule_long_block(int& sfb, const int end, SCALE_FACTOR* prev)
    {
        while (sfb < end)
        {
            long_block[sfb] = prev->long_block[sfb];
            sfb++;
        }
    }
};

class CCritBand
{
public:
    CCritBand() { }
    int nBand = 0;
    int scaleFactor_bandIndex = 0;
};

#define MAX_BANDS 576

class CCritBandChannel
{
public:
    CCritBandChannel(const int version)
    {
        m_version = version;
        granule_info = new GR_INFO(version);
    }
    ~CCritBandChannel() { delete granule_info; }
    void dequant(CCritBands* bands) { cb_info.dequant(this,bands); }
    int sampleCount = 0;
    float sample[MAX_BANDS] = {0};
    GR_INFO* granule_info;
    SCALE_FACTOR scale_factor;
    CCritBandInfo cb_info;
private:
    int m_version = 0;
};

class CCritBandGranule
{
public:
    CCritBandGranule(const int version)
    {
        for (int i = 0; i < 2; i++) m_Channel[i] = new CCritBandChannel(version);
    }
    ~CCritBandGranule()
    {
        for (int i = 0; i < 2; i++) delete m_Channel[i];
    }
    inline SCALE_FACTOR* scaleFactor(const int ch) { return  &m_Channel[ch]->scale_factor; }
    inline GR_INFO* granuleInfo(const int ch) { return m_Channel[ch]->granule_info; }
    inline CCritBandInfo* cb_info(const int ch) { return &m_Channel[ch]->cb_info; }
    inline int sampleCount(const int ch) { return m_Channel[ch]->sampleCount; }
    inline int setSampleCount(const int ch, const int v) { return m_Channel[ch]->sampleCount = v; }
    inline float* sample(const int ch) { return m_Channel[ch]->sample; }
    inline CCritBandChannel* channel(const int ch) { return m_Channel[ch]; }
private:
    CCritBandChannel* m_Channel[2];
};

struct sfBandTable {
    int long_band[MAX_BANDS_LONG + 2];
    int short_band[MAX_BANDS_SHORT + 2];
};

class CCritBandType
{
public:
    CCritBandType(const CB_INFO::cbTypes ls, const sfBandTable bt)
    {
        cbType = ls;
        (ls == CB_INFO::long_cb) ? initLong(bt) : initShort(bt);
    }
    inline int bandIndex(const uint cb) { return bands[cb].scaleFactor_bandIndex; }
    inline int bandCount(const uint cb) { return bands[cb].nBand; }
    inline int bandLimitClip(const int n) { return (n > bandLimit) ? bandLimit : n; }
    int bandLimit = 0;
    CB_INFO::cbTypes cbType = CB_INFO::long_cb;
private:
    std::vector<CCritBand> bands;
    void initLong(const sfBandTable bt)
    {
        bands.resize(MAX_BANDS_LONG + 1);
        for (uint i = 0; i < MAX_BANDS_LONG + 1; i ++)
        {
            bands[i].scaleFactor_bandIndex = bt.long_band[i + 1];
            bands[i].nBand = bt.long_band[i + 1] - bt.long_band[i];
        }
    }
    void initShort(const sfBandTable bt)
    {
        bands.resize(MAX_BANDS_SHORT + 1);
        for (uint i = 0; i < MAX_BANDS_SHORT + 1; i ++)
        {
            bands[i].scaleFactor_bandIndex = 3 * bt.short_band[i + 1];
            bands[i].nBand = bt.short_band[i + 1] - bt.short_band[i];
        }
    }
};

class CCritBands
{
public:
    CCritBands(const int version, const int frequency_index, const int limit)
    {
        const sfBandTable bt=selectBandTable(version,frequency_index);
        for (int i = 0; i < 2; i++) m_Granule[i] = new CCritBandGranule(version);
        for (int i = 0; i < 2; i++) m_Type[i] = new CCritBandType(CB_INFO::cbTypes(i),bt);
        mixedBandCount = (version == 1) ? 8 : 6;
        if (version == 1) //MPEG-1         // init limits
        {
            m_Type[CB_INFO::short_cb]->bandLimit = 3 * bt.short_band[MAX_BANDS_SHORT + 1];
            m_band_limit = m_Type[CB_INFO::long_cb]->bandLimit = bt.long_band[MAX_BANDS_LONG + 1];
        }
        else //MPEG-2, MPEG-2.5
        {
            m_Type[CB_INFO::short_cb]->bandLimit = 3 * bt.short_band[MAX_BANDS_SHORT];
            m_band_limit = m_Type[CB_INFO::long_cb]->bandLimit = bt.long_band[MAX_BANDS_LONG];
        }
        m_band_limit += 8; // allow for antialias
        m_band_limit = clip(m_band_limit,limit);
        for (int i = 0; i < 2; i++) m_Type[i]->bandLimit = bandlimitClip(m_Type[i]->bandLimit);
    }
    ~CCritBands()
    {
        for (int i = 0; i < 2; i++) delete m_Granule[i];
    }
    int mixedBandCount;
    int m_band_limit;
    inline int bandLimit(CB_INFO::cbTypes ls) { return m_Type[ls]->bandLimit; }
    inline int clip(int n, const int limit) { return (n > limit) ? limit : n; }
    inline int bandlimitClip(const int n) { return clip(n,m_band_limit); }
    inline int bandlimitClip(const int n, const CB_INFO::cbTypes ls) { return m_Type[ls]->bandLimitClip(n); }
    inline int bandIndex(const CB_INFO::cbTypes ls, const uint cb) { return  m_Type[ls]->bandIndex(cb); }
    inline int bandCount(const CB_INFO::cbTypes ls, const uint cb) { return  m_Type[ls]->bandCount(cb); }
    inline SCALE_FACTOR* scaleFactor(const int gr, const int ch) { return  m_Granule[gr]->scaleFactor(ch); }
    inline GR_INFO* granuleInfo(const int gr, const int ch) { return m_Granule[gr]->granuleInfo(ch); }
    inline CCritBandInfo* cbInfo(const int gr, const int ch) { return m_Granule[gr]->cb_info(ch); }
    inline int sampleCount(const int gr, const int ch) { return m_Granule[gr]->sampleCount(ch); }
    inline int setSampleCount(const int gr, const int ch, const int v) { return m_Granule[gr]->setSampleCount(ch,v); }
    inline float* sample(const int gr, const int ch) { return m_Granule[gr]->sample(ch); }
    inline int sampleSize() { return MAX_BANDS; }
    inline CCritBandChannel* channel(const int gr, const int ch) { return m_Granule[gr]->channel(ch); }
    inline CCritBandGranule* granule(const int gr) { return m_Granule[gr]; }
    void dequant(const int gr, const int ch) { channel(gr,ch)->dequant(this); }
private:
    CCritBandGranule* m_Granule[2]; //gr
    CCritBandType* m_Type[2]; // long short
    sfBandTable selectBandTable(int version, int frequency_index)
    {
        static const sfBandTable bandTable[3][3] = {
            // MPEG-1
            {{
                 {0, 4, 8, 12, 16, 20, 24, 30, 36, 44, 52, 62, 74, 90, 110, 134, 162, 196, 238, 288, 342, 418, 576},
                 {0, 4, 8, 12, 16, 22, 30, 40, 52, 66, 84, 106, 136, 192}
             },{
                 {0, 4, 8, 12, 16, 20, 24, 30, 36, 42, 50, 60, 72, 88, 106, 128, 156, 190, 230, 276, 330, 384, 576},
                 {0, 4, 8, 12, 16, 22, 28, 38, 50, 64, 80, 100, 126, 192}
             },{
                 {0, 4, 8, 12, 16, 20, 24, 30, 36, 44, 54, 66, 82, 102, 126, 156, 194, 240, 296, 364, 448, 550, 576},
                 {0, 4, 8, 12, 16, 22, 30, 42, 58, 78, 104, 138, 180, 192}
             }},
            // MPEG-2
            {{
                 {0, 6, 12, 18, 24, 30, 36, 44, 54, 66, 80, 96, 116, 140, 168, 200, 238, 284, 336, 396, 464, 522, 576},
                 {0, 4, 8, 12, 18, 24, 32, 42, 56, 74, 100, 132, 174, 192}
             },{
                 {0, 6, 12, 18, 24, 30, 36, 44, 54, 66, 80, 96, 114, 136, 162, 194, 232, 278, 332, 394, 464, 540, 576},
                 {0, 4, 8, 12, 18, 26, 36, 48, 62, 80, 104, 136, 180, 192}
             },{
                 {0, 6, 12, 18, 24, 30, 36, 44, 54, 66, 80, 96, 116, 140, 168, 200, 238, 284, 336, 396, 464, 522, 576},
                 {0, 4, 8, 12, 18, 26, 36, 48, 62, 80, 104, 134, 174, 192}
             }},
            // MPEG-2.5, 11 & 12 KHz seem ok, 8 ok
            {{
                 {0, 6, 12, 18, 24, 30, 36, 44, 54, 66, 80, 96, 116, 140, 168, 200, 238, 284, 336, 396, 464, 522, 576},
                 {0, 4, 8, 12, 18, 26, 36, 48, 62, 80, 104, 134, 174, 192}
             },{
                 {0, 6, 12, 18, 24, 30, 36, 44, 54, 66, 80, 96, 116, 140, 168, 200, 238, 284, 336, 396, 464, 522, 576},
                 {0, 4, 8, 12, 18, 26, 36, 48, 62, 80, 104, 134, 174, 192}
             },{
                 // this 8khz table, and only 8khz, from mpeg123)
                 {0, 12, 24, 36, 48, 60, 72, 88, 108, 132, 160, 192, 232, 280, 336, 400, 476, 566, 568, 570, 572, 574, 576},
                 {0, 8, 16, 24, 36, 52, 72, 96, 124, 160, 162, 164, 166, 192}
             }},
        };
        return bandTable[version - 1][frequency_index];
    }
};

#pragma pack(pop)

#endif // L3SF_H
