#ifndef CVIDEODESIGNER_H
#define CVIDEODESIGNER_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMap>
#include <QGraphicsObject>
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
#include "avfoundation_wrapper.h"

#define defaultResolution 720

class CVideoItem : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit CVideoItem(QGraphicsItem* parent = nullptr);
    CVideoItem(const QImage& pix) : CVideoItem() {
        setThumbnail(pix);
    }
    ~CVideoItem();

    QRectF boundingRect() const override;
    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem*,
               QWidget*) override;
    void setThumbnail(const QImage& pix);
    void invokeVideoPlayProperties(CWaveTrack* t, ulong64 sample) {
        if (t->waveGenerator.hasVideo() & t->videoVisible) {
            QMetaObject::invokeMethod(
                this,
                [this, t, sample]() {
                    setVideoPlayProperties(t,sample);
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
                m_AVFPlayer.pause();
                update();
                setEnabled(o);
            },
            Qt::QueuedConnection
            );
    }
    bool setVideoPlayProperties(CWaveTrack* t, ulong64 sample) {
        if (t->waveGenerator.hasVideo()) {
            const long64 mSec = CPresets::samplesTomSecs(sample);
            if (mSec > t->videoLength) {
                frameTimer.stop();
                m_AVFPlayer.pause();
                return false;
            }
            bool o = enabled();
            setEnabled(false);
            if (m_AVFPlayer.Url != t->waveGenerator.videoURL) {
                m_AVFPlayer.setSource(t->waveGenerator.videoURL);
            }
            if (qAbs<long64>((m_AVFPlayer.position() * 1000) - mSec) > 10) {
                m_AVFPlayer.setPosition(mSec / 1000.0);
            }
            if (!closeEnough(m_AVFPlayer.playbackRate(),t->loopParameters.Speed)) {
                m_AVFPlayer.setPlaybackRate(t->loopParameters.Speed);
            }
            if (m_Playing) {
                if (!m_AVFPlayer.playing) m_AVFPlayer.play();
            }
            setEnabled(o);
            return true;
        }
        return false;
    }
    bool setVideoExportProperties(CWaveTrack* t, long64 mSec) {
        if (t->waveGenerator.hasVideo()) {
            if (imgExtract.videoUrl != t->waveGenerator.videoURL) {
                imgExtract.setSource(t->waveGenerator.videoURL,m_frameSize);
            }
            if (mSec <= t->videoLength) {
                m_exportImage = imgExtract.getImage(mSec / 1000.0);
                m_frameGeneration = m_exportGeneration;
                return true;
            }
        }
        setVideoExportEmptyFrame();
        return false;
    }
    bool setVideoExportEmptyFrame() {
        m_exportGeneration++;
        return true;
    }
    void setExportMode(bool m) {
        m_ExportMode = m;
        if (m) {
            m_exportGeneration = 0;
            m_frameGeneration = 0;
        }
        else {
        }
    }
    bool setVideoStillProperties(CWaveTrack* t, long64 mSec) {
        if (m_Playing) return false;
        if (t->waveGenerator.hasVideo()) {
            if (imgExtract.videoUrl != t->waveGenerator.videoURL) {
                imgExtract.setSource(t->waveGenerator.videoURL,m_frameSize);
            }
            if (mSec <= t->videoLength) {
                m_stillImage = imgExtract.getImage(mSec / 1000.0).copy();
                update();
                return true;
            }
        }
        m_stillImage = QImage();
        update();
        return false;
    }
    bool setVideoStillEmptyFrame() {
        if (m_Playing) return false;
        m_stillImage = QImage();
        update();
        return true;
    }
    void play() {
        m_Playing = true;
        frameTimer.start();
    }
    void stop() {
        //m_player.pause();
        frameTimer.stop();
        m_AVFPlayer.pause();
        m_Playing = false;
        update();
    }
    void setEnabled(bool v) {
        m_Enabled = v;
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
    void setRenderRect(qreal newHeight) {
        qreal zoom = defaultResolution / newHeight;
        QRectF r(m_rect.topLeft() / zoom,m_rect.size() / zoom);
        r.translate(pos() / zoom);
        renderRect = r.toRect();
    }
    QRect rect() {
        return m_rect.translated(pos().toPoint());
    }
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
private:
    QTimer frameTimer;
    QRect renderRect;
    ImageExtractor imgExtract;
    uint64_t m_exportGeneration = 0;
    uint64_t m_frameGeneration = 0;
    bool m_ExportMode = false;
    std::atomic_bool m_Enabled{true};
    std::atomic_bool m_Playing{false};
    QImage thumbnail;
    AVFVideoPlayer m_AVFPlayer;
    QImage m_currentPlaybackImage;
    QImage m_exportImage;
    QImage m_stillImage;
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

class Guides
{
public:
    Guides(QRect rect) {
        left    = rect.left();
        right   = rect.right();
        hcenter = rect.center().x();

        top     = rect.top();
        bottom  = rect.bottom();
        vcenter = rect.center().y();
    }
    int left;
    int right;
    int hcenter;
    int top;
    int bottom;
    int vcenter;
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
    void mousePressEvent(QMouseEvent *event) override {
        m_MD = true;
        QGraphicsView::mousePressEvent(event);
        drawGuideLines();
    }
    void mouseMoveEvent(QMouseEvent *event) override {
        QGraphicsView::mouseMoveEvent(event);
        drawGuideLines();
    }
    void mouseReleaseEvent(QMouseEvent *event) override {
        guides.clear();
        viewport()->update();
        m_MD = false;
        QGraphicsView::mouseReleaseEvent(event);
    }
    void drawForeground(QPainter *painter, const QRectF &/*rect*/) override {
        painter->setPen(QPen(Qt::yellow,1,Qt::DashLine));
        for (auto &l : guides)
            painter->drawLine(l);
    }
private:
    bool m_MD = false;
    QGraphicsScene m_scene;
    QList<QLine>guides;
    void drawVerticalGuide(int x)
    {
        guides.append(QLine(x, 0, x, 720));
    }

    void drawHorizontalGuide(int y)
    {
        guides.append(QLine(0, y, 1280, y));
    }
    void drawGuideLines() {
        guides.clear();
        if (m_MD) {
            const double snapTol = 1.0;
            QGraphicsItem* item = m_scene.mouseGrabberItem();
            if (auto movingItem = dynamic_cast<CVideoItem*>(item)) {
                for (CVideoItem* other : videoItems())
                {
                    if (other == movingItem)
                        continue;

                    Guides a(movingItem->rect());
                    Guides b(other->rect());
                    if (qAbs(a.left - b.left) < snapTol)
                        drawVerticalGuide(b.left);

                    if (qAbs(a.right - b.right) < snapTol)
                        drawVerticalGuide(b.right);

                    if (qAbs(a.hcenter - b.hcenter) < snapTol)
                        drawVerticalGuide(b.hcenter);

                    if (qAbs(a.top - b.top) < snapTol)
                        drawHorizontalGuide(b.top);

                    if (qAbs(a.bottom - b.bottom) < snapTol)
                        drawHorizontalGuide(b.bottom);

                    if (qAbs(a.vcenter - b.vcenter) < snapTol)
                        drawHorizontalGuide(b.vcenter);

                    if (qAbs(a.right - b.left) < snapTol)
                        drawVerticalGuide(b.left);

                    if (qAbs(a.left - b.right) < snapTol)
                        drawVerticalGuide(b.right);

                    QRect sceneRect = QRect(0,0,1280,720);

                    int cx = sceneRect.center().x();
                    int cy = sceneRect.center().y();

                    if (qAbs(a.hcenter - cx) < snapTol)
                        drawVerticalGuide(cx);

                    if (qAbs(a.vcenter - cy) < snapTol)
                        drawHorizontalGuide(cy);

                    scene()->invalidate(QRectF(),QGraphicsScene::ForegroundLayer);
                }
            }

        }
    }
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
        return canvasSize(currentAspect,resolution());
    }
    int resolution() {
        switch (currentResolution) {
        case Resolution::R480: return 480;
        case Resolution::R720: return 720;
        case Resolution::R1080: return 1080;
        }
    }
    QSize inputSize() {
        return canvasSize(currentAspect);
    }
    QSize canvasSize(Aspect aspect, int h = defaultResolution)
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
