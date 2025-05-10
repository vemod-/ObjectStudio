#ifndef IWAVEFILE_H
#define IWAVEFILE_H

#include <QtCore>
#include <QFile>
#include "softsynthsdefines.h"
#include "csimplebuffer.h"

#pragma pack(push,1)

class IWaveFile
{
public:
    enum Endian { BigEndian = QSysInfo::BigEndian, LittleEndian = QSysInfo::LittleEndian };
    IWaveFile(){};
    IWaveFile(QString /*path*/);
    virtual ~IWaveFile();
    virtual uint channels() const { return m_Channels; }
    virtual uint rate() const { return m_Frequency; }
    bool assign(const QByteArray& /*b*/)
    {
        return false;
    }
    virtual bool save(const QString &/*filename*/, CChannelBuffer& /*data*/, const uint /*SampleRate*/)
    {
        return false;
    }
    virtual void createFloatBuffer(CChannelBuffer& OutBuffer, const uint Samplerate);
protected:
    struct chunk
    {
        char        id[4];
        uint     size;
    };
    short inline getShort(const short val) const
    {
        if (m_ByteOrder==LittleEndian) return qFromLittleEndian<short>(val);
        if (m_ByteOrder==BigEndian) return qFromBigEndian<short>(val);
        return val;
    }
    ushort inline getUShort(const ushort val) const
    {
        if (m_ByteOrder==LittleEndian) return qFromLittleEndian<ushort>(val);
        if (m_ByteOrder==BigEndian) return qFromBigEndian<ushort>(val);
        return val;
    }
    int inline getInt(const int val) const
    {
        if (m_ByteOrder==LittleEndian) return qFromLittleEndian<int>(val);
        if (m_ByteOrder==BigEndian) return qFromBigEndian<int>(val);
        return val;
    }
    qint64 inline getLong(const long val) const
    {
        if (m_ByteOrder==LittleEndian) return qFromLittleEndian<qint64>(val);
        if (m_ByteOrder==BigEndian) return qFromBigEndian<qint64>(val);
        return val;
    }
    uint inline getUInt(const uint val) const
    {
        if (m_ByteOrder==LittleEndian) return qFromLittleEndian<uint>(val);
        if (m_ByteOrder==BigEndian) return qFromBigEndian<uint>(val);
        return val;
    }
    inline bool findChunk(const char* s, ulong64 &ptr, char* pSrc, const ulong64 filesize) const
    {
        chunk* Chnk=reinterpret_cast<chunk*>(pSrc + ptr);
        while (!descriptorMatch(Chnk->id, s))
        {
            if (ptr > filesize) return false;
            ptr+=getUInt(Chnk->size)+sizeof(chunk);
            Chnk=reinterpret_cast<chunk*>(pSrc + ptr);
        }
        return true;
    }
    inline bool findChunk(const char *s, ulong64 &ptr, const QByteArray& b) const
    {
        return findChunk(s,ptr, const_cast<char*>(b.constData()),ulong(b.size()));
    }
    int16_t inline MuLaw_Decode(int8_t number) const
    {
        const uint16_t MULAW_BIAS = 33;
        uint8_t sign = 0, position = 0;
        int16_t decoded = 0;
        number = ~number;
        if (number & 0x80)
        {
            number &= ~(1 << 7);
            sign = -1;
        }
        position = ((number & 0xF0) >> 4) + 5;
        decoded = ((1 << position) | ((number & 0x0F) << (position - 4)) | (1 << (position - 5))) - MULAW_BIAS;
        return (sign == 0) ? (decoded) : (-(decoded));
    }
    int16_t inline ALaw_Decode(int8_t number) const
    {
        uint8_t sign = 0x00;
        uint8_t position = 0;
        int16_t decoded = 0;
        number ^= 0x55;
        if (number & 0x80)
        {
            number &= ~(1 << 7);
            sign = -1;
        }
        position = ((number & 0xF0) >> 4) + 4;
        decoded = (position != 4) ? ((1 << position) | ((number & 0x0F)<<(position - 4)) | (1 << (position - 5))) :
                                    (number << 1) | 1;
        return (sign==0) ? (decoded) : (-decoded);
    }
    float inline ReadAuMem(const void* pSrc, const bool HalfByte=false)
    {
        switch (m_SampleBitSize) {
            case 32:
                return (m_AuEncoding==AUDIO_FILE_ENCODING_FLOAT) ? *(static_cast<const float*>(pSrc)) :
                                                                   getInt(*static_cast<const int*>(pSrc))*MAXINTMULTIPLY_F;
            case 16: return getShort(*static_cast<const short*>(pSrc))*MAXSHORTMULTIPLY_F;
            case 8:
            {
                if (m_AuEncoding==AUDIO_FILE_ENCODING_MULAW_8) return MuLaw_Decode(*static_cast<const int8_t*>(pSrc))*MAXSHORTMULTIPLY_F;
                if (m_AuEncoding==AUDIO_FILE_ENCODING_ALAW_8) return ALaw_Decode(*static_cast<const int8_t*>(pSrc))*MAXSHORTMULTIPLY_F;
                return (*static_cast<const char*>(pSrc))*MAXCHARMULTIPLY_F;
            }
            case 4:
                return (HalfByte) ? (((*static_cast<const char*>(pSrc)) & 0xF) << 4)*MAXCHARMULTIPLY_F :
                                    ((*static_cast<const char*>(pSrc)) & 0xF0)*MAXCHARMULTIPLY_F;
            case 64: return (m_AuEncoding==AUDIO_FILE_ENCODING_DOUBLE) ? float(*(static_cast<const double*>(pSrc))) :
                                                                         getLong(*static_cast<const qint64*>(pSrc))*MAXLONGMULTIPLY_F;
            case 12:
                return (m_ByteOrder==BigEndian) ? qFromBigEndian<short>((*static_cast<const short*>(pSrc)) & 0xFFF)*MAXSHORTMULTIPLY_F :
                                                  (((*static_cast<const short*>(pSrc)) & 0xFFF) << 4)*MAXSHORTMULTIPLY_F;
            case 24:
                return (m_ByteOrder==BigEndian) ? qFromBigEndian<short>(*static_cast<const short*>(pSrc))*MAXSHORTMULTIPLY_F :
                                                  (*reinterpret_cast<const short*>(static_cast<const char*>(pSrc)+1))*MAXSHORTMULTIPLY_F;
            default:
                return 0;
        }
    }
    void inline DecompressIMAPacket( const uint8_t* pSrc, float* pDst, const uint stride ) const;
    void DecompressIMA(const uint channels, const uint8_t* pSrc, CChannelBuffer& b, const ulong64 srcSize) const;

    QRecursiveMutex mutex;
    uint m_Channels;
    uint m_Frequency;
    Endian m_ByteOrder;
    uint m_AuEncoding;
    byte* m_WaveStart;
    uint m_SampleBitSize;
    ulong64 m_ChunkSize;
    QByteArray byteArray;
    /* Define the encoding fields */
    static const int AUDIO_FILE_ENCODING_MULAW_8=1; 	/* 8-bit ISDN u-law */
    static const int AUDIO_FILE_ENCODING_LINEAR_8=2; 	/* 8-bit linear PCM */
    static const int AUDIO_FILE_ENCODING_LINEAR_16=3; 	/* 16-bit linear PCM */
    static const int AUDIO_FILE_ENCODING_LINEAR_24=4; 	/* 24-bit linear PCM */
    static const int AUDIO_FILE_ENCODING_LINEAR_32=5; 	/* 32-bit linear PCM */
    static const int AUDIO_FILE_ENCODING_FLOAT=6; 	/* 32-bit IEEE floating point */
    static const int AUDIO_FILE_ENCODING_DOUBLE=7; 	/* 64-bit IEEE floating point */
    static const int AUDIO_FILE_ENCODING_ADPCM_G721=23; 	/* 4-bit CCITT g.721 ADPCM */
    static const int AUDIO_FILE_ENCODING_ADPCM_G722=24; 	/* CCITT g.722 ADPCM */
    static const int AUDIO_FILE_ENCODING_ADPCM_G723_3=25; 	/* CCITT g.723 3-bit ADPCM */
    static const int AUDIO_FILE_ENCODING_ADPCM_G723_5=26; 	/* CCITT g.723 5-bit ADPCM */
    static const int AUDIO_FILE_ENCODING_ALAW_8=27; 	/* 8-bit ISDN A-law */
};

#pragma pack(pop)

#endif // IWAVEFILE_H
