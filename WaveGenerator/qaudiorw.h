#ifndef QAUDIORW_H
#define QAUDIORW_H

#include "iwavefile.h"
#include "cpresets.h"
#include "QApplication"
#include <QAudioDecoder>
#include <QAudioBuffer>
#include <QVector>
//#include <QMediaRecorder>
//#include <QMediaFormat>

class CQAudioDecoderReader : public IWaveFile {
public:
    CQAudioDecoderReader(QString path) : IWaveFile(path) {
        m_Frequency = CPresets::presets().SampleRate;
        m_Channels = 2;
        QAudioDecoder decoder;
        decoder.setSource(QUrl::fromLocalFile(path));

        QAudioFormat format;
        format.setSampleRate(CPresets::presets().SampleRate);
        format.setSampleFormat(QAudioFormat::Float);
        format.setChannelCount(2);
        decoder.setAudioFormat(format);

        decoder.start();
        data.resize(2);
        data[0].clear();
        data[1].clear();
        while (!decoder.error()) {
            QApplication::processEvents();
            if (decoder.bufferAvailable()) {
                QAudioBuffer buffer = decoder.read();
                processBuffer(buffer);
            }
            else {
                break;
            }
        }
    }

    void createFloatBuffer(CChannelBuffer& OutBuffer, const uint SampleRate) {
        if (SampleRate == m_Frequency) {
            const auto Length = ulong64(data[0].size());
            OutBuffer.init(Length, m_Channels);
            for (uint c = 0; c < m_Channels; c++) {
                memcpy(OutBuffer.channelPointer(c),data[c].data(),Length*sizeof(float));
                //OutBuffer.copyFloatBuffer(OutBuffer.channelPointer(c),data[c].data(),Length);
            }
        }
        else {
            const ldouble RateFactor = ldouble(m_Frequency) / SampleRate;
            const auto Length = ulong64(ldouble(data[0].size()) / RateFactor);
            OutBuffer.init(Length, m_Channels);
            for (uint c = 0; c < m_Channels; c++) {
                ldouble Ptr = 0;
                for (ulong64 i = 0; i < Length; i++) {
                    OutBuffer.setAt(i, c, data[c][Ptr]);
                    Ptr += RateFactor;
                }
            }
        }
    }

private:
    std::vector<std::vector<float>> data;
    void processBuffer(const QAudioBuffer &buffer) {
        const int channelCount = buffer.format().channelCount();
        const ulong64 sampleCount = buffer.sampleCount() / channelCount;
        float *rawData = (float*)buffer.constData<float>();

        for (ulong64 i = 0; i < sampleCount; i++) {
            for (int c = 0; c < channelCount; c++) {
                data[c].push_back(*rawData++); // Normalisera till -1.0 till 1.0
            }
        }
    }
};
/*
class CQAudioRecorderWriter : public IWaveFile {
public:
    CQAudioRecorderWriter() : IWaveFile() {}

    bool save(const QString &filename, CChannelBuffer& data, const uint SampleRate) {
        QMediaRecorder recorder;
        recorder.setOutputLocation(QUrl::fromLocalFile(filename));

        const QString s = filename.toLower();
        QMediaFormat::FileFormat f = QMediaFormat::MP3;
        if (s.endsWith(".m4a") || s.endsWith(".mp4")) {
            f = QMediaFormat::Mpeg4Audio;
        }
        else if (s.endsWith(".flac")) {
            f = QMediaFormat::FLAC;
        }
        else if (s.endsWith(".ogg")) {
            f = QMediaFormat::Ogg;
        }

        QMediaFormat format;
        format.setFileFormat(f);

        recorder.setMediaFormat(format);
        recorder.record();
        return true;
    }
};
*/

#endif // QAUDIORW_H
