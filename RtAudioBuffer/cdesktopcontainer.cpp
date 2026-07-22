#include "cdesktopcontainer.h"
#include "ui_cdesktopcontainer.h"
#include <QPropertyAnimation>
#include <QScrollBar>

CDesktopContainer::CDesktopContainer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CDesktopContainer)
{
    ui->setupUi(this);
    Rack = new CParametersContainer(this);
    ui->UIMap->setVisible(false);
    splitter=new QMacSplitter(this);
    splitter->setHandleWidth(1);
    splitter->setOrientation(Qt::Vertical);
    layout()->addWidget(splitter);
    splitter->addWidget(Rack);
    splitter->addWidget(ui->DesktopComponent);
    splitter->setStretchFactor(0,1);
    splitter->setStretchFactor(1,100);
    splitter->setChildrenCollapsible(false);
    Desktop=ui->DesktopComponent;
    setAcceptDrops(false);

    Rack->Init(Desktop->deviceList());

    connect(ui->DesktopComponent,&CDesktopComponent::parametersChanged,Rack,&CParametersContainer::showParameters,Qt::DirectConnection);
    connect(ui->DesktopComponent,&CDesktopComponent::controlChanged,Rack,&CParametersContainer::updateControl,Qt::DirectConnection);
    connect(ui->DesktopComponent,&CDesktopComponent::deviceAdded,Rack,&CParametersContainer::addDevice,Qt::DirectConnection);
    connect(ui->DesktopComponent,&CDesktopComponent::deviceRemoved,Rack,&CParametersContainer::removeDevice,Qt::DirectConnection);
    connect(ui->DesktopComponent,&CDesktopComponent::devicesReordered,Rack,&CParametersContainer::moveDevice,Qt::DirectConnection);
    connect(ui->DesktopComponent,&CDesktopComponent::devicesCleared,Rack,&CParametersContainer::clear,Qt::DirectConnection);
    connect(ui->DesktopComponent,&CDesktopComponent::connectionsChanged,Rack,&CParametersContainer::updateConnections,Qt::QueuedConnection);
    connect(ui->DesktopComponent,&CDesktopComponent::jacksChanged,Rack,&CParametersContainer::DrawConnections,Qt::QueuedConnection);
    connect(ui->DesktopComponent,&CDesktopComponent::requestSerializeAutomationXML,Rack,&CParametersContainer::serialize,Qt::DirectConnection);
    connect(ui->DesktopComponent,&CDesktopComponent::requestUnserializeAutomationXML,Rack,&CParametersContainer::unserialize,Qt::DirectConnection);
    connect(ui->DesktopComponent,&CDesktopComponent::requestCloseAutomation,Rack,&CParametersContainer::closeAutomation,Qt::DirectConnection);
    connect(ui->DesktopComponent,&CDesktopComponent::requestParametersPixmap,Rack,&CParametersContainer::parametersPixmap,Qt::DirectConnection);

    connect(Rack,&CParametersContainer::connectionsChanged,ui->DesktopComponent,&CDesktopComponent::DrawConnections,Qt::QueuedConnection);
    connect(Rack,&CParametersContainer::aboutToChange,ui->DesktopComponent->MainMenu->UndoMenu,&CUndoMenu::addItem,Qt::DirectConnection);
    connect(Rack,&CParametersContainer::ParametersChanged,ui->DesktopComponent,&CDesktopComponent::DrawConnections,Qt::QueuedConnection);
    connect(Rack,&CParametersContainer::deviceRemoved,ui->DesktopComponent,&CDesktopComponent::RemoveDevice);
    connect(Rack,&CParametersContainer::devicesReordered,ui->DesktopComponent,&CDesktopComponent::moveDevice);
    connect(ui->UIMap,&CUIMap::deviceSelected,this,&CDesktopContainer::hideMap,Qt::QueuedConnection);
}

CDesktopContainer::~CDesktopContainer()
{
    delete ui;
}

void CDesktopContainer::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    resizeContent();
}

void CDesktopContainer::resizeContent()
{
    const int MaxHeight = Rack->deviceCount() * Rack->unitHeight();
    //splitter->setCollapsible(0,(MaxHeight == 0));
    //Rack->setMinimumHeight((MaxHeight > 0) * rackUnitHeight);
    //Rack->setMaximumHeight(MaxHeight);
    splitter->setMinimumHeight((MaxHeight > 0) * Rack->unitHeight());
    //splitter->setCollapsible(1,(splitter->height()<=MaxHeight));
    //ui->DesktopComponent->updateGeometry();
    //Desktop->DrawConnections();
}
/*
void CDesktopContainer::addDevice(IDevice* d)
{
    QMutexLocker locker(&mutex);
    Rack->addDevice(d);
    resizeContent();
}

void CDesktopContainer::removeDevice(IDevice* d)
{
    QMutexLocker locker(&mutex);
    Rack->removeDevice(d);
    resizeContent();
}

void CDesktopContainer::clear()
{
    QMutexLocker locker(&mutex);
    Rack->clear();
    resizeContent();
}
*/
void CDesktopContainer::showMap()
{
    ui->UIMap->showMap(Desktop->deviceList(),this,splitter);
}

void CDesktopContainer::showParameters(IDevice* d)
{
    Rack->showParameters(d);
}
/*
void CDesktopContainer::showAutomation(IDevice* d)
{
    ui->RackContainer->showAutomation(d, Desktop->deviceList());
}
*/
/*
void CDesktopContainer::duplicateMenu(QMenu* dst, QMenu& origin)
{
    QMenu* sub = dst->addMenu(origin.title());
    QList<QAction*> actions=origin.actions();

    for(QList<QAction*>::iterator it=actions.begin(); it!=actions.end(); it++)
    {
        QMenu* itMenu = (*it)->menu();

        if(itMenu!=NULL)
            duplicateMenu(sub, *itMenu);
        else
            sub->addAction(*it);
    }
}
*/
/*
void CDesktopContainer::getParametersMenu(QMenu* m, IDevice* d)
{
    QMenu* menu = ui->RackContainer->parametersMenu(d);
    duplicateMenu(m,*menu);
}

void CDesktopContainer::getPasteParameters(QMenu* m, IDevice* d)
{
    QAction* a = ui->RackContainer->pasteParameters(d);
    m->addAction(a);
}
*/
/*
void CDesktopContainer::updateControls(IDevice* d)
{
    Rack->updateControls(d);
}
*/
void CDesktopContainer::hideMap()
{
    ui->UIMap->setVisible(false);
    splitter->setVisible(true);
}

void CDesktopContainer::hideUIs()
{
    Desktop->deviceList()->hideForms();
}

void CDesktopContainer::cascadeUIs(QPoint& p)
{
    Desktop->deviceList()->cascadeForms(p);
}
