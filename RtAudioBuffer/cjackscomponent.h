#ifndef CJACKSCOMPONENT_H
#define CJACKSCOMPONENT_H

#include <QGraphicsView>
//#include <QMouseEvent>
//#include <qsignalmenu.h>
#include "cdevicelist.h"

namespace Ui {
    class CJacksComponent;
}

class CJacksDevice
{
public:
    CJacksDevice();
    ~CJacksDevice();
    void init(IDevice* device);
    void paint(QGraphicsScene* scene, int index, int width);
    QString deviceID();
    QString jackID(const int j);
    QPoint jackPoint(int i);
    int MouseOverJack(const QPoint& p);
    void setLeft(const int l) { m_Left=l; }
    int left() { return m_Left; }
    int width() {
        if (m_Device)
        {
            int i = qMax<int>(m_Device->inJackCount(),m_Device->outJackCount());
            return (i*60) + 60 + 60;
        }
        return 0;
    }
private:
    int m_Index;
    IDevice* m_Device;
    QList<QRect> JackRects;
    int m_Left;
    int calcLeft(int i)
    {
        return (i*60) + 60 + m_Left;
    }
    int calcTop(int i, int index)
    {
        return (index*112) + i;
    }
};

class CJacksComponent : public QGraphicsView
{
    Q_OBJECT

public:
    explicit CJacksComponent(QWidget *parent = 0);
    ~CJacksComponent();
    void Init(CDeviceList* DeviceList);
public slots:
    void addDevice(IDevice* device);
    void removeDevice(IDevice* device);
    void moveDevice(int, int);
    void clear();
    void DrawConnections();
    void DrawDeviceConnections(IDevice* device, QList<IDevice*>& paintedContainers);
    void DrawConnection(QPoint p1, QPoint p2, const QColor& color);
    //void JackMenuPopup(IJack* jack, QPoint pos);
    //void ToggleConnection(QString JackID);
private slots:
    void DrawChangedConnections();
signals:
    void connectionsChanged();
    void aboutToChange(const QString&);
    void mousePress(IDevice*, QPoint);
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent* event);
    void resizeEvent(QResizeEvent* event);
    void wheelEvent(QWheelEvent* event);
private:
    Ui::CJacksComponent *ui;
    QGraphicsScene Scene;
    CDeviceList* m_DL;
    QString MouseOverJack(const QPoint& Pos);
    QList<CJacksDevice*> devices;
    int deviceIndex(IDevice* device);
    QPoint jackPoint(IDevice* device, int i);
    QPoint MousePoint;
    //QSignalMenu* JackPopup;
    QString MenuJackID;
    QRecursiveMutex mutex;
};

#endif // CJACKSCOMPONENT_H
