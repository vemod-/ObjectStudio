#ifndef CPARAMETERSCONTAINER_H
#define CPARAMETERSCONTAINER_H

#include "cparameterscomponent.h"

namespace Ui {
class CParametersContainer;
}

class CParametersContainer : public QWidget
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
    //QMenu* parametersMenu(IDevice*);
    //QAction* pasteParameters(IDevice*);
protected:
private:
    Ui::CParametersContainer *ui;
    QList<CParametersComponent*> devices;
signals:
    void automationRequested(IDevice*,int);
    void ParametersChanged(IDevice* Device);
    void aboutToChange(const QString&);
    void popupTriggered(IDevice* Device, QPoint Pos);
    void mousePress(IDevice*, QPoint);
};

#endif // CPARAMETERSCONTAINER_H
