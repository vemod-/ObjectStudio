#ifndef CWAVETRACK_H
#define CWAVETRACK_H

#include "../WaveGenerator/cwavegenerator.h"
#include <QGraphicsScene>
#include "../WaveGenerator/avfoundation_wrapper.h"
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
    static inline bool isImageByExtension(const QString& path)
    {
        static const QSet<QString> exts = {
            "png","jpg","jpeg","bmp","gif","webp","tiff"
        };

        QString ext = QFileInfo(path).suffix().toLower();
        return exts.contains(ext);
    }
    static inline bool isImageFile(const QString& path)
    {
        if (!isImageByExtension(path)) return false;
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
    inline ulong64 pos(ulong64 Counter) const {
        return loopParameters.pos(Counter - start);
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
    void cropEnd(const long64 sample) {
        long64 s = std::max(sample,waveStart());
        if (!hasImage()) s = std::min(s,waveEnd());
        loopParameters.End = (s - waveStart()) * loopParameters.Speed;
        if (hasImage()) size = loopParameters.End;
    }
    void stretchEnd(const long64 sample) {
        if (hasImage()) {
            cropEnd(sample);
            return;
        }
        const ldouble origlen = loopParameters.End - loopParameters.Start;
        const ldouble newLen = (sample - start) - loopParameters.Start;
        loopParameters.stretch(origlen / newLen);
    }
    void cropStart(const long64 sample) {
        const long64 s = std::clamp(sample,waveStart(),waveEnd());
        loopParameters.Start = (s - waveStart()) * loopParameters.Speed;
        start = s;
        if (hasImage()) {
            size = loopParameters.End - loopParameters.Start;
            loopParameters.Start = 0;
            loopParameters.End = size;
        }
    }
    void stretchStart(const long64 sample) {
        if (hasImage()) {
            cropStart(sample);
            return;
        }
        long64 s = sample;
        if (s < 0) s = 0;
        const ldouble origlen = loopParameters.End - loopParameters.Start;
        const ldouble newLen = end() - s;
        loopParameters.stretch(origlen / newLen);
        start = s;
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
        const QSize s = avf_displaySize(path);
        return avf_extract_fullframe(path).scaled(s,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    }
private:

};

#endif // CWAVETRACK_H
