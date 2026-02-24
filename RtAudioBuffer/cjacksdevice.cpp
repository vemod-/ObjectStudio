#include "cjacksdevice.h"
#include "qdprpixmap.h"
#include "cconnectionhelper.h"

CJacksDevice::CJacksDevice()
{
}

CJacksDevice::~CJacksDevice()
{
    JackRects.clear();
}

void CJacksDevice::init(IDevice* device)
{
    m_Device = device;
    m_Left = 1160;
}

void CJacksDevice::paint(QGraphicsScene* scene, int index)
{
    static QDPRPixmap freeDeviceJack = QDPRPixmap(rackJackSize,":/Jack.png").shadowedPixmap(4);
    static QDPRPixmap connectedDeviceJack = QDPRPixmap(rackJackSize,":/Plug.png").shadowedPixmap(10);
    JackItems.addToScene(scene);
    PlugImages.addToScene(scene);
    JackItems.setPos(0,calcTop(0,index));
    PlugImages.setPos(0,calcTop(0,index));
    if (!JackRects.isEmpty()) {
        if (index == m_Index) {
            if (m_Device->jackCount() == JackRects.size()) {
                bool match = true;
                for (int i = 0; i < m_Device->jackCount(); i++) {
                    if (m_Device->jack(i) != JackRects[i].jack) match = false;
                }
                if (match) return;
            }
        }
    }
    JackRects.clear();
    PlugImages.clear();
    JackItems.clear();
    m_Index = index;
    QFont f;
    int InIndex = m_Device->inJackCount() - 1;
    int OutIndex = m_Device->outJackCount() - 1;
    for (int i = 0; i < m_Device->jackCount(); i++)
    {
        CJackRect r(m_Device->jack(i));
        QString txt = r.jack->caption();
        QFont f;
        f.setPointSizeF(9.5);
        if (r.jack->isInJack())
        {
            r.moveTopLeft(QPoint(calcLeft(InIndex),34));
            JackRects.append(r);
            JackRects.last().translate(0,calcTop(0,index));
            JackItems.append(CConnectionHelper::DrawShadowTextCenter(txt,f,r.topLeft() + QPoint(-18,-36),QSize(56,34), Qt::AlignHCenter | Qt::AlignBottom));
            InIndex--;
        }
        else
        {
            r.moveTopLeft(QPoint(calcLeft(OutIndex),60));
            JackRects.append(r);
            JackRects.last().translate(0,calcTop(0,index));
            JackItems.append(CConnectionHelper::DrawShadowTextCenter(txt,f,r.bottomLeft() + QPoint(-18,0),QSize(56,34),Qt::AlignHCenter | Qt::AlignTop));
            OutIndex--;
        }
        QRectF sr(scene->sceneRect());
        sr.setTopLeft(QPoint(0,0));
        scene->setSceneRect(sr);
        QColor c(r.jack->JackColor());
        c.setAlpha(95);
        JackItems.append(ellipseItem(QRect(r.topLeft(),r.size() - QSize(3,3)),QPen(c,3),Qt::NoBrush));
        QGraphicsPixmapItem* px = new QGraphicsPixmapItem(freeDeviceJack);
        px->setPos(r.topLeft() - QPoint(1,1));
        JackItems.append(px);
        QGraphicsPixmapItem* px1 = new QGraphicsPixmapItem(connectedDeviceJack);
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

void CJacksDevice::setLeft(const int l) { m_Left = l; }

int CJacksDevice::left() { return m_Left; }

int CJacksDevice::width() {
    if (m_Device)
    {
        int i = qMax<int>(m_Device->inJackCount(),m_Device->outJackCount());
        return (i * 60) + 60 + 60;
    }
    return 0;
}

int CJacksDevice::calcLeft(int i)
{
    return m_Left - (i * 56);
}

int CJacksDevice::calcTop(int i, int index)
{
    return (index * 112) + i;
}

