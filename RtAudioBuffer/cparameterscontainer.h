#ifndef CPARAMETERSCONTAINER_H
#define CPARAMETERSCONTAINER_H

#include "cparameterscomponent.h"
#include "../../QGraphicsViewZoomer/qgraphicsviewzoomer.h"

class CParametersContainer : public QGraphicsView
{
    Q_OBJECT

public:
    explicit CParametersContainer(QWidget *parent = 0);
    ~CParametersContainer();
public slots:
    void showParameters(IDevice* Device);
    void updateControls(IDevice* Device);
    void updateControl(IDevice* Device, const CParameter* Parameter);
    void addDevice(IDevice* Device);
    void removeDevice(IDevice* Device);
    void clear();
    int deviceIndex(IDevice* Device);
    int deviceCount() { return devices.size(); }
    void moveDevice(int, int);
    void parametersPixmap(IDevice* d, QPixmap* p);
    void drawParameters() {
        int i = 0;
        for (CParametersComponent* p : std::as_const(devices)) {
            p->showParameters(i++);
        }
    }
protected:
    void mousePressEvent(QMouseEvent* event) {
        const QPointF scenePos = mapToScene(event->pos());
        int i = scenePos.y() / rackUnitHeight;
        if ((i >= 0) && (i < devices.size())) {
            QGraphicsItem* item = scene()->itemAt(scenePos, transform());
            if (!devices[i]->swallowMousePress(event,item)) QGraphicsView::mousePressEvent(event);
        }
    }
private:
    QList<CParametersComponent*> devices;
    QGraphicsScene Scene;
    QGraphicsViewZoomer* zoomer;
signals:
    void automationRequested(IDevice*,int);
    void ParametersChanged(IDevice* Device);
    void aboutToChange(const QString&);
    void popupTriggered(IDevice* Device, QPoint Pos);
    void mousePress(IDevice*, QPoint);
};

#endif // CPARAMETERSCONTAINER_H
