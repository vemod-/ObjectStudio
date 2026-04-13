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

class CWaveLanesSidebar : public QGraphicsItem {
public:
    CWaveLanesSidebar(int closedWidth, int openWidth, QGraphicsItem* parent = nullptr) : QGraphicsItem(parent) {
        m_closedWidth = closedWidth;
        m_openWidth = openWidth;
        m_rect.setWidth(closedWidth);
        setOpacity(1);
        setZValue(9999);
        setAcceptHoverEvents(true);
        setAcceptedMouseButtons(Qt::LeftButton);
        setVisible(true);
    }
    QRectF boundingRect() const override {
        return m_rect;
    }
    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem*,
               QWidget*) override {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(0,0,0,(m_Open) ? 60 : 20));
        painter->drawRect(m_rect);
    }
    void setHeight(int h) {
        prepareGeometryChange();
        m_rect.setHeight(h);
        update();
    }
    bool isOpen() {
        return m_Open;
    }
    void setPopup(bool p) {
        m_PopupActive = p;
    }
    void collapse() {
        if (m_Open) {
            m_Open = false;
            prepareGeometryChange();
            m_rect.setWidth(currentWidth());
            for (CWaveLaneSidebarItem* i : laneItems()) i->setVisible(false);
            update();
        }
    }
    void open() {
        if (!m_Open) {
            m_Open = true;
            prepareGeometryChange();
            m_rect.setWidth(currentWidth());
            for (CWaveLaneSidebarItem* i : laneItems()) i->setVisible(true && i->isEnabled());
            update();
        }
    }
    QList<CWaveLaneSidebarItem*> laneItems() {
        QList<CWaveLaneSidebarItem*> l;
        for (QGraphicsItem* i : childItems()) {
            if (CWaveLaneSidebarItem* li = qgraphicsitem_cast<CWaveLaneSidebarItem*>(i)) {
                l.append(li);
            }
        }
        return l;
    }
protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent*) override {
        qDebug() << "HoverEnter";
        open();
    }
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override {
        qDebug() << "HoverLeave";
        if (!m_PopupActive) collapse();
    }
private:
    QRect m_rect = QRect(0,0,20,1);
    bool m_Open = false;
    bool m_PopupActive = false;
    int m_openWidth = 140;
    int m_closedWidth = 20;
    int currentWidth() {
        return (m_Open) ? m_openWidth : m_closedWidth;
    }
};

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
    //int rulerBeats;
    //double rulerTempo;
    QList<CWaveLane*> lanes;
    QList<IDevice*> effects;
    CStereoMixer* m_Mixer;
    CMixerWidget* m_MixerWidget;
    CVideoDialog* videoWindow = nullptr;
    QAction* QuantizeStraightAction;
    QAction* QuantizeTripletAction;
    QAction* AddLaneAction;
    QAction* InsertLaneAction;
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
    void AddLaneInternal(int index = -1);
public slots:
    void paint();
    void zoomIn();
    void zoomOut();
    void zoomMin();
    void zoomMax();
    void setEditMenu();
    void exportLaneAudio(const QString& filename);
    void exportAudio(const QString &filename);
    void exportVideo(const QString& filename);
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
    void drawForeground(QPainter *painter, const QRectF &rect) override;
signals:
    void FileAdded(QString path);
    void FileRemoved(QString path);
private slots:
    void AddLane();
    void InsertLane();
    void RemoveLane();
    void UpdateEditTrack(CWaveGenerator::LoopParameters LP);
    void ShowMixer();
    void QuantizeStraight();
    void QuantizeTriplet();
    void Split();
    QGraphicsProxyWidget* addProxyWidget(QWidget* a, int lane);
    QList<QGraphicsItem*> ProxyItems() const;
    QList<QWidget*> ProxyWidgets() const;
    bool automationVisible(const QPointF& scenePos);
    CAutomationLane* automationWidget(int lane);
    void Automation(int lane = -1);
    void setZoom(double z);
    void ZoomToCursor(double z, double o);
    void UpdateAutomationGeometry();
    void closeAutomation() override ;
    void EditTrack();
    void updateVideoWindow(int Lane = -1);
    void ToggleLaneVideo();
    void ToggleTrackVideo();
    void EditLane();
    void EffectRack();
    bool canCopy();
    bool canVideo();
    bool trackCanVideo();
    bool trackIsImage();
    void setExportMode(bool m);
    void getExportFrame(double t);
    void sidebarItemChanged(CWaveLaneSidebarItem* item);
private:
    Ui::CWaveLanes *ui;
    QGraphicsViewZoomer* zoomer;
    CTimeLine m_TimeLine;
    CWaveLanesSidebar* m_sidebarItem;
    QGraphicsScene Scene;
    CDeviceList deviceList;
    QEventLoop m_loop;
    std::atomic<int> pendingFrames = 0;
    std::atomic<bool> abortExport = false;
    QString LaneID(int i) {
        return"Lane " + QString::number(i + 1);
    }
    void UpdateGeometry();
    int MouseOverLane(QPoint Pos);
    int MouseOverTrack(QPoint Pos, int Lane);
    void RemoveTrackAt(int Lane, int Track);
    ulong64 pos2Sample(int Pos) const;
    int sample2Pos(ulong64 sample) const;
    //double sample2Beat(ulong64 sample, int div=1) const;
    //ulong64 beat2Sample(int beat, int div=1) const;
    //double quarterRate() const;
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
