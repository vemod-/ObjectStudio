#include "cwavefile.h"
//#include "mp3lib/mp3play.h"
//#include "cminimp3.h"
#ifdef FFMPEGLIB
    #include "audiorw.h"
#endif
#ifdef QTMMLIB
    #include "qaudiorw.h"
#endif

/* ************************* ConvertFloat() *****************************
 * Converts an 80 bit IEEE Standard 754 floating point number to an unsigned
 * long.
 ********************************************************************** */

uint x80_2_uint(const byte* buffer)
{
   uint mantissa;
   uint last = 0;
   byte exp;

   mantissa = qFromBigEndian<uint>(*reinterpret_cast<const uint*>(buffer+2));
   exp = 30 - *(buffer+1);
   while (exp--)
   {
     last = mantissa;
     mantissa >>= 1;
   }
   if (last & 0x00000001) mantissa++;
   return(mantissa);
}

bool CAiffFile::assign(const QByteArray& b)
{
    QMutexLocker locker(&mutex);
    const auto header=reinterpret_cast<const FormChunk*>(b.constData());
    if (!descriptorMatch(header->descriptor.id, "FORM")) return false;
    if (descriptorMatch(header->formType,"AIFF"))
    {
        m_ByteOrder=IWaveFile::BigEndian;
    }
    else if (descriptorMatch(header->formType,"AIFC"))
    {
        m_ByteOrder=IWaveFile::BigEndian;
    }
    else
    {
        return false;
    }
    ulong64 ptr=sizeof(FormChunk);
    if (findChunk("COMM",ptr,b))
    {
        const auto CommonHeader=reinterpret_cast<const CommonChunk*>(b.constData() + ptr);
        if (descriptorMatch(CommonHeader->descriptor.id,"COMM"))
        {
            ptr += sizeof(chunk)+getUInt(CommonHeader->descriptor.size);
            qDebug() << ptr << getUInt(CommonHeader->descriptor.size) << CommonHeader->descriptor.size;
            if (findChunk("SSND",ptr,b))
            {
                // Read off remaining header information
                const auto dataheader=reinterpret_cast<const SoundDataChunk*>(b.constData() + ptr);
                m_ChunkSize=getUInt(dataheader->descriptor.size)-8;
                m_Channels = getUShort(CommonHeader->numChannels);
                m_Frequency = x80_2_uint(CommonHeader->sampleRate);
                m_SampleBitSize = getUShort(CommonHeader->sampleSize);
                AiffEncoding=QString(QByteArray(CommonHeader->compressionType,4)).toLower();
                m_WaveStart=reinterpret_cast<byte*>(const_cast<char*>(b.data()) + ptr + sizeof(SoundDataChunk));
                if (AiffEncoding=="fl32")
                {
                    m_SampleBitSize=32;
                    m_AuEncoding=AUDIO_FILE_ENCODING_FLOAT;
                }
                if (AiffEncoding=="fl64")
                {
                    m_SampleBitSize=64;
                    m_AuEncoding=AUDIO_FILE_ENCODING_DOUBLE;
                }
                if (AiffEncoding=="alaw")
                {
                    m_SampleBitSize=8;
                    m_AuEncoding=AUDIO_FILE_ENCODING_ALAW_8;
                }
                if (AiffEncoding=="ulaw")
                {
                    m_SampleBitSize=8;
                    m_AuEncoding=AUDIO_FILE_ENCODING_MULAW_8;
                }
                return true;
            }
        }
    }
    return false;
}

void CAiffFile::createFloatBuffer(CChannelBuffer& OutBuffer, const uint Samplerate)
{
    QMutexLocker locker(&mutex);
    if (AiffEncoding=="ima4")
    {
        CChannelBuffer b;
        DecompressIMA(m_Channels, m_WaveStart, b, m_ChunkSize);
        m_WaveStart=reinterpret_cast<byte*>(b.data());
        m_AuEncoding=AUDIO_FILE_ENCODING_FLOAT;
        m_SampleBitSize=32;
        m_ChunkSize=b.chunkSize();
        IWaveFile::createFloatBuffer(OutBuffer,Samplerate);
    }
    else if (m_SampleBitSize)
    {
        if (AiffEncoding=="sowt") m_ByteOrder=LittleEndian;
        IWaveFile::createFloatBuffer(OutBuffer,Samplerate);
    }
}

bool CAuFile::assign(const QByteArray& b)
{
    QMutexLocker locker(&mutex);
    const auto auHeader=reinterpret_cast<const Audio_filehdr*>(b.constData());
    //int HeaderSize=0;
    if (qFromLittleEndian<uint>(auHeader->magic) == AUDIO_FILE_MAGIC)
    {
        m_ByteOrder=IWaveFile::LittleEndian;
    }
    else if (qFromBigEndian<uint>(auHeader->magic) == AUDIO_FILE_MAGIC)
    {
        m_ByteOrder=IWaveFile::BigEndian;
    }
    else
    {
        return false;
    }
    m_WaveStart=reinterpret_cast<byte*>(const_cast<char*>(b.constData()) + getUInt(auHeader->hdr_size));
    m_Channels=getUInt(auHeader->channels);
    m_AuEncoding=getUInt(auHeader->encoding);
    m_Frequency=getUInt(auHeader->sample_rate);
    //HeaderSize=getInt(auHeader->hdr_size);
    switch (m_AuEncoding)
    {
    case AUDIO_FILE_ENCODING_ADPCM_G723_3:
        m_SampleBitSize=3;
        break;
    case AUDIO_FILE_ENCODING_ADPCM_G723_5:
        m_SampleBitSize=5;
        break;
    case AUDIO_FILE_ENCODING_ADPCM_G721:
        m_SampleBitSize=4;
        break;
    case AUDIO_FILE_ENCODING_MULAW_8:
    case AUDIO_FILE_ENCODING_LINEAR_8:
    case AUDIO_FILE_ENCODING_ALAW_8:
    case AUDIO_FILE_ENCODING_ADPCM_G722:
        m_SampleBitSize=8;
        break;
    case AUDIO_FILE_ENCODING_LINEAR_16:
        m_SampleBitSize=16;
        break;
    case AUDIO_FILE_ENCODING_LINEAR_24:
        m_SampleBitSize=24;
        break;
    case AUDIO_FILE_ENCODING_LINEAR_32:
    case AUDIO_FILE_ENCODING_FLOAT:
        m_SampleBitSize=32;
        break;
    case AUDIO_FILE_ENCODING_DOUBLE:
        m_SampleBitSize=64;
        break;
    default:
        m_SampleBitSize=8;
        break;
    }
    m_ChunkSize=ulong64(b.size())-getUInt(auHeader->hdr_size);
    return true;
}

bool CAuFile::save(const QString &filename, CChannelBuffer& data, const uint SampleRate)
{
    QMutexLocker locker(&mutex);
    std::vector<int> b=data.toIntInterleaved();
    ulong64 PCMSize = data.dataSize();

    Audio_filehdr WH;
    WH.magic=qToBigEndian<uint>(AUDIO_FILE_MAGIC);
    WH.hdr_size=qToBigEndian<uint>(sizeof(Audio_filehdr));
    WH.data_size=qToBigEndian<uint>(uint(PCMSize)*sizeof(int));
    WH.encoding=qToBigEndian<uint>(AUDIO_FILE_ENCODING_LINEAR_32);
    WH.sample_rate=qToBigEndian<uint>(SampleRate);
    WH.channels=qToBigEndian<uint>(data.channels());

    QFile m_RecordFile(filename);
    if (m_RecordFile.open(QIODevice::WriteOnly))
    {
        m_RecordFile.write(reinterpret_cast<const char*>(&WH),sizeof(WH));
        m_RecordFile.write(reinterpret_cast<const char*>(b.data()),long64(PCMSize*sizeof(int)));
        m_RecordFile.close();
    }
    return true;

}

bool CWavFile::assign(const QByteArray& b)
{
    QMutexLocker locker(&mutex);
    //const auto header=reinterpret_cast<const CombinedHeader*>(b.constData());
    const auto riffHeader=reinterpret_cast<const RIFFHeader*>(b.constData());
    ushort audioFormat=0;
    if (descriptorMatch(riffHeader->descriptor.id, "RIFF"))
    {
        m_ByteOrder = IWaveFile::LittleEndian;
    }
    else if (descriptorMatch(riffHeader->descriptor.id, "RIFX"))
    {
        m_ByteOrder = IWaveFile::BigEndian;
    }
    else
    {
        return false;
    }
    if (!descriptorMatch(riffHeader->type,"WAVE")) return false;
    ulong64 ptr = sizeof(RIFFHeader);
    if (!findChunk("fmt ",ptr,b)) return false;
    auto waveHeader = reinterpret_cast<const WAVEHeader*>(b.constData() + ptr);
    audioFormat=getUShort(waveHeader->audioFormat);
    if (audioFormat == 1 || audioFormat == 0 || audioFormat == 3 || audioFormat == 6 || audioFormat == 7)
    {
        ptr += sizeof(WAVEHeader);
        if (findChunk("data",ptr,b))
        {
            auto dataheader=reinterpret_cast<const DATAHeader*>(b.constData() + ptr);
            m_ChunkSize=getUInt(dataheader->descriptor.size);
            m_Channels = getUShort(waveHeader->numChannels);
            m_Frequency = getUInt(waveHeader->sampleRate);
            m_SampleBitSize = getUShort(waveHeader->bitsPerSample);
            m_AuEncoding = 0;
            if (audioFormat==3)
            {
                m_AuEncoding=AUDIO_FILE_ENCODING_FLOAT;
                m_SampleBitSize=32;
            }
            if (audioFormat==6)
            {
                m_AuEncoding=AUDIO_FILE_ENCODING_ALAW_8;
                m_SampleBitSize=8;
            }
            if (audioFormat==7)
            {
                m_AuEncoding=AUDIO_FILE_ENCODING_MULAW_8;
                m_SampleBitSize=8;
            }
            m_WaveStart=reinterpret_cast<byte*>(const_cast<char*>(b.constData()) + ptr + sizeof(DATAHeader));
            return true;
        }
    }
    return false;
}

bool CWavFile::save(const QString &filename, CChannelBuffer& data, const uint SampleRate)
{
    QMutexLocker locker(&mutex);

    const std::vector<short> b=data.toShortInterleaved();
    const ulong64 PCMSize = data.dataSize();

    CombinedHeader WH;
    setDescriptor(WH.riff.descriptor.id,"RIFF");
    WH.riff.descriptor.size=qToLittleEndian<uint>(uint(sizeof(WAVEHeader)+sizeof(DATAHeader)+(PCMSize*sizeof(ushort))));
    setDescriptor(WH.riff.type,"WAVE");
    setDescriptor(WH.wave.descriptor.id,"fmt ");
    WH.wave.descriptor.size=qToLittleEndian<uint>(sizeof(WAVEHeader)-sizeof(WH.wave.descriptor));

    WH.wave.audioFormat=WAVE_FORMAT_PCM;         // Format category
    WH.wave.numChannels=qToLittleEndian<ushort>(ushort(data.channels()));          // Number of channels
    WH.wave.sampleRate=qToLittleEndian<uint>(SampleRate);
    WH.wave.bitsPerSample=qToLittleEndian<ushort>(sizeof(short)*8);
    WH.wave.blockAlign=qToLittleEndian<ushort>(ushort(data.channels()*sizeof(short)));        // Data block size
    WH.wave.byteRate=qToLittleEndian<uint>(SampleRate*data.channels()*sizeof(short));   // For buffer estimation

    DATAHeader WDI;
    setDescriptor(WDI.descriptor.id,"data");
    WDI.descriptor.size=qToLittleEndian<uint>(uint(PCMSize*sizeof(short)));

    QFile m_RecordFile(filename);
    if (m_RecordFile.open(QIODevice::WriteOnly))
    {
        m_RecordFile.write(reinterpret_cast<const char*>(&WH),sizeof(WH));
        m_RecordFile.write(reinterpret_cast<const char*>(&WDI),sizeof(WDI));
        m_RecordFile.write(reinterpret_cast<const char*>(b.data()),long64(PCMSize*sizeof(short)));
        m_RecordFile.close();
    }
    return true;
}

CWaveFile::CWaveFile()
{
}

bool CWaveFile::load(const QString &fileName, const uint SampleRate)
{
    QMutexLocker locker(&mutex);
    m_SampleRate=SampleRate;
    IWaveFile* WF=nullptr;
    const QString s = fileName.toLower();
    if (s.endsWith(".wav") || s.endsWith(".wave")) WF=new CWavFile(fileName);
    else if (s.endsWith(".au")) WF=new CAuFile(fileName);
#ifdef FFMPEGLIB
    else if (s.endsWith(".mp3") || s.endsWith(".m4a") || s.endsWith(".mp4") || s.endsWith(".flac") || s.endsWith(".ogg")) WF = new CFFMpegReader(fileName); //WF=new CMiniMP3;//CMP3File;
#endif
#ifdef QTMMLIB
    else if (s.endsWith(".mp3") || s.endsWith(".m4a") || s.endsWith(".mp4") || s.endsWith(".flac") || s.endsWith(".ogg")) WF = new CQAudioDecoderReader(fileName); //WF=new CMiniMP3;//CMP3File;
#endif
    else if (s.endsWith(".aif") || s.endsWith(".aiff") || s.endsWith(".aifc")) WF=new CAiffFile(fileName);
    if (WF) {
        if (WF->channels()) {
            WF->createFloatBuffer(data,m_SampleRate);
            frequency=WF->rate();
            qDebug() << "Length" << data.size() << "Channels" << data.channels() << "Frequency" << frequency << "Data" << data.data();
            delete WF;
            return true;
        }
        delete WF;
    }
    return false;
}

void CWaveFile::startRecording(const uint Channels, const uint SampleRate)
{
    data.init(0,Channels);
    frequency=SampleRate;
    m_SampleRate=SampleRate;
}

void CWaveFile::finishRecording()
{
    data.squeeze();
    data.normalize();
}

bool CWaveFile::save(const QString &fileName)
{
    QMutexLocker locker(&mutex);
    if (data.isEmpty()) return false;
    finishRecording();
    IWaveFile* WF=nullptr;
    const QString s = fileName.toLower();
    if (s.endsWith(".wav") || s.endsWith(".wave")) WF=new CWavFile;
    else if (s.endsWith(".au")) WF=new CAuFile;
#ifdef FFMPEGLIB
    else if (s.endsWith(".mp3") || s.endsWith(".m4a") || s.endsWith(".mp4") || s.endsWith(".flac") || s.endsWith(".ogg")) WF = new CFFMpegWriter(); //WF=new CMiniMP3;//CMP3File;
#endif
#ifdef QTMMLIB
    //else if (s.endsWith(".mp3") || s.endsWith(".m4a") || s.endsWith(".mp4") || s.endsWith(".flac") || s.endsWith(".ogg")) WF = new CQAudioRecorderWriter(); //WF=new CMiniMP3;//CMP3File;
#endif
    else if (s.endsWith(".aif") || s.endsWith(".aiff") || s.endsWith(".aifc")) WF=new CAiffFile;
    if (WF)
    {
        bool Result = WF->save(fileName,data,m_SampleRate);
        delete WF;
        return Result;
    }
    return false;
}

CWaveFile::~CWaveFile()
{
}
