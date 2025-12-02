#include "cmacroboxform.h"
#include "ui_cmacroboxform.h"
#include <QInputDialog>
//#include <QMessageBox>
#include <QClipboard>
#include <QMenu>
#include "cparametersmenu.h"
#include "ccustomparameterdialog.h"

CMacroBoxForm::CMacroBoxForm(IDevice* Device, QWidget *parent) :
    CSoftSynthsForm(Device,false,parent),
    ui(new Ui::CMacroBoxForm)
{
    ui->setupUi(this);
    ui->verticalLayout->setContentsMargins(0,0,0,0);
    DesktopContainer=ui->DesktopContainer;
    DesktopComponent=DesktopContainer->Desktop;
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
    /*
    m_ParameterDevice = new IDevice();
    m_ParameterDevice->init(0,this);
    m_ParametersComponent = new CParametersComponent(this);
    m_ParametersComponent->init(m_ParameterDevice);
    ui->verticalLayout->insertWidget(0,m_ParametersComponent);
*/
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
    DesktopComponent->unserialize(xml);
    fillList(m_Device->currentProgram());
    if (const QDomLiteElement* CustomParameters=xml->elementByTag("CustomParameters"))
    {
        /*
        for (int i = 0; i < CustomParameters->childCount(); i++) {
            QDomLiteElement* p = CustomParameters->childElement(i);
            toggleCustomParameter(p->attribute("Id"));
            if (i < m_Device->parameterCount()) m_Device->parameter(i)->unserialize(p);
        }
*/
        m_CustomParameterList.unserialize(CustomParameters,DesktopComponent->deviceList(),m_Device);
    }
    updateDeviceParameter(nullptr);
}

void CMacroBoxForm::serializeCustom(QDomLiteElement* xml) const
{
    QDomLiteElement* CustomParameters=xml->appendChild("CustomParameters");
    /*
    for (int i = 0; i < m_Device->parameterCount(); i++) {
        QDomLiteElement* p = CustomParameters->appendChild("Parameter","Id",m_CustomParameterList[i].parameterID());
        m_Device->parameter(i)->serialize(p);
    }
*/
    m_CustomParameterList.serialize(CustomParameters);
    DesktopComponent->serialize(xml);
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
            //actionPasteParameters->setEnabled(QApplication::clipboard()->text().startsWith("<Parameters"));
            //parametersMenu->popup(mapToGlobal(dynamic_cast<QMouseEvent*>(event)->pos()));
            CParametersMenu* m = new CParametersMenu(m_Device,this,false);
            m->setAttribute(Qt::WA_DeleteOnClose,true);
            connect(m,&CParametersMenu::aboutToChange,DesktopComponent->MainMenu->UndoMenu,&CUndoMenu::addItem,Qt::DirectConnection);
            connect(m,&CParametersMenu::parametersChanged,this,&CMacroBoxForm::PlugInIndexChanged);
            m->addSeparator();
            if (allowCustomParameters) {
                addParameterMenu(m);
                m->addSeparator();
                QAction* a = new QAction("Custom Parameter Dialog");
                m->addAction(a);
                connect(a,&QAction::triggered,this,&CMacroBoxForm::showParameterDialog);
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
    QMenu* menu = m->addMenu("add Parameter");
    if (devices->size() == 0) {
        menu->setEnabled(false);
        return;
    }
    for (const IDevice* d : *devices) {
        QSignalMenu* deviceMenu = new QSignalMenu(d->deviceID(),menu);
        menu->addMenu(deviceMenu);
        connect(deviceMenu,qOverload<QString>(&QSignalMenu::menuClicked),this,&CMacroBoxForm::toggleCustomParameter);

        for (int i = 0; i < d->parameterCount(); i++) {
            //QAction* a =
            deviceMenu->addAction(d->parameter(i)->Name,CParameterID::parameterID(d->deviceID(), d->parameter(i)));
            //a->setCheckable(true);
            //a->setChecked(m_CustomParameterList.contains(d->parameter(i)));
        }
    }
}

void CMacroBoxForm::showParameterDialog(){
    CCustomParameterDialog* d = new CCustomParameterDialog(this);
    d->fill(DesktopComponent->deviceList(),&m_CustomParameterList,m_Device);
    d->exec();
}

void CMacroBoxForm::toggleCustomParameter(QString id){
    CParameterID pid(id);
    if (IDevice* d = DesktopComponent->deviceList()->device(pid.DeviceID)) {
        if (CParameter* p = d->parameter(pid.ParameterName)) {
            /*
            if (m_CustomParameterList.contains(p)) {
                removeCustomParameter(p);
            }
            else {
*/
                addCustomParameter(d,p);
           // }
        }
    }
}

void CMacroBoxForm::addCustomParameter(IDevice* d, CParameter* p)
{
    //if (!m_CustomParameterList.contains(p)) {
        CCustomParameter* c = m_CustomParameterList.append(d->deviceID(),p);
        c->masterParameter = DesktopComponent->deviceList()->addCustomParameter(m_Device,p->Type,d->deviceID() + " " + p->Name,p->Unit,p->Min,p->Max,p->DecimalFactor,p->List,p->Value);
        m_Device->updateHostParameter();
    //}
}
/*
void CMacroBoxForm::removeCustomParameter(CParameter* p)
{
    if (CCustomParameter* c = m_CustomParameterList.customParameter(p)) {
        DesktopComponent->deviceList()->removeCustomParameter(m_Device,c->Caption);
        m_CustomParameterList.remove(p);
        m_Device->updateHostParameter();
    }
}
*/
void CMacroBoxForm::removeDeviceParameters(IDevice *device) {
    m_CustomParameterList.removeDevice(device->deviceID());
}

void CMacroBoxForm::removeAllParameters() {
    for (IDevice* d : *DesktopComponent->deviceList()->devices()) {
        removeDeviceParameters(d);
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
