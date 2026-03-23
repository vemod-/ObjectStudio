#ifndef CWAVETRACK_H
#define CWAVETRACK_H

#include "cwavegenerator.h"
#include <QGraphicsScene>
#include "avfoundation_wrapper.h"

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
    bool isValid;
    bool isActive;
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
    inline float fadeOpacity(ulong64 Counter) const {
        return loopParameters.fadeOpacity(Counter - start);
    }
    inline float fadeVolume(ulong64 Counter) const {
        return loopParameters.fadeVolume(Counter - start);
    }
    void cutEnd(const long64 sample) {
        long64 s = sample;
        if (s < 0) s = 0;
        if (s < waveStart()) s = waveStart();
        if (s > waveEnd()) s = waveEnd();
        loopParameters.End = (s - waveStart()) * loopParameters.Speed;
    }
    void cutStart(const long64 sample) {
        long64 s = sample;
        if (s < 0) s = 0;
        if (s < waveStart()) s = waveStart();
        if (s >= waveEnd()) s = waveEnd();
        loopParameters.Start = (s - waveStart()) * loopParameters.Speed;
        start = s;
    }
    inline long64 waveStart() const {
        return start - (loopParameters.Start / loopParameters.Speed);
    }
    inline long64 waveEnd() {
        return waveStart() + (waveGenerator.size() / loopParameters.Speed);
    }
    QImage videoThumbnail;
    long64 videoLength = 0;
    bool videoVisible = true;
    QImage getThumbnail(QString path) const {
        QSize s = avf_displaySize(path);
        return avf_extract_fullframe(path).scaled(s,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    }
private:

};

#endif // CWAVETRACK_H
