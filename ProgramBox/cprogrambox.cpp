#include "cprogrambox.h"
#include <QVBoxLayout>

CProgramBox::CProgramBox() {}

CProgramBox::~CProgramBox()
{
    qDebug() << "~CProgramBox";
    if (m_Initialized)
    {
        for (CDesktopComponent* d : std::as_const(form()->DesktopComponents)) {
            d->clearJacksCreated();
        }
        //qDeleteAll(JacksCreated);
    }
}

void CProgramBox::init(const int Index, QWidget* MainWindow)
{
    QMutexLocker locker(&mutex);
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
/*
    addJackStereoOut(0);
    addJackDualMonoOut(1);
    addJackMIDIOut(3);
    addJackModulationOut(4,"Modulation Out");
    addJackModulationOut(5,"Frequency Out");
    addJackModulationOut(6,"Trigger Out");
    addJackStereoIn();
    addJackDualMonoIn();
    addJackMIDIIn();
    addJackModulationIn("Modulation In");
    addJackModulationIn("Frequency In");
    addJackModulationIn("Trigger In");
*/
    //addParameterMIDIChannel();
    addParameter(CParameterVars::Numeric,"Program","",1,MaxPrograms,0,"",1);

    m_Form=new CMacroBoxForm(this,MainWindow);
    form()->allowCustomJacks = true;
    CDesktopContainer* d = form()->DesktopContainer;
    //form()->JacksCreated = &d->Desktop->JacksCreated;
    //form()->InsideJacks = &d->Desktop->InsideJacks;
    auto l = dynamic_cast<QVBoxLayout*>(m_Form->layout());
    l->removeWidget(d);
    buttonPanel = new QSynthButtonPanel(m_Form);
    buttonPanel->setFixedHeight(100);
    buttonPanel->init(MaxPrograms,"Program");
    CDesktopComponent::connect(buttonPanel,&QSynthButtonPanel::valueChanged,m_Form,qOverload<QString,int>(&CSoftSynthsForm::setParameter));
    l->addWidget(buttonPanel);

    for (int i = 1; i < MaxPrograms; i++)
    {
        CDesktopContainer* d = new CDesktopContainer(m_Form);
        d->Desktop->init(MainWindow);
        CDesktopComponent::connect(d->Desktop,&CDesktopComponent::parametersChanged,form(),&CMacroBoxForm::PlugInIndexChanged);
        CDesktopComponent::connect(d->Desktop,&CDesktopComponent::deviceRemoved,form(),&CMacroBoxForm::removeDeviceParameters);
        CDesktopComponent::connect(d->Desktop,&CDesktopComponent::devicesCleared,form(),&CMacroBoxForm::removeAllParameters);
        form()->DesktopContainers.append(d);
        form()->DesktopComponents.append(d->Desktop);
    }

    layout=new QStackedLayout();
    l->addLayout(layout);
    l->setStretchFactor(layout,1000);
    for (CDesktopContainer* c : std::as_const(form()->DesktopContainers)) {
        layout->addWidget(c);
        /*
        for (uint i = 0; i < m_Jacks.size(); i++)
        {
            IJack* J = m_Jacks[i];
            IJack* J1 = c->Desktop->addJack(J->createInsideJack(i,this),0);
            JacksCreated.append(J1);
            (J->isOutJack()) ? InsideJacks.append(dynamic_cast<CInJack*>(J1)) : InsideJacks.append(dynamic_cast<CInJack*>(J));
        }
*/
    }
    currentIndex = -1;
    updateDeviceParameter();
}

CAudioBuffer* CProgramBox::getNextA(const int ProcIndex)
{
    return FORMFUNC(CMacroBoxForm)->DesktopComponents[program()]->InsideJacks[ProcIndex]->getNextA();
}

CMIDIBuffer* CProgramBox::getNextP(const int ProcIndex)
{
    return FORMFUNC(CMacroBoxForm)->DesktopComponents[program()]->InsideJacks[ProcIndex]->getNextP();
}

float CProgramBox::getNext(const int ProcIndex)
{
    return FORMFUNC(CMacroBoxForm)->DesktopComponents[program()]->InsideJacks[ProcIndex]->getNext();
}

void CProgramBox::updateDeviceParameter(const CParameter* /*p*/)
{
    if (currentIndex != program())
    {
        QMutexLocker locker(&mutex);
        form()->DesktopComponent->hideForms();
        layout->setCurrentIndex(program());
        addTickerDevice(deviceList());
        setDeviceParent(deviceList());
        form()->DesktopContainer = desktopContainer();
        form()->DesktopComponent = desktopComponent();
        buttonPanel->setValue(program()+1);
        currentIndex = program();
    }
}
/*
void CProgramBox::serializeCustom(QDomLiteElement* xml) const
{
    QDomLiteElement* desktops = xml->appendChild("Desktops");
    for (const CDesktopContainer* d : std::as_const(form()->DesktopContainers)) d->Desktop->serialize(desktops->appendChild("Desktop"));
}

void CProgramBox::unserializeCustom(const QDomLiteElement* xml)
{
    if (!xml) return;
    QMutexLocker locker(&mutex);
    if (const QDomLiteElement* desktops = xml->elementByTag("Desktops"))
    {
        int i = 0;
        for (const QDomLiteElement* d : (const QDomLiteElementList)desktops->elementsByTag("Desktop"))
        {
            if (i < form()->DesktopContainers.size()) form()->DesktopContainers[i++]->Desktop->unserialize(d);
        }
    }
    form()->fillList(currentProgram());
}
*/
void CProgramBox::tick()
{
    for (CDesktopContainer* c : std::as_const(form()->DesktopContainers)) c->Desktop->deviceList()->tick();
    IDevice::tick();
}
