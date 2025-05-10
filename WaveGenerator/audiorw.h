#ifndef AUDIORW_H
#define AUDIORW_H
#include <string>
#include <vector>
#include "iwavefile.h"
#include "csimplebuffer.h"

extern "C" {
//#include <libavfilter/avfilter.h>
//#include <libavutil/avutil.h>
/*
#include <libavutil/time.h>
#include <libavutil/timecode.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
*/
//#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
};

namespace audiorw {

static const int OUTPUT_BIT_RATE = 320000;
static const int DEFAULT_FRAME_SIZE = 2048;

std::vector<std::vector<double>> read(
    const std::string & filename,
    double & sample_rate,
    double start_seconds=0,
    double end_seconds=-1);

void write(
    const std::vector<std::vector<double>> & audio,
    const std::string & filename,
    double sample_rate);

void cleanup(
    AVCodecContext * codec_context,
    AVFormatContext * format_context,
    SwrContext * resample_context,
    AVFrame * frame,
    AVPacket packet);

}

class CFFMpegReader : public IWaveFile {
public:
    CFFMpegReader(QString path) : IWaveFile(path) {
        double rate = 0;
        data = audiorw::read(path.toStdString(),rate);
        m_Channels = data.size();
        m_Frequency = rate;
    }
    void createFloatBuffer(CChannelBuffer& OutBuffer, const uint Samplerate) {
        const ldouble RateFactor=ldouble(m_Frequency)/Samplerate;
        const auto Length=ulong64(ldouble(data[0].size())/RateFactor);
        OutBuffer.init(Length,m_Channels);
        for (uint c=0;c<m_Channels;c++)
        {
            ldouble Ptr=0;
            for (ulong64 i=0; i<Length; i++)
            {
                OutBuffer.setAt(i,c,data[c][Ptr]);
                Ptr+=RateFactor;
            }
        }
    }
private:
    std::vector<std::vector<double>> data;
};

class CFFMpegWriter : public IWaveFile {
public:
    CFFMpegWriter() : IWaveFile() {}
    bool save(const QString &filename, CChannelBuffer& data, const uint SampleRate) {
        std::vector<std::vector<double>> b;
        b.resize(data.channels());
        for (uint c = 0; c < data.channels(); c++) {
            for (ulong64 p = 0; p < data.dataSize(); p++) {
                b[c].push_back(data.at(p,c));
            }
        }
        audiorw::write(b,filename.toStdString(),SampleRate);
        return true;
    }
};

#endif // AUDIORW_H
