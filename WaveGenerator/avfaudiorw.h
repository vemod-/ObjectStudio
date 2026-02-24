#ifndef AVFAUDIORW_H
#define AVFAUDIORW_H

#include "iwavefile.h"
#include "avfoundation_wrapper.h"
#include <QImage>

class CAvFoundationReader : public IWaveFile {
public:
    static QImage thumbnail(QString path) {
        std::vector<uint8_t> outRGBA;
        int width = 0;
        int height = 0;
        avf_extract_thumbnail(path.toStdString().c_str(),0,outRGBA,width,height);
        return QImage(outRGBA.data(), width, height, QImage::Format_RGBA8888);
    }
    static bool isValid(QString path) {
        return avf_is_valid(path.toStdString().c_str());
    }
    CAvFoundationReader(QString path) : IWaveFile(path) {
        double rate = 0;
        int chans = 0;
        if (avf_read_audio(path.toStdString().c_str(), data, rate, chans)) {
            m_Channels = chans;
            m_Frequency = rate;
        }
        hasVideo = avf_has_video(path.toStdString().c_str());
    }
    void createFloatBuffer(CChannelBuffer& OutBuffer, const uint Samplerate) override {
        const ldouble RateFactor = ldouble(m_Frequency) / Samplerate;
        const auto Length = ulong64(ldouble(data[0].size()) / RateFactor);
        OutBuffer.init(Length, m_Channels);
        for (uint c = 0; c < m_Channels; ++c) {
            ldouble Ptr = 0;
            for (ulong64 i = 0; i < Length; ++i) {
                OutBuffer.setAt(i, c, data[c][Ptr]);
                Ptr += RateFactor;
            }
        }
    }
private:
    std::vector<std::vector<float>> data;
};

class CAvFoundationWriter : public IWaveFile {
public:
    CAvFoundationWriter() : IWaveFile() {}

    bool save(const QString &filename, CChannelBuffer& buffer, const uint SampleRate) override {
        std::vector<std::vector<float>> outData(buffer.channels());
        for (uint c = 0; c < buffer.channels(); ++c) {
            outData[c].resize(buffer.dataSize());
            for (ulong64 i = 0; i < buffer.dataSize(); ++i) {
                outData[c][i] = buffer.at(i, c);
            }
        }

        return avf_write_audio(filename.toStdString().c_str(), outData, SampleRate);
    }
};

#endif // AVFAUDIORW_H
