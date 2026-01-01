#include "cmacroboxform.h"
#include "ui_cmacroboxform.h"
#include <QInputDialog>
//#include <QMessageBox>
#include <QClipboard>
#include <QMenu>
#include "cparametersmenu.h"
#include "ccustomparameterdialog.h"
#include "ccustomjacksdialog.h"

CMacroBoxForm::CMacroBoxForm(IDevice* Device, QWidget *parent) :
    CSoftSynthsForm(Device,false,parent),
    ui(new Ui::CMacroBoxForm)
{
    ui->setupUi(this);
    ui->verticalLayout->setContentsMargins(0,0,0,0);
    DesktopContainer=ui->DesktopContainer;
    DesktopComponent=DesktopContainer->Desktop;
    DesktopContainers.append(DesktopContainer);
    DesktopComponents.append(DesktopComponent);
    DesktopComponent->init(parent);
    ui->verticalLayout_2->setContentsMargins(0,0,0,0);
    ui->li->setAttribute(Qt::WA_MacShowFocusRect,false);
    ui->widget->hide();

    connect(ui->li,qOverload<int>(&QComboBox::currentIndexChanged),this,&CMacroBoxForm::ChangeProgram);

    connect(DesktopComponent,&CDesktopComponent::parametersChanged,this,&CMacroBoxForm::PlugInIndexChanged);

    connect(DesktopComponent,&CDesktopComponent::deviceRemoved,this,&CMacroBoxForm::removeDeviceParameters);
    connect(DesktopComponent,&CDesktopComponent::devicesCleared,this,&CMacroBoxForm::removeAllParameters);

    fillList();
    m_TimerID=startTimer(0);
}

CMacroBoxForm::~CMacroBoxForm()
{
    QMutexLocker locker(&mutex);
    qDebug() << "~CMacroBoxForm";
    killTimer(m_TimerID);
    m_TimerID=0;
    delete ui;
}

void CMacroBoxForm::unserializeCustom(const QDomLiteElement* xml)
{
    QMutexLocker locker(&mutex);
    if (allowCustomJacks) {
        if (const QDomLiteElement* CustomJacks=xml->elementByTag("CustomJacks"))
        {
            m_CustomJackList.unserialize(CustomJacks,m_Device,&DesktopComponents);
        }
    }
    if (DesktopComponents.size() == 1) {
        DesktopComponent->unserialize(xml);
    }
    else {
        if (const QDomLiteElement* desktops = xml->elementByTag("Desktops"))
        {
            int i = 0;
            for (const QDomLiteElement* d : (const QDomLiteElementList)desktops->elementsByTag("Desktop"))
            {
                if (i < DesktopComponents.size()) DesktopComponents[i++]->unserialize(d);
            }
        }
    }
    fillList(m_Device->currentProgram());
    if (allowCustomParameters) {
        if (const QDomLiteElement* CustomParameters = xml->elementByTag("CustomParameters"))
        {
            m_CustomParameterList.unserialize(CustomParameters,DesktopComponent->deviceList(),m_Device);
        }
    }
    updateDeviceParameter(nullptr);
}

void CMacroBoxForm::serializeCustom(QDomLiteElement* xml) const
{
    if (allowCustomParameters) {
        QDomLiteElement* CustomParameters = xml->elementByTagCreate("CustomParameters");
        m_CustomParameterList.serialize(CustomParameters);
    }
    if (allowCustomJacks) {
        QDomLiteElement* CustomJacks = xml->elementByTagCreate("CustomJacks");
        m_CustomJackList.serialize(CustomJacks, m_Device);
    }
    if (DesktopComponents.size() == 1) {
        DesktopComponent->serialize(xml);
    }
    else {
        QDomLiteElement* desktops = xml->elementByTagCreate("Desktops");
        desktops->clearChildren();
        for (const CDesktopComponent* d : std::as_const(DesktopComponents)) d->serialize(desktops->appendChild("Desktop"));
    }
}

void CMacroBoxForm::fillList(int CurrentProgram)
{
    ui->li->blockSignals(true);
    ui->li->clear();
    ui->li->addItems(m_Device->programNames());
    ui->widget->setVisible(ui->li->count() > 1);
    if (CurrentProgram > -1)
    {
        ui->li->setCurrentIndex(CurrentProgram);
    }
    ui->li->blockSignals(false);
}

void CMacroBoxForm::PlugInIndexChanged()
{
    fillList(m_Device->currentProgram());
    //setVisible(true);
    m_Device->updateHostParameter();
}

void CMacroBoxForm::ChangeProgram(int programIndex)
{
    m_Device->setCurrentProgram(programIndex);
    m_Device->updateHostParameter();
}

void CMacroBoxForm::setProgram(const int programIndex)
{
    ChangeProgram(programIndex);
    if (ui->li->count() > programIndex)
    {
        ui->li->blockSignals(true);
        ui->li->setCurrentIndex(programIndex);
        ui->li->blockSignals(false);
        m_Device->updateHostParameter();
    }
}

void CMacroBoxForm::updateDeviceParameter(const CParameter *p) {
    if (p == nullptr) {
        for (int i = 0; i < m_Device->parameterCount(); i++) {
            updateParameter(i,m_Device->parameter(i)->Value);
        }
    }
    else {
        for (int i = 0; i < m_Device->parameterCount(); i++) {
            if (m_Device->parameter(i) == p) {
                updateParameter(i,p->Value);
            }
        }
    }
}

void CMacroBoxForm::cascadeUIs()
{
    QPoint p(24,24);
    DesktopContainer->cascadeUIs(p);
}

bool CMacroBoxForm::event(QEvent *event)
{
    if (event->type()==QEvent::NonClientAreaMouseButtonPress)
    {
        if (dynamic_cast<QMouseEvent*>(event)->button()==Qt::RightButton)
        {
            CParametersMenu* m = new CParametersMenu(m_Device,this,false);
            m->setAttribute(Qt::WA_DeleteOnClose,true);
            connect(m,&CParametersMenu::aboutToChange,DesktopComponent->MainMenu->UndoMenu,&CUndoMenu::addItem,Qt::DirectConnection);
            connect(m,&CParametersMenu::parametersChanged,this,&CMacroBoxForm::PlugInIndexChanged);
            m->addSeparator();
            if (allowCustomParameters) {
                addParameterMenu(m);
                QAction* a = new QAction("Edit Custom Parameters");
                a->setEnabled(!m_CustomParameterList.isEmpty());
                m->addAction(a);
                connect(a,&QAction::triggered,this,&CMacroBoxForm::showParameterDialog);
                m->addSeparator();
            }
            if (allowCustomJacks) {
                addJackMenu(m);
                removeJackMenu(m);
                QAction* a = new QAction("Edit Custom Jacks");
                a->setEnabled(!CJackCompareList(m_Device).isEmpty());
                m->addAction(a);
                connect(a,&QAction::triggered,this,&CMacroBoxForm::showJackDialog);
                m->addSeparator();
            }
            m->addAction("UI map",DesktopContainer,&CDesktopContainer::showMap);
            m->addAction("Hide UIs",DesktopContainer,&CDesktopContainer::hideUIs);
            m->addAction("Cascade UIs",this,&CMacroBoxForm::cascadeUIs);

            m->popup(mapToGlobal(dynamic_cast<QMouseEvent*>(event)->pos()));
        }
    }
    return CSoftSynthsForm::event(event);
}

void CMacroBoxForm::addParameterMenu(QMenu* m) {
    const QList<IDevice*>* devices = DesktopComponent->deviceList()->devices();
    QMenu* menu = m->addMenu("Add Parameter");
    if (devices->size() == 0) {
        menu->setEnabled(false);
        return;
    }
    for (const IDevice* d : *devices) {
        QSignalMenu* deviceMenu = new QSignalMenu(d->deviceID(),menu);
        menu->addMenu(deviceMenu);
        connect(deviceMenu,qOverload<QString>(&QSignalMenu::menuClicked),this,qOverload<QString>(&CMacroBoxForm::addCustomParameter));
        for (int i = 0; i < d->parameterCount(); i++) {
            QAction* a = deviceMenu->addAction(d->parameter(i)->Name,CParameterID::parameterID(d->deviceID(), d->parameter(i)));
            if (m_CustomParameterList.containsAdditional(d->deviceID(), d->parameter(i)->Name)) a->setEnabled(false);
        }
    }
}

void CMacroBoxForm::showParameterDialog(){
    CCustomParameterDialog* d = new CCustomParameterDialog(this);
    d->fill(DesktopComponent->deviceList(),&m_CustomParameterList,m_Device);
    d->exec();
}

void CMacroBoxForm::showJackDialog(){
    CCustomJacksDialog* d = new CCustomJacksDialog(this);
    d->fill(&DesktopComponents,DesktopComponent,&m_CustomJackList,m_Device);
    d->exec();
}

void CMacroBoxForm::addCustomParameter(QString id){
    CParameterID pid(id);
    if (IDevice* d = DesktopComponent->deviceList()->device(pid.DeviceID)) {
        if (CParameter* p = d->parameter(pid.ParameterName)) {
            addCustomParameter(d,p);
        }
    }
}

void CMacroBoxForm::addCustomParameter(IDevice* d, CParameter* p)
{
    CCustomParameter* c = m_CustomParameterList.append(d->deviceID(),p);
    c->masterParameter = DesktopComponent->deviceList()->addCustomParameter(m_Device,p->Type,c->defaultCaption(),p->Unit,p->Min,p->Max,p->DecimalFactor,p->List,p->Value);
    m_Device->updateHostParameter();
}

void CMacroBoxForm::removeDeviceParameters(IDevice *device) {
    m_CustomParameterList.removeDevice(device->deviceID());
}

void CMacroBoxForm::removeAllParameters() {
    for (IDevice* d : *DesktopComponent->deviceList()->devices()) {
        removeDeviceParameters(d);
    }
}

void CMacroBoxForm::addJackMenu(QMenu *m){
    const QList<IDevice*>* devices = DesktopComponent->deviceList()->devices();
    QMenu* menu = m->addMenu("Add Jack");
    if (devices->size() == 0) {
        menu->setEnabled(false);
        return;
    }
    for (const IDevice* d : *devices) {
        QSignalMenu* deviceMenu = new QSignalMenu(d->deviceID(),menu);
        menu->addMenu(deviceMenu);
        connect(deviceMenu,qOverload<QString>(&QSignalMenu::menuClicked),this,qOverload<QString>(&CMacroBoxForm::addCustomJack));
        for (int i = 0; i < d->jackCount(); i++) {
            QAction* a = deviceMenu->addAction(d->jack(i)->caption(),d->jackID(i));
            for (int j = 0; j < m_Device->jackCount(); j++) {
                if (DesktopComponent->JacksCreated[j]->isConnectedTo(d->jack(i))) a->setEnabled(false);
            }
        }
    }
}

void CMacroBoxForm::removeJackMenu(QMenu *m){
    QSignalMenu* menu = new QSignalMenu("Remove Jack",m);
    m->addMenu(menu);
    if (m_Device->jackCount() == 0) {
        menu->setEnabled(false);
        return;
    }
    connect(menu,qOverload<QString>(&QSignalMenu::menuClicked),this,qOverload<QString>(&CMacroBoxForm::removeCustomJack));
    for (int i = 0; i < m_Device->jackCount(); i++) {
        menu->addAction(m_Device->jack(i)->caption(),m_Device->jackID(i));
    }
}

void CMacroBoxForm::addCustomJack(QString id){
    IJack* j = DesktopComponent->deviceList()->jack(id);
    m_CustomJackList.addJack(j->jackID(),"",j->attachMode,j->direction,m_Device,&DesktopComponents);
    DesktopComponent->SelectDevice(DesktopComponent->deviceList()->device(j->owner()));
    if (IJack* J = DesktopComponent->deviceList()->jack(DesktopComponent->JacksCreated.last()->jackID())) j->connectTo(J);
    for (CDesktopComponent* desktop : std::as_const(DesktopComponents)) {
        desktop->DrawConnections();
        desktop->connectionsChanged();
    }
}

void CMacroBoxForm::removeCustomJack(QString id){
    IJack* J = nullptr;
    for (int i = 0; i < m_Device->jackCount(); i++) {
        if (m_Device->jackID(i) == id) {
            J = m_Device->jack(i);
            break;
        }
    }
    m_CustomJackList.removeJack(J,m_Device,&DesktopComponents);
    for (CDesktopComponent* desktop : std::as_const(DesktopComponents)) {
        desktop->DrawConnections();
        desktop->connectionsChanged();
    }
}

void CMacroBoxForm::updateParameter(int index, int v) {
    m_CustomParameterList.setValue(DesktopComponent->deviceList(),index,v);
}

void CMacroBoxForm::timerEvent(QTimerEvent */*event*/)
{
    if (!m_TimerID) return;
    if (ui->li->count() != m_Device->programNames().size())
    {
        fillList(m_Device->currentProgram());
    }
    else if (ui->li->count())
    {
        const int p=m_Device->currentProgram();
        if (p != ui->li->currentIndex())
        {
            ui->li->blockSignals(true);
            ui->li->setCurrentIndex(p);
            ui->li->blockSignals(false);
        }
    }
}
