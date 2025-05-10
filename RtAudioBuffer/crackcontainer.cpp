#include "crackcontainer.h"
#include "ui_crackcontainer.h"
#include <QScrollBar>
#include "cautomationlane.h"

CRackContainer::CRackContainer(QWidget *parent) :
    QScrollArea(parent),
    ui(new Ui::CRackContainer)
{
    ui->setupUi(this);
    connect(ui->ParametersContainer,&CParametersContainer::ParametersChanged,this,&CRackContainer::ParametersChanged);
    connect(ui->ParametersContainer,&CParametersContainer::aboutToChange,this,&CRackContainer::aboutToChange,Qt::DirectConnection);
    //connect(ui->ParametersContainer,&CParametersContainer::popupTriggered,this,&CRackContainer::ParameterPopupTriggered,Qt::DirectConnection);
    connect(ui->ParametersContainer,&CParametersContainer::automationRequested,this,&CRackContainer::showAutomation);
    connect(ui->ParametersContainer,&CParametersContainer::mousePress,this,&CRackContainer::mousePress);
    //connect(ui->JacksComponent,&CJacksComponent::popupTriggered,this,&CRackContainer::JackPopupTriggered,Qt::DirectConnection);
    connect(ui->JacksComponent,&CJacksComponent::mousePress,this,&CRackContainer::mousePress);
    connect(ui->JacksComponent,&CJacksComponent::connectionsChanged,this,&CRackContainer::connectionsChanged);
    connect(ui->JacksComponent,&CJacksComponent::aboutToChange,this,&CRackContainer::aboutToChange,Qt::DirectConnection);
    verticalScrollBar()->setPageStep(112);
}

CRackContainer::~CRackContainer()
{
    delete ui;
}

void CRackContainer::Init(CDeviceList* dl)
{
    m_DL = dl;
    ui->JacksComponent->Init(dl);
    ui->scrollAreaWidgetContents->setAutoFillBackground(false);
    ui->scrollAreaWidgetContents->setVisible(false);
    setMaximumHeight(0);
}

void CRackContainer::addDevice(IDevice* d)
{
    ui->scrollAreaWidgetContents->setVisible(true);
    ui->ParametersContainer->addDevice(d);
    ui->JacksComponent->addDevice(d);
    setMaximumHeight(112 * deviceCount());
    const int i = ui->ParametersContainer->deviceIndex(d);
    if (i > -1) animateTo(i);
}

void CRackContainer::removeDevice(IDevice* d)
{
    ui->ParametersContainer->removeDevice(d);
    ui->JacksComponent->removeDevice(d);
    setMaximumHeight(112 * deviceCount());
    if (!ui->ParametersContainer->deviceCount())
    {
        ui->scrollAreaWidgetContents->setVisible(false);
    }
}

void CRackContainer::moveDevice(int index, int move)
{
    ui->ParametersContainer->moveDevice(index, move);
    ui->JacksComponent->moveDevice(index, move);
}

void CRackContainer::clear()
{
    ui->ParametersContainer->clear();
    ui->JacksComponent->clear();
    ui->scrollAreaWidgetContents->setVisible(false);
}

int CRackContainer::deviceCount()
{
    return ui->ParametersContainer->deviceCount();
}

void CRackContainer::showParameters(IDevice* d)
{
    qDebug() << "CRackContainer showParameters";
    ui->ParametersContainer->showParameters(d);
    const int i = ui->ParametersContainer->deviceIndex(d);
    if (((i*112) > verticalScrollBar()->sliderPosition()) && (((i+1)*112)<verticalScrollBar()->sliderPosition()+height())) return;
    if (i > -1) animateTo(i);
}

void CRackContainer::updateControls(IDevice* d)
{
    ui->ParametersContainer->updateControls(d);
}

void CRackContainer::updateControl(IDevice* d, const CParameter* p)
{
    ui->ParametersContainer->updateControl(d,p);
}

void CRackContainer::drawConnections()
{
    ui->JacksComponent->DrawConnections();
}

void CRackContainer::animateTo(int i)
{
    const int oldPos = verticalScrollBar()->sliderPosition();
    const int newPos = 112*i;
    if (newPos == oldPos) return;
    QPropertyAnimation *animation = new QPropertyAnimation(verticalScrollBar(), "sliderPosition");
    animation->setEasingCurve(QEasingCurve::OutQuart);
    animation->setDuration((abs(oldPos-newPos)/4)+500);
    animation->setStartValue(oldPos);
    animation->setEndValue(newPos);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void CRackContainer::resizeEvent(QResizeEvent* event)
{
    const int w = event->size().width()*0.75;
    ui->ParametersContainer->setFixedWidth(w);
    ui->JacksComponent->setFixedWidth(w);
    verticalScrollBar()->setPageStep(w);
    for (CAutomationLane* a : (const QList<CAutomationLane*>)findChildren<CAutomationLane*>()) a->setFixedWidth(event->size().width());
    QScrollArea::resizeEvent(event);
}

void CRackContainer::scrollContentsBy(int dx, int dy)
{
    for (CAutomationLane* a : (const QList<CAutomationLane*>)findChildren<CAutomationLane*>()) {
        QRect r(a->geometry());
        r.moveTop(r.top()+dy);
        a->setGeometry(r);
    }
    QScrollArea::scrollContentsBy(dx,dy);
}

void CRackContainer::createAutomationLane(IDevice* d, int parameterIndex)
{
    const int i = ui->ParametersContainer->deviceIndex(d);
    const int newPos = (112*i)-verticalScrollBar()->sliderPosition();
    CAutomationLane* a = new CAutomationLane(this);
    a->setGeometry(0,newPos,width(),112);
    a->updateGeometry();
    a->fill(d,parameterIndex,m_DL);
    a->show();
}

void CRackContainer::showAutomation(IDevice* d, int ParameterIndex)
{
    if (ParameterIndex == -1) ParameterIndex = 0;
    createAutomationLane(d, ParameterIndex);
}

void CRackContainer::unserialize(const QDomLiteElement* xml) {
    if (QDomLiteElement* Lanes = xml->elementByTag("AutomationLanes")) {
        for (const QDomLiteElement* e : (const QDomLiteElementList)Lanes->elementsByTag("AutomationLane")) {
            if (IDevice* d = m_DL->device(e->attribute("DeviceID"))) {
                createAutomationLane(d,e->attributeValueInt("ParameterIndex"));
            }
        }
    }
}

void CRackContainer::serialize(QDomLiteElement* xml) const
{
    QDomLiteElement* Lanes = xml->appendChild("AutomationLanes");
    for (CAutomationLane* a : (const QList<CAutomationLane*>)findChildren<CAutomationLane*>()) {
        if (QDomLiteElement* l = Lanes->appendChild("AutomationLane")) a->serialize(l);
    }
}
