#ifndef CWAVELANE_H
#define CWAVELANE_H

#include "cwavetrack.h"
#include "smbpitchshifter.h"
#include <QGraphicsScene>
#include "ctimeline.h"
//#include <QtMultimedia/QMediaPlayer>
//#include <QtMultimediaWidgets/QVideoWidget>
#include <unistd.h>
#include "cvideodesigner.h"

class CWaveLane : public IDevice
{
public:
    enum TrackEdges {
        NoEdge,
        FrontEdge,
        EndEdge
    };
    CWaveLane();
    ~CWaveLane();
    void play(const bool FromStart);
    void pause();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
    QRect geometry;
    void pitchShift(CWaveTrack* T);
    //void modifyBuffers(CStereoBuffer* Buffer, const float MixFactor);
    void reset();
    void UpdateGeometry(ldouble ZoomFactor, long CanvasRight);
    void paint(QGraphicsScene& Scene, ldouble ZoomFactor, QRect viewportGeometry, bool Active);
    void paintTrack(int Track, QGraphicsScene& Scene, ldouble ZoomFactor, QRect visibleRect, int edge);
    int paintEdges(QPoint p, int t, QGraphicsScene& Scene, ldouble ZoomFactor, QRect visibleRect);
    QList<CWaveTrack*> tracks;
    bool fileInUse(const QString& Filename);
    const QStringList fileList();
    void renameFile(const QString& oldName, const QString& newName);
    void removeFile(const QString& Filename);
    void addFile(CWaveTrack* t) {
        for (CWaveTrack* track : std::as_const(tracks)) {
            if (track) {
                if ((track != t) && (track->isValid) && (t->isValid)) {
                    if ((t->end() >= track->start) && (t->start <= track->start)) {
                        t->cutEnd(track->start);
                    }
                }
            }
        }
        tracks.append(t);
        sanityCheck(t);
        createVideoWidget();
    }
    void serialize(QDomLiteElement* xml) const;
    void unserialize(const QDomLiteElement* xml,ldouble ZoomFactor);
    void serializeTrack(QDomLiteElement* xml, const CWaveTrack* WT) const;
    CWaveTrack* unserializeTrack(const QDomLiteElement* xml, ldouble ZoomFactor);
    CWaveTrack* cloneTrack(const CWaveTrack* WT, ldouble ZoomFactor);
    ulong milliSeconds() const;
    ulong64 samples() const;
    void skip(const ulong64 samples);
    bool setVideoExportTime(const ulong64 mSec);
    int MouseOverTrack(QPoint Pos);
    void drawOutsideWave(QGraphicsScene& Scene, QRect visibleRect);
    long64 handleMousePress(QPoint p);
    long64 handleMouseMove(QPoint p, CTimeLine* timeLine);
    CWaveTrack* handleMouseRelease();
    void sanityCheck(CWaveTrack* d);
    ulong64 pos2Sample(int Pos) const;
    int sample2Pos(long64 sample) const;
    bool videoVisible = true;
    QList<CParameterWrapper*> parameters;
    bool hasVisible() {
        for (CWaveTrack* t : std::as_const(tracks)) {
            if (t->hasVisible()) return true;
        }
        return false;
    }
    QImage thumbnailImage() {
        for (CWaveTrack* t : std::as_const(tracks)) {
            if (t->hasVisible()) return t->thumbnail();
        }
        return QImage();
    }
    bool trackVisible(CWaveTrack* t) {
        return (t->videoVisible & videoVisible & t->hasVisible());
    }
    void createVideoWidget() {
        if (hasVisible()) {
            if (!videoItem) {
                videoItem = new CVideoItem(thumbnailImage());
                videoItem->name = "Lane " + QString::number(m_Index);
                videoDialog->addVideo(videoItem);
            }
        }
    }
    void destroyVideoWidget() {
        if (!hasVisible()) {
            if (videoItem) {
                videoDialog->removeVideo(videoItem);
                videoItem->deleteLater();
                videoItem = nullptr;
            }
        }
    }
    inline bool setExportTime(double sec)
    {
        return setVideoExportTime(sec * 1000.0);
    }
    void setExportMode(bool m) {
        if (videoItem) videoItem->setExportMode(m);
    }
    //CEffectRack* m_EffectRack;
    QList<int> DragTracks;
    QString ID;
    TrackEdges DragTrackEdge;
    CVideoItem* videoItem = nullptr;
    CVideoDialog* videoDialog;
private:
    ulong64 Counter;
    uint ModulationCounter;
    int syncCounter;
    CChannelBuffer CurrentBuffer;
    CStereoBuffer TempBuffer;
    float Vol;
    smbPitchShifter pitchShifterL;
    smbPitchShifter pitchShifterR;
    smbPitchShifter* PS[2];
    ulong64 DragTrackStart;
    ulong64 DragTrackEnd;
    QRect waveRect;
    int DragTrack;
    QPoint StartPos;
    ldouble m_Zoom;
    QList<ulong64> DragTrackStarts;
    bool closeToLine(const ulong64 sample, CTimeLine* timeLine) const;
    long64 snapTo(const long64 sample, const long64 snapSample, CTimeLine* timeLine) const;
};

#endif // CWAVELANE_H
