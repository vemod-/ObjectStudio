#ifndef CWAVELANE_H
#define CWAVELANE_H

#include "cwavetrack.h"
#include "smbpitchshifter.h"
#include <QGraphicsScene>
#include "ctimeline.h"
#include "ceffectrack.h"

class CWaveLane : public IDevice
{
public:
    CWaveLane();
    ~CWaveLane();
    void play(const bool FromStart);
    //void pause();
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
    void serialize(QDomLiteElement* xml) const;
    void unserialize(const QDomLiteElement* xml,ldouble ZoomFactor);
    void serializeTrack(QDomLiteElement* xml, const CWaveTrack* WT) const;
    CWaveTrack* unserializeTrack(const QDomLiteElement* xml, ldouble ZoomFactor);
    CWaveTrack* cloneTrack(const CWaveTrack* WT, ldouble ZoomFactor);
    ulong milliSeconds() const;
    ulong64 samples() const;
    void skip(const ulong64 samples);
    int MouseOverTrack(QPoint Pos);
    void drawOutsideWave(QGraphicsScene& Scene, QRect visibleRect);
    long64 handleMousePress(QPoint p);
    long64 handleMouseMove(QPoint p, CTimeLine* timeLine);
    CWaveTrack* handleMouseRelease();
    void sanityCheck(CWaveTrack* d);
    ulong64 pos2Sample(int Pos) const;
    int sample2Pos(ulong64 sample) const;
    QList<CParameterWrapper*> parameters;
    //CEffectRack* m_EffectRack;
    QList<int> DragTracks;
    QString ID;
private:
    ulong64 Counter;
    uint ModulationCounter;
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
    enum TrackEdges {
        NoEdge,
        FrontEdge,
        EndEdge
    };
    TrackEdges DragTrackEdge;
    QList<ulong64> DragTrackStarts;
    bool closeToLine(const ulong64 sample, CTimeLine* timeLine) const;
    long64 snapTo(const long64 sample, const long64 snapSample, CTimeLine* timeLine) const;
};

#endif // CWAVELANE_H
