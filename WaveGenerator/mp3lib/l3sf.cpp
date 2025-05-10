#include "l3sf.h"
//#include "math.h"

void CCritBandInfo::dequant(CCritBandChannel* channel, CCritBands* bands)
{
    static const int pretab[2][MAX_BANDS_LONG + 1] =
    {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 3, 3, 3, 2, 0},
    };
    float* sample = channel->sample;
    auto sampleStart = reinterpret_cast<const int*>(sample);
    info.init(channel->granule_info,bands->mixedBandCount);
    /* global gain pre-adjusted by 2 if ms_mode, 0 otherwise */
    double x_start = look_global[(2 + 4) + channel->granule_info->global_gain];
    int block_index = 0;
    if (info.long_block_cb_end > 0) //----- long blocks ---//
    {
        info.init_cbmax_short(0);
        for (int cb = 0; cb < info.long_block_cb_end; cb++)
        {
            const double xs = x_start * look_scale[channel->granule_info->scalefac_scale][pretab[channel->granule_info->preflag][cb]][channel->scale_factor.long_block[cb]];
            const int n = bands->bandCount(CB_INFO::long_cb,cb);
            if (reorderBand(sampleStart,&sample[block_index],n,block_index,xs)) info.cbmax_short[0] = cb;
            if (block_index >= channel->sampleCount) break;
        }
        info.init_cbmax_tot(CB_INFO::long_cb); // type = long
    }
    if (info.short_block_cb_start < 12) //----- block type = 2  short blocks ---//
    {
        info.init_cbmax_short(info.short_block_cb_start);
        int short_block_index_start = block_index;   /* save for reorder */
        int bufIndex = 0;
        double x_short_block[3];
        for (int w = 0; w < 3; w++) x_short_block[w] = x_start * look_subblock[channel->granule_info->subblock_gain[w]];
        for (int cb = info.short_block_cb_start; cb < MAX_BANDS_SHORT + 1; cb++)
        {
            const int n = bands->bandCount(CB_INFO::short_cb,cb);
            for (int w = 0; w < 3; w++)
            {
                const double xs = x_short_block[w] * look_scale[channel->granule_info->scalefac_scale][0][channel->scale_factor.short_block[w][cb]];
                if (reorderBand(sampleStart,&re_buf[bufIndex][w],n,block_index,xs)) info.cbmax_short[w] = cb;
            }
            if (block_index >= channel->sampleCount) break;
            bufIndex += n;
        }
        copyMemory(&sample[short_block_index_start], re_buf, sizeof(float) * (block_index - short_block_index_start));
        channel->sampleCount = block_index; /* update nsamp */
        info.init_cbmax_tot(CB_INFO::short_cb);  /* type = short */
    }
}

void SCALE_FACTOR::getScaleFactor_v1(const int gr, GR_INFO* grdat, const uint scfsi, CBitget& bitget, SCALE_FACTOR* prev)
{
    static const int slen_table[16][2] =
    {
        {0, 0}, {0, 1},
        {0, 2}, {0, 3},
        {3, 0}, {1, 1},
        {1, 2}, {1, 3},
        {2, 1}, {2, 2},
        {2, 3}, {3, 1},
        {3, 2}, {3, 3},
        {4, 2}, {4, 3}
    };
    int sfb;
    const int* slen = slen_table[grdat->scalefac_compress];

    if (grdat->block_type == GR_INFO::block_type_2)
    {
        if (grdat->mixedBlockFlag)
        {    /* mixed */
            bitget_long_block(sfb,slen[0],8,bitget);
            sfb = 3;
            bitget_short_block(sfb,slen[0],6,bitget);
            bitget_short_block(sfb,slen[1],12,bitget);
            return;
        }
        sfb = 0;
        bitget_short_block(sfb,slen[0],6,bitget);
        bitget_short_block(sfb,slen[1],12,bitget);
        return;
    }
    /* long blocks types 0 1 3, first granule */
    if (gr == 0)
    {
        sfb = 0;
        bitget_long_block(sfb,slen[0],11,bitget);
        bitget_long_block(sfb,slen[1],21,bitget);
        return;
    }
    /* long blocks 0, 1, 3, second granule */
    sfb = 0;
    (scfsi & 8) ? prev_granule_long_block(sfb,6,prev) : bitget_long_block(sfb,slen[0],6,bitget);
    (scfsi & 4) ? prev_granule_long_block(sfb,11,prev) : bitget_long_block(sfb,slen[0],11,bitget);
    (scfsi & 2) ? prev_granule_long_block(sfb,16,prev) : bitget_long_block(sfb,slen[1],16,bitget);
    (scfsi & 1) ? prev_granule_long_block(sfb,21,prev) : bitget_long_block(sfb,slen[1],21,bitget);
}

IS_SF_INFO SCALE_FACTOR::getScaleFactor_v2(const int ch, GR_INFO* grdat, const int intensity_stereo_mode, CBitget& bitget)
{
    /* nr_table[size+3*is_right][block type 0,1,3  2, 2+mixed][4]  */
    /* for bt=2 nr is count for group of 3 */
    static const int nr_table[6][3][4] =
    {
        {{6, 5, 5, 5},
         {3, 3, 3, 3},
         {6, 3, 3, 3}},
        {{6, 5, 7, 3},
         {3, 3, 4, 2},
         {6, 3, 4, 2}},
        {{11, 10, 0, 0},
         {6, 6, 0, 0},
         {6, 3, 6, 0}},   /* adjusted *//* 15, 18, 0, 0,   */
        /*-intensity stereo right chan--*/
        {{7, 7, 7, 0},
         {4, 4, 4, 0},
         {6, 5, 4, 0}},
        {{6, 6, 6, 3},
         {4, 3, 3, 2},
         {6, 4, 3, 2}},
        {{8, 8, 5, 0},
         {5, 4, 3, 0},
         {6, 6, 3, 0}}
    };
    IS_SF_INFO is_info;

    int is_and_ch = intensity_stereo_mode & ch;

    int sfb;
    int slen[4];
    int k;
    uint preflag = 0;
    int intensity_scale = 0;
    uint scalefac_compress = grdat->scalefac_compress;

    if (is_and_ch == 0)
    {
        if (scalefac_compress < 400)
        {
            slen[1] = scalefac_compress >> 4;
            slen[0] = slen[1] / 5;
            slen[1] = slen[1] % 5;
            slen[3] = scalefac_compress & 15;
            slen[2] = slen[3] >> 2;
            slen[3] = slen[3] & 3;
            k = 0;
        }
        else if (scalefac_compress < 500)
        {
            scalefac_compress -= 400;
            slen[1] = scalefac_compress >> 2;
            slen[0] = slen[1] / 5;
            slen[1] = slen[1] % 5;
            slen[2] = scalefac_compress & 3;
            slen[3] = 0;
            k = 1;
        }
        else
        {
            scalefac_compress -= 500;
            slen[0] = scalefac_compress / 3;
            slen[1] = scalefac_compress % 3;
            slen[2] = slen[3] = 0;
            if (grdat->mixedBlockFlag)
            {
                slen[2] = slen[1]; /* adjust for long/short mix logic */
                slen[1] = slen[0];
            }
            preflag = 1;
            k = 2;
        }
    }
    else
    {    /* intensity stereo ch = 1 (right) */
        intensity_scale = scalefac_compress & 1;
        scalefac_compress >>= 1;
        if (scalefac_compress < 180)
        {
            slen[0] = scalefac_compress / 36;
            slen[1] = scalefac_compress % 36;
            slen[2] = slen[1] % 6;
            slen[1] = slen[1] / 6;
            slen[3] = 0;
            k = 3 + 0;
        }
        else if (scalefac_compress < 244)
        {
            scalefac_compress -= 180;
            slen[2] = scalefac_compress & 3;
            scalefac_compress >>= 2;
            slen[1] = scalefac_compress & 3;
            slen[0] = scalefac_compress >> 2;
            slen[3] = 0;
            k = 3 + 1;
        }
        else
        {
            scalefac_compress -= 244;
            slen[0] = scalefac_compress / 3;
            slen[1] = scalefac_compress % 3;
            slen[2] = slen[3] = 0;
            k = 3 + 2;
        }
    }

    int i = (grdat->block_type == GR_INFO::block_type_2) ? (grdat->mixedBlockFlag & 1) + 1 : 0;
    const int* nr = nr_table[k][i];

    /* return is scale factor info (for right chan is mode) */
    if (is_and_ch)
    {
        is_info.fillSubBlocks(nr,slen);
        is_info.intensityScale = intensity_scale;
    }
    grdat->preflag = preflag; /* return preflag */

    /*--------------------------------------*/
    if (grdat->block_type == GR_INFO::block_type_2)
    {
        if (grdat->mixedBlockFlag)
        {    /* mixed */
            sfb = 0; /* long block portion */
            (slen[0] != 0) ? bitget_long_block(sfb,slen[0],6,bitget) : zero_long_block(sfb,6);
            sfb = 3;  /* start sfb for short */
        }
        else
        {    /* all short, initial short blocks */
            sfb = 0;
            (slen[0] != 0) ? bitget_short_block(sfb,slen[0],sfb + nr[0],bitget) : zero_short_block(sfb,sfb + nr[0]);
        }
        /* remaining short blocks */
        for (int i = 1; i < 4; i++)
        {
            (slen[i] != 0) ? bitget_short_block(sfb,slen[i],sfb + nr[i],bitget) : zero_short_block(sfb,sfb + nr[i]);
        }
        return is_info;
    }
    /* long blocks types 0 1 3 */
    sfb = 0;
    for (int i = 0; i < 4; i++)
    {
        (slen[i] != 0) ? bitget_long_block(sfb,slen[i],sfb + nr[i],bitget) : zero_long_block(sfb,sfb + nr[i]);
    }
    return is_info;
}


