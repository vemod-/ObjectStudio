#ifndef CWAVETRACK_H
#define CWAVETRACK_H

#include "cwavegenerator.h"
#include "qcanvas.h"
#include <QGraphicsScene>

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
private:

};

#endif // CWAVETRACK_H
