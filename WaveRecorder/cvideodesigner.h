#ifndef CVIDEODESIGNER_H
#define CVIDEODESIGNER_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QtMultimedia/QMediaPlayer>
#include <QMap>
#include <QGraphicsObject>
#include <QtMultimedia/QVideoSink>
#include <QtMultimedia/QVideoFrame>
#include <QtMultimedia/QMediaMetaData>
#include <QImage>
#include "cwavetrack.h"
#include <QPinchGesture>
#include <QDialog>
#include <QVBoxLayout>
#include <QToolBar>
#include <QComboBox>
#include <QtWidgets/qgraphicssceneevent.h>
#include <QLabel>
#include "idevice.h"
#include <QProgressBar>
#include <QPushButton>
#include "avfaudiorw.h"

class CVideoItem : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit CVideoItem(QGraphicsItem* parent = nullptr);
    CVideoItem(const QPixmap& pix) : CVideoItem() {
        setThumbnail(pix);
    }
    ~CVideoItem();

    QRectF boundingRect() const override;
    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem*,
               QWidget*) override;
    void setThumbnail(const QPixmap& pix);
    void invokeVideoProperties(CWaveTrack* t, ulong64 sample) {
        if (t->waveGenerator.hasVideo() & t->videoVisible) {
            QMetaObject::invokeMethod(
                this,
                [this, t, sample]() {
                    setVideoProperties(t,sample);
                },
                Qt::QueuedConnection
                );
        }
    }
    void invokePause() {
        QMetaObject::invokeMethod(
            this,
            [this]() {
                bool o = enabled();
                setEnabled(false);
                m_player->pause();
                m_currentFrame = {};
                update();
                setEnabled(o);
            },
            Qt::QueuedConnection
            );
    }
    bool setVideoProperties(CWaveTrack* t, ulong64 sample) {
        if (t->waveGenerator.hasVideo()) {
            bool o = enabled();
            setEnabled(false);
            if (m_player->source() != t->waveGenerator.videoURL) {
                qDebug() << m_player->source() << t->waveGenerator.videoURL;
                m_player->setSource(t->waveGenerator.videoURL);
                m_currentFrame = {};
            }
            const long64 mSec = CPresets::samplesTomSecs(sample);
            if (mSec > t->videoLength) m_currentFrame = {};
            if (std::llabs(m_player->position() - mSec) > 5) {
                qDebug() << "Set Position" << mSec << sample;
                m_player->setPosition(mSec);
            }
            if (!closeEnough(m_player->playbackRate(),t->loopParameters.Speed)) {
                qDebug() << "Set rate" << t->loopParameters.Speed;
                m_player->setPlaybackRate(t->loopParameters.Speed);
            }
            if (m_Playing) {
                if (m_player->playbackState() != QMediaPlayer::PlayingState) {
                    m_player->play();
                    m_currentFrame = {};
                    qDebug() << "start video play";
                }
            }
            setEnabled(o);
            return true;
        }
        return false;
    }
    bool setVideoExportProperties(CWaveTrack* t, long64 mSec) {
        if (t->waveGenerator.hasVideo()) {
            /*
            if (m_ExportPlayer->source() != t->waveGenerator.videoURL) {
                m_ExportPlayer->setSource(t->waveGenerator.videoURL);
                m_ExportPlayer->setPlaybackRate(1000.0);
                m_ExportPlayer->pause();
                m_ExportPlayer->setPosition(mSec);
                qDebug() << "setPosition" << mSec;
                return true;
            }
            if (mSec <= t->videoLength) {
                if (m_ExportPlayer->position() != mSec) {
                    m_ExportPlayer->setPosition(mSec);
                    qDebug() << "setPosition" << mSec;
                    return true;
                }
            }
*/
            if (imgExtract.url != t->waveGenerator.videoURL) {
                imgExtract.init(t->waveGenerator.videoURL,m_frameSize);
            }
            if (mSec <= t->videoLength) {
                m_currentImage = imgExtract.getImage(mSec / 1000.0);
                m_frameGeneration = m_exportGeneration;
                emit frameReady();
                return true;
            }
        }
        qDebug() << "send empty frame" << mSec;
        setVideoExportEmptyFrame();
        return false;
    }
    bool setVideoExportEmptyFrame() {
        m_exportGeneration++;
        emit frameReady();
        return true;
    }
    /*
    void waitForFrame()
    {
        QElapsedTimer t;
        t.start();
        while (!m_frameReady && t.elapsed() < 5000)
        {
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            if (m_frameReady) break;
            QThread::msleep(50);
        }
        if (!m_frameReady) {
            qDebug() << "Missed frame";
        }
    }
*/
    /*
    bool waitForFrame(int timeoutMs = 5000)
    {
        if (m_frameReady)
            return true;

        QEventLoop loop;
        QTimer timeout;

        timeout.setSingleShot(true);

        connect(this, &CVideoItem::frameReady,
                &loop, &QEventLoop::quit);

        connect(&timeout, &QTimer::timeout,
                &loop, &QEventLoop::quit);

        timeout.start(timeoutMs);

        m_waitingForFrame = true;
        loop.exec(QEventLoop::ExcludeUserInputEvents);
        m_waitingForFrame = false;

        bool ok = m_frameReady;
        m_frameReady = false;

        return ok;
    }
*/
    void setExportMode(bool m) {
        m_ExportMode = m;
        if (m) {
            m_exportGeneration = 0;
            m_frameGeneration = 0;
            if (m_ExportPlayer == nullptr) {
                m_ExportPlayer = new QMediaPlayer(this);
                m_ExportSink = new QVideoSink(this);
                m_ExportPlayer->setAudioOutput(nullptr);   // aldrig ljud
                m_ExportPlayer->setVideoSink(m_ExportSink);
                connect(m_ExportSink, &QVideoSink::videoFrameChanged, this, &CVideoItem::onExportFrameChanged);
            }
            setFlags(GraphicsItemFlags());
            ungrabGesture(Qt::PinchGesture);
            setAcceptedMouseButtons(Qt::NoButton);
            setAcceptTouchEvents(false);
        }
        else {
            if (m_ExportPlayer) {
                m_ExportPlayer->setSource(QUrl());
            }
            setFlags(ItemIsMovable | ItemIsSelectable | ItemUsesExtendedStyleOption);
            setCacheMode(QGraphicsItem::NoCache);
            grabGesture(Qt::PinchGesture);
            setAcceptedMouseButtons(Qt::AllButtons);
            setAcceptTouchEvents(true);
        }
    }
    void play() {
        m_Playing = true;
        m_currentFrame = {};
        //setEnabled(false);
    }
    void stop() {
        m_player->pause();
        m_Playing = false;
        m_currentFrame = {};
        //setEnabled(false);
        update();
    }
    void setEnabled(bool v) {
        m_Enabled = v;
        if (m_sink->signalsBlocked() == v)  m_sink->blockSignals(!v);
        if (!v) update();
    }
    bool enabled() {
        return m_Enabled;
    }
    void unserialize(const QDomLiteElement* xml) {
        if (!xml) return;
        if (QDomLiteElement* v = xml->elementByTag("VideoWidget")) {
            m_rect = QDomLite::getRectAttribute(v,"Rect");
            m_sourceRect = QDomLite::getRectAttribute(v,"SourceRect");
            setPos(QDomLite::getPointAttribute(v,"Pos"));
        }
    }
    void serialize(QDomLiteElement* xml) const {
        QDomLiteElement* v = xml->elementByTagCreate("VideoWidget");
        QDomLite::setRectAttribute(v,"Rect",m_rect);
        QDomLite::setRectAttribute(v,"SourceRect",m_sourceRect);
        QDomLite::setPointFAttribute(v,"Pos",pos());
    }
    QString name;
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* e) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* e) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e) override {
        if (m_Playing) return;
        if (e->modifiers() & Qt::ShiftModifier) {
            m_sourceRect.setSize(m_frameSize);
            m_sourceRect.moveTopLeft(QPoint(0,0));
        }
        else {
            m_rect.setSize(m_frameSize);
        }
        update();
        QGraphicsItem::mouseDoubleClickEvent(e);
    }
    bool gestureEvent(QGestureEvent *event)
    {
        if (QGesture *g = event->gesture(Qt::PinchGesture)) pinchTriggered(static_cast<QPinchGesture *>(g));
        return true;
    }
    bool sceneEvent(QEvent *event) override
    {
        if (event->type() == QEvent::Gesture) return gestureEvent(static_cast<QGestureEvent *>(event));
        return QGraphicsObject::sceneEvent(event);
    }
private slots:
    void onVideoFrameChanged(const QVideoFrame& frame);
    void onExportFrameChanged(const QVideoFrame& frame);
signals:
    void frameReady();
private:
    ImageExtractor imgExtract;
    uint64_t m_exportGeneration = 0;
    uint64_t m_frameGeneration = 0;
    bool m_ExportMode = false;
    std::atomic_bool m_Enabled{true};
    std::atomic_bool m_Playing{false};
    QPixmap thumbnail;
    QMediaPlayer* m_player;
    QVideoSink*   m_sink;
    QMediaPlayer* m_ExportPlayer = nullptr;
    QVideoSink* m_ExportSink;
    QVideoFrame m_currentFrame;
    QImage m_currentImage;
    qint64 m_LastFrameChange = 0;
    QSize m_frameSize { 320, 240 };

    enum Handle {
        NoHandle,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

    bool m_MD = false;
    QPointF m_pressPos;
    QRectF  m_pressSourceRect;
    Handle m_activeHandle = NoHandle;
    qreal m_handleSize = 8.0;
    QRect m_rect = QRect(0, 0, 320, 240);
    QRect m_sourceRect = QRect(0,0,320,240);
    void pinchTriggered(QPinchGesture *gesture)
    {
        if (m_Playing) return;
        static QRect startRect;
        static QRectF startSource;
        if (gesture->state() == Qt::GestureStarted) {
            m_MD = true;
            startRect = m_rect;
            startSource = m_sourceRect;
        }
        if (gesture->state() == Qt::GestureUpdated)
        {
            bool shift = (qApp->queryKeyboardModifiers() == Qt::ShiftModifier);
            if (shift)
            {
                m_sourceRect.setSize(QSizeF(startSource.size() * gesture->totalScaleFactor()).toSize());
            }
            else
            {
                m_rect.setSize(startRect.size() * gesture->totalScaleFactor());
            }

            update();
        }
        if ((gesture->state() == Qt::GestureFinished) || (gesture->state() == Qt::GestureCanceled)) m_MD = false;
    }
    void drawHandles(QPainter* p)
    {
        p->setBrush(Qt::white);
        p->setPen(Qt::black);

        QRect r = m_rect;

        QList<QRectF> handles = {
            QRect(r.topLeft()     - QPoint(m_handleSize/2, m_handleSize/2),
                   QSize(m_handleSize, m_handleSize)),
            QRect(r.topRight()    - QPoint(m_handleSize/2, m_handleSize/2),
                   QSize(m_handleSize, m_handleSize)),
            QRect(r.bottomLeft()  - QPoint(m_handleSize/2, m_handleSize/2),
                   QSize(m_handleSize, m_handleSize)),
            QRect(r.bottomRight() - QPoint(m_handleSize/2, m_handleSize/2),
                   QSize(m_handleSize, m_handleSize))
        };

        for (auto& h : handles)
            p->drawRect(h);
    }
    Handle handleAt(const QPoint& pos)
    {
        QRect r = m_rect;

        QRect tl(r.topLeft() - QPoint(4,4), QSize(8,8));
        QRect tr(r.topRight() - QPoint(4,4), QSize(8,8));
        QRect bl(r.bottomLeft() - QPoint(4,4), QSize(8,8));
        QRect br(r.bottomRight() - QPoint(4,4), QSize(8,8));

        if (tl.contains(pos)) return TopLeft;
        if (tr.contains(pos)) return TopRight;
        if (bl.contains(pos)) return BottomLeft;
        if (br.contains(pos)) return BottomRight;

        return NoHandle;
    }
};

class CVideoDesigner : public QGraphicsView
{
    Q_OBJECT
public:
    explicit CVideoDesigner(QWidget* parent = nullptr);
    ~CVideoDesigner() {
        qDebug() << "Exit Videodesigner";
    }
    CVideoItem* addVideo(CVideoItem* item)
    {
        m_scene.addItem(item);

        item->setPos(100, 100);
        return item;
    }
    void removeVideo(CVideoItem* item)
    {
        m_scene.removeItem(item);
    }
    QList<CVideoItem*> videoItems() const;
    QSize sizeHint() const override {
        return QSize(640,480);
    }
protected:
    void resizeEvent(QResizeEvent* e) override;

private:
    QGraphicsScene m_scene;
};

class CVideoDialog : public QDialog {
    Q_OBJECT
public:
    enum class Aspect {
        Square,     // 1:1
        Portrait,   // 9:16
        Landscape   // 16:9
    };

    enum class Resolution {
        R480,
        R720,
        R1080
    };
    CVideoDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
        setAttribute(Qt::WA_OpaquePaintEvent);
        setAttribute(Qt::WA_NoSystemBackground);
        QToolBar* tb = new QToolBar(this);
        tb->setIconSize(QSize(32,32));

        QAction* square =
            tb->addAction(QIcon(":/formats/instagram.png"), "1:1");

        QAction* portrait =
            tb->addAction(QIcon(":/formats/reels.png"), "9:16");

        QAction* landscape =
            tb->addAction(QIcon(":/formats/youtube.png"), "16:9");

        resolutionBox = new QComboBox(tb);
        resolutionBox->addItems({
            "480p",
            "720p",
            "1080p"
        });

        connect(square, &QAction::triggered, this, [&]{
            currentAspect = Aspect::Square;
            setCanvasSize();
        });

        connect(portrait, &QAction::triggered, this, [&]{
            currentAspect = Aspect::Portrait;
            setCanvasSize();
        });

        connect(landscape, &QAction::triggered, this, [&]{
            currentAspect = Aspect::Landscape;
            setCanvasSize();
        });

        connect(resolutionBox, &QComboBox::currentTextChanged,
                this,
                [&](const QString& text)
                {
                    Resolution r = Resolution::R720;

                    if (text == "480p") r = Resolution::R480;
                    if (text == "720p") r = Resolution::R720;
                    if (text == "1080p") r = Resolution::R1080;

                    currentResolution = r;

                    setCanvasSize();
                });
        tb->addSeparator();
        tb->addWidget(resolutionBox);
        designer = new CVideoDesigner(this);
        QVBoxLayout* l = new QVBoxLayout(this);
        l->setContentsMargins(0,0,0,0);
        l->setSpacing(0);
        l->addWidget(tb);
        l->addWidget(designer);
        setLayout(l);
        resolutionBox->setCurrentIndex((int)currentResolution);
    }
    void addVideo(CVideoItem* item) {
        designer->addVideo(item);
        setVisible(true);
    }
    void removeVideo(CVideoItem* item) {
        designer->removeVideo(item);
        setVisible(!designer->videoItems().isEmpty());
    }
    void setCanvasSize() {
        designer->setFixedSize(canvasSize(currentAspect));
        adjustSize();
    }
    QSize outputSize() {
        switch (currentResolution) {
        case Resolution::R480: return canvasSize(currentAspect,480);
        case Resolution::R720: return canvasSize(currentAspect,720);
        case Resolution::R1080: return canvasSize(currentAspect,1080);
        }
    }
    QSize inputSize() {
        return canvasSize(currentAspect);
    }
    QSize canvasSize(Aspect aspect, int h = 720)
    {
        QSize s;

        switch (aspect)
        {
        case Aspect::Landscape: // 16:9
            s = QSize(h * 16 / 9, h);
            break;
        case Aspect::Portrait:  // 9:16
            s = QSize(h, h * 16 / 9);
            break;
        case Aspect::Square:
            s = QSize(h, h);
            break;
        }

        return s;
    }
    void unserialize(const QDomLiteElement* xml) {
        if (!xml) return;
        if (QDomLiteElement* v = xml->elementByTag("VideoDialog")) {
            currentAspect = (Aspect)v->attributeValueInt("Aspect");
            currentResolution = (Resolution)v->attributeValueInt("Resolution");
            resolutionBox->setCurrentIndex((int)currentResolution);
        }
    }
    void serialize(QDomLiteElement* xml) const {
        QDomLiteElement* v = xml->elementByTagCreate("VideoDialog");
        v->setAttribute("Aspect",(int)currentAspect);
        v->setAttribute("Resolution",(int)currentResolution);
    }
    QGraphicsScene* scene() {
        return designer->scene();
    }
    void setScene(QGraphicsScene* s) {
        designer->setScene(s);
    }
    qreal sceneDevicePixelRatio() {
        return designer->devicePixelRatio();
    }
protected:
    void closeEvent(QCloseEvent* e) {
        hide();
        e->ignore();
        QDialog::closeEvent(e);
    }
    void showEvent(QShowEvent*) {
        fixMaximizeButton(this,false);
    }
private:
    CVideoDesigner* designer;
    Aspect currentAspect = Aspect::Landscape;
    Resolution currentResolution = Resolution::R720;
    QComboBox* resolutionBox;
};

class CVideoProgressWindow : public QDialog {
    Q_OBJECT
public:
    CVideoProgressWindow(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
        progressBar = new QProgressBar(this);
        infoLabel = new QLabel(this);
        abortButton = new QPushButton(this);
        abortButton->setText("Abort");
        connect(abortButton,&QPushButton::clicked,this,&CVideoProgressWindow::abort);
        QVBoxLayout* l = new QVBoxLayout(this);
        l->addWidget(progressBar);
        l->addWidget(infoLabel);
        l->addWidget(abortButton);
        adjustSize();
    }
    void setMax(ulong64 v) {
        if (v >= INT_MAX) {
            factor = (long double)v / INT_MAX;
        }
        progressBar->setMaximum(v * factor);
        maxString = " of "+QString::number(v);
    }
    void setValue(ulong64 v) {
        progressBar->setValue(v * factor);
        infoLabel->setText("Rendering frame " + QString::number(v) + maxString);
    }
signals:
    void abort();
private:
    QProgressBar* progressBar;
    QLabel* infoLabel;
    QPushButton* abortButton;
    double factor = 1;
    QString maxString;
};

#endif // CVIDEODESIGNER_H
