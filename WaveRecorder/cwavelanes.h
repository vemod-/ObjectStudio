#ifndef CWAVELANES_H
#define CWAVELANES_H

#include <QMenu>
#include <QLabel>
#include "cdevicelist.h"
#include "cstereomixer.h"
#include "cmixerwidget.h"
#include "cwavelane.h"
#include "cdevicecontainer.h"
#include <QGraphicsLineItem>
#include <QScrollBar>
#include "qgraphicsviewzoomer.h"
#include "ctimeline.h"
#include "cprojectapp.h"
#include "avfoundation_wrapper.h"

namespace Ui {
    class CWaveLanes;
}

class CWaveLanes : public QGraphicsView, public IDevice//, public IEditDocument
{
    Q_OBJECT
public:
    explicit CWaveLanes(QWidget *parent = nullptr);
    ~CWaveLanes();
    void init(const int Index, QWidget* MainWindow) override;
    void reset();
    void stop();
    bool fileInUse(const QString& Filename);
    const QStringList fileList();
    void renameFile(const QString& oldName, const QString& newName);
    void removeFile(const QString& Filename);
    void updateMixer();
    void serialize(QDomLiteElement* xml) const;
    void unserialize(const QDomLiteElement* xml);
    CAudioBuffer* getNextA(const int ProcIndex) override;
    void play(const bool) override;
    void pause() override;
    void skip(const ulong64 samples) override;
    int rulerBeats;
    double rulerTempo;
    QList<CWaveLane*> lanes;
    QList<IDevice*> effects;
    CStereoMixer* m_Mixer;
    CMixerWidget* m_MixerWidget;
    CVideoDialog* videoWindow = nullptr;
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
    QAction* VideoWidgetAction;
    QAction* VideoTrackAction;

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
    void exportAudio(const QString &filename) {
        //QFile(filename).remove();
        IDevice::exportWave(filename);
    }
    void exportVideo(const QString& filename) {
        if (!videoWindow) return;
        const QSize outputSize(videoWindow->outputSize());
        const qreal frameRate = 30;
        QFile(filename).remove();

        QGraphicsScene* tempScene = videoWindow->scene();
        videoWindow->setScene(nullptr);
        CChannelBuffer audio = IDevice::render();

        VideoExporter exporter(filename, outputSize, frameRate, CPresets::presets().SampleRate, 2);

        ulong64 mSec = 0;
        for (CWaveLane* l : std::as_const(lanes)) {
            ulong64 ms = l->milliSeconds();
            if (ms > mSec) mSec = ms;
        }
        ulong64 totalFrames = frameRate * mSec / 1000.0;

        //QImage img(outputSize, QImage::Format_ARGB32);
        QImage img(outputSize, QImage::Format_ARGB32_Premultiplied);
        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        for (CWaveLane* l : std::as_const(lanes)) {
            if (l->videoItem) {
                l->videoItem->setRenderRect(videoWindow->resolution());
                l->videoItem->setVisible(l->videoVisible);
            }
        }
        abortExport = false;
        CVideoProgressWindow exportProgress;
        exportProgress.setMax(totalFrames);
        exportProgress.setVisible(true);
        connect(&exportProgress,&CVideoProgressWindow::abort,
                this,
                [this]()
                {
                    abortExport = true;
                });

        setExportMode(true);

        CChannelBuffer frameBuffer(CPresets::presets().SampleRate / frameRate,2);
        ulong64 sample = 0;
        for (ulong64 f = 0; f < totalFrames; ++f)
        {
            double t = f / frameRate;
            exportProgress.setValue(f);
            getExportFrame(t);
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

            img.fill(Qt::black);
            for (CWaveLane* l : std::as_const(lanes)) {
                if (l->videoItem) {
                    if (l->videoItem->isVisible()) l->videoItem->paint(&p,nullptr,nullptr);
                }
            }
            exporter.addFrame(img,f);
            frameBuffer.copy(audio,sample);
            std::vector<float>b = frameBuffer.toInterleaved();
            sample += frameBuffer.size();
            exporter.addAudio(b.data());
            if (abortExport) break;
        }

        QEventLoop loop;
        exporter.finish([&](){
            loop.quit();
        });
        loop.exec();
        exportProgress.setVisible(false);

        setExportMode(false);

        if (abortExport) {
            QFile(filename).remove();
            abortExport = false;
        }
        videoWindow->setScene(tempScene);
    }
protected:
    bool event(QEvent* event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent* e) override;
    void dropEvent(QDropEvent *e) override;
    void timerEvent(QTimerEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override
    {
        Q_UNUSED(rect);
        m_TimeLine.drawPlayLine(painter);
    }
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
    QGraphicsProxyWidget* addProxyWidget(QWidget* a) {
        a->resize(lanes[CurrentLane]->geometry.adjusted(0,0,-50,0).size());
        QGraphicsProxyWidget* w = Scene.addWidget(a);
        w->setZValue(5);
        return w;
    }
    QList<QWidget*> ProxyWidgets() const {
        QList<QWidget*> l;
        const QGraphicsItemList g(Scene.items());
        for (QGraphicsItem* i : g) {
            if (auto proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(i)) {
                if (i->zValue() > 4) l.append(proxy->widget());
            }
        }
        return l;
    }
    bool automationVisible(const QPointF& scenePos) {
        QGraphicsItem* w = Scene.itemAt(scenePos,transform());
        if (w) {
            if (w->zValue() > 4) return true;
        }
        return false;
    }
    void Automation();
    void setZoom(double z);
    void ZoomToCursor(double z, double o);
    void UpdateAutomationGeometry();
    void deleteAutomation();
    void EditTrack();
    void updateVideoWindow(int Lane = -1) {
        if (m_Playing) return;
        if (Lane == -1) Lane = CurrentLane;
        if (Lane > -1) lanes[CurrentLane]->skip(requestCurrentSample());
    }
    void ToggleLaneVideo() {
        if (CurrentLane > -1) {
            lanes[CurrentLane]->videoVisible = !lanes[CurrentLane]->videoVisible;
            updateVideoWindow();
            //lanes[CurrentLane]->videoItem->setVisible(lanes[CurrentLane]->videoVisible);
        }
        setEditMenu();
    }
    void ToggleTrackVideo() {
        if (CurrentLane > -1) {
            for (int t : std::as_const(CurrentTrack)) {
                lanes[CurrentLane]->tracks[t]->videoVisible = !lanes[CurrentLane]->tracks[t]->videoVisible;
            }
            updateVideoWindow();
        }
        setEditMenu();
    }
    void EditLane();
    void EffectRack();
    bool canCopy() { return (!CurrentTrack.isEmpty()) && (CurrentLane > -1); }
    bool canVideo() { if (CurrentLane > -1) {
            if (lanes[CurrentLane]->hasVideo()) return true;
        }
        return false;
    }
    bool trackCanVideo() {
        if (CurrentLane > -1 ) {
            if (!CurrentTrack.isEmpty()) {
                if (lanes[CurrentLane]->tracks[CurrentTrack.first()]->waveGenerator.hasVideo()) {
                    return true;
                }
            }
        }
        return false;
    }
    void setExportMode(bool m) {
        for (CWaveLane* l : std::as_const(lanes)) {
            if (l->videoItem) l->setExportMode(m);
        }
    }
    void getExportFrame(double t) {
        for (CWaveLane* l : std::as_const(lanes)) {
            if (l->videoItem) l->setExportTime(t);
        }
    }
private:
    Ui::CWaveLanes *ui;
    QGraphicsViewZoomer* zoomer;
    CTimeLine m_TimeLine;
    QGraphicsScene Scene;
    CDeviceList deviceList;
    QEventLoop m_loop;
    std::atomic<int> pendingFrames = 0;
    std::atomic<bool> abortExport = false;
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
    bool MD = false;
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
