#ifndef CWAVELANE_H
#define CWAVELANE_H

#include "cwavetrack.h"
#include "smbpitchshifter.h"
#include <QGraphicsScene>
#include "ctimeline.h"
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimediaWidgets/QVideoWidget>
#include <unistd.h>

class CVideoWindow : public QWidget {
    Q_OBJECT
public:
    CVideoWindow(QWidget* parent = nullptr) : QWidget(parent) {
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
        //setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        QPalette p(palette());
        p.setBrush(QPalette::Window,Qt::black);
        setPalette(p);
        m_Layout = new QGridLayout(this);
        m_Layout->setSpacing(0);
        m_Layout->setContentsMargins(0,0,0,0);
        setLayout(m_Layout);
        relayout();
    }
    void relayout()
    {
        const QList<QVideoWidget*> visibleVids = visibleVideos();
        int N = visibleVids.size();

        while (auto* i = m_Layout->takeAt(0)) delete i;

        if (N == 0) {
            hide();
            return;
        }

        int cols = std::ceil(std::sqrt(N));
        //int rows = std::ceil(double(N) / cols);

        for (int i = 0; i < N; ++i)
            m_Layout->addWidget(visibleVids[i], i / cols, i % cols);

        setVisible(!visibleVids.isEmpty());
        //adjustSize();     // ← viktigt
    }
    void addVideo(QVideoWidget* v) {
        if (m_Videos.contains(v)) return;
        m_Videos.append(v);
        relayout();
    }
    void removeVideo(QVideoWidget* v) {
        if (!m_Videos.contains(v)) return;
        m_Videos.removeOne(v);
        relayout();
    }
    void toggleVideo(QVideoWidget* v) {
        //v->setVisible(!v->isVisible());
        v->setHidden(!v->isHidden());
        relayout();
    }
    QSize sizeHint() const override
    {
        return QSize(640,480);
        /*
        int N = m_Videos.size();
        if (N == 0)
            return QSize(320, 240);   // fallback

        int cols = std::ceil(std::sqrt(N));
        int rows = std::ceil(double(N) / cols);

        // Ta sizeHint från en video (de brukar vara lika)
        QSize videoSize = m_Videos.first()->sizeHint();
        if (videoSize.isEmpty()) videoSize = QSize(320, 240);

        return QSize(videoSize.width() * cols,
                     videoSize.height() * rows);
*/
    }
protected:
    void showEvent(QShowEvent* e) override {
        QWidget::showEvent(e);
        relayout();
    }
private:
    QList<QVideoWidget*> visibleVideos() {
        QList<QVideoWidget*> l;
        for (QVideoWidget* v : std::as_const(m_Videos)) {
            if (!v->isHidden()) l.append(v);
        }
        return l;
    }
    QGridLayout* m_Layout;
    QList<QVideoWidget*> m_Videos;
};

class CVideoWidget : public QVideoWidget {
    Q_OBJECT
public:
    CVideoWidget(QWidget* parent = nullptr) : QVideoWidget(parent) {
        mediaplayer = new QMediaPlayer(this);
        mediaplayer->setVideoOutput(this);
        mediaplayer->setAudioOutput(nullptr);
        mediaplayer->setActiveAudioTrack(-1);
        setVisible(true);
    }
public slots:
    /*
    void invokeSetProperties(CWaveTrack* t, ulong64 sample, bool play = false) {
        if (m_State == QMediaPlayer::PlayingState) play = false;
        QMetaObject::invokeMethod(
            this,
            [this, t, sample, play]() {
                setVideoProperties(t,sample);
                if (play) playVideo(t);
            },
            Qt::QueuedConnection
            );
    }
*/
    void invokePlay(CWaveTrack* t) {
        if (isHidden()) return;
        if (m_State == QMediaPlayer::PlayingState) return;
        QMetaObject::invokeMethod(
            this,
            [this, t]() {
                playVideo(t);
            },
            Qt::QueuedConnection
            );
    }
    void invokeStop() {
        if (isHidden()) return;
        if (m_State != QMediaPlayer::PlayingState) return;
        QMetaObject::invokeMethod(
            this,
            [this]() {
                stopVideo();
            },
            Qt::QueuedConnection
            );
    }
    bool setVideoProperties(CWaveTrack* t, ulong64 sample) {
        if (isHidden()) return false;
        if (t->waveGenerator.hasVideo()) {
            qDebug() << mediaplayer->source() << t->waveGenerator.videoURL;
            if (mediaplayer->source() != t->waveGenerator.videoURL) {
                mediaplayer->setSource(t->waveGenerator.videoURL);
            }
            const long64 mSec = CPresets::samplesTomSecs(sample);
            if (std::llabs(mediaplayer->position() - mSec) > 5)
                mediaplayer->setPosition(mSec);
            if (!closeEnough(mediaplayer->playbackRate(),t->loopParameters.Speed)) {
                mediaplayer->setPlaybackRate(t->loopParameters.Speed);
            }
            return true;
        }
        return false;
    }
    void playVideo(CWaveTrack* t) {
        if (isHidden()) return;
        if (t->waveGenerator.hasVideo()) {
            if (mediaplayer->source() == t->waveGenerator.videoURL) {
                if (m_State != QMediaPlayer::PlayingState) {
                    mediaplayer->play();
                    m_State = QMediaPlayer::PlayingState;
                }
            }
        }
        else {
            stopVideo();
        }
    }
    void stopVideo() {
        if (isHidden()) return;
        //if (mediaplayer->isPlaying())
        if (m_State == QMediaPlayer::PlayingState) {
            mediaplayer->pause();
            m_State = QMediaPlayer::PausedState;
        }
    }
private:
    QMediaPlayer::PlaybackState m_State = QMediaPlayer::StoppedState;
    QMediaPlayer* mediaplayer;
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
    int MouseOverTrack(QPoint Pos);
    void drawOutsideWave(QGraphicsScene& Scene, QRect visibleRect);
    long64 handleMousePress(QPoint p);
    long64 handleMouseMove(QPoint p, CTimeLine* timeLine);
    CWaveTrack* handleMouseRelease();
    void sanityCheck(CWaveTrack* d);
    ulong64 pos2Sample(int Pos) const;
    int sample2Pos(long64 sample) const;
    QList<CParameterWrapper*> parameters;
    bool hasVideo() {
        for (CWaveTrack* t : std::as_const(tracks)) {
            if (t->waveGenerator.hasVideo()) return true;
        }
        return false;
    }
    QPixmap videoThumbnail() {
        for (CWaveTrack* t : std::as_const(tracks)) {
            if (t->waveGenerator.hasVideo()) return t->videoThumbnail;
        }
        return QPixmap();
    }
    /*
    void showVideoWidget() {
        if (hasVideo()) {
            if (videoWidget) {
                videoWidget->setVisible(true);
            }
        }
    }
    void hideVideoWidget() {
        if (hasVideo()) {
            if (videoWidget) {
                videoWidget->setVisible(false);
            }
        }
    }
*/
    void toggleVideoWidget() {
        if (hasVideo()) {
            if (videoWidget) {
                videoWindow->toggleVideo(videoWidget);
            }
        }
    }
    void createVideoWidget() {
        if (hasVideo()) {
            if (!videoWidget) {
                videoWidget = new CVideoWidget();
                videoWidget->setWindowTitle("Lane " + QString::number(m_Index));
                videoWindow->addVideo(videoWidget);
            }
        }
    }
    void destroyVideoWidget() {
        if (!hasVideo()) {
            if (videoWidget) {
                videoWindow->removeVideo(videoWidget);
                delete videoWidget;
                videoWidget = nullptr;
            }
        }
    }
    //CEffectRack* m_EffectRack;
    QList<int> DragTracks;
    QString ID;
    TrackEdges DragTrackEdge;
    CVideoWidget* videoWidget = nullptr;
    CVideoWindow* videoWindow;
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
    QList<ulong64> DragTrackStarts;
    bool closeToLine(const ulong64 sample, CTimeLine* timeLine) const;
    long64 snapTo(const long64 sample, const long64 snapSample, CTimeLine* timeLine) const;
};

#endif // CWAVELANE_H
