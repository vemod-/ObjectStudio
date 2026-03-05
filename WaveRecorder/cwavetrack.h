#ifndef CWAVETRACK_H
#define CWAVETRACK_H

#include "cwavegenerator.h"
#include <QGraphicsScene>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QVideoSink>
#include <QtMultimedia/QVideoFrame>

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
    QPixmap videoThumbnail;
    long64 videoLength = 0;
    bool videoVisible = true;
    QPixmap getThumbnail(QString path) {
        QMediaPlayer player;
        QVideoSink sink;
        player.setVideoSink(&sink);
        player.setSource(path);
        QEventLoop loop;
        QPixmap tempPixmap;
        QObject::connect(&sink, &QVideoSink::videoFrameChanged,
                         &loop,
                         [&](const QVideoFrame &frame) {
                             if (!frame.isValid()) return;
                             tempPixmap = QPixmap::fromImage(frame.toImage());
                             player.stop();
                             loop.quit();
                         });
        // Timeout
        QTimer::singleShot(2000, &loop, [&]()
                           {
                               if (tempPixmap.isNull())
                               {
                                   //qWarning() << "Thumbnail timeout!";
                                   player.stop();
                                   loop.quit();
                               }
                           });
        player.play();
        loop.exec();
        return tempPixmap;
    }
private:

};

#endif // CWAVETRACK_H
