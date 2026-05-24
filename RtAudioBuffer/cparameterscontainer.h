#ifndef CPARAMETERSCONTAINER_H
#define CPARAMETERSCONTAINER_H

#include "cparameterscomponent.h"
#include "../../QGraphicsViewZoomer/qgraphicsviewzoomer.h"
#include "cjacksdevice.h"
#include "cdevicelist.h"
#include <QDrag>
#include <QScrollBar>

class CAutomationLane;

class CParametersContainer : public QGraphicsView
{
    Q_OBJECT

public:
    explicit CParametersContainer(QWidget *parent = 0);
    ~CParametersContainer();
    void unserialize(const QDomLiteElement* xml);
    void serialize(QDomLiteElement* xml) const;
    CAutomationLane* createAutomationLane(IDevice* d, int parameterIndex);
    void animateTo(int i);
public slots:
    void Init(CDeviceList* l);
    void showParameters(IDevice* Device);
    //void updateControls(IDevice* Device);
    void updateControl(IDevice* Device, const CParameter* Parameter);
    void addDevice(IDevice* Device);
    void removeDevice(IDevice* Device);
    void clear();
    int deviceIndex(IDevice* Device);
    int deviceCount();
    void moveDevice(int, int);
    void parametersPixmap(IDevice* d, QPixmap* p);
    void drawParameters();
    void DrawConnections();
    void updateConnections();
    QRectF deviceRect(IDevice* device);
    double constantWidth();
    double unitHeight();
    QGraphicsProxyWidget* addProxyWidget(QWidget* a);
    QList<QWidget*> ProxyWidgets() const;
    void setZoom(double zoom);
    void showAutomation(IDevice* d, int ParameterIndex);
private slots:
    void DrawChangedConnections();
protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void dragEnterEvent(QDragEnterEvent *e);
    void dragMoveEvent(QDragMoveEvent* e);
    void dragLeaveEvent(QDragLeaveEvent* e);
    void dropEvent(QDropEvent *e);
    void resizeEvent(QResizeEvent* event);
private:
    QList<CParametersComponent*> parameterDevices;
    QGraphicsScene Scene;
    QGraphicsViewZoomer* zoomer;
    QGraphicsItemList connectionItems;
    QRect MaxRect;
    CDeviceList* m_DL;
    QString MouseOverJack(const QPoint& Pos);
    QList<CJacksDevice*> devices;
    QPoint jackPoint(IDevice* device, int i);
    QPoint MousePoint;
    QString MenuJackID;
    QRecursiveMutex mutex;
    bool hasAutomation(const QPoint& p);
    QGraphicsItemList DrawThisConnections();
    QGraphicsItemList DrawDeviceConnections(IDevice* device, QList<IDevice*>& paintedContainers);
    QGraphicsItemList DragList;
    bool Dragging;
    IJack* DragJack;
    QPoint DragJackPos;
    void adjustSizes();
    void startDrag(IDevice* d, QPoint globalPos);
signals:
    void automationRequested(IDevice*,int);
    void ParametersChanged(IDevice* Device);
    void connectionsChanged();
    void devicesReordered(int deviceIndex, int move);
    void deviceRemoved(IDevice* Device);
    void aboutToChange(const QString&);
    void popupTriggered(IDevice* Device, QPoint Pos);
    //void zoomed();
    void closeAutomation(IDevice* d);
    void sizeChanged();
};

#endif // CPARAMETERSCONTAINER_H
