#ifndef CJACKSDEVICE_H
#define CJACKSDEVICE_H

#include "idevice.h"
#include <QGraphicsScene>
#include "qgraphicsitemlist.h"

#define rackJackSize QSize(22,22)

class CJackRect : public QRect
{
public:
    CJackRect(IJack* j) {
        jack = j;
        setSize(rackJackSize + QSize(1,1));
    }
    IJack* jack = nullptr;
    void setRect(QPoint p, QSize s) {
        QRect::setRect(p.x(),p.y(),s.width(),s.height());
    }
};

class CJacksDevice
{
public:
    CJacksDevice();
    ~CJacksDevice();
    void init(IDevice* device);
    void paint(QGraphicsScene* scene, int index);
    QString deviceID();
    QString jackID(const int j);
    QPoint jackPoint(int i);
    int MouseOverJack(const QPoint& p);
    void setLeft(const int l);
    int left();
    int width();
    QGraphicsContainerItem PlugImages;
    QGraphicsContainerItem JackItems;
private:
    int m_Index = -1;
    IDevice* m_Device = nullptr;
    QList<CJackRect> JackRects;
    int m_Left = 0;
    int calcLeft(int i);
    int calcTop(int i, int index);
};


#endif // CJACKSDEVICE_H
