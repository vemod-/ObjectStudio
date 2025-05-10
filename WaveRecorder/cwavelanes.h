#ifndef CWAVELANES_H
#define CWAVELANES_H

#include <QMenu>
#include <QLabel>
#include "cdevicelist.h"
#include "cstereomixer.h"
#include "cmixerwidget.h"
#include "cwavelane.h"
#include "cdevicecontainer.h"
//#include <QGesture>
//#include <QGraphicsView>
#include <QGraphicsLineItem>
#include <QScrollBar>
#include "qgraphicsviewzoomer.h"
#include "ctimeline.h"
//#include "ceditmenu.h"
#include "cprojectapp.h"

namespace Ui {
    class CWaveLanes;
}

class CWaveLanes : public QGraphicsView, public IDevice//, public IEditDocument
{
    Q_OBJECT
public:
    explicit CWaveLanes(QWidget *parent = nullptr);
    ~CWaveLanes();
    void init(const int Index, QWidget* MainWindow);
    void reset();
    void stop();
    bool fileInUse(const QString& Filename);
    const QStringList fileList();
    void renameFile(const QString& oldName, const QString& newName);
    void removeFile(const QString& Filename);
    void updateMixer();
    void serialize(QDomLiteElement* xml) const;
    void unserialize(const QDomLiteElement* xml);
    CAudioBuffer* getNextA(const int ProcIndex);
    void play(const bool);
    void pause();
    void skip(const ulong64 samples);
    QRect visibleRect() {
        return QRect(horizontalScrollBar()->value(),verticalScrollBar()->value(),viewport()->width(),viewport()->height());
    }
    int rulerBeats;
    double rulerTempo;
    QList<CWaveLane*> lanes;
    QList<IDevice*> effects;
    CStereoMixer* m_Mixer;
    CMixerWidget* m_MixerWidget;
    QAction* QuantizeStraightAction;
    QAction* QuantizeTripletAction;
    QAction* AddLaneAction;
    QAction* RemoveLaneAction;
    QAction* RemoveTrackAction;
    QAction* CutAction;
    QAction* CopyAction;
    QAction* PasteAction;
    QAction* SplitAction;
    QAction* AutomationAction;
    QAction* EditTrackAction;
    QAction* EditLaneAction;
    QAction* EffectRackAction;

    CMainMenu* MainMenu;
    void DeleteDoc();
    void CopyDoc(QDomLiteElement* xml);
    void PasteDoc(const QDomLiteElement* xml);
    bool AddFile(QString FN,ulong64 Start);
    void AddLaneInternal();
public slots:
    void paint();
    void zoomIn();
    void zoomOut();
    void zoomMin();
    void zoomMax();
    void setEditMenu();
protected:
    bool event(QEvent* event);
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void dragEnterEvent(QDragEnterEvent *e);
    void dragMoveEvent(QDragMoveEvent* e);
    void dropEvent(QDropEvent *e);
    void timerEvent(QTimerEvent *);
    void mouseDoubleClickEvent(QMouseEvent *event);
signals:
    void FileAdded(QString path);
    void FileRemoved(QString path);
private slots:
    void AddLane();
    void RemoveLane();
    void UpdateEditTrack(CWaveGenerator::LoopParameters LP);
    void ShowMixer();
    void QuantizeStraight();
    void QuantizeTriplet();
    void Split();
    void Automation();
    void setZoom(double z);
    void UpdateAutomationGeometry();
    void EditTrack();
    void EditLane();
    void EffectRack();
    bool canCopy() { return (!CurrentTrack.isEmpty()) && (CurrentLane > -1); }
private:
    Ui::CWaveLanes *ui;
    QGraphicsViewZoomer* zoomer;
    CTimeLine m_TimeLine;
    QGraphicsScene Scene;
    CDeviceList deviceList;
    float MixFactor;
    void UpdateGeometry();
    int MouseOverLane(QPoint Pos);
    int MouseOverTrack(QPoint Pos, int Lane);
    void RemoveTrackAt(int Lane, int Track);
    ulong64 pos2Sample(int Pos) const;
    int sample2Pos(ulong64 sample) const;
    double sample2Beat(ulong64 sample, int div=1) const;
    ulong64 beat2Sample(int beat, int div=1) const;
    double quarterRate() const;
    void CalcMixFactor();
    void ShowInfoLabel(ulong64 Start,CWaveLane* Lane);
    void ShowInfoLabel(ulong64 Start,int Lane);
    QString DropFileName(const QMimeData* d, const QObject* s);
    QPoint StartPos;
    int CurrentLane;
    QList<int> CurrentTrack;
    int DragTrack;
    int m_OldDragLane;
    int m_OldDragTrack;
    QDomLiteElement* DragBackup = nullptr;
    int m_TimerID;
    bool Loading;
    QLabel* InfoLabel;
    CWaveTrack* m_EditTrack;
    static const int LaneHeight=80;
    static const int LaneGap=4;
    static const int RulerHeight=20;
    static const int BorderWidth=8;
    static const int LaneTrail=50;
    QList<CDeviceContainer*> Effects;
    int m_EditLane = -1;
    double m_EditZoom = 1;
};

#endif // CWAVELANES_H
