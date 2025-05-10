#include "iwavefile.h"

static int32_t IMA_INDEX_TABLE[16] =
{
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8
};

static int32_t IMA_STEP_TABLE[89] =
{
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350,  22385, 24623, 27086, 29794, 32767
};

void IWaveFile::DecompressIMAPacket( const uint8_t* pSrc, float* pDst, const uint stride ) const
{
    // Read packet header
    const uint16_t value  = *reinterpret_cast<const uint16_t*>(pSrc);
    auto header = uint16_t(( value >> 8 ) | ( value << 8 ));
    int32_t predictor  = header & 0xff80;
    int32_t step_index = header & 0x007f;
    int32_t step, nibble, diff;

    // Sign extend predictor
    if( predictor & 0x8000 )
        predictor |= 0xffff0000;

    // Skip header
    pSrc += 2;

    // Read 64 nibbles, 2 at a time
    uint32_t byteCount = 32;
    while( byteCount-- )
    {
        // Read 2 nibbles
        uint8_t byte = *pSrc++;

        // Process low nibble
        nibble = byte & 0x0f;
        if( step_index < 0 ) step_index = 0;
        else if( step_index > 88 ) step_index = 88;
        step = IMA_STEP_TABLE[ step_index ];
        step_index += IMA_INDEX_TABLE[ nibble ];
        diff = step >> 3;
        if (nibble & 4) diff += step;
        if (nibble & 2) diff += (step >> 1);
        if (nibble & 1) diff += (step >> 2);
        if (nibble & 8) predictor -= diff;
        else predictor += diff;
        if( predictor < SHRT_MIN ) predictor = SHRT_MIN;
        else if( predictor > SHRT_MAX ) predictor = SHRT_MAX;
        *pDst = predictor*MAXSHORTMULTIPLY_F;
        pDst += stride;

        // Process high nibble
        nibble = byte >> 4;
        if( step_index < 0 ) step_index = 0;
        else if( step_index > 88 ) step_index = 88;
        step = IMA_STEP_TABLE[ step_index ];
        step_index += IMA_INDEX_TABLE[ nibble ];
        diff = step >> 3;
        if (nibble & 4) diff += step;
        if (nibble & 2) diff += (step >> 1);
        if (nibble & 1) diff += (step >> 2);
        if (nibble & 8) predictor -= diff;
        else predictor += diff;
        if( predictor < SHRT_MIN ) predictor = SHRT_MIN;
        else if( predictor > SHRT_MAX ) predictor = SHRT_MAX;
        *pDst = predictor*MAXSHORTMULTIPLY_F;
        pDst += stride;
    }
}

void IWaveFile::DecompressIMA(const uint channels, const uint8_t* pSrc, CChannelBuffer& b, const ulong64 srcSize) const
{
    auto packetCount = ulong64((srcSize / 34.0) / channels);
    b.init(packetCount*64,channels);
    for (uint pck=0;pck<packetCount;pck++)
    {
        const uint channelPacket=pck*channels;
        for (uint c=0;c<channels;c++)
        {
            DecompressIMAPacket( pSrc+((channelPacket+c)*34), b.dataPointer(64*channelPacket,c), channels );
        }
    }
}


IWaveFile::IWaveFile(QString path)
{
    m_AuEncoding=0;
    m_ByteOrder=LittleEndian;
    m_Channels=1;
    m_Frequency=44100;
    QFile f(path);
    if (f.open(QIODevice::ReadOnly))
    {
        byteArray = f.readAll();
        f.close();
    }
}

IWaveFile::~IWaveFile()
{

}

void IWaveFile::createFloatBuffer(CChannelBuffer& OutBuffer, const uint Samplerate)
{
    QMutexLocker locker(&mutex);
    const ldouble PointerInc = (m_SampleBitSize / 8.L) * m_Channels;
    const ldouble RateFactor = ldouble(m_Frequency) / Samplerate;
    const auto Length = ulong64((ldouble(m_ChunkSize) / RateFactor) / PointerInc);
    const auto ByteCount = uint(ceilf(m_SampleBitSize / 8.f));
    qDebug() << double(PointerInc) << double(RateFactor) << Length << ByteCount;
    OutBuffer.init(Length,m_Channels);
    //OutBuffer=new float[Length*m_Channels];
    //qDebug() << "m_SampleSize" << m_SampleSize << "Bytecount" << ByteCount << "channels" << m_Channels;
    //qDebug() << "m_Frequency" << m_Frequency << "SampleRate" << Samplerate << "m_ChunkSize" << m_ChunkSize;
    //qDebug() << "Ratefactor" << RateFactor << "pointerinc" << PointerInc << "Length" << Length;
    //qDebug() << "m_AuEncoding" << m_AuEncoding << "m_ByteOrder" << m_ByteOrder;

    if (closeEnough(m_SampleBitSize / 8.0, double(ByteCount)))
    {
        const uint UnitSize = ByteCount*m_Channels;
        if (isZero(double(fmodl(RateFactor,1.L))))
        {
            qDebug() << "No float!";
            const auto PtrInc = uint(UnitSize * RateFactor);
            const byte* waveEnd = m_WaveStart + m_ChunkSize;
            for (uint c = 0; c < m_Channels; c++)
            {
                for (byte* ptr=m_WaveStart + (c * ByteCount); ptr < waveEnd; ptr += PtrInc)
                {
                    OutBuffer.set(ReadAuMem(ptr));
                }
            }
            return;
        }
        qDebug() << "Float RateFactor!" << double(RateFactor);
        for (uint c = 0; c < m_Channels; c++)
        {
            const byte* ChannelPtr = m_WaveStart + (c * ByteCount);
            for (ulong64 i = 0; i < Length; i++)
            {
                OutBuffer.set(ReadAuMem(ChannelPtr + (ulong64(i * RateFactor) * UnitSize)));
            }
        }
        return;
    }
    const ldouble stridef = RateFactor * PointerInc;
    bool HalfByte = false;
    qDebug() << "Float all!" << double(stridef) << double(RateFactor);
    for (uint c = 0; c < m_Channels; c++)
    {
        const byte* ChannelPtr = m_WaveStart + (c * ByteCount);
        for (ulong64 i = 0; i < Length; i++)
        {
            ldouble pos = i * stridef;
            if (m_SampleBitSize==4) HalfByte = (fmodl(pos,1.L) >= 0.5L);
            auto TempPos = ulong64(pos);
            if (ByteCount > 1) TempPos = (TempPos / ByteCount) * ByteCount;
            OutBuffer.set(ReadAuMem(ChannelPtr + TempPos, HalfByte));
        }
    }
}






