#ifndef CRACKCONTAINER_H
#define CRACKCONTAINER_H

#include <QScrollArea>
#include "cdevicelist.h"
#include <QMenu>

namespace Ui {
class CRackContainer;
}

class CRackContainer : public QScrollArea
{
    Q_OBJECT

public:
    explicit CRackContainer(QWidget *parent = 0);
    ~CRackContainer();
    void Init(CDeviceList* dl);
    int deviceCount();
    void unserialize(const QDomLiteElement* xml);
    void serialize(QDomLiteElement* xml) const;
public slots:
    void addDevice(IDevice* d);
    void removeDevice(IDevice* d);
    void moveDevice(int, int);
    void clear();
    void showParameters(IDevice* d);
    void updateControls(IDevice* d);
    void updateControl(IDevice* d, const CParameter* p);
    void drawConnections();
    void animateTo(int i);
    void showAutomation(IDevice* d, int ParameterIndex);
protected:
    void resizeEvent(QResizeEvent* event);
    void scrollContentsBy(int dx, int dy);
signals:
    void ParameterPopupTriggered(IDevice* d, QPoint pos);
    void ParametersChanged(IDevice* d);
    void mousePress(IDevice*, QPoint);
    void connectionsChanged();
    void aboutToChange(const QString&);
private:
    Ui::CRackContainer *ui;
    CDeviceList* m_DL;
    void createAutomationLane(IDevice* d, int parameterIndex);
};

#endif // CRACKCONTAINER_H
