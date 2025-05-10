#include "cparameterscontainer.h"
#include "ui_cparameterscontainer.h"
#include <QScrollBar>

CParametersContainer::CParametersContainer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CParametersContainer)
{
    ui->setupUi(this);
    setFixedHeight(112);
}

CParametersContainer::~CParametersContainer()
{
    delete ui;
}

int CParametersContainer::deviceIndex(IDevice* Device)
{
    if (Device == nullptr) return -1;
    const QString ID = Device->deviceID();
    for (int i = 0; i < devices.size(); i++)
    {
        if (ID == devices[i]->deviceID()) return i;
    }
    return -1;
}

void CParametersContainer::addDevice(IDevice* Device)
{
    if (Device != nullptr)
    {
        ui->emptyFrame->setVisible(false);
        auto p = new CParametersComponent(this);
        layout()->addWidget(p);
        devices.append(p);
        p->show();
        p->init(Device);
        p->showParameters();
        setFixedHeight(112*devices.size());
        connect(p,&CParametersComponent::parametersChanged,this,&CParametersContainer::ParametersChanged);
        connect(p,&CParametersComponent::aboutToChange,this,&CParametersContainer::aboutToChange,Qt::DirectConnection);
        connect(p,&CParametersComponent::showAutomationRequested,this,&CParametersContainer::automationRequested);
        connect(p,&CParametersComponent::popupTriggered,this,&CParametersContainer::popupTriggered);
        connect(p,&CParametersComponent::mousePress,this,&CParametersContainer::mousePress);
    }
}

void CParametersContainer::removeDevice(IDevice* Device)
{
    const int i=deviceIndex(Device);
    if (i > -1)
    {
        CParametersComponent* p = devices[i];
        layout()->removeWidget(p);
        delete devices.takeAt(i);
        if (devices.isEmpty())
        {
            ui->emptyFrame->setVisible(true);
            setFixedHeight(112);
        }
        else
        {
            setFixedHeight(112*devices.size());
        }
    }
}

void CParametersContainer::moveDevice(int index, int move)
{
    if (index < 0) return;
    if (index > devices.size() -1) return;
    if (move == 0) return;
    //int newIndex = index + move;
    int newIndex = std::clamp<int>(index + move,0,devices.size()-1);
    //newIndex = qMax(0, newIndex);
    //newIndex = qMin(devices.size() -1, newIndex);
    if (newIndex == index) return;
    QLayout* l = layout();
    for (CParametersComponent* p : std::as_const(devices)) l->removeWidget(p);
    CParametersComponent* temp = devices.takeAt(index);
    devices.insert(newIndex, temp);
    for (CParametersComponent* p : std::as_const(devices)) l->addWidget(p);
}

void CParametersContainer::clear()
{
    for (CParametersComponent* p: std::as_const(devices))
    {
        layout()->removeWidget(p);
        devices.removeOne(p);
        delete p;
    }
    ui->emptyFrame->setVisible(true);
    setFixedHeight(112);
}

void CParametersContainer::showParameters(IDevice* Device)
{
    qDebug() << "CParametersContainer showParameters";
    const int i=deviceIndex(Device);
    if (i > -1) devices[i]->showParameters();
}

void CParametersContainer::updateControls(IDevice* Device)
{
    //qDebug() << "CParametersContainer updateControls";
    const int i=deviceIndex(Device);
    if (i > -1) devices[i]->updateControls();
}

void CParametersContainer::updateControl(IDevice* Device, const CParameter* Parameter)
{
    //qDebug() << "CParametersContainer updateControl";
    if (Device == nullptr) return;
    const int i=deviceIndex(Device);
    if (i > -1) devices[i]->updateControl(Parameter);
}
/*
QMenu* CParametersContainer::parametersMenu(IDevice* Device)
{
    const int i=deviceIndex(Device);
    if (i < 0) return new QMenu();
    return devices[i]->parametersMenu();
}

QAction* CParametersContainer::pasteParameters(IDevice* Device)
{
    const int i=deviceIndex(Device);
    if (i < 0) return new QAction();
    return devices[i]->pasteParameters();
}
*/
