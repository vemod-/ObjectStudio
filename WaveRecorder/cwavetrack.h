#ifndef CWAVETRACK_H
#define CWAVETRACK_H

#include "cwavegenerator.h"
#include <QGraphicsScene>
#include "avfoundation_wrapper.h"
#include <QImageReader>

class CWaveTrack
{
public:
    CWaveTrack(const QString& Filename, ulong64 StartPointer=0);
    QRect geometry;
    QString name;
    CWaveGenerator waveGenerator;
    CWaveGenerator::LoopParameters loopParameters;
    void paint(QGraphicsScene& Scene, ldouble ZoomFactor, QRect viewportGeometry, int edge);
    ulong64 start;
    ulong64 size;
    bool isValid;
    bool isActive;
    inline bool isImageFile(const QString& path)
    {
        QImageReader reader(path);
        return reader.canRead();
    }
    inline bool hasVideo() const {
        return !videoThumbnail.isNull();
    }
    inline bool hasImage() const {
        return !image.isNull();
    }
    inline bool hasVisible() const {
        return hasImage() || hasVideo();
    }
    inline bool hasOpacity() const {
        return (loopParameters.VideoOpacity < 1 || loopParameters.VideoFadeIn > 0 || loopParameters.VideoFadeOut > 0);
    }
    inline long64 length() const {
        return loopParameters.playLength();
    }
    inline ulong64 end() const {
        return start + length();
    }
    inline ulong64 pos(ldouble Counter) const {
        return ((Counter - start) * loopParameters.Speed) + loopParameters.Start;
    }
    inline ulong64 startPos() const {
        return loopParameters.Start;
    }
    inline float fadeOpacity(ulong64 Counter) const {
        return loopParameters.fadeOpacity(Counter - start);
    }
    inline float fadeVolume(ulong64 Counter) const {
        return loopParameters.fadeVolume(Counter - start);
    }
    inline float* getNext() {
        return waveGenerator.getNextSpeed(loopParameters.Speed);
    }
    inline int channels() const {
        return waveGenerator.channels();
    }
    inline double rate() const {
        return loopParameters.Speed;
    }
    void cutEnd(const long64 sample) {
        long64 s = sample;
        if (s < 0) s = 0;
        if (s < waveStart()) s = waveStart();
        if (!hasImage()) if (s > waveEnd()) s = waveEnd();
        loopParameters.End = (s - waveStart()) * loopParameters.Speed;
        if (hasImage()) size = loopParameters.End;
    }
    void cutStart(const long64 sample) {
        long64 s = sample;
        if (s < 0) s = 0;
        if (s < waveStart()) s = waveStart();
        if (s >= waveEnd()) s = waveEnd();
        loopParameters.Start = (s - waveStart()) * loopParameters.Speed;
        start = s;
        if (hasImage()) {
            size = loopParameters.End - loopParameters.Start;
            loopParameters.Start = 0;
            loopParameters.End = size;
        }
    }
    inline long64 waveStart() const {
        return start - (loopParameters.Start / loopParameters.Speed);
    }
    inline long64 waveEnd() {
        return waveStart() + (size / loopParameters.Speed);
    }
    QImage videoThumbnail;
    QImage image;
    inline QImage thumbnail() {
        if (!videoThumbnail.isNull()) return videoThumbnail;
        return image;
    }
    long64 videoLength = 0;
    bool videoVisible = false;
    QImage getThumbnail(QString path) const {
        QSize s = avf_displaySize(path);
        return avf_extract_fullframe(path).scaled(s,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    }
private:

};

#endif // CWAVETRACK_H
