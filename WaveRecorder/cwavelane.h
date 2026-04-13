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
#include <QLineEdit>
#include <QToolButton>
#include <QGraphicsProxyWidget>
#include <QGraphicsObject>
#include "qtogglebutton.h"
#include "qlcdlabel.h"

class CWaveLaneSidebarItem : public QGraphicsObject {
    Q_OBJECT
public:
    CWaveLaneSidebarItem() : QGraphicsObject(){
        editWidget = new QGraphicsProxyWidget(this);
        editWidget->setWidget(&nameEdit);
        editWidget->setVisible(true);
        nameEdit.setFixedSize(120,18);
        editWidget->setParentItem(this);
        //nameEdit.setAttribute(Qt::WA_TranslucentBackground);
        editWidget->setPos(0,20);
        connect(&nameEdit,&QLCDEdit::editingFinished,this,&CWaveLaneSidebarItem::change);

        int l = 0;
        setupButton(muteButton,muteWidget,l);
        muteButton.setText("M");
        setupButton(soloButton,soloWidget,l + 30);
        soloButton.setText("S");
        setupButton(videoMuteButton,videoWidget,l + (30 * 2));
        videoMuteButton.setText("V");
        setupButton(automationButton,automationWidget,l + (30 * 3));
        automationButton.setText("A");

    }
    QRectF boundingRect() const override {
        return childrenBoundingRect();
    }
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override {
        // tom om du bara använder widgets
    }
    QLCDEdit nameEdit;
    QToggleButton muteButton;
    QToggleButton soloButton;
    QToggleButton videoMuteButton;
    QToggleButton automationButton;
signals:
    void changed(CWaveLaneSidebarItem* item);
private slots:
    void change() {
        emit changed(this);
    }
private:
    QGraphicsProxyWidget* editWidget;
    QGraphicsProxyWidget* muteWidget;
    QGraphicsProxyWidget* soloWidget;
    QGraphicsProxyWidget* videoWidget;
    QGraphicsProxyWidget* automationWidget;
    void setupButton(QToggleButton& b, QGraphicsProxyWidget* w, int left) {
        w = new QGraphicsProxyWidget();
        w->setWidget(&b);
        w->setVisible(true);
        b.setFixedSize(28,28);
        b.setCheckable(true);
        w->setParentItem(this);
        w->setPos(left,40);
        connect(&b,&QToolButton::clicked,this,&CWaveLaneSidebarItem::change);
        b.setAttribute(Qt::WA_TranslucentBackground);
    }
};

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
    void addFile(CWaveTrack* t);
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
                videoItem->name = alias();
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
    CWaveLaneSidebarItem sideBarItem;
    QList<int> DragTracks;
    QString ID;
    TrackEdges DragTrackEdge;
    CVideoItem* videoItem = nullptr;
    CVideoDialog* videoDialog;
private:
    std::atomic<ulong64> Counter;
    std::atomic<uint> ModulationCounter;
    int syncCounter;
    CChannelBuffer CurrentBuffer;
    smbPitchShifter pitchShifterL;
    smbPitchShifter pitchShifterR;
    std::array<smbPitchShifter*, 2> PS;
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
