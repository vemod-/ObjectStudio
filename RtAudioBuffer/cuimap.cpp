#include "cuimap.h"
#include "ui_cuimap.h"
#include <QGraphicsPixmapItem>
#include <QVariantAnimation>
#include <QGraphicsBlurEffect>

CUIMap::CUIMap(QWidget *parent) :
    QGraphicsView(parent),
    ui(new Ui::CUIMap)
{
    ui->setupUi(this);
    setScene(&Scene);
    setOptimizationFlags(QGraphicsView::DontSavePainterState | QGraphicsView::DontAdjustForAntialiasing);
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    Scene.setItemIndexMethod(QGraphicsScene::NoIndex);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::TextAntialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setMouseTracking(true);
    setFrameStyle(0);
    setLineWidth(0);
}

CUIMap::~CUIMap()
{
    qDeleteAll(pixmaps);
    delete ui;
}

QPixmap applyEffectToPixmap(const QPixmap& src, QGraphicsEffect *effect, int extent = 0)
{
    if(src.isNull()) return QPixmap();   //No need to do anything else!
    if(!effect) return src;             //No need to do anything else!
    QGraphicsScene scene;
    QGraphicsPixmapItem* item = scene.addPixmap(src);
    item->setTransformationMode(Qt::TransformationMode::SmoothTransformation);
    item->setGraphicsEffect(effect);
    QPixmap res(src.size()+QSize(extent*2, extent*2));
    res.fill(Qt::transparent);
    QPainter ptr(&res);
    ptr.setRenderHint(QPainter::Antialiasing);
    ptr.setRenderHint(QPainter::TextAntialiasing);
    ptr.setRenderHint(QPainter::SmoothPixmapTransform);
    scene.render(&ptr, QRectF( extent, extent, src.width(), src.height() ));
    return res;
}

void CUIMap::showMap(IDeviceParent* DeviceList,QWidget* parent, QWidget* replaces)
{
    QPixmap bg = parent->grab().scaled(parent->size());
    m_DL=DeviceList;
    Scene.setBackgroundBrush(QBrush());

    DeviceList->hideForms();
    parent->setUpdatesEnabled(false);
    replaces->setVisible(false);
    setVisible(true);

    Scene.clear();
    setSceneRect(0,0,bg.width(),bg.height());
    auto blur = new QGraphicsBlurEffect;
    blur->setBlurRadius(2);
    blur->setBlurHints(QGraphicsBlurEffect::QualityHint);
    bg = applyEffectToPixmap(bg,blur);
    Scene.setBackgroundBrush(QBrush(bg));
    deviceWidth = bg.width() / 4.0;
    m_ColumnCount = bg.width() / deviceWidth;
    if (m_ColumnCount < 1) m_ColumnCount = 1;
    matrixTop = 0;
    loadImages();

    animate();
    parent->setUpdatesEnabled(true);
}

void CUIMap::animate()
{
    auto animation=new QVariantAnimation();
    animation->setDuration(500);
    animation->setStartValue(10);
    animation->setEndValue(deviceWidth);
    connect(animation,SIGNAL(valueChanged(const QVariant)),this,SLOT(drawDevices(QVariant)));
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void CUIMap::loadDeviceImage(IDevice* d)
{
    if (d->hasUI())
    {
        const QPixmap* px = d->picture();
        if (px) pixmaps.append(new CDeviceFrame(px,d));
    }
    for (int i = 0; i < d->childDeviceCount(); i++) loadDeviceImage(d->childDevice(i));
}

void CUIMap::loadImages()
{
    qDeleteAll(pixmaps);
    pixmaps.clear();
    for (int i = 0; i < m_DL->deviceCount(); i++) loadDeviceImage(m_DL->device(i));
}

void CUIMap::drawDevices(QVariant itemSize)
{
    Scene.clear();
    matrixHeight = 0;
    const QSizeF deviceSize(itemSize.toDouble(),itemSize.toDouble()*0.75);
    const QSizeF imageSize(deviceSize*0.95);
    int p = 0;
    for (CDeviceFrame* f : std::as_const(pixmaps))
    {
        QGraphicsPixmapItem* i = Scene.addPixmap(f->pixmap->scaled(imageSize.toSize(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
        QGraphicsDropShadowEffect* e = new QGraphicsDropShadowEffect;
        e->setBlurRadius(2);
        e->setOffset(6,6);
        e->setColor(QColor(0, 0, 0, 60));
        i->setGraphicsEffect(e);
        const QSizeF sz = (deviceSize - i->boundingRect().size()) / 2.0;
        QPoint topLeft((p%m_ColumnCount)*deviceSize.width(),((p/m_ColumnCount)*deviceSize.height()) + matrixTop);
        topLeft += QPoint(sz.width(),sz.height());
        i->setPos(topLeft);
        f->rect = QRect(topLeft,i->boundingRect().size().toSize());
        if (matrixHeight < ((p/m_ColumnCount)+1)*deviceSize.height()) matrixHeight = ((p/m_ColumnCount)+1)*deviceSize.height();
        p++;
    }
}

int CUIMap::mouseOverDevice(QPoint Pos)
{
    for (int p = 0; p < pixmaps.size(); p++) if (pixmaps[p]->rect.contains(Pos)) return p;
    return -1;
}

void CUIMap::mouseMoveEvent(QMouseEvent* event)
{
    const QPoint Pos=mapToScene(event->pos().x(),event->pos().y()).toPoint();
    int i = mouseOverDevice(Pos);
    setToolTip(QString());
    if (i > -1) setToolTip(pixmaps[i]->device->deviceID());
}

void CUIMap::mousePressEvent(QMouseEvent* event)
{
    const QPoint Pos=mapToScene(event->pos().x(),event->pos().y()).toPoint();
    int i = mouseOverDevice(Pos);
    if (i > -1)
    {
        IDevice* d = pixmaps[i]->device;
        qDebug() << "UIMap" << d->deviceID();
        d->activate();
        d->execute(true);
        d->raiseForm();
    }
    emit deviceSelected();
}

void CUIMap::wheelEvent(QWheelEvent* event)
{
    if (event->pixelDelta().ry())
    {
        matrixTop += event->pixelDelta().ry();
        if (matrixTop > 0) matrixTop = 0;
        if (matrixTop < height() - matrixHeight) matrixTop = height() - matrixHeight;
        drawDevices(deviceWidth);
    }
    QGraphicsView::wheelEvent(event);
}

void CUIMap::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    QSize s(Scene.backgroundBrush().textureImage().size());
    if ((s != QSize(0,0)) && (event->size() != s)) emit deviceSelected();
}

