#include "cjackscomponent.h"
//#include "ui_cjackscomponent.h"
#include <QGraphicsTextItem>
//#include <QMenu>
#include "cconnectionhelper.h"
#include "QScrollBar"
#include "cparametersmenu.h"
#include "qdprpixmap.h"

CJacksDevice::CJacksDevice() : m_Device(nullptr)
{
}

CJacksDevice::~CJacksDevice()
{

}

void CJacksDevice::init(IDevice* device)
{
    m_Device = device;
    m_Left = 1000;
}

void CJacksDevice::paint(QGraphicsScene* scene, int index, int width)
{
    static QDPRPixmap screwPix(QSize(10,10),":/screwhead.png");
    static QDPRPixmap freeDeviceJack(QSize(18,18),":/Jack.png");
    static QDPRPixmap connectedDeviceJack(QSize(18,18),":/Plug.png");
    JackRects.clear();
    for (QGraphicsItem* i : std::as_const(PlugImages)) {
        scene->removeItem(i);
        delete i;
    }
    PlugImages.clear();
    for (QGraphicsItem* i : std::as_const(JackItems)) {
        scene->removeItem(i);
        delete i;
    }
    JackItems.clear();
    m_Index = index;
    int w = qMax<int>(width,this->width());
    const int top = calcTop(0,index);
    JackItems.append(scene->addLine(m_Left,top,w,top,QPen(Qt::darkGray)));
    QGraphicsPixmapItem* s = scene->addPixmap(screwPix);
    s->setPos(m_Left + 4,top + 4);
    JackItems.append(s);
    QGraphicsPixmapItem* s1 = scene->addPixmap(screwPix);
    s1->setPos(m_Left + 4,top + 98);
    JackItems.append(s1);
    QFont f;
    JackItems.append(CConnectionHelper::DrawShadowText("In",f,QPoint(20+m_Left,calcTop(42,index)),scene));
    JackItems.append(CConnectionHelper::DrawShadowText("Out",f,QPoint(20+m_Left,calcTop(62,index)),scene));
    int InIndex = 0;
    int OutIndex = 0;
    for (int i = 0; i < m_Device->jackCount(); i++)
    {
        QRect r;
        IJack* j = m_Device->jack(i);
        QString txt = j->caption();
        QFont f;
        f.setPointSizeF(9.5);
        if (j->isInJack())
        {
            r = QRect(calcLeft(InIndex),calcTop(40,index), 19,19);
            JackRects.append(r);
            JackItems.append(CConnectionHelper::DrawShadowTextCenter(txt,f,QPoint(calcLeft(InIndex)-22,calcTop(4,index)),QSize(56,34), Qt::AlignHCenter | Qt::AlignBottom, scene));
            InIndex++;
        }
        else
        {
            r = QRect(calcLeft(OutIndex),calcTop(60,index),19,19);
            JackRects.append(r);
            JackItems.append(CConnectionHelper::DrawShadowTextCenter(txt,f,QPoint(calcLeft(OutIndex)-22,calcTop(76,index)),QSize(56,34),Qt::AlignHCenter | Qt::AlignTop,scene));
            OutIndex++;
        }
        JackItems.append(scene->addEllipse(QRect(r.topLeft(),r.size() - QSize(3,3)),QPen(j->JackColor(),3),QBrush(QColor(0,0,0,100))));
        JackItems.append(scene->addEllipse(QRect(r.translated(5,5).topLeft(),r.size() - QSize(3,3)),Qt::NoPen,QBrush(QColor(0,0,0,40))));
        QGraphicsPixmapItem* px = scene->addPixmap(freeDeviceJack);
        px->setPos(r.topLeft() - QPoint(1,1));
        JackItems.append(px);
        QGraphicsPixmapItem* px1 = scene->addPixmap(connectedDeviceJack);
        px1->setPos(r.topLeft() - QPoint(1,1));
        px1->setVisible(false);
        PlugImages.append(px1);
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
    return JackRects[i].center() - QPoint(1,1);
}

int CJacksDevice::MouseOverJack(const QPoint& p)
{
    for (int i = 0; i < JackRects.size(); i++)
    {
        if (JackRects[i].contains(p)) return i;
    }
    return -1;
}

CJacksComponent::CJacksComponent(QWidget *parent, QGraphicsScene* s) :
    QGraphicsView(parent)//,
{
    if (!s) {
        Scene = new QGraphicsScene;
        Scene->setItemIndexMethod(QGraphicsScene::NoIndex);
        Scene->setBackgroundBrush(QDPRPixmap(":/Brushed Aluminium Tile.bmp"));
    }
    else {
        Scene = s;
        setVisible(false);
    }
    setScene(Scene);
    setOptimizationFlags(QGraphicsView::DontSavePainterState | QGraphicsView::DontAdjustForAntialiasing);
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::TextAntialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setMouseTracking(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    setFrameStyle(0);
    setLineWidth(0);
    setAcceptDrops(false);
    zoomer = new QGraphicsViewZoomer(this);
}

CJacksComponent::~CJacksComponent()
{
    //Scene.clear();
}

void CJacksComponent::Init(CDeviceList *DeviceList)
{
    m_DL = DeviceList;
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
    int newIndex = std::clamp<int>(index + move,0,devices.size()-1);
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

void CJacksComponent::wheelEvent(QWheelEvent* /*event*/)
{
    /*
    int move = event->pixelDelta().rx();
    if (move != 0)
    {
        QPoint Pos = mapToScene(event->position().x(),event->position().y()).toPoint();
        int index = Pos.ry() / 112;
        if ((index > -1) && (index < devices.size()))
        {
            if (devices[index]->width() > width())
            {
                int l = devices[index]->left()+move;
                if (l > 0) l = 0;
                if (l < width()-devices[index]->width()) l = width()-devices[index]->width();
                devices[index]->setLeft(l);
                updateConnections();
                event->accept();
                return;
            }
        }
    }
    event->ignore();
*/
}

void CJacksComponent::DrawConnections()
{
    //Scene.clear();
    MaxRect = QRect(0,0,width(),devices.size()*112);
    /*
    if (devices.isEmpty())
    {
        MaxRect.setHeight(112);
        Scene.addLine(MaxRect.left(),MaxRect.top(),MaxRect.width(),MaxRect.top(),QPen(Qt::darkGray));
    }
*/
    for (int i = 0; i < devices.size(); i++) devices[i]->paint(Scene,i,MaxRect.width());
    updateConnections();
    setSceneRect(MaxRect);
    horizontalScrollBar()->setMaximum(0);
    setFixedHeight(MaxRect.height());
}

void CJacksComponent::updateConnections(){
    for (QGraphicsItem* i : std::as_const(connectionItems)) {
        Scene->removeItem(i);
    }
    connectionItems.clear();
    for (CJacksDevice* d : std::as_const(devices)) {
        IDevice* d1 = m_DL->device(d->deviceID());
        for (int i = 0; i < d1->jackCount(); i++) {
            if (d->PlugImages.size() > i) d->PlugImages[i]->setVisible(d1->jack(i)->isConnected());
        }
    }
    QList<IDevice*> paintedContainers;
    for (IDevice* inDevice : std::as_const(*m_DL->devices()))
    {
        connectionItems.append(DrawDeviceConnections(inDevice,paintedContainers));
    }
    connectionItems.append(DrawThisConnections());
}

QList<QGraphicsItem *> CJacksComponent::DrawThisConnections(){
    QList<QGraphicsItem*> l;
    int inJackCount = 0;
    int outJackCount = 0;
    for (int ji = 0; ji < m_DL->jackCount(); ji++) {
        if (m_DL->jack(ji)->owner() == "This") {
            const IJack* thisJack = m_DL->jack(ji);
            for (IDevice* d : std::as_const(*m_DL->devices())) {
                for (int i = 0; i < d->jackCount(); i++) {
                    const IJack* j = d->jack(i);
                    if (j->isConnectedTo(thisJack)) {
                        if (thisJack->isInJack()) {
                            l.append(DrawConnection(QPoint(inJackCount * 15,MaxRect.height()),jackPoint(d,i),j->JackColor()));
                        }
                        else {
                            l.append(DrawConnection(jackPoint(d,i),QPoint(outJackCount * 15,0),j->JackColor()));
                        }
                    }
                }
            }
            (thisJack->isInJack()) ? inJackCount++ : outJackCount++;
        }
    }
    return l;
}

QList<QGraphicsItem*> CJacksComponent::DrawDeviceConnections(IDevice* device, QList<IDevice*>& paintedContainers)
{
    QList<QGraphicsItem*> l;
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
                                l.append(DrawConnection(point2,point1,device->jack(i)->JackColor()));
                            }
                            else
                            {
                                l.append(DrawConnection(point1,point2,device->jack(i)->JackColor()));
                            }
                        }
                    }
                }
            }
        }
    }
    paintedContainers.append(device);
    return l;
}

void CJacksComponent::DrawChangedConnections()
{
    emit connectionsChanged();
    updateConnections();
}

QList<QGraphicsItem*> CJacksComponent::DrawConnection(QPoint p1, QPoint p2, const QColor& color, const qreal linewidth)
{
    QList<QGraphicsItem*> l;
    QPainterPath p;
    QRect r(p1,p2);
    r=r.normalized();
    int adjust = 60 - r.width();
    if (adjust < 0) adjust=0;
    r.adjust(adjust,(r.height()/5)+50,-adjust,(r.height()/5)+100);
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
    l.append(Scene->addPath(p.translated(5,5),QPen(QColor(0,0,0,40),linewidth,Qt::SolidLine,Qt::RoundCap)));
    QColor c(color);
    c.setAlphaF(CPresets::presets().ConnectionsOpacity);
    l.append(Scene->addPath(p,QPen(c,linewidth,Qt::SolidLine,Qt::RoundCap)));
    return l;
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
    QPoint Pos = mapToScene(event->pos().x(),event->pos().y()).toPoint();
    if (Pos != MousePoint)
    {
        setToolTip(MouseOverJack(Pos));
        MousePoint = Pos;
    }
    QGraphicsView::mouseMoveEvent(event);
}

void CJacksComponent::mousePressEvent(QMouseEvent *event)
{
    const QPoint Pos = mapToScene(event->pos().x(),event->pos().y()).toPoint();
    const QString JackID = MouseOverJack(Pos);
    if (!JackID.isEmpty()) {
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
