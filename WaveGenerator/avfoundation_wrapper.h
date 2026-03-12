#ifndef AVFOUNDATION_WRAPPER_H
#define AVFOUNDATION_WRAPPER_H

#include <vector>
#include <QImage>
#include <QString>
#include <QSize>
#include <QUrl>

bool avf_read_audio(const QString& path,
                    std::vector<std::vector<float>> &outData,
                    double &outSampleRate,
                    int &outChannels);

bool avf_write_audio(const QString& path,
                     const std::vector<std::vector<float>> &inData,
                     double sampleRate);
bool avf_has_video(const QString& path);
bool avf_is_valid(const QString& path);
/*
bool avf_extract_thumbnail(const QString& path,
                           double seconds,
                           std::vector<uint8_t>& outRGBA,
                           int& width,
                           int& height);

bool avf_naturalsize(const QString& path,
                     int& width,
                     int& height);
*/
QImage avf_extract_fullframe(const QString& path,
                           double seconds = 0);
//double avf_lastVideoFrameTime(const QString& path);

double avf_video_track_duration(const QString& path);

QSize avf_displaySize(const QString& path);

class ImageExtractor {
public:
    ImageExtractor();
    ~ImageExtractor();
    void setSource(const QUrl& url, const QSize& s);
    QImage getImage(double time);
    QUrl videoUrl;
    QSize frameSize;
private:
    struct Impl;
    Impl* d;
};

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

class AVFVideoPlayer
{
public:
    AVFVideoPlayer();
    ~AVFVideoPlayer();

    void setSource(const QUrl& url);
    void play();
    void pause();
    void setPosition(double seconds);
    double position() const;
    double duration() const;
    void setPlaybackRate(double rate);
    double playbackRate() const;
    QImage currentFrame();
    QUrl Url;
    bool playing = false;
private:
    struct Impl;
    Impl* d;
};
#endif // AVFOUNDATION_WRAPPER_H
