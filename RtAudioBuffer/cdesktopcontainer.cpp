#include "cdesktopcontainer.h"
#include "ui_cdesktopcontainer.h"
#include <QPropertyAnimation>
#include <QScrollBar>

CDesktopContainer::CDesktopContainer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CDesktopContainer)
{
    ui->setupUi(this);
    ui->UIMap->setVisible(false);
    splitter=new QMacSplitter(this);
    splitter->setHandleWidth(1);
    splitter->setOrientation(Qt::Vertical);
    layout()->addWidget(splitter);
    splitter->addWidget(ui->RackContainer);
    splitter->addWidget(ui->DesktopComponent);
    splitter->setStretchFactor(0,1);
    splitter->setStretchFactor(1,100);
    splitter->setChildrenCollapsible(false);
    Desktop=ui->DesktopComponent;

    ui->RackContainer->Init(Desktop->deviceList());

    connect(ui->DesktopComponent,&CDesktopComponent::parametersChanged,ui->RackContainer,&CRackContainer::showParameters,Qt::QueuedConnection);
    connect(ui->DesktopComponent,&CDesktopComponent::controlChanged,ui->RackContainer,&CRackContainer::updateControl,Qt::QueuedConnection);

    connect(ui->DesktopComponent,&CDesktopComponent::deviceAdded,this,&CDesktopContainer::addDevice,Qt::DirectConnection);
    connect(ui->DesktopComponent,&CDesktopComponent::deviceRemoved,this,&CDesktopContainer::removeDevice,Qt::DirectConnection);
    connect(ui->DesktopComponent,&CDesktopComponent::devicesCleared,this,&CDesktopContainer::clear,Qt::DirectConnection);

    connect(ui->DesktopComponent,&CDesktopComponent::connectionsChanged,ui->RackContainer,&CRackContainer::drawConnections,Qt::QueuedConnection);
    connect(ui->RackContainer,&CRackContainer::connectionsChanged,ui->DesktopComponent,&CDesktopComponent::DrawConnections,Qt::QueuedConnection);
    connect(ui->RackContainer,&CRackContainer::aboutToChange,ui->DesktopComponent->MainMenu->UndoMenu,&CUndoMenu::addItem,Qt::DirectConnection);
    connect(ui->RackContainer,&CRackContainer::ParametersChanged,ui->DesktopComponent,&CDesktopComponent::DrawConnections,Qt::QueuedConnection);

    connect(ui->UIMap,&CUIMap::deviceSelected,this,&CDesktopContainer::hideMap,Qt::QueuedConnection);
    connect(ui->DesktopComponent,&CDesktopComponent::requestSerializeAutomationXML,ui->RackContainer,&CRackContainer::serialize,Qt::DirectConnection);
    connect(ui->DesktopComponent,&CDesktopComponent::requestUnserializeAutomationXML,ui->RackContainer,&CRackContainer::unserialize,Qt::DirectConnection);
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
    const int MaxHeight=ui->RackContainer->deviceCount()*112;
    //splitter->setCollapsible(0,(MaxHeight == 0));
    ui->RackContainer->setMinimumHeight((MaxHeight > 0) * 112);
    ui->RackContainer->setMaximumHeight(MaxHeight);
    splitter->setMinimumHeight((MaxHeight > 0) * 112);
    //splitter->setCollapsible(1,(splitter->height()<=MaxHeight));
    //ui->DesktopComponent->updateGeometry();
    //Desktop->DrawConnections();
}

void CDesktopContainer::addDevice(IDevice* d)
{
    QMutexLocker locker(&mutex);
    ui->RackContainer->addDevice(d);
    resizeContent();
}

void CDesktopContainer::removeDevice(IDevice* d)
{
    QMutexLocker locker(&mutex);
    ui->RackContainer->removeDevice(d);
    resizeContent();
}

void CDesktopContainer::clear()
{
    QMutexLocker locker(&mutex);
    ui->RackContainer->clear();
    resizeContent();
}

void CDesktopContainer::showMap()
{
    ui->UIMap->showMap(Desktop->deviceList(),this,splitter);
}

void CDesktopContainer::showParameters(IDevice* d)
{
    ui->RackContainer->showParameters(d);
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
void CDesktopContainer::updateControls(IDevice* d)
{
    ui->RackContainer->updateControls(d);
}

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
