#include "cjackscomponent.h"
#include "ui_cjackscomponent.h"
#include <QGraphicsTextItem>
//#include <QMenu>
#include "cconnectionhelper.h"
#include "QScrollBar"
#include "cparametersmenu.h"

CJacksDevice::CJacksDevice() : m_Device(nullptr)
{
}

CJacksDevice::~CJacksDevice()
{

}

void CJacksDevice::init(IDevice* device)
{
    m_Device=device;
    m_Left=0;
}

void CJacksDevice::paint(QGraphicsScene* scene, int index, int width)
{
    m_Index=index;
    int w=qMax<int>(width,this->width());
    const int top=calcTop(0,index);
    QBrush bg(QPixmap(":/Black Aluminium Tile.jpg"));
    scene->addRect(m_Left,top,w,112,QPen(Qt::black),bg);
    scene->addLine(m_Left,top,w,top,QPen(Qt::darkGray));
    QPixmap screwPix=QPixmap::fromImage(QImage(":/screwhead.png"), Qt::AutoColor | Qt::DiffuseDither | Qt::DiffuseAlphaDither).scaled(QSize(10,10),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    QGraphicsPixmapItem* s = scene->addPixmap(screwPix);
    s->setPos(m_Left + 4,top + 4);
    QGraphicsPixmapItem* s1 = scene->addPixmap(screwPix);
    s1->setPos(m_Left + 4,top + 98);
    QFont f;
    CConnectionHelper::DrawShadowText("In",f,QPoint(20+m_Left,calcTop(42,index)),scene);
    CConnectionHelper::DrawShadowText("Out",f,QPoint(20+m_Left,calcTop(62,index)),scene);
    int InIndex = 0;
    int OutIndex = 0;
    JackRects.clear();
    for (int i=0;i<m_Device->jackCount();i++)
    {
        QRect r;
        IJack* j = m_Device->jack(i);
        QString txt=j->name();
        QFont f;
        f.setPointSizeF(9.5);
        if (j->isInJack())
        {
            r=QRect(calcLeft(InIndex),calcTop(42,index), 12,12);
            JackRects.append(r);
            CConnectionHelper::DrawShadowTextCenter(txt,f,QPoint(calcLeft(InIndex)-22,calcTop(4,index)),QSize(56,34), Qt::AlignHCenter | Qt::AlignBottom, scene);
            InIndex++;
        }
        else
        {
            r=QRect(calcLeft(OutIndex),calcTop(62,index),12,12);
            JackRects.append(r);
            CConnectionHelper::DrawShadowTextCenter(txt,f,QPoint(calcLeft(OutIndex)-22,calcTop(76,index)),QSize(56,34),Qt::AlignHCenter | Qt::AlignTop,scene);
            OutIndex++;
        }
        scene->addEllipse(r.translated(5,5),Qt::NoPen,QBrush(QColor(0,0,0,40)));
        scene->addEllipse(r,QPen(j->JackColor(),3),QBrush(QColor(0,0,0,100)));
    }
}

QString CJacksDevice::deviceID()
{
    if (m_Device) return m_Device->deviceID();
    return QString();
}

QString CJacksDevice::jackID(const int j)
{
    if (m_Device) return m_Device->jackID(j);
    return QString();
}

QPoint CJacksDevice::jackPoint(int i)
{
    return JackRects[i].center();
}

int CJacksDevice::MouseOverJack(const QPoint& p)
{
    for (int i = 0; i < JackRects.size(); i++)
    {
        if (JackRects[i].contains(p)) return i;
    }
    return -1;
}

CJacksComponent::CJacksComponent(QWidget *parent) :
    QGraphicsView(parent),
    ui(new Ui::CJacksComponent)
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
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    setFrameStyle(0);
    setLineWidth(0);
    //JackPopup=new QSignalMenu(this);
    //connect(JackPopup,qOverload<QString>(&QSignalMenu::menuClicked),this,&CJacksComponent::ToggleConnection);
}

CJacksComponent::~CJacksComponent()
{
    delete ui;
}

void CJacksComponent::Init(CDeviceList *DeviceList)
{
    m_DL=DeviceList;
}

int CJacksComponent::deviceIndex(IDevice* device)
{
    if (!device) return -1;
    const QString ID=device->deviceID();
    for (int i = 0; i < devices.size(); i++)
    {
        if (ID == devices[i]->deviceID()) return i;
    }
    return -1;
}

QPoint CJacksComponent::jackPoint(IDevice* device, int i)
{
    int di = deviceIndex(device);
    if (di > -1)
    {
        return devices[di]->jackPoint(i);
    }
    return {-1,-1};
}

void CJacksComponent::addDevice(IDevice* device)
{
    devices.append(new CJacksDevice());
    devices.last()->init(device);
    DrawConnections();
    //animateTo(devices.size()-1);
}

void CJacksComponent::removeDevice(IDevice* device)
{
    int i = deviceIndex(device);
    if (i > -1)
    {
        delete devices.takeAt(i);
        DrawConnections();
    }
}

void CJacksComponent::moveDevice(int index, int move)
{
    if (index < 0) return;
    if (index > devices.size() -1) return;
    if (move == 0) return;
    //int newIndex = index + move;
    int newIndex = std::clamp<int>(index + move,0,devices.size()-1);
    //newIndex = qMax(0, newIndex);
    //newIndex = qMin(devices.size() -1, newIndex);
    if (newIndex == index) return;
    CJacksDevice* temp = devices.takeAt(index);
    devices.insert(newIndex, temp);
    DrawConnections();
}

void CJacksComponent::clear()
{
    for (CJacksDevice* d : std::as_const(devices))
    {
        devices.removeOne(d);
        delete d;
    }
    DrawConnections();
}

void CJacksComponent::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    DrawConnections();
}

void CJacksComponent::wheelEvent(QWheelEvent* event)
{
    int move = event->pixelDelta().rx();
    if (move != 0)
    {
        QPoint Pos=mapToScene(event->position().x(),event->position().y()).toPoint();
        int index = Pos.ry() / 112;
        if ((index > -1) && (index < devices.size()))
        {
            if (devices[index]->width() > width())
            {
                int l = devices[index]->left()+move;
                if (l > 0) l = 0;
                if (l < width()-devices[index]->width()) l = width()-devices[index]->width();
                devices[index]->setLeft(l);
                DrawConnections();
                event->accept();
                return;
            }
        }
    }
    event->ignore();
}

void CJacksComponent::DrawConnections()
{
    Scene.clear();
    QRect MaxRect(0,0,width(),devices.size()*112);
    if (devices.isEmpty())
    {
        MaxRect.setHeight(112);
        QBrush bg(QPixmap(":/Black Aluminium Tile.jpg"));
        Scene.addRect(MaxRect,QPen(Qt::black),bg);
        Scene.addLine(MaxRect.left(),MaxRect.top(),MaxRect.width(),MaxRect.top(),QPen(Qt::darkGray));
    }
    for (int i = 0; i < devices.size(); i++)
    {
        devices[i]->paint(&Scene,i,MaxRect.width());
    }
    QList<IDevice*> paintedContainers;
    for (IDevice* inDevice : std::as_const(*m_DL->devices()))
    {
        DrawDeviceConnections(inDevice,paintedContainers);
    }
    if (IJack* thisIn = m_DL->jack("This In")) {
        for (IDevice* d : std::as_const(*m_DL->devices())) {
            for (int i = 0; i < d->jackCount(); i++) {
                IJack* j = d->jack(i);
                if (j->isConnectedTo(thisIn)) DrawConnection(jackPoint(d,i),QPoint(0,0),j->JackColor());
            }
        }
    }
    if (IJack* thisOut = m_DL->jack("This Out")) {
        for (IDevice* d : std::as_const(*m_DL->devices())) {
            for (int i = 0; i < d->jackCount(); i++) {
                IJack* j = d->jack(i);
                if (j->isConnectedTo(thisOut)) DrawConnection(QPoint(0,MaxRect.height()),jackPoint(d,i),j->JackColor());
            }
        }
    }
    int x = 0;
    for (int i = 100; i > 0; i = i - 10) {
        Scene.addLine(0,x,MaxRect.width(),x,QColor(0,0,0,i));
        x++;
    }
    setSceneRect(MaxRect);
    horizontalScrollBar()->setMaximum(0);
    setFixedHeight(MaxRect.height());
}

void CJacksComponent::DrawDeviceConnections(IDevice* device, QList<IDevice*>& paintedContainers)
{
    for (int i = 0; i < device->jackCount(); i++)
    {
        for (IDevice* outDevice : paintedContainers)
        {
            for (int j = 0; j < outDevice->jackCount(); j++)
            {
                if (device->jack(i)->isConnectedTo(outDevice->jack(j)))
                {
                    QPoint point1 = jackPoint(device,i);
                    if (point1 != QPoint(-1,-1))
                    {
                        QPoint point2 = jackPoint(outDevice,j);
                        if (point2 != QPoint(-1,-1))
                        {
                            if (device->jack(i)->isInJack())
                            {
                                DrawConnection(point2,point1,device->jack(i)->JackColor());
                            }
                            else
                            {
                                DrawConnection(point1,point2,device->jack(i)->JackColor());
                            }
                        }
                    }
                }
            }
        }
    }
    paintedContainers.append(device);
}

void CJacksComponent::DrawChangedConnections()
{
    emit connectionsChanged();
    DrawConnections();
}

void CJacksComponent::DrawConnection(QPoint p1, QPoint p2, const QColor& color)
{
    QPainterPath p;
    QRect r(p1,p2);
    r=r.normalized();
    int adjust = 60 - r.width();
    if (adjust < 0) adjust=0;
    r.adjust(adjust,(r.height()/5)+50,-adjust,(r.height()/5)+100);
    //r=r.intersected(rect());
    if (p1.x() > p2.x()) std::swap(p1,p2);
    p.moveTo(p1);
    if (p1.y() < p2.y())
    {
        p.cubicTo(r.bottomLeft(),p2+((r.bottomRight()-p2)/2),p2);
    }
    else
    {
        p.cubicTo(p1+((r.bottomLeft()-p1)/2),r.bottomRight(),p2);
    }
    Scene.addPath(p.translated(5,5),QPen(QColor(0,0,0,40),4));
    QColor c(color);
    c.setAlphaF(CPresets::presets().ConnectionsOpacity);
    Scene.addPath(p,QPen(c,4));
}


QString CJacksComponent::MouseOverJack(const QPoint &Pos)
{
    for (CJacksDevice* d : std::as_const(devices))//(int i = 0; i < devices.size(); i++)
    {
        int j = d->MouseOverJack(Pos);
        if (j > -1) return d->jackID(j);
    }
    return QString();
}

void CJacksComponent::mouseMoveEvent(QMouseEvent* event)
{
    QPoint Pos=mapToScene(event->pos().x(),event->pos().y()).toPoint();
    if (Pos != MousePoint)
    {
        setToolTip(MouseOverJack(Pos));
        MousePoint = Pos;
    }
    QGraphicsView::mouseMoveEvent(event);
}

void CJacksComponent::mousePressEvent(QMouseEvent *event)
{
    const QPoint Pos=mapToScene(event->pos().x(),event->pos().y()).toPoint();
    const QString JackID=MouseOverJack(Pos);
    if (!JackID.isEmpty()) {
        //emit popupTriggered(m_DL->jack(JackID),mapToGlobal(event->pos()));
        //JackMenuPopup(m_DL->jack(JackID),mapToGlobal(event->pos()));
        CConnectionsMenu* m = new CConnectionsMenu(m_DL->jack(JackID),m_DL,this);
        connect(m,&CConnectionsMenu::aboutToChange,this,&CJacksComponent::aboutToChange,Qt::DirectConnection);
        connect(m,&CConnectionsMenu::connectionsChanged,this,&CJacksComponent::DrawChangedConnections);
        m->popup(mapToGlobal(event->pos()));
        return;
    }
    if (event->button() == Qt::LeftButton)
    {
        int index = Pos.y() / 112;
        emit mousePress(m_DL->device(index),mapToGlobal(event->pos()));
    }
}
/*
void CJacksComponent::JackMenuPopup(IJack* jack, QPoint pos)
{
    MenuJackID=jack->jackID();
    QSignalMenu* JackPopup=new QSignalMenu(this);
    JackPopup->setAttribute(Qt::WA_DeleteOnClose,true);
    connect(JackPopup,qOverload<QString>(&QSignalMenu::menuClicked),this,&CJacksComponent::ToggleConnection);


    for (int i=0;i<m_DL->jackCount();i++)
    {
        IJack* J=m_DL->jack(i);
        if (jack->canConnectTo(J))
        {
            QAction* a=JackPopup->addAction(J->jackID(),J->jackID());
            a->setCheckable(true);
            a->setChecked(jack->isConnectedTo(J));
        }
    }
    if (JackPopup->actions().isEmpty())
    {
        QAction* a=JackPopup->addAction("(No Available Connections)");
        a->setEnabled(false);
    }
    JackPopup->popup(pos);
}

void CJacksComponent::ToggleConnection(QString JackID)
{
    QMutexLocker locker(&mutex);
    (m_DL->isConnected(JackID,MenuJackID)) ? m_DL->disconnect(JackID,MenuJackID) :
        m_DL->connect(JackID,MenuJackID);
    DrawConnections();
    emit connectionsChanged();
}
*/
