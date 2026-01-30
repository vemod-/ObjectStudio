#include "crackcontainer.h"
#include "ui_crackcontainer.h"
#include <QScrollBar>
#include "cautomationlane.h"

CRackContainer::CRackContainer(QWidget *parent) :
    QScrollArea(parent),
    ui(new Ui::CRackContainer)
{
    ui->setupUi(this);
    m_ParametersContainer = new CParametersContainer(this);
    m_JacksComponent = new CJacksComponent(this, m_ParametersContainer->scene());
    ui->scrollAreaWidgetContents->layout()->addWidget(m_ParametersContainer);
    ui->scrollAreaWidgetContents->layout()->addWidget(m_JacksComponent);
    connect(m_ParametersContainer,&CParametersContainer::ParametersChanged,this,&CRackContainer::ParametersChanged);
    connect(m_ParametersContainer,&CParametersContainer::aboutToChange,this,&CRackContainer::aboutToChange,Qt::DirectConnection);
    //connect(ui->ParametersContainer,&CParametersContainer::popupTriggered,this,&CRackContainer::ParameterPopupTriggered,Qt::DirectConnection);
    connect(m_ParametersContainer,&CParametersContainer::automationRequested,this,&CRackContainer::showAutomation);
    connect(m_ParametersContainer,&CParametersContainer::mousePress,this,&CRackContainer::mousePress);
    connect(this,&CRackContainer::parametersPixmap,m_ParametersContainer,&CParametersContainer::parametersPixmap,Qt::DirectConnection);
    //connect(ui->JacksComponent,&CJacksComponent::popupTriggered,this,&CRackContainer::JackPopupTriggered,Qt::DirectConnection);
    connect(m_JacksComponent,&CJacksComponent::mousePress,this,&CRackContainer::mousePress);
    connect(m_JacksComponent,&CJacksComponent::connectionsChanged,this,&CRackContainer::connectionsChanged);
    connect(m_JacksComponent,&CJacksComponent::aboutToChange,this,&CRackContainer::aboutToChange,Qt::DirectConnection);
    verticalScrollBar()->setPageStep(rackUnitHeight);
}

CRackContainer::~CRackContainer()
{
    delete ui;
}

void CRackContainer::Init(CDeviceList* dl)
{
    m_DL = dl;
    m_JacksComponent->Init(dl);
    ui->scrollAreaWidgetContents->setAutoFillBackground(false);
    ui->scrollAreaWidgetContents->setVisible(false);
    setMaximumHeight(0);
}

void CRackContainer::addDevice(IDevice* d)
{
    ui->scrollAreaWidgetContents->setVisible(true);
    m_ParametersContainer->addDevice(d);
    m_JacksComponent->addDevice(d);
    setMaximumHeight(rackUnitHeight * deviceCount());
    const int i = m_ParametersContainer->deviceIndex(d);
    if (i > -1) animateTo(i);
}

void CRackContainer::removeDevice(IDevice* d)
{
    m_ParametersContainer->removeDevice(d);
    m_JacksComponent->removeDevice(d);
    setMaximumHeight(rackUnitHeight * deviceCount());
    if (!m_ParametersContainer->deviceCount())
    {
        ui->scrollAreaWidgetContents->setVisible(false);
    }
}

void CRackContainer::moveDevice(int index, int move)
{
    m_ParametersContainer->moveDevice(index, move);
    m_JacksComponent->moveDevice(index, move);
}

void CRackContainer::clear()
{
    m_ParametersContainer->clear();
    m_JacksComponent->clear();
    ui->scrollAreaWidgetContents->setVisible(false);
}

int CRackContainer::deviceCount()
{
    return m_ParametersContainer->deviceCount();
}

void CRackContainer::showParameters(IDevice* d)
{
    qDebug() << "CRackContainer showParameters";
    m_ParametersContainer->showParameters(d);
    const int i = m_ParametersContainer->deviceIndex(d);
    if (((i*rackUnitHeight) > verticalScrollBar()->sliderPosition()) && (((i+1)*rackUnitHeight)<verticalScrollBar()->sliderPosition()+height())) return;
    if (i > -1) animateTo(i);
}

void CRackContainer::updateControls(IDevice* d)
{
    m_ParametersContainer->updateControls(d);
}

void CRackContainer::updateControl(IDevice* d, const CParameter* p)
{
    m_ParametersContainer->updateControl(d,p);
}

void CRackContainer::updateConnections()
{
    m_JacksComponent->updateConnections();
}

void CRackContainer::animateTo(int i)
{
    const int oldPos = verticalScrollBar()->sliderPosition();
    const int newPos = rackUnitHeight*i;
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
    m_ParametersContainer->setFixedWidth(w);
    m_JacksComponent->setFixedWidth(w);
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
    const int i = m_ParametersContainer->deviceIndex(d);
    const int newPos = (rackUnitHeight*i)-verticalScrollBar()->sliderPosition();
    CAutomationLane* a = new CAutomationLane(this);
    a->setGeometry(0,newPos,width(),rackUnitHeight);
    a->updateGeometry();
    a->fill(d,parameterIndex,m_DL);
    a->show();
    connect(this,&CRackContainer::closeAutomation,a,&CAutomationLane::close);
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
