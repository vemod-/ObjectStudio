#include "claneeditcontrol.h"
#include "ui_claneeditcontrol.h"

CLaneEditControl::CLaneEditControl(QWidget *parent) :
    QGraphicsView(parent),
    ui(new Ui::CLaneEditControl)
{
    ui->setupUi(this);
    zoomer = new QGraphicsViewZoomer(this);
    zoomer->disableMatrix();
    zoomer->setMin(0.0001);
    zoomer->setMax(1);
    zoomer->setZoom(0.001L);
    connect(zoomer,&QGraphicsViewZoomer::ZoomChanged,this,&CLaneEditControl::setZoom);

    setBackgroundBrush(QPixmap(":/Brushed Aluminium 3 Tile.bmp"));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalScrollBar()->setEnabled(false);
    connect(horizontalScrollBar(),&QAbstractSlider::valueChanged,this,&CLaneEditControl::Paint);
    setScene(&Scene);
    setSceneRect(0,0,1,1);
    setAlignment(Qt::AlignTop | Qt::AlignLeft);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    Scene.setItemIndexMethod(QGraphicsScene::NoIndex);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::TextAntialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setMouseTracking(true);
    setStyleSheet("QGraphicsView{background-color: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 1, stop: 0 #ddd, stop: 1 #999);}");
    QAction* actionTogglePlay = new QAction(this);
    actionTogglePlay->setShortcut(QKeySequence(Qt::Key_Space));
    connect(actionTogglePlay,&QAction::triggered,this,&CLaneEditControl::togglePlay);
    addAction(actionTogglePlay);
}

CLaneEditControl::~CLaneEditControl()
{
    if (m_TimerID) killTimer(m_TimerID);
    m_TimerID = 0;
    delete ui;
}
