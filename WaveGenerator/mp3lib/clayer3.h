#ifndef CLAYER3_H
#define CLAYER3_H

#include "imp3layer.h"
#include "l3msis.h"
#include "QDebug"

#pragma pack(push,1)

#define NBUF (2*1024)
//#define BUF_TRIGGER (NBUF-1500)

class CXForm
{
public:
    enum XFormProc
    {
        xform_mono,
        xform_dual,
        xform_dual_mono,
        xform_dual_left,
        xform_dual_right,
    };
    CXForm(const int xfi, const int sbtCount)
    {
        m_XFormIndex = xfi;
        m_sbtCount = sbtCount;
    }
    void xform(short *pcm, const int igr, const int channels, CSbt* sbt, CLayer3Process* l3process)
    {
        switch (m_XFormIndex)
        {
        case xform_mono:
        case xform_dual_left:
            xFormChannel(pcm,igr,0,sbt,l3process);
            break;
        case xform_dual:
            for (int ch = 0; ch < channels; ch++) xFormChannel(pcm,igr,ch,sbt,l3process);
            break;
        case xform_dual_right:
            xFormChannel(pcm,igr,1,sbt,l3process);
            break;
        case xform_dual_mono:
            int sampleCount;
            int longBandCountL = l3process->getLongBandCount(igr,0);
            int totalBandCountL = l3process->getTotalBandCount(igr,0);
            int longBandCountR = l3process->getLongBandCount(igr,1);
            int totalBandCountR = l3process->getTotalBandCount(igr,1);
            if ((l3process->granuleInfo(igr,0)->block_type == l3process->granuleInfo(igr,1)->block_type)
                    && (l3process->granuleInfo(igr,0)->mixedBlockFlag == 0)
                    && (l3process->granuleInfo(igr,1)->mixedBlockFlag == 0))
            {
                const int longBandCount = qMax<int>(longBandCountL,longBandCountR);
                const int totalBandCount = qMax<int>(totalBandCountL,totalBandCountR);
                l3process->sum_f_bands(igr, totalBandCount);
                sampleCount = l3process->hybrid(longBandCount,totalBandCount,igr,0);
            }
            else // transform and then sum (not tested - never happens in test)
            {
                // left chan
                sampleCount = l3process->hybrid(longBandCountL,totalBandCountL,igr,0);
                // right chan
                l3process->setSampleCount(igr,1,l3process->hybrid_sum(longBandCountR, totalBandCountR, igr));
                if (sampleCount < l3process->sampleCount(igr,1)) longBandCountR = l3process->sampleCount(igr,1); //??? n3
            }
            l3process->freq_invert(sampleCount);
            sbt->sbt(l3process->hybridBuffer(),pcm,m_sbtCount,0);
            break;
        }
    }
private:
    int m_XFormIndex;
    int m_sbtCount;
    void xFormChannel(short *pcm, const int igr, const int ch ,CSbt* sbt, CLayer3Process* l3process) // hybrid + sbt
    {
        const int longBandCount = l3process->getLongBandCount(igr,ch);
        const int totalBandCount = l3process->getTotalBandCount(igr,ch);
        l3process->hybrid(longBandCount,totalBandCount,igr,ch);
        l3process->freq_invert(l3process->sampleCount(igr,ch));
        sbt->sbt(l3process->hybridBuffer(),pcm,m_sbtCount,ch);
    }
};

class CLayer3 : public IMP3Layer
{
public:
    CLayer3(const MPEG_HEADER* h) : IMP3Layer(h)
    {
        sbtCount = 18;
        frameSlotCount = 144;
        frameSlotSize = 1;
    }
    ~CLayer3();
    int decode_start()
    {
        //qDebug() << CurrentFrameInfo.bitRate << CurrentFrameInfo.bitsPerSample << CurrentFrameInfo.channels << CurrentFrameInfo.dataSize << CurrentFrameInfo.frequency << CurrentFrameInfo.maxInputSize << CurrentFrameInfo.minInputSize << CurrentFrameInfo.outputSize << CurrentFrameInfo.frames << CurrentFrameInfo.skipSize;
        m_bufsz_prev = 0;
        m_gr = 0;
        setSubbandCountLimit();
        /*
    gain_adjust = 0; // adjust gain e.g. cvt to mono sum channel
    if ((h.mode != 3) && (m_option.convert == 1))
        gain_adjust = -4;
    */
        /*
    //already done!
    m_channels = (h.mode == 3) ? 1 : 2;
    int channels = m_channels;
    if (m_option.convert) channels = 1;
    */
        l3process = new CLayer3Process(CurrentFrameHeader.version,CurrentFrameHeader.frequency_index,18 * m_subBand_count_limit); // init band tables
        Subband_transform = new CSbt(CurrentFrameHeader.layer,m_option.reduction,m_option.convert,CurrentFrameInfo.channels);
        const int k = (CurrentFrameHeader.mode != MPEG_HEADER::Single_channel) ? (1 + m_option.convert) : 0;
        xform = new CXForm(k,sbtCount);
        /*
    if (bit_code) zero_level_pcm = 128;// 8 bit output
    else zero_level_pcm = 0;
    */
        return 1;
    }
    void decode_frame(byte* mpeg, short* pcm)
    {
        if (CurrentFrameHeader.mode == MPEG_HEADER::Joint_stereo)
        {
            l3process->setStereoModes(CurrentFrameHeader.mode_ext.stereoMode >> 1,CurrentFrameHeader.mode_ext.stereoMode & 1);
        }
        else
        {
            l3process->setStereoModes(0,0);
        }
        CBitget bitget;
        bitget_init(bitget,mpeg);
        l3process->getSideInfo(m_gr,CurrentFrameInfo.channels,bitget);
        copyMemory(m_buf, m_buf + (m_bufsz_prev - l3process->main_data_begin()), l3process->main_data_begin()); // decode start point //

        const uint skip_size = 4 + getCRCSize() + getSideSize(CurrentFrameInfo.channels);
        const uint copy_size = CurrentFrameInfo.frameSize - skip_size;
        copyMemory(m_buf + l3process->main_data_begin(), mpeg + skip_size, copy_size);
        m_bufsz_prev = copy_size + l3process->main_data_begin();

        int main_pos_bit = 0; //buf_ptr0 << 3;
        if (CurrentFrameHeader.version == MPEG_HEADER::mpeg_version_1)
        {
            L3decode_main(pcm, 0, main_pos_bit);
            L3decode_main(pcm + (CurrentFrameInfo.outputSize / 4), 1, main_pos_bit);
        }
        else
        {
            L3decode_main(pcm, m_gr, main_pos_bit);
            m_gr = m_gr ^ 1;
        }
    }
    uint getSideSize(const int channels)
    {
        return SIDE_INFO::getSideSize(CurrentFrameHeader.version,channels);
    }
    int calcFrameSize()
    {
        const int i = (CurrentFrameHeader.version == MPEG_HEADER::mpeg_version_1) ? 1 : 2;
        return sizeCalc() / i + CurrentFrameHeader.padding;
    }
    void calcIOSizes()
    {
        MPEG_DECODE_INFO& info=CurrentFrameInfo;
        const int i = (CurrentFrameHeader.version == MPEG_HEADER::mpeg_version_1) ? 1 : 2;
        info.outputSize = ((sbtCount * 64) >> m_option.reduction) / i;
        /*
        if (!info.vbr)
        {
            info.skipSize = 0;
            info.minInputSize = sizeCalc() / i;
            info.maxInputSize = info.minInputSize + 1;
        }
        else
        {
            info.skipSize = sizeCalc() / i + CurrentFrameHeader.padding;
            info.minInputSize = minSizeCalc() / i;
            info.maxInputSize = maxSizeCalc() / i + 1;
        }
        */
    }
    void decode_reset()
    {
        m_bufsz_prev = 0;
    }
private:
    uint m_bufsz_prev;
    int m_gr;
    byte m_buf[NBUF];
    void L3decode_main(short *pcm, const int gr, int& main_pos_bit)
    {
        for (int ch = 0; ch < CurrentFrameInfo.channels; ch ++)
        {
            CBitget bitget(m_buf + (main_pos_bit >> 3));
            const int bit0 = (main_pos_bit & 7);
            if (bit0) bitget.bitget(bit0);
            main_pos_bit += l3process->granuleInfo(gr,ch)->part2_3_length;
            bitget.bitget_init_end(m_buf + ((main_pos_bit + 39) >> 3));
            l3process->getScaleFactor(gr, ch, bitget); // scale factors
            l3process->huff(gr,ch,bit0,bitget); // huff data
        }
        for (int ch = 0; ch < CurrentFrameInfo.channels; ch++) l3process->dequant(gr, ch); // dequant
        if (l3process->midSideStereoMode()) l3process->midSideStereoProcess(gr); // ms stereo processing
        if (l3process->intensityStereoMode()) l3process->intensityStereoProcess(gr); // is stereo processing
        if (l3process->midSideStereoMode() || l3process->intensityStereoMode()) // adjust ms and is modes to max of left/right
        {
            const int m0 = qMax<int>(l3process->sampleCount(gr,0),l3process->sampleCount(gr,1));
            for (int ch = 0; ch < CurrentFrameInfo.channels; ch ++) l3process->setSampleCount(gr,ch,m0);
        }
        for (int ch = 0; ch < CurrentFrameInfo.channels; ch ++) l3process->antialias(gr, ch); // antialias
        xform->xform(pcm,gr,CurrentFrameInfo.channels,Subband_transform,l3process); // hybrid + sbt
    }
    CLayer3Process* l3process = nullptr;
    CXForm* xform = nullptr;
};

#pragma pack(pop)

#endif // CLAYER3_H
