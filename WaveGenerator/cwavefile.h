#ifndef CWAVEFILE_H
#define CWAVEFILE_H

//#include <QtCore/qendian.h>
//#include <QFile>
//#include "softsynthsdefines.h"
#include "iwavefile.h"
#include "csinglemap.h"

namespace WaveFile
{
const QString WaveFilter("Sound Files (*.wav;*.wave;*.au;*.mp3;*.m4a;*.mp4;*.flac;*.ogg;*.aif;*.aiff;*.aifc)");
}

#pragma pack(push,1)
class CAiffFile : public IWaveFile
{
public:
    CAiffFile() {}
    CAiffFile(QString path)
        : IWaveFile(path) {
        if (byteArray.size()) assign(byteArray);
    }
    void createFloatBuffer(CChannelBuffer& OutBuffer, const uint Samplerate);
    bool assign(const QByteArray& b);
private:
    struct FormChunk
    {
        chunk descriptor;
        char formType[4];
    };
    struct CommonChunk
    {
      chunk descriptor;
      ushort numChannels; /* # audio channels */
      uint numSampleFrames; /* # sample frames = samples/channel */
      ushort sampleSize; /* # bits/sample */
      byte sampleRate[10]; /* sample_frames/sec */
      char compressionType[4]; /* compression type ID code */
      char compressionNameLen; /* human-readable compression type name */
      char compressionName[1];
    };
    struct SoundDataChunk
    {
        chunk descriptor;
        uint       offset;
        uint       blockSize;
    };
    QString AiffEncoding;
};

class CAuFile : public IWaveFile
{
public:
    CAuFile() {}
    CAuFile(QString path)
        : IWaveFile(path) {
        if (byteArray.size()) assign(byteArray);
    }
    bool assign(const QByteArray& b);
    bool save(const QString &filename, CChannelBuffer& data, const uint SampleRate);
private:
    struct Audio_filehdr
    {
        uint	magic;	/* magic number */
        uint	hdr_size;	/* size of this header */
        uint 	data_size;	/* length of data (optional) */
        uint 	encoding;	/* data encoding format */
        uint 	sample_rate;	/* samples per second */
        uint 	channels;	/* number of interleaved channels */
    } ;
    static const uint AUDIO_FILE_MAGIC=0x2e736e64;
};

class CWavFile : public IWaveFile
{
public:
    static const short WAVE_FORMAT_PCM=0x0001; /* Microsoft Corporation */
    struct RIFFHeader
    {
        chunk       descriptor;     // "RIFF"
        char        type[4];        // "WAVE"
    };
    struct WAVEHeader
    {
        chunk       descriptor;
        ushort     audioFormat;
        ushort     numChannels;
        uint     sampleRate;
        uint     byteRate;
        ushort     blockAlign;
        ushort     bitsPerSample;
    };
    struct _PPEAK
    {
      float Value;    // peak value
      uint Position;    // sample frame for peak
    };
    struct PEAKheader
    {
       chunk descriptor;
       uint Version;    // peak chunk version
       uint timestamp;  // UNIX timestamp of creation
      _PPEAK *peak;    // one for each channel
    };
    struct FACTHeader
    {
        chunk descriptor;
        int dwSampleLength;
    };
    struct DATAHeader
    {
        chunk       descriptor;
    };
    struct CombinedHeader
    {
        RIFFHeader  riff;
        WAVEHeader  wave;
    };
    CWavFile() {}
    CWavFile(QString path)
        : IWaveFile(path) {
        if (byteArray.size()) assign(byteArray);
    }
    bool assign(const QByteArray& b);
    bool save(const QString &filename, CChannelBuffer& data, const uint SampleRate);
private:
    enum SampleType { Unknown, SignedInt, UnSignedInt, Float };
};

#pragma pack(pop)

class CWaveFile : public IRefCounter
{
public:
    CWaveFile();
    ~CWaveFile();
    bool load(const QString &fileName, const uint SampleRate);
    void startRecording(const uint Channels, const uint SampleRate);
    void finishRecording();
    inline void pushBuffer(float* buffer, const ulong64 Size) { data.append(buffer,Size,0xFFFFFF); }
    CChannelBuffer data;
    uint frequency;
    bool save(const QString &fileName);
private:
    uint m_SampleRate;
    QRecursiveMutex mutex;
};

#endif // CWAVEFILE_H
