#ifndef CMINIMP3_H
#define CMINIMP3_H

#include "iwavefile.h"
#include "minimp3_ex.h"

class CMiniMP3 : public IWaveFile {
public:
    CMiniMP3() {
        m_Channels = 0;
        m_Frequency = 0;
    }
    ~CMiniMP3() {

    }
    bool assign(const QByteArray& b) {
        if (mp3dec_load_buf(&mp3d, (const uchar*)b.constData(), b.size(), &info, NULL, NULL)) return false;
        /* mp3dec_file_info_t contains decoded samples and info, use free(info.buffer) to deallocate samples */
        if (!info.channels || !info.samples)  {
            free(info.buffer);
            return false;
        }
        return true;
    }
    void createFloatBuffer(CChannelBuffer& OutBuffer, const uint Samplerate) {
        m_WaveStart = (uchar*)info.buffer;
        m_AuEncoding=AUDIO_FILE_ENCODING_LINEAR_16;
        m_SampleBitSize=16;
        m_ChunkSize=info.samples*sizeof(short);
        m_Channels = info.channels;
        m_Frequency = info.hz;
        IWaveFile::createFloatBuffer(OutBuffer,Samplerate);
        free(info.buffer);
    }
private:
    mp3dec_t mp3d;
    mp3dec_file_info_t info;
};


#endif // CMINIMP3_H
