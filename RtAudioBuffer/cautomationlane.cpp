#include "cautomationlane.h"
#include "ui_cautomationlane.h"

CAutomationLane::CAutomationLane(QWidget *parent) :
    QGraphicsView(parent),
    ui(new Ui::CAutomationLane)
{
    ui->setupUi(this);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    zoomer = new QGraphicsViewZoomer(this);
    zoomer->disableMatrix();
    zoomer->setMin(1);
    zoomer->setMax(width());
    connect(zoomer,&QGraphicsViewZoomer::ZoomChanged,this,&CAutomationLane::setZoom);
    m_ParameterMenu = new QSignalMenu(this);
    setFrameStyle(0);
    //setBackgroundBrush(Qt::darkGray);
    //setAttribute(Qt::WA_TranslucentBackground);
    //setBackgroundBrush(QColor("#A9A9A9A9"));
    setStyleSheet("QGraphicsView{background-color: rgba(128, 128, 128, 190);}");
    //setBackgroundBrush(QBrush(QColor(128, 128, 128, 190)));
    setScene(&Scene);
    setOptimizationFlags(QGraphicsView::DontSavePainterState | QGraphicsView::DontAdjustForAntialiasing);
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    Scene.setItemIndexMethod(QGraphicsScene::NoIndex);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::TextAntialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setMouseTracking(true);
    InfoLabel=new QLabel(parent);
    InfoLabel->setAutoFillBackground(true);
    InfoLabel->setFrameStyle(QFrame::Box | QFrame::Plain);
    InfoLabel->hide();
    connect(horizontalScrollBar(),&QAbstractSlider::valueChanged,this,&CAutomationLane::Paint);
}

CAutomationLane::~CAutomationLane()
{
    delete ui;
}
