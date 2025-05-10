#ifndef IMP3LAYER_H
#define IMP3LAYER_H

#include "string.h"
#include "sbt.h"
#include "math.h"
#include "bstream.h"
#include "QDebug"
#include "array"
#include "qendian.h"

#pragma pack(push,1)

class MPEG_HEADER
{
public:
    enum mpegVersions : int
    {
        invalidVersion,
        mpeg_version_1,
        mpeg_version_2,
        mpeg_version_2_5
    };
    enum mpegLayers : int
    {
        invalidLayer,
        layer_1,
        layer_2,
        layer_3
    };
    enum channelModes : int
    {
        Stereo,
        Joint_stereo,
        Dual_channel,
        Single_channel
    };
    enum L1_L2_channelModeExtensions : int
    {
        bands_4_to_31,
        bands_8_to_31,
        bands_12_to_31,
        bands_16_to_31
    };
    enum L3_StereoModeExtensions_intensity_MS : int
    {
        off_off,
        on_off,
        off_on,
        on_on
    };
    typedef union
    {
        L1_L2_channelModeExtensions bands;
        L3_StereoModeExtensions_intensity_MS stereoMode;
    } channelModeExtension;

    mpegVersions version;  //1:MPEG-1, 2:MPEG-2, 3:MPEG-2.5
    mpegLayers layer;   //1:Layer1, 2:Layer2, 3:Layer3
    int   CRC_error_protection;  //1:CRC on, 0:CRC off
    int   bitRate_index;
    int   frequency_index;
    int   padding;
    int   extension;
    channelModes mode;
    channelModeExtension mode_ext;
    int   copyright;
    int   original;
    int   emphasis;
    MPEG_HEADER(){}
    MPEG_HEADER(const byte* buf) { fill(buf); }
    int fill(const byte* buf)
    {
        if (buf[0] != 0xFF) return 0;//sync error
        int v = (buf[1] & 0x08) >> 3;
        int l = (buf[1] & 0x06) >> 1;
        int e = (buf[1] & 0x01);
        bitRate_index = (buf[2] & 0xf0) >> 4;
        frequency_index = (buf[2] & 0x0c) >> 2;
        padding = (buf[2] & 0x02) >> 1;
        extension = (buf[2] & 0x01);
        mode = channelModes((buf[3] & 0xc0) >> 6);
        mode_ext.bands = L1_L2_channelModeExtensions((buf[3] & 0x30) >> 4);
        copyright = (buf[3] & 0x08) >> 3;
        original = (buf[3] & 0x04) >> 2;
        emphasis = (buf[3] & 0x03);

        if ((buf[1] & 0xF0) == 0xF0)  //MPEG-1, MPEG-2
        {
            version = mpegVersions((v) ? 1 : 2);
        }
        else if ((buf[1] & 0xF0) == 0xE0) //MPEG-2.5
        {
            version = mpeg_version_2_5;
        }
        else //m_last_error = MP3_ERROR_INVALID_HEADER;
        {
            return 0;
        }
        if (frequency_index >= 3 || bitRate_index == 0 || bitRate_index >= 15 || l == 0 || l >= 4) return 0;//m_last_error = MP3_ERROR_INVALID_HEADER;
        layer = mpegLayers(4 - l);
        CRC_error_protection = (e) ? 0 : 1;
        return 1;
    }
    uint frequencyLookup()
    {
        static const uint frequency_table[3][4] = {
            // MPEG-1
            { 44100, 48000, 32000, 0/*reserved*/ },
            // MPEG-2
            { 22050, 24000, 16000, 0/*reserved*/ },
            // MPEG-2.5
            { 11025, 12000, 8000, 0/*reserved*/ },
        };
        return frequency_table[version-1][frequency_index];
    }
    uint bitRateLookup()
    {
        static const uint bitrate_table[3][3][16] = {
            {// MPEG-1
             // Layer1
             { 0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0 },
             // Layer2
             { 0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 0 },
             // Layer3
             { 0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0 },
            },
            {// MPEG-2
             // Layer1
             { 0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, 0 },
             // Layer2
             { 0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0 },
             // Layer3
             { 0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0 },
            },
            {// MPEG-2.5
             // Layer1 (not available)
             { 0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, 0 },
             // Layer2 (not available)
             { 0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0 },
             // Layer3
             { 0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0 },
            },
        };
        return bitrate_table[version-1][layer-1][bitRate_index]*1000;
    }
};

typedef struct MPEG_DECODE_OPTION {
    int   reduction = 0;
    int   convert = 0;
    int   freqLimit = 24000;
} MPEG_DECODE_OPTION;

typedef struct MPEG_DECODE_INFO {
    int   channels;  //出力チャネル
    int   bitsPerSample; //
    int   frequency;  //サンプリングレート（Hz）
    int   bitRate;  //ビットレート（bps）
    int   frames;   //フレーム数（VBR only）
    int   skipSize;  //（VBR only）
    int   dataSize;  //データサイズ（VBR only）
    //int   minInputSize; //1フレームの最小入力サイズ
    //int   maxInputSize; //1フレームの最大入力サイズ
    int   outputSize;  //1フレームの出力サイズ
    //int   inputSize;
    int         frameSize;
    int         vbr;
} MPEG_DECODE_INFO;

class IMP3Layer
{
public:
    IMP3Layer(const MPEG_HEADER* h)
    {
        CurrentFrameHeader=*h;
    }
    virtual ~IMP3Layer();
    virtual int decode_start() { return 0; }
    virtual void decode_frame(byte* /*mpeg*/, short* /*pcm*/) { }
    void bitget_init(CBitget& bitget, byte* mpeg)
    {
        bitget.bitget_init(mpeg + 4 + getCRCSize());
    }
    virtual uint getCRCSize()
    {
        return (CurrentFrameHeader.CRC_error_protection) ? 2 : 0;
    }
    virtual uint getSideSize(const int /*channels*/){return 0;}
    virtual int calcFrameSize(){ return 0; }
    virtual void calcIOSizes() {}
    virtual void decode_reset() {}
    int mp3GetDecodeInfo(const byte* mpeg)
    {
        MPEG_DECODE_INFO& info = CurrentFrameInfo;
        VBRflag flags;

        if (!CurrentFrameHeader.fill(mpeg)) return 0;
        //check VBR Header
        byte* p = const_cast<byte*>(mpeg) + 4; //skip mpeg header
        p += getCRCSize();//skip crc
        info.channels = ((CurrentFrameHeader.mode == MPEG_HEADER::Single_channel) || (m_option.convert & 3)) ? 1 : 2;
        p += getSideSize(info.channels); //skip side info
        info.bitRate = CurrentFrameHeader.bitRateLookup();
        //info.frequency = CurrentFrameHeader.frequencyLookup();
        if (descriptorMatch("Xing",p)) //VBR
        {
            p += 4; //skip Xing
            flags = VBRflag(extractInt4(p));
            if (!(flags & (VBR_FRAMES_FLAG | VBR_BYTES_FLAG))) return 0;//m_last_error = MP3_ERROR_INVALID_HEADER;
            info.frames = extractInt4(p);
            info.dataSize = extractInt4(p);
            if (flags & VBR_TOC_FLAG) p += 100;
            if (flags & VBR_SCALE_FLAG) p += 4;
            info.vbr = 1;
        }
        else if (descriptorMatch("VBRI",p)) //VBRI
        {
            p += 10; //skip VBRI
            info.dataSize = extractInt4(p);
            info.frames = extractInt4(p);
            info.vbr = 1;
        }
        else //not VBR
        {
            info.vbr = 0;
            info.frames = 0;
            info.dataSize = 0;
        }
        calcIOSizes();
        //info.inputSize = CurrentFrameHeader.padding ? info.maxInputSize : info.minInputSize;

        if (m_option.convert & 8) //not available
        {
            info.bitsPerSample = 8;
            info.outputSize *= info.channels;
        }
        else
        {
            info.bitsPerSample = 16;
            info.outputSize *= info.channels * 2;
        }
        info.frameSize = calcFrameSize();
        info.frequency = CurrentFrameHeader.frequencyLookup() >> m_option.reduction;
        return info.outputSize;
    }
    int mp3DecodeFrame(byte* inbuf,short*outbuf)
    {
        //if (CurrentFrameInfo.inputSize <= 4) return 0;//m_last_error = MP3_ERROR_OUT_OF_BUFFER;
        //if (CurrentFrameInfo.frameSize != CurrentFrameInfo.inputSize) qDebug() << "1" << CurrentFrameInfo.frameSize << CurrentFrameInfo.inputSize << CurrentFrameInfo.bitRate << CurrentFrameInfo.frequency << CurrentFrameInfo.dataSize << "Diff";
        decode_frame(inbuf, outbuf);
        return CurrentFrameInfo.frameSize;
    }
    void setEQ(const int* value)
    {
        Subband_transform->setEQ(value);
    }
    float getEQ(const int band)
    {
        return Subband_transform->getEQ(band);
    }
    void setEQEnabled(const bool value)
    {
        Subband_transform->setEQEnabled(value);
    }
    bool getEQEnabled()
    {
        return Subband_transform->getEQEnabled();
    }
    MPEG_DECODE_INFO CurrentFrameInfo;
    MPEG_HEADER CurrentFrameHeader;
    MPEG_DECODE_OPTION m_option;
protected:
    int sbtCount;
    int frameSlotCount;
    int frameSlotSize;
    enum VBRflag
    {
        VBR_FLAGS_NONE=0x0000,
        VBR_FRAMES_FLAG=0x0001,
        VBR_BYTES_FLAG=0x0002,
        VBR_TOC_FLAG=0x0004,
        VBR_SCALE_FLAG=0x0008
    };
    CSbt* Subband_transform = nullptr;
    uint m_subBand_count_limit;
    inline uint sizeCalc() { return sizeCalc(CurrentFrameHeader.bitRateLookup()); }
    //inline uint minSizeCalc() { return sizeCalc(CurrentFrameHeader.minBitRate()); }
    //inline uint maxSizeCalc() { return sizeCalc(CurrentFrameHeader.maxBitRate()); }
    inline uint sizeCalc(const uint br) { return frameSlotCount * br / CurrentFrameHeader.frequencyLookup(); }
    inline int extractInt4(byte*& buf) // big endian extract
    {
        const int r = qFromBigEndian<int>(*reinterpret_cast<int*>(buf));
        buf += 4;
        return r;
        /*
        int r = buf[3] | (buf[2] << 8) | (buf[1] << 16) | (buf[0] << 24);
        buf += 4;
        return r;
        */
    }
    inline void clip(uint& n, const uint limit) { if (n > limit) n = limit; }
    void setSubbandCountLimit()
    {
        m_subBand_count_limit = (m_option.freqLimit * 64L + CurrentFrameInfo.frequency / 2) / CurrentFrameInfo.frequency;
        uint limit = 32 >> m_option.reduction; // caller limit
        if (limit > 8) limit--;
        clip(m_subBand_count_limit,limit);
    }
};

class CAllocator
{
public:
    CAllocator(){}
    CAllocator(const CAllocator& other) { assign(other); }
    inline uint init(const uint v, const double* look_c_value)
    {
        bit_allocation = samp_dispatch = v;
        c_value = look_c_value[v];
        return v;
    }
    inline void operator = (const CAllocator& other) { assign(other); }
    inline CAllocator& flagAsJoint(const uint monoDispatchCount)
    {
        samp_dispatch += monoDispatchCount;
        return *this;
    }
    inline void flagAsTerminate(const uint monoDispatchCount)
    {
        samp_dispatch = monoDispatchCount * 2; // terminate the dispatcher //
    }
    inline void flagAsSkipAndTerminate(const uint monoDispatchCount)
    {
        samp_dispatch = (monoDispatchCount * 2) + 1; // terminate the dispatcher with skip //
    }
    inline void allocateScaleFactor(const int v, const uint count = 1, const uint offset = 0)
    {
        static const double scale_factor_table[64] = { //scale factor table, for 16 pcm output (SHRT_MAX + 1) * 2.0 * pow(2.0, i / -3.0)
            65536, 52016, 41285.1, 32768, 26008, 20642.5, 16384, 13004,
            10321.3, 8192, 6501.99, 5160.64, 4096, 3251, 2580.32, 2048,
            1625.5, 1290.16, 1024, 812.749, 645.08, 512, 406.375, 322.54,
            256, 203.187, 161.27, 128, 101.594, 80.6349, 64, 50.7968,
            40.3175, 32, 25.3984, 20.1587, 16, 12.6992, 10.0794, 8,
            6.3496, 5.03968, 4, 3.1748, 2.51984, 2, 1.5874, 1.25992,
            1, 0.793701, 0.629961, 0.5, 0.39685, 0.31498, 0.25, 0.198425,
            0.15749, 0.125, 0.0992126, 0.0787451, 0.0625, 0.0496063, 0.0393725, 0.03125
        };
        const double f = c_value * scale_factor_table[v];
        for (uint i = 0; i < count; i++) cs_factor[i + offset] = f;
    }
    inline float unpack(const uint i, const int bit) { return cs_factor[i]*bit; }
    inline bool isAllocated() { return (bit_allocation != 0); }
    inline uint sampDispatch() { return samp_dispatch; }
    inline void setCSFactorSize(const int s) { cs_factor.resize(s); }
private:
    uint samp_dispatch = 0;
    uint bit_allocation = 0;
    double c_value = 0;
    std::vector<double> cs_factor;
    inline void assign(const CAllocator& other)
    {
        c_value = other.c_value;
        bit_allocation = other.bit_allocation;
        samp_dispatch = other.samp_dispatch;
    }
};

class IMP3LayerIandII : public IMP3Layer
{
public:
    IMP3LayerIandII(const MPEG_HEADER* h) : IMP3Layer(h) { }
    virtual void decode_frame(byte* mpeg, short* pcm)
    {
        static const int look_joint[16] =
        {    /* lookup stereo sb's by mode+ext */
                        64, 64, 64, 64,  /* stereo */
                        2 * 4, 2 * 8, 2 * 12, 2 * 16, /* joint */
                        64, 64, 64, 64,  /* dual */
                        32, 32, 32, 32,  /* mono */
        };
        bitget_init(m_bitget,mpeg);
        m_stereo_sb_count = look_joint[(CurrentFrameHeader.mode << 2) + CurrentFrameHeader.mode_ext.bands];
        unpack_bit_allocation();  // unpack bit allocation
        unpack_scaleFactor();  // unpack scale factor
        unpack_samples();  // unpack samples
        Subband_transform->sbt(m_sample.data(),pcm,sbtCount);
    }
    virtual ~IMP3LayerIandII();
protected:
    int decode_start() { return 0; }
    int calcFrameSize()
    {
        return (sizeCalc() + CurrentFrameHeader.padding) * frameSlotSize;
    }
    void calcIOSizes()
    {
        MPEG_DECODE_INFO& info=CurrentFrameInfo;
        info.outputSize = (32 * sbtCount) >> m_option.reduction;
        /*
        if (!info.vbr)
        {
            info.skipSize = 0;
            info.minInputSize = sizeCalc();
            info.maxInputSize = info.minInputSize + 1;
        }
        else
        {
            info.skipSize = sizeCalc() + CurrentFrameHeader.padding;
            info.minInputSize = minSizeCalc();
            info.maxInputSize = maxSizeCalc() + 1;
        }
        info.skipSize *= frameSlotSize;
        info.minInputSize *= frameSlotSize;
        info.maxInputSize *= frameSlotSize;
        */
    }
    void setCallerLimit()
    {
        m_sample.resize(sbtCount * 64);
        for (uint i = 0; i < m_sample.size(); i++) m_sample[i] = 0; // clear sample buffer, unused sub bands must be 0
        m_max_subBands = 0;
        for (uint i = 0; i < m_bit_allocation_table_count.size(); i++) m_max_subBands += m_bit_allocation_table_count[i];
        setSubbandCountLimit();
        clip(m_subBand_count_limit,m_max_subBands);
        if (CurrentFrameHeader.mode != MPEG_HEADER::Single_channel) // adjust for 2 channel modes
        {
            for (uint i = 0; i < m_bit_allocation_table_count.size(); i++) m_bit_allocation_table_count[i] *= 2;
            m_max_subBands *= 2;
            m_subBand_count_limit *= 2;
        }
        m_allo.resize(m_max_subBands + 1); // allow terminator
    }
    inline int getbit(const int n) { return m_bitget.bitget(n)-((1 << (n-1)) -1); }
    std::vector<float> m_sample;
    uint m_max_subBands;
    int m_stereo_sb_count;

    std::vector<CAllocator> m_allo;
    std::vector<uint> m_bit_allocation_table_count;// = {3, 8, 12, 7};

    int m_bit_skip;
    CBitget m_bitget;
    virtual void unpack_bit_allocation(){}
    virtual void unpack_scaleFactor(){}
    virtual void unpack_samples(){}
};

#pragma pack(pop)

#endif // IMP3LAYER_H
