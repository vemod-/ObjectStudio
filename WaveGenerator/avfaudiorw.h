#ifndef AVFAUDIORW_H
#define AVFAUDIORW_H

#include "iwavefile.h"
#include "avfoundation_wrapper.h"

class CAvFoundationReader : public IWaveFile {
public:
    CAvFoundationReader(QString path) : IWaveFile(path) {
        double rate = 0;
        int chans = 0;
        if (avf_read_audio(path.toStdString().c_str(), data, rate, chans)) {
            m_Channels = chans;
            m_Frequency = rate;
        }
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
