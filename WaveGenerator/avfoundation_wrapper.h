#ifndef AVFOUNDATION_WRAPPER_H
#define AVFOUNDATION_WRAPPER_H

#include <vector>
#include <QImage>
#include <QString>
#include <QSize>

bool avf_read_audio(const char* path,
                    std::vector<std::vector<float>> &outData,
                    double &outSampleRate,
                    int &outChannels);

bool avf_write_audio(const char* path,
                     const std::vector<std::vector<float>> &inData,
                     double sampleRate);
bool avf_has_video(const char* path);
bool avf_is_valid(const char* path);
bool avf_extract_thumbnail(const char* path,
                           double seconds,
                           std::vector<uint8_t>& outRGBA,
                           int& width,
                           int& height);
bool avf_naturalsize(const char* path,
                     int& width,
                     int& height);
bool avf_extract_fullframe(const char* path,
                           double seconds,
                           std::vector<unsigned char>& rgba,
                           int& width,
                           int& height);
double avf_lastVideoFrameTime(const char* path);

double avf_video_track_duration(const char* path);

class VideoExporter
{
public:
    VideoExporter(const QString& file,
                  QSize size,
                  int fps, int sampleRate, int channels);

    ~VideoExporter();

    bool addFrame(const QImage& img, quint64 frameIndex);
    bool addAudio(float* b);
    void finish(std::function<void()> done);

private:
    bool writeAudioBlock(float* b, int frames = 1024);
    struct Impl;
    Impl* d;
};

#endif // AVFOUNDATION_WRAPPER_H
