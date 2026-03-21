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
    bool hasOpacity() {
        return (loopParameters.VideoOpacity < 1 || loopParameters.VideoFadeIn > 0 || loopParameters.VideoFadeOut > 0);
    }
    long64 length() const {
        return loopParameters.playLength();
    }
    ulong64 end() const {
        return start + length();
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
    long64 waveStart() const {
        return start - (loopParameters.Start / loopParameters.Speed);
    }
    long64 waveEnd() {
        return waveStart() + (waveGenerator.size() / loopParameters.Speed);
    }
    QImage videoThumbnail;
    long64 videoLength = 0;
    bool videoVisible = true;
    QImage getThumbnail(QString path) {
        QSize s = avf_displaySize(path);
        return avf_extract_fullframe(path).scaled(s,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    }
private:

};

#endif // CWAVETRACK_H
